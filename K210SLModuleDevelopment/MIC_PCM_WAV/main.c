#include "SD/FATFS/ff.h"
#include "SD/sdcard.h"
#include "LOG_SYSTEM/LOG.h"
#include "Pin_Config.h"
#include "sysctl.h"
#include "plic.h"
#include "dmac.h"
#include "timer.h"
#include "uarths.h"
#include "RGB_KEYPAD_TIMER.h"
#include "stdio.h"
#include "MUSIC\music_app.h"
#include "sleep.h"

FATFS fs;                         /* FatFs文件系统对象 */
FIL fnew;                         /* 文件对象 */
UINT fnum;                        /* 文件成功读写数量 */
BYTE ReadBuffer[1024]= {0};       /* 读缓冲区 */
BYTE WriteBuffer[] = "仅仅是测试";   /* 写缓冲区*/
FRESULT res;                      /* 文件操作结果 */

MusicDIR_t music_dir;

FRESULT SDCARD_FASFT_Init(void);
void FATFS_Test(void);

/**
 * @brief  硬件初始化
 * @param  无
 * @retval None
 */
void Hardware_Init(void){
    fpioa_set_function(PIN_KEYPAD_LEFT,FUNC_KEYPAD_LEFT);
    fpioa_set_function(PIN_KEYPAD_RIGHT,FUNC_KEYPAD_RIGHT);
    fpioa_set_function(PIN_KEYPAD_MIDDLE,FUNC_KEYPAD_MIDDLE);

    fpioa_set_function(PIN_MIC_DATA,FUNC_MIC_DATA);
    fpioa_set_function(PIN_MIC_SCK,FUNC_MIC_SCK);
    fpioa_set_function(PIN_MIC_WS,FUNC_MIC_WS);

    fpioa_set_function(PIN_SPK_BCK,FUNC_SPK_BCK);
    fpioa_set_function(PIN_SPK_DA,FUNC_SPK_DA);
    fpioa_set_function(PIN_SPK_WS,FUNC_SPK_WS);

    fpioa_set_function(PIN_TF_MISO,FUNC_TF_SPI_MISO);
    fpioa_set_function(PIN_TF_CLK ,FUNC_TF_SPI_CLK);
    fpioa_set_function(PIN_TF_MOSI,FUNC_TF_SPI_MOSI);
    fpioa_set_function(PIN_TF_CS  ,FUNC_TF_SPI_CS);

    fpioa_set_function(PIN_RGB_R,FUNC_RGB_R);
    fpioa_set_function(PIN_RGB_G,FUNC_RGB_G);
    fpioa_set_function(PIN_RGB_B,FUNC_RGB_B);
}

void System_Init(void){
    /* 系统时钟设置 */
    sysctl_pll_set_freq(SYSCTL_PLL0, 320000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 160000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
    uarths_init();
    /* 中断设置 */
    plic_init();
    sysctl_enable_irq();
    dmac_init();
}

uint8_t key_event_porcess(void){
    static uint8_t key_cont[3] = {0};
    static uint8_t num;
    num = 0;
    switch(FIFO_Pop()){
        case KEY_EVENT_NONE:
            break;
        case KEY_EVENT_LEFT_DOWN:
            key_cont[0]+=1;key_cont[0]%=2;
            num = 1;
            RGB_OFF();
            RGB_R_State(key_cont[0]);//红灯亮
            break;
        case KEY_EVENT_LEFT_UP:
            //RGB_R_State(LIGHT_OFF);//红灯灭
            break;
        case KEY_EVENT_RIGHT_DOWN:
            num = 2;
            key_cont[1]+=1;key_cont[1]%=2;
            RGB_OFF();
            RGB_G_State(key_cont[1]);//绿灯亮
            break;
        case KEY_EVENT_RIGHT_UP:
            //RGB_G_State(LIGHT_OFF);//绿灯灭
            break;
        case KEY_EVENT_MIDDLE_DOWN:
            num = 3;
            key_cont[2]+=1;key_cont[2]%=2;
            RGB_OFF();
            RGB_B_State(key_cont[2]);//蓝灯灭
            break;
        case KEY_EVENT_MIDDLE_UP:
            //RGB_B_State(LIGHT_OFF);//蓝灯灭
            break;
        default: break;
    }
    return num;
}
/* 写文件测试 */
void FATFS_Write_Test(void){        
    res=f_open(&fnew,"0:MIC_SPE_TEST.txt",FA_CREATE_ALWAYS|FA_WRITE);
    if ( res == FR_OK ) {
        /* 将指定存储区内容写入到文件内 */
        res=f_write(&fnew,WriteBuffer,sizeof(WriteBuffer),&fnum);
        /* 实测SPI_SD驱动下写入大于512字节的数据在SD卡里打开会显示乱码，如
        需写入大量数据使用f_write_co替代上面f_write即可
        res=f_write_co(&fnew,WriteBuffer,sizeof(WriteBuffer),&fnum); */
        if (res!=FR_OK){ 
            EMLOG(LOG_ERROR,"! ! files write fail, ERROR ID:(%d)\n",res);
        }
        /* 不再读写，关闭文件 */
        f_close(&fnew);
    } else {
        EMLOG(LOG_ERROR,"! ! open files fail\n");
    }
}

int main(){
    uint8_t key_num;
    //初始化
    Hardware_Init();
    System_Init();
    /* SD卡初始化以及挂载FATFS文件系统  */
    if(SDCARD_FASFT_Init() != FR_OK){
        while(1);
    }
    Music_Init("0:");
    /* 按键、RGB灯初始化 */
    RGB_KEYPAD_Init();
    EMLOG(LOG_DEBUG,"RGB_KEYPAD_Init OK!\n");
    Show_WavFiles(&music_dir,"0:");
    // FATFS_Write_Test();  //写文件测试
    // FATFS_Test();        //进行文件系统测试
    uint8_t i=0,i2=0,x=0;
    while(1){
        key_num = key_event_porcess();
        switch(key_num){
            case 1: // 红色
                if(i2==0){Music_Play_Malloc();}
                else{Music_Play_End();}
                i2+=1;i2%=2;
                break;
            case 2: // 绿色
                i+=1;i%=2;
                if(i){
                    Music_Recording_Start("0:REC.wav");
                }else{
                    Music_Recording_Save();
                }
                break;
            case 3: // 蓝色
                Music_Play_Next(music_dir.MusicFiles[x]);
                x+=1;x%=music_dir.MusicNum;
                break;
            default: break;
        }
        msleep(100);
    }
    /* 不再使用文件系统，取消挂载文件系统 */
    f_mount(NULL,"0:",1);
    return 0;
}

FRESULT SDCARD_FASFT_Init(void){
    res = f_mount(&fs,"0:",1);//会自动调用初始化SD卡函数的
    if(res == FR_NO_FILESYSTEM){
        EMLOG(LOG_INFO,"SD_CARD NO FileSystem, next will init FatfsSystem\n");
        res = f_mkfs("0:",NULL,ReadBuffer,1024);
        if(res == FR_OK){
            EMLOG(LOG_INFO,"SD_CARD Make Files System is OK\n");
            res = f_mount(NULL,"0:",1);// 卸载文件系统
            res = f_mount(&fs,"0:",1);// 重新挂载文件系统
        }else{
            EMLOG(LOG_ERROR,"SD_CARD Make Files System ERRROR\n");
            return res;
        }
    }else if(res != FR_OK){
        EMLOG(LOG_ERROR,"SD_CARD mount ERROR\n");
        return res;
    }
    EMLOG(LOG_INFO,"SD_CARD mount OK\n");
    return FR_OK;
}

void FATFS_Test(void){
    DIR dj; // 定义目录对象，用于保存打开的目录信息
    FILINFO fno; //定义文件信息对象，用于保存当前文件或目录的详细信息
    printf_green("/*******************************************/\n"); // 打印调试信息，表示开始输出文件名  printf filename
    printf_green(" *                   FATFS Test            *\n");
    printf_green("/*******************************************/\n");
    res = f_findfirst(&dj, &fno, _T("0:"), _T("*")); 
    /* 使用f_findfirst函数打开目录"0:"（即根目录），并找到第一个文件或目录，
       将其信息保存在'dj'和'fno'中。"_T("*")"表示搜索所有文件和目录。*/
    while(res == FR_OK && fno.fname[0]) { 
        // 只要操作成功且文件名不为空（即搜索到文件或目录），就进入循环。
        if(fno.fattrib & AM_DIR){ // 如果当前文件的属性包含AM_DIR标志，说明这是一个目录。
            printf_red("dir:");printf("%s\n",fno.fname);
        }else{ 
            printf_red("files:");printf("%s\n",fno.fname);
        }
        res = f_findnext(&dj, &fno);// 调用f_findnext函数继续搜索目录中的下一个文件或目录，并更新'fno'结构体。
    }
    f_closedir(&dj); //关闭打开的目录，释放与目录相关的资源
    printf_blue("filename print over\n"); // 打印调试信息，表示文件名输出完成
    
    //                     /* 读文件测试 */
    // res=f_open(&fnew,"0:FatFs读写测试文件.txt",FA_OPEN_EXISTING|FA_READ);
    // if ( res == FR_OK ) {
    //     /* 将指定存储区内容写入到文件内 */
    //     res=f_read(&fnew,ReadBuffer,sizeof(ReadBuffer),&fnum);
    //     if (res!=FR_OK){
    //         EMLOG(LOG_ERROR,"! ! files read fail, ERROR ID:(%d)\n",res);
    //     }
    //     /* 不再读写，关闭文件 */
    //     f_close(&fnew);
    // } else {
    //     EMLOG(LOG_ERROR,"! ! open files fail\n");
    // }
    // /* 不再使用文件系统，取消挂载文件系统 */
    // f_mount(NULL,"0:",1);
}