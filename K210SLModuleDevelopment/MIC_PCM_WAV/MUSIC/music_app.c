#include "music_app.h"
#include "SPEAKER_BSP\speaker_bsp.h"
#include "WAV_Files\wav_bsp.h"
#include "RGB_KEYPAD_TIMER.h"
#include "MIC_BSP\mic_bsp.h"
#include "LOG_SYSTEM\LOG.h"
#include "stdlib.h"
#include "sleep.h"

/***************************************************
 1.不知道为什么 一旦将 MIC_FRAME_LEN 512就会报错
 *************************************************/

#define MUSIC_SAMPLE_RATE 22050

int MIC_Callback(void* ctx); //MIC中断回调函数声明
int Speaker_Callback(void* ctx); //扬声器回调函数声明


/*******************************************************
 *  0x01 :录音资源是否释放
 *  0x02 :录音是否写入文件
 *  0x04 :双缓存区录音索引
 *  0x08 :是否允许获取音频        -----仅在MIC中断中会使数据写入无效
 * 
 *  0x10 :播放资源是否有误
 *  0x20 :播放是否正在运行
 *  0x40 :播放缓冲区索引号
 *  0x80 :本次播放是否结束
 * 
 *******************************************************/
uint16_t Music_FLAG = 0;             

/***************************************************
 *                 全局定义                         *
 ***************************************************/

/****************** 录音相关**********************/
FIL           *f_rec        = 0;     // 录音文件流变量
wave_header_t *wavhead      = 0;     // wav文件头
int16_t*      rx_buff       = NULL;  // 录音缓冲区
uint32_t*     g_rx_dma_buff = NULL;  // DMA缓冲区
uint32_t      sectorsize    = 0;     // 写入次数
// TCHAR*        rec_file_name  = NULL; // 录音文件名
/****************** 播放相关**********************/
#define PLAY_BUFF_LEN 512           // 播放缓冲区大小

FIL           *f_spe        = 0;     // 录音文件流变量
wave_header_t *wavhead_spe  = 0;     // wav文件头
uint32_t* play_buff0;                // 播放缓冲区0
uint32_t* play_buff1;                // 播放缓冲区1
uint32_t play_buff0_read_len;        // 播放缓冲区0读取长度
uint32_t play_buff1_read_len;        // 播放缓冲区1读取长度
TCHAR*   Play_DIRPath;               // 音乐文件目录路径           
/***************************************************
 *                 音乐外设                         *
 ***************************************************/

/**
 * @brief  初始化音乐播放器和录音器(需要在一开始调用)
 * @param  DIR :音乐文件目录路径
*/
void Music_Init(TCHAR* DIR){
    if(Play_DIRPath == NULL){
        Play_DIRPath = DIR;
        EMLOG(LOG_INFO, "音乐播放文件目录更改成功\n");
    }
    Music_FLAG = 0x0;
    MIC_Init(MIC_Callback,MUSIC_SAMPLE_RATE);
    Speaker_init(Speaker_Callback,MUSIC_SAMPLE_RATE);
    EMLOG(LOG_INFO, "Music_Init OK\n");
}

void Show_WavFiles(MusicDIR_t *wavdir_Struct,TCHAR* DirPath){
    uint8_t cont =Found_Wavfiles((char*)wavdir_Struct->MusicFiles,MAX_MUSIC_FILE_NUM,MAX_MUSIC_FILE_NAME_LEN,&DirPath);
    printf_pink("*******************************************\n");
    printf_pink("*               .wav文件表                *\n");
    printf_pink("*******************************************\n");
    printf("共找到 %d 个.wav文件:\n",cont);
    for(uint8_t i = 0;i < cont;i+=1){
        printf("\t%s\n",wavdir_Struct->MusicFiles[i]);
    }
    wavdir_Struct->MusicNum = cont;
    EMLOG(LOG_INFO, "该目录中所有.wav文件搜索完成!!!\n");
    return ;
}

/**
 * @brief  注销音乐播放器和录音器
 * @param  None
*/
void Music_DeInit(){
    Music_Recording_Save();
    Music_Play_End();
    Speaker_deinit();
    MIC_deinit();
    EMLOG(LOG_INFO, "Music_DeInit OK\n");
}

/***************************************************
 *                  录音功能                        *
 ***************************************************/
/**
 * @brief  麦克风回调函数
 * @param  None
 */
