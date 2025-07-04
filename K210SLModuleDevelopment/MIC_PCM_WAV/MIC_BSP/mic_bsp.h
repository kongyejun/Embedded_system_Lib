#ifndef __MIC_BSP_H__
#define __MIC_BSP_H__
#include "plic.h"

//MIC硬件定义
#define MIC_I2S_DEVICE   I2S_DEVICE_0
#define MIC_I2S_CHANNEL  I2S_CHANNEL_0
#define MIC_DMAC_CHANNE  DMAC_CHANNEL1  //注意SD卡抢占了0号通道
//MIC数据定义
#define MIC_FRAME_LEN 256
#define MIC_GAIN      10     /* 麦克风增益值，可以根据实际调大录音的音量 */


void MIC_Init(plic_irq_callback_t callback,uint32_t sample_rate);
void MIC_deinit(void);
int8_t MIC_Buff_Init(int16_t** rx_buff,uint32_t** g_rx_dma_buf);
#endif