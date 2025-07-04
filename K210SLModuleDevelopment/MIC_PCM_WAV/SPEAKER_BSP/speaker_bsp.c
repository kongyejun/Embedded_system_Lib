#include "SPEAKER_BSP/speaker_bsp.h"
#include "LOG_SYSTEM\LOG.h"
#include "WAV_Files\wav_bsp.h"

/**
 * @brief  扬声器I2S初始化
 * @param  无
 * @retval --32位宽16位深
 *         --I2S0的0号通道
 *         --右对齐模式
 */
void Speaker_init(plic_irq_callback_t callback,uint32_t sample_rate){
    //初始化为 发送模式 通道掩码：
    i2s_init(SPEAKER_I2S_DEVICE,I2S_TRANSMITTER,I2S_CHANNE0_MASK << (SPEAKER_I2S_CHANNE*2));
    //设置I2S发送通道
    i2s_tx_channel_config(
        SPEAKER_I2S_DEVICE   ,   /*设备号*/
        SPEAKER_I2S_CHANNE   ,   /*通道号*/
        RESOLUTION_16_BIT    ,   /*单次接收数据位数  (位深)*/
        SCLK_CYCLES_32       ,   /*单次数据时钟数    (位宽)*/
        TRIGGER_LEVEL_4      ,   /*DMA的FIFO触发深度*/
        RIGHT_JUSTIFYING_MODE);  /*I2S工作模式       (右对齐模式)*/
    
    i2s_set_sample_rate(SPEAKER_I2S_DEVICE,sample_rate);//设置采样率
    /*虽然音频样本的有效位数是16位，但每个样本占据32个时钟周期，剩余的位可以用作填充或对齐*/
    //注册回调函数
    if(callback==NULL){
        EMLOG(LOG_INFO,"Speaker Init Success,");
        printf_red("But don't set callback!!!\n");
        return ;
    }
    dmac_set_irq(SPEAKER_DMAC_CHANNE, callback, NULL, 2);
    EMLOG(LOG_INFO,"Speaker Init Success\n");
}

// 注销扬声器设备
void Speaker_deinit(void){
    dmac_wait_done(SPEAKER_DMAC_CHANNE);
    dmac_channel_disable(SPEAKER_DMAC_CHANNE);
    dmac_free_irq(SPEAKER_DMAC_CHANNE);
    EMLOG(LOG_INFO,"Speaker deinit\n");
}

// 扬声器停止工作
void Speaker_stop(void){
    dmac_wait_done(SPEAKER_DMAC_CHANNE);
    dmac_channel_disable(SPEAKER_DMAC_CHANNE);
    EMLOG(LOG_INFO,"Speaker Stop\n");
}
// 扬声器开始工作
void Speaker_start(void){
    dmac_channel_enable(SPEAKER_DMAC_CHANNE);
    EMLOG(LOG_INFO,"Speaker Start\n");
}

/**
 * @brief  播放PCM数据
 * @param  PCM_DATA:PCM数据
 * @param  DATA_size:PCM数据大小
 * @param  frame:单次发送字节数
 */
void Speaker_play_data(void *PCM_DATA,uint64_t DATA_size,size_t frame){
    if(!PCM_DATA){EMLOG(LOG_ERROR,"PCM_DATA is NULL\n");return ;}
    /*sizeof()  的本质是: 直接返回编译器在编译时已知的类型或变量的内存大小
                对于数组,在定义时编译器就将它的内存大小记录下来了,在使用sizeof时会直接返回整体占用内存大小
                对于一般变量或指针,这些在定义时也被编译器记录下来了
    */                    
    i2s_play(
        SPEAKER_I2S_DEVICE,         /* I2S设备号 */
        SPEAKER_I2S_CHANNE,         /* DMA通道号 */ 
        (uint8_t *)PCM_DATA,        /* 播放的PCM数据 */
        DATA_size,                  /* PCM数据的长度 */
        frame,                      /* 单次发送数量 */
        16,                         /* 单次采样位宽 */
        2);                         /* 声道数 */
    //EMLOG(LOG_DEBUG,"Speaker Play Success\n");
}