int  __attribute__((optimize("O0")))MIC_Callback(void* ctx){
    // if(!(Music_FLAG & 0x01)){// 如果录音资源已经释放
    //     EMLOG(LOG_ERROR,"Recording is freed!\n");
    //     return -1;
    // }
    uint32_t i;int16_t temp;
    if(!(Music_FLAG & 0x04)){//如果当前使用的是第一个缓冲区
        Music_FLAG |= 0x04;//标志下次使用第二个缓冲区
        // 如果允许获取音频
        if(Music_FLAG & 0x08){
            i2s_receive_data_dma(MIC_I2S_DEVICE,//I2S设备号
                                (g_rx_dma_buff+MIC_FRAME_LEN * 2),//接收缓冲区起始地址
                                MIC_FRAME_LEN*2,//接收数长度
                                MIC_DMAC_CHANNE);//DMA通道号
        }
        // 搬运数据
        for(i=0 ;i < MIC_FRAME_LEN ; i++){
            //因为32位采样16位有效的原因，所以需要将高16位清零
            //取奇数区域的原因是因为，当前程序只获取单个声道采集的数据
            temp = (int16_t)((g_rx_dma_buff[i*2+1]*MIC_GAIN)&0xffff);
            //将单个通过的数据,作为左右声道的数据写入缓冲区
            *(rx_buff + 2 * i) = temp;
            *(rx_buff + 2 * i + 1) = temp;
        }
        // 如果正在运行录音 则将数据写入文件中
        if(Music_FLAG & 0x02){
            // 解释: (MIC_FRAME_LEN*2) 2字节[16bit]   *2 双声道
            i=0;
            int res = f_write(f_rec,(const void *)rx_buff,(MIC_FRAME_LEN*2)*2,&i);
            if(res){
                EMLOG(LOG_ERROR,"\nerr1:%d\n", res);
                EMLOG(LOG_DEBUG,"bw:%d\n",i);
                f_close(f_rec);
                free(wavhead);wavhead = NULL;
                free(f_rec); f_rec = NULL;
                free(rx_buff);rx_buff = NULL;
                free(g_rx_dma_buff);g_rx_dma_buff = NULL;
                Music_FLAG &= ~0x0f;//清除录音相关标志位
                return -1; //写入出错.
            }
            sectorsize+=1; //写入次数+1
        }
    }else{
        Music_FLAG &= ~0x04;//标志下次使用第一个缓冲区
        // 如果允许获取音频
        if(Music_FLAG & 0x08){
            i2s_receive_data_dma(MIC_I2S_DEVICE,//I2S设备号
                                g_rx_dma_buff,//接收缓冲区起始地址
                                MIC_FRAME_LEN*2,//接收数长度
                                MIC_DMAC_CHANNE);//DMA通道号
        }
        // 搬运数据
        for(i = MIC_FRAME_LEN; i < MIC_FRAME_LEN*2; i++){
            temp = (int16_t)((*(g_rx_dma_buff+i*2+1)*MIC_GAIN)&0xffff);
            //将单个通过的数据,作为左右声道的数据写入缓冲区
            *(rx_buff + 2 * i) = temp;
            *(rx_buff + 2 * i + 1) = temp;
        }
        // 如果正在运行录音 则将数据写入文件中
        if(Music_FLAG & 0x02){
            i=0;
            int res = f_write(f_rec,(const void *)(rx_buff+MIC_FRAME_LEN*2),(MIC_FRAME_LEN*2)*2,&i);
            if(res){
                EMLOG(LOG_ERROR,"\nerr2:%d\n", res);
                EMLOG(LOG_DEBUG,"bw:%d\n",i);
                f_close(f_rec);
                free(wavhead);wavhead = NULL;
                free(f_rec); f_rec = NULL;
                free(rx_buff);rx_buff = NULL;
                free(g_rx_dma_buff);g_rx_dma_buff = NULL;
                Music_FLAG &= ~0x0f;//清除 数据有效标志位 录音启用标志位
                return -1; //写入出错
            }
            sectorsize+=1; //写入次数+1
        }
    }
    return 0;    
}

