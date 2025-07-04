#ifndef __WAV_BSP_H__
#define __WAV_BSP_H__
#include "SD\FATFS\ff.h"
#include "stdint.h"
/************************************************
 *    定义用于存储WAV文件信息和读取缓冲区的结构     *
 ************************************************/
typedef struct _wave_header_t {
    uint8_t chunkID[4];
    uint32_t chunkSize;
    uint8_t format[4];

    uint8_t subchunk1ID[4];
    uint32_t subchunk1Size;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;

    uint8_t subchunk2ID[4];
    uint32_t subchunk2Size;
} __attribute__((packed, aligned(4))) wave_header_t; 

typedef struct{
	FIL     *fp;           		// 文件指针，指向当前正在读取的文件
	uint8_t *buff0;        		// 第一个缓冲区，用于存储读取的音频数据
	uint8_t *buff1;        		// 第二个缓冲区，双缓冲处理音频数据
	uint8_t *buff_current; 		// 指向当前正在使用的缓冲区
	uint32_t buff_current_len;  // 当前缓冲区的长度

	uint32_t buff0_len;  		// buff0的大小
	uint32_t buff1_len;  		// buff1的大小

	uint32_t buff0_read_len;  	// buff0中已读取的数据长度
	uint32_t buff1_read_len;  	// buff1中已读取的数据长度

	uint32_t samplerate;        // 音频采样率
	uint16_t blockalign;        // 块对齐大小，等于通道数*采样位深/8
	uint8_t  bitspersample;     // 采样深度（8位或16位）
	uint32_t byterate;          // 每秒的字节数，等于采样率*块对齐
	uint32_t datasize;          // 数据块的大小
	/**
	 * 0x01: buff0/buff1已满(0/1)
	 * 0x02: 缓冲区索引
	 * 0x04: 缓冲区达到末端
	 * 0x08: 通道数(0单通道/1立体声)
	 */
	uint8_t FLAG; 
}wav_file_t;

/************************************************
 *              错误码枚举                       *
 ************************************************/
typedef enum {
	OK = 0                       ,       // 操作成功
	FILES_END                    ,       // 文件已到末尾
	FILES_FAIL                   ,       // 文件操作失败
	UNVALID_RIFF_ID              ,       // 无效的RIFF标识
	UNVALID_RIFF_SIZE            ,       // 无效的RIFF大小
	UNVALID_WAVE_ID              ,       // 无效的WAVE标识
	UNVALID_FMT_ID               ,       // 无效的fmt标识
	UNVALID_FMT_SIZE             ,       // 无效的fmt块大小
	UNSUPPORETD_FORMATTAG        ,       // 不支持的格式标签（如非PCM格式）
	UNSUPPORETD_NUMBER_OF_CHANNEL,       // 不支持的通道数
	UNSUPPORETD_SAMPLE_RATE      ,       // 不支持的采样率
	UNSUPPORETD_BITS_PER_SAMPLE  ,       // 不支持的采样位深
	UNVALID_LIST_SIZE            ,       // 无效的LIST块大小
	UNVALID_DATA_ID              ,       // 无效的数据块标识
}errorcode_e;

wave_header_t* waveheader_init(void);
wave_header_t* creat_wave_header(uint32_t length, uint32_t sample_rate, uint8_t bits_perSample, uint8_t num_chans);
errorcode_e is_wav_files(FIL** FLE,wave_header_t** header);
void wavheader_Info(FIL** file,wave_header_t** header);
uint8_t Found_Wavfiles(char* wavfiles,uint8_t MAX_FILE,uint8_t MAX_FILENAME_LEN,TCHAR** PATH);
#endif