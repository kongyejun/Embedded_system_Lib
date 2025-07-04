#include "wav_bsp.h"
#include "SD\FATFS\ff.h"
#include "LOG_SYSTEM\LOG.h"
#include "malloc.h"
#include "string.h"
#include "stdio.h"

#define  RIFF_ID		0x52494646  /* 'RIFF'标识 */
#define  WAVE_ID		0x57415645  /* 'WAVE'标识 */
#define  FMT_ID			0x666D7420  /* 'fmt '标识 */
#define  LIST_ID		0x4C495354  /* 'LIST'标识 有待考虑此处应该是FACT*/
#define  DATA_ID		0x64617461  /* 'data'标识 */

/*从大端和小端字节序读取32位整数*/
#define BG_READ_WORD(x)	((((uint32_t)wavhead_buff[x + 0]) << 24) | (((uint32_t)wavhead_buff[x + 1]) << 16) |\
			(((uint32_t)wavhead_buff[x + 2]) << 8) | (((uint32_t)wavhead_buff[x + 3]) << 0))

#define LG_READ_WORD(x)	((((uint32_t)wavhead_buff[x + 3]) << 24) | (((uint32_t)wavhead_buff[x + 2]) << 16) |\
			(((uint32_t)wavhead_buff[x + 1]) << 8) | (((uint32_t)wavhead_buff[x + 0]) << 0))
/*用于从小端读取16位整数*/
#define LG_READ_HALF(x)	((((uint16_t)wavhead_buff[x + 1]) << 8) | (((uint16_t)wavhead_buff[x + 0]) << 0))

/* 创建wav头文件 */
wave_header_t* creat_wave_header(uint32_t length, uint32_t sample_rate, uint8_t bits_perSample, uint8_t num_chans){
    wave_header_t *header = (wave_header_t *)malloc(sizeof(wave_header_t)+4);
    if(header == NULL){
        EMLOG(LOG_ERROR,"header malloc err!\n");
        return NULL;
    }
    // set all the ASCII literals
    memcpy(header->chunkID, "RIFF", 4);
    header->chunkSize = length + (44 - 8);
    memcpy(header->format, "WAVE", 4);

    memcpy(header->subchunk1ID, "fmt ", 4);//子块ID
    header->subchunk1Size = 16;//子块数据大小
    header->audioFormat = 0x01;//PCM无损编码
    header->numChannels = num_chans;  // 2
    header->sampleRate = sample_rate; // 22050 or 44100
    header->bitsPerSample = bits_perSample; // 16
    header->blockAlign = header->numChannels * header->bitsPerSample / 8; // 4
    header->byteRate = header->sampleRate * header->blockAlign; // 88200 or 176.4k

    memcpy(header->subchunk2ID, "data", 4);
    header->subchunk2Size = length;
    return header;
}

wave_header_t* waveheader_init(void){
    wave_header_t *header = (wave_header_t *)malloc(sizeof(wave_header_t)+4);
    if(header == NULL){return NULL;}
    // set all the ASCII literals
    memset(header,0,sizeof(wave_header_t)+4);
    memcpy(header->chunkID, "RIFF", 4);
    memcpy(header->format, "WAVE", 4);
    memcpy(header->subchunk1ID, "fmt ", 4);//子块ID
    memcpy(header->subchunk2ID, "data", 4);
    return header;
}