/*判断pname路径是否可以使用 */
bool recoder_pathname_check(TCHAR *pname){	
    FIL ftemp; 
	uint8_t res;					 
	res = f_open(&ftemp, (const TCHAR*)pname, FA_READ);//尝试打开这个文件
	if(res == FR_NO_FILE){//如果文件不存在，路径有效
        f_close(&ftemp);
        return true;
    }else if(res == FR_OK){
        EMLOG(LOG_INFO,"filepath is exist!, next del file!!!!\n");
        if(f_unlink(pname) == FR_OK){//如果文件成功删除，路径有效
            EMLOG(LOG_INFO,"del success!\n");
        }else{
            EMLOG(LOG_ERROR,"del fail!\n");
            f_close(&ftemp);
            return false;
        }    
        f_close(&ftemp);
        return true;
    }else{
        f_close(&ftemp);
        return false;
    }             
}

/**
 * @brief  启动录音
 * @param  None
*/
void Music_Recording_Start(TCHAR *FilePath){
    // 录音资源是否已释放 -- 可在在创建缓冲区时加入判断，从而省略此处
    if(Music_FLAG & 0x01){
        EMLOG(LOG_ERROR,"Recording source is doing\n");
        return ;
    }
    if(f_spe != NULL){
        Music_Play_End();
        EMLOG(LOG_WARN,"Music Play End\n");
    }
    uint8_t res = 0;uint32_t wrute_num;
    // 重置相关资源
    sectorsize = 0;
    EMLOG(LOG_INFO,"Music_FLAG= %#02x\n",Music_FLAG);

    //申请FIL空间
    f_rec = (FIL*)malloc(sizeof(FIL));
    if(f_rec == NULL){
        EMLOG(LOG_ERROR,"f_rec malloc fail!\n");
        return ;
    }
    //判断存放路径是否合法
    if(recoder_pathname_check(FilePath)==false){return ;}
    //如果路径合法，则继续任务
    EMLOG(LOG_INFO,"FilePath:%s   ",FilePath);
    printf_green("FilePath is ok!\n");
    //申请并配置WAV文件头空间
    wavhead = creat_wave_header(0,MUSIC_SAMPLE_RATE,16,2);
    if(wavhead == NULL){
        EMLOG(LOG_ERROR,"wavhead malloc fail!\n");
        free(f_rec);f_rec = NULL;
        return ;
    }
    //EMLOG(LOG_INFO,"wavhead malloc success, wavhead->sampleRate=%d!\n",wavhead->sampleRate);
    //打开文件流
    res = f_open(f_rec, (const TCHAR *)FilePath, FA_CREATE_ALWAYS | FA_WRITE);
    if(res){ //文件打开失败
        EMLOG(LOG_ERROR,"bsp_recorder_start: file open err %d\n", res);
        free(wavhead);wavhead = NULL;
        f_close(f_rec);
        free(f_rec);f_rec = NULL;
        return ;     
    }else{
        //写入WAV文件头
        res = f_write(f_rec, (const void *)wavhead, sizeof(wave_header_t), &wrute_num);
        //成功则设置 录音开始标志位
        if(res != FR_OK){
            EMLOG(LOG_ERROR,"write wavhead fail:%d\n", res);
            free(wavhead);wavhead = NULL;
            f_close(f_rec);
            free(f_rec);f_rec = NULL;
            return ;
        }else{
            //启动麦克风,写入音频数据到文件流
            EMLOG(LOG_DEBUG,"Mic buff start init!\n");
            if(MIC_Buff_Init(&rx_buff,&g_rx_dma_buff)){// 初始化麦克风缓冲区
                EMLOG(LOG_ERROR,"Mic buff init fail!\n");
                return ;//初始化失败
            }
            // 启动后接收到数据后会跳转到 MIC_Callback函数
            Music_FLAG |= (0x02|0x01); // 设置 录音资源已申请标志位 和 录音写入文件标志位
            Recording_Restart(); // 启动获取音频
        }
    }
}

/**
 * @brief  暂停录音写入文件
 * @param  None
*/
void Recording_Stop_Wfiles(){
    if(Music_FLAG & 0x01){
        if(!(Music_FLAG & 0x02)){
            EMLOG(LOG_ERROR,"Recording is not stop write files!\n");
            return ;
        }
        //清除 录音写入文件标志位
        Music_FLAG &= ~0x02;
        EMLOG(LOG_INFO,"Recording Stop write files!\n"); 
    }else{
        EMLOG(LOG_ERROR,"Recording is freed!\n");
    }
}

/**
 * @brief  停止获取音频数据
 * @param  None
*/
void Recording_Stop(){
    if(Music_FLAG & 0x01){
        if(!(Music_FLAG & 0x08)){
            EMLOG(LOG_ERROR,"Recording is not stop!\n");
            return ;
        }
        // 清除 音频数据获取标志位
        Music_FLAG &= ~0x08;
        // 等待DMA传输完成
        dmac_wait_done(MIC_DMAC_CHANNE);
        EMLOG(LOG_INFO,"Recording Stop!\n"); 
    }else{
        EMLOG(LOG_ERROR,"Recording is freed!\n");
    }
}

/**
 * @brief  重启录音写入文件
 * @param  None
*/
void Recording_Restart_Wfiles(){
    if(!(Music_FLAG & 0x01)){// 判断录音是否在中断中崩溃
        // 录音资源已经释放,说明录音程序奔溃,无法重启
        EMLOG(LOG_ERROR,"Recording is freed!\n");
        return ;
    }
    if(!(Music_FLAG & 0x02)){// 录音并没有停止写入文件
        EMLOG(LOG_ERROR,"Recording is doing write files!\n");
        return ;
    }
    //设置 录音写入文件标志位
    Music_FLAG |= 0x02;
    EMLOG(LOG_INFO,"Recording restart write files!\n");
}

/**
 * @brief  重启获取音频数据
 * @param  None
*/
void Recording_Restart(){
    if(!(Music_FLAG & 0x01)){// 判断录音是否在中断中崩溃
        // 录音开启,但无数据,说明录音程序奔溃,无法重启
        EMLOG(LOG_ERROR,"Recording is freed!\n");
        return ;
    }
    if(Music_FLAG & 0x08){// 音频数据正在获取
        EMLOG(LOG_ERROR,"Recording is doing!\n");
        return ;
    }
    //设置 音频数据获取标志位
    Music_FLAG |= 0x08;
    i2s_receive_data_dma(MIC_I2S_DEVICE,(uint32_t*)(g_rx_dma_buff+(Music_FLAG&0x04)*MIC_FRAME_LEN*2),
                        MIC_FRAME_LEN*2,MIC_DMAC_CHANNE);
    EMLOG(LOG_INFO,"Recording Restart Music_FLAG=%#02x\n",Music_FLAG);
}

/**
 * @brief  保存录音
 * @param  FilesPath:文件路径
*/
void Music_Recording_Save(){
    uint8_t res = 0;
    uint32_t write_num;
    EMLOG(LOG_INFO,"Recording Save task start\n");
    if(Music_FLAG & 0x01){ // 确保录音相关资源没有释放
        LOG_Stop(); //关闭日志
        printf_green("0%%\n");
        Recording_Stop();// 停止录音
        printf("Stop Recording\n");
        printf_green("20%%\n");msleep(50);
        wavhead->chunkSize = sectorsize*(MIC_FRAME_LEN*2)*2+36; //整个文件的大小-8;
        wavhead->subchunk2Size = sectorsize*(MIC_FRAME_LEN*2)*2;  //数据大小
        printf_green("40%%\n");msleep(50);
        res = f_lseek(f_rec, 0);
        if(res==FR_OK){
            res = f_write(f_rec, wavhead, sizeof(wave_header_t), &write_num);
            if(res==FR_OK){
                printf_green("60%%\n");msleep(50);
                res = f_close(f_rec);
                if(res==FR_OK){
                    printf_green("100%%\n");msleep(50);
                    LOG_Start(); //开启日志
                    EMLOG(LOG_INFO,"wavhead->subchunk2Size=%d\n",wavhead->subchunk2Size);
                }else{
                    LOG_Start();
                    EMLOG(LOG_ERROR,"\nf_close error\n");
                }
            }else{
                LOG_Start();
                EMLOG(LOG_ERROR,"\nf_write error\n");
            }
        }else{
            LOG_Start();
            EMLOG(LOG_ERROR,"\nf_lseek error\n");
        }
        //释放资源
        free(wavhead);wavhead = NULL;
        free(f_rec); f_rec = NULL;
        free(rx_buff);rx_buff = NULL;
        free(g_rx_dma_buff);g_rx_dma_buff = NULL;
        sectorsize = 0;
        Music_FLAG &= ~0x0f;
        if(res==FR_OK)
            EMLOG(LOG_INFO,"\nRecording Save Success!\n");
    }else{
        // 数据无效时,说明录音程序奔溃,相关空间已被释放,故不需要释放空间
        EMLOG(LOG_DEBUG,"\nMusic Recording don't have data\n");
    }
    return ;
}

/***************************************************
 *                  播放功能                        *
 ***************************************************/

