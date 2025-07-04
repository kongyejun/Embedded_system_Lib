#ifndef __SPEAKER_BSP_H__
#define __SPEAKER_BSP_H__
#include "i2s.h"
#include "dmac.h"

#define SPEAKER_I2S_DEVICE I2S_DEVICE_2
#define SPEAKER_I2S_CHANNE I2S_CHANNEL_0
#define SPEAKER_DMAC_CHANNE DMAC_CHANNEL5


void Speaker_init(plic_irq_callback_t callback,uint32_t sample_rate);
void Speaker_deinit(void);
void Speaker_stop(void);
void Speaker_start(void);
void Speaker_play_data(void *PCM_DATA,uint64_t DATA_size,size_t frame);
#endif