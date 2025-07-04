#ifndef __PCM_MIC__
#define __PCM_MIC__
#include"i2s.h"
#include "dmac.h"

#define PCM_I2S_DEVICE I2S_DEVICE_1
#define PCM_I2S_CHANNE I2S_CHANNEL_0
#define PCM_DMAC_CHANNE DMAC_CHANNEL1

#define MIC_I2S_DEVICE I2S_DEVICE_2
#define MIC_I2S_CHANNE I2S_CHANNEL_0
#define MIC_DMAC_CHANNE DMAC_CHANNEL0

#define MIC_GAIN      1     /* 麦克风增益值，可以根据实际调大录音的音量 */
#define FRAME_LEN     512   /* 一个声道的数据量大小 */

void PCM_I2S_Init(void);
void PCM_Play(void *PCM_DATA,uint64_t DATA_size);
void MIC_I2S_Init(void);
#endif