errorcode_e wavfile_read(){
	if (!(Music_FLAG & 0x40)){// 播放缓冲区0正在使用
        Music_FLAG |= 0x40;
		if (FR_OK != f_read(f_spe, play_buff1, PLAY_BUFF_LEN, &(play_buff1_read_len))){
			EMLOG(LOG_ERROR,"f_read0 failed!\n");
			return FILES_FAIL;
		}
		if (PLAY_BUFF_LEN > play_buff1_read_len) {
			if (f_tell(f_spe) != f_size(f_spe)){
                EMLOG(LOG_ERROR,"读取数量低于期望值,且并未读到文件末\n"); 
                return FILES_FAIL;
            }
            return FILES_END;
		}
	}else if(Music_FLAG & 0x40){// 播放缓冲区1正在使用
        Music_FLAG &= ~0x40;
		if (FR_OK != f_read(f_spe, play_buff0, PLAY_BUFF_LEN, &(play_buff0_read_len))){
			EMLOG(LOG_ERROR,"f_read1 failed!\n");
			return FILES_FAIL;
		}
		if (PLAY_BUFF_LEN > play_buff0_read_len) {
			if (f_tell(f_spe) != f_size(f_spe)){
                EMLOG(LOG_ERROR,"读取数量低于期望值,且并未读到文件末\n");
			    return FILES_FAIL;
            }
            return FILES_END;
		}
	}
	return OK;
}

/**
 * @brief  扬声器回调函数
 * @param  None
*/
int __attribute__((optimize("O0")))Speaker_Callback(void* ctx){// 只要不执行到i2s_play就不会进入该回调函数
    // printf("%#x\n",Music_FLAG);
    // 如果播放允许运行
    if(Music_FLAG & 0x20){
        // 播放完一首处理
        if(Music_FLAG & 0x80){
            EMLOG(LOG_WARN,"Music Play Finshes!!!\n");
            return -1;
        }
        // 播放数据
        if(!(Music_FLAG & 0x40)){// 播放缓冲区索引为0号缓冲区
            i2s_play(SPEAKER_I2S_DEVICE  ,      /* I2S设备号 */
                    SPEAKER_DMAC_CHANNE  ,      /* DMA通道号 */
                    (void *)play_buff0  ,      /* 播放的PCM数据 */
                    play_buff0_read_len,       /* PCM数据的长度 */
                    play_buff0_read_len,       /* 单次发送数量 */
                    16 , 2);                   /* 单次采样位宽 \ 声道数 */
        }else{
            i2s_play(SPEAKER_I2S_DEVICE  ,      /* I2S设备号 */
                    SPEAKER_DMAC_CHANNE  ,      /* DMA通道号 */
                    (void *)play_buff1  ,      /* 播放的PCM数据 */
                    play_buff1_read_len,       /* PCM数据的长度 */
                    play_buff1_read_len,       /* 单次发送数量 */
                    16 , 2);                   /* 单次采样位宽 \ 声道数 */
        }
        // 读取文件数据
        if(wavfile_read()!=OK){
            Music_FLAG |= 0x80; // 设置结束播放标志位
            f_close(f_spe);
            return -1; 
        }
        
    }
    return 0;
}

/**
 * @brief  打开wav音乐文件
 * @param  MusicName:音乐文件名
*/
void Music_Files_Open(TCHAR* MusicName){
    uint8_t status;FRESULT status1;
    TCHAR* Path = (TCHAR*)malloc(strlen((char*)MusicName)+strlen((char*)Play_DIRPath)-1);
    sprintf(Path,"%s%s",Play_DIRPath,MusicName);
    // 打开wav文件流
    EMLOG(LOG_INFO,"本次播放的音乐是: %s\n",(char*)MusicName);
    status1 = f_open(f_spe, Path, FA_OPEN_EXISTING | FA_READ);
    if(status1 != FR_OK){
        free(f_spe);f_spe = NULL;
        free(wavhead_spe);wavhead_spe = NULL;
        EMLOG(LOG_ERROR,"文件打开错误:%d\n",status1);
        return ;
    }
    status = is_wav_files(&f_spe,&wavhead_spe);
    if(status != OK){
        f_close(f_spe);
        free(f_spe);f_spe = NULL;
        free(wavhead_spe);wavhead_spe = NULL;
        EMLOG(LOG_ERROR,"is_wav_files error:%d\n",status);
        return ;
    }
    free(Path);
    // 输出wav文件信息
    wavheader_Info(&f_spe,&wavhead_spe);
}

/**
 * @brief  音乐播放器资源初始化
*/
void Music_Play_Malloc(){
    if(Music_FLAG & 0x10){ // 播放资源并没有出问题
        EMLOG(LOG_ERROR,"Music Source is don't error!\n");
        return ;
    }
    if(f_rec != NULL){// 录音正在运行
        Music_Recording_Save();
        EMLOG(LOG_WARN,"Music Recording Save and Stop\n");
    }
    // 清除播放标志
    Music_FLAG &= ~0xf0;
    // 申请FIL、wavhead_spe空间
    f_spe = (FIL*)malloc(sizeof(FIL));
    if(f_spe == NULL){EMLOG(LOG_ERROR,"f_spe malloc error\n");return ;}
    wavhead_spe = waveheader_init(); // 申请并初始化wavhead_spe空间
    if(wavhead_spe == NULL){free(f_spe);f_spe = NULL;EMLOG(LOG_ERROR,"wavhead_spe malloc error\n");return ;}
    // 申请播放双缓冲区空间 
    play_buff0 = (uint32_t*)malloc(PLAY_BUFF_LEN);
    if(play_buff0 == NULL){EMLOG(LOG_ERROR,"play_buff0 malloc error\n");return ;}
    play_buff1 = (uint32_t*)malloc(PLAY_BUFF_LEN);
    if(play_buff1 == NULL){free(play_buff0);play_buff0 = NULL;EMLOG(LOG_ERROR,"play_buff1 malloc error\n");return ;}
    Music_FLAG |= 0x10;// 设置播放资源存在标志
    EMLOG(LOG_INFO,"音乐缓冲空间创建成功,可以播放音乐了!!\n");
}

/**
 * @brief  音乐播放暂停
*/
void Music_Play_Stop(void){
    if(!(Music_FLAG & 0x20)){
        EMLOG(LOG_ERROR,"并没有正在播放音乐\n");
        return ;
    }
    Music_FLAG &= ~0x20;
    dmac_wait_done(SPEAKER_DMAC_CHANNE);
    EMLOG(LOG_INFO,"Music Play Stop\n");
}
/**
 * @brief  音乐播放开始
*/
void Music_Play_Restart(void){
    if(!(Music_FLAG & 0x10)){
        EMLOG(LOG_ERROR,"Music Play is free!\n");
        return ;
    }
    if(Music_FLAG & 0x20){
        EMLOG(LOG_ERROR,"Music Play is doing!!\n");
        return ;
    }
    Music_FLAG |= 0x20;
    Speaker_Callback(NULL);
    EMLOG(LOG_INFO,"Music Play start\n");
}
/**
 * @brief  播放下一首音乐
 * @param  NextMusicName:下一首音乐文件名(路径)
*/
void Music_Play_Next(TCHAR* NextMusicName){
    if(Music_FLAG & 0x20){// 如果前一个音乐正在播放，则先停止播放
        Music_FLAG |= 0x80; // 设置播放结束标志
        Music_Play_Stop();
    }
    Music_Files_Open(NextMusicName); // 打开下一个音乐文件
    // 预先读取音频数据到缓冲区
    Music_FLAG &= ~0x40; // 设置播放索引为0
    if (FR_OK != f_read(f_spe, play_buff0, PLAY_BUFF_LEN, &(play_buff0_read_len))){
        EMLOG(LOG_ERROR,"f_read failed!\n");return ;
    }
    if (PLAY_BUFF_LEN > play_buff0_read_len && f_tell(f_spe) != f_size(f_spe)) {
        EMLOG(LOG_ERROR,"f_read failed!\n");return ;
    }
    Music_FLAG &= ~0x80; // 清除播放结束标志
    Music_Play_Restart(); // 启动播放
}
/**
 * @brief  音乐播放器关闭
*/
void Music_Play_End(void){
    Music_Play_Stop(); // 停止播放
    if(!(Music_FLAG & 0x80)&&(Music_FLAG & 0x20)){f_close(f_spe);}
    free(play_buff0);play_buff0 = NULL;
    free(play_buff1);play_buff1 = NULL;
    free(f_spe);f_spe = NULL;
    free(wavhead_spe);wavhead_spe = NULL;
    Play_DIRPath = NULL;
    Music_FLAG &= ~0xf0;// 清除播放相关标志
    EMLOG(LOG_INFO,"成功释放音乐播放缓冲空间!!\n");
}