errorcode_e is_wav_files(FIL** FLE,wave_header_t** header){
    uint8_t wavhead_buff[512];
	uint32_t index;

	if (FR_OK != f_read(*FLE, wavhead_buff, 512, &index)){
		EMLOG(LOG_ERROR,"f_read failed!\r\n");
		return FILES_FAIL;
	}	
	//文件格式
	index = 0;
	if (BG_READ_WORD(index) != RIFF_ID)
		return UNVALID_RIFF_ID;
	//文件大小
	index += 4;
	if ((LG_READ_WORD(index) + 8) != f_size(*FLE))
		return UNVALID_RIFF_SIZE;
	//文件类型
	index += 4;
	if (BG_READ_WORD(index) != WAVE_ID)
		return UNVALID_WAVE_ID;
	//子块1是否为 "fmt "
	index += 4;
	if (BG_READ_WORD(index) != FMT_ID)
		return UNVALID_FMT_ID;
	//子块1的字节数
	index += 4;
	if (LG_READ_WORD(index) != 0x10)
		return UNVALID_FMT_SIZE;
	//音频格式
	index += 4;
	if (LG_READ_HALF(index) != 0x01)
		return UNSUPPORETD_FORMATTAG;
	//声道数
	index += 2;
	(*header)->numChannels = LG_READ_HALF(index);
	if ((*header)->numChannels != 1 && (*header)->numChannels != 2)
		return UNSUPPORETD_NUMBER_OF_CHANNEL;
	//采样率
	index += 2;
	(*header)->sampleRate = LG_READ_WORD(index);
	if ((*header)->sampleRate != 11025 && (*header)->sampleRate != 22050 && (*header)->sampleRate != 44100)
		return UNSUPPORETD_SAMPLE_RATE;
	//数据传输速率
	index += 4;
	(*header)->byteRate = LG_READ_WORD(index);
	//数据块对齐
	index += 4;
	(*header)->blockAlign = LG_READ_HALF(index);
	//采样位数
	index += 2;
	(*header)->bitsPerSample = LG_READ_HALF(index);
	if ((*header)->bitsPerSample != 8 && (*header)->bitsPerSample != 16 && (*header)->bitsPerSample != 24)
		return UNSUPPORETD_BITS_PER_SAMPLE;
	//子块2是否为 "LIST",有待考虑此处应该是FACT
	index += 2;
	if (BG_READ_WORD(index) == LIST_ID) {
		//如果是,则跳过子块2
		index += 4;
		index += LG_READ_WORD(index);
		index += 4;
		//如果跳过子块2导致index超过500,则返回错误
		if (index >= 500)
			return UNVALID_LIST_SIZE;
	}
	//下一个子块是否为 "data"
	if (BG_READ_WORD(index) != DATA_ID)
		return UNVALID_DATA_ID;
	//音频数据大小
	index += 4;
	(*header)->subchunk2Size = LG_READ_WORD(index);
	//
	index += 4;
	if (FR_OK != f_lseek((*FLE), index)){
		EMLOG(LOG_ERROR,"f_lseek failed!\n");
		return FILES_FAIL;
	}
	return OK;
}

void wavheader_Info(FIL** file,wave_header_t** header){
    EMLOG(LOG_INFO,"WAV Header Info:\n");
    EMLOG(LOG_DEBUG,"point:0x%08x\n", (uint32_t)f_tell(*file));
    EMLOG(LOG_DEBUG,"numchannels:%d\n", (*header)->numChannels);
    EMLOG(LOG_DEBUG,"samplerate:%d\n", (*header)->sampleRate);
    EMLOG(LOG_DEBUG,"byterate:%d\n", (*header)->byteRate);
    EMLOG(LOG_DEBUG,"blockalign:%d\n", (*header)->blockAlign);
    EMLOG(LOG_DEBUG,"bitspersample:%d\n", (*header)->bitsPerSample);
    EMLOG(LOG_DEBUG,"datasize: %ld\n", (*header)->subchunk2Size);
    EMLOG(LOG_DEBUG,"bit_rate:%dkbps\n", (*header)->byteRate * 8 / 1000);
}

/**
 * @brief  判断是否是wav文件
 * @param  MusicName:文件名
*/
int is_wav_file(const char *filename) {
    const char *ext = strrchr(filename, '.');  // 找到最后一个'.'
    if (ext != NULL && strcmp(ext, ".wav") == 0) {
        return 1;  // 是.wav文件
    }
    return 0;  // 不是.wav文件
}
/**
 * @brief  搜寻该目录下所有的.wav文件
 * @param  wavfiles:存放搜索到的文件名的数组
 * @param  array_filename_len:文件名最大长度
 * @param  PATH:目录路径
 * @return uint8_t:成功找到的文件个数
*/
uint8_t Found_Wavfiles(char* wavfiles,uint8_t MAX_FILE,uint8_t MAX_FILENAME_LEN,TCHAR** PATH){
	if(wavfiles == NULL || *PATH == NULL || MAX_FILENAME_LEN == 0){
		EMLOG(LOG_ERROR,"wavfiles or PATH or array_filename_len is NULL!\n");
		return 0;
	}
	uint8_t file_count = 0;
	DIR dir;FILINFO fno;
	FRESULT res = f_opendir(&dir, *PATH);  // 打开目录
	if (res != FR_OK) {
        EMLOG(LOG_ERROR,"无法打开目录: %s\n",*PATH);
        return res;
    }
	// 遍历读取目录中的每个文件或子目录
	while(1){
		res = f_readdir(&dir, &fno);
		if (res != FR_OK || fno.fname[0] == 0) {// 如果打开失败 或者 文件名为空
			break;  // 遍历完毕或出错
		}
		if (!(fno.fattrib & AM_DIR)) {  // 查看文件属性，忽略子目录，仅处理文件
            if (is_wav_file(fno.fname)) {
				sprintf(wavfiles+file_count*MAX_FILENAME_LEN,"%s",fno.fname);
				file_count++;
            }
			if(file_count >= MAX_FILE){
				break;
			}
        }
	}
	f_closedir(&dir);  // 关闭目录
    return file_count;
}
