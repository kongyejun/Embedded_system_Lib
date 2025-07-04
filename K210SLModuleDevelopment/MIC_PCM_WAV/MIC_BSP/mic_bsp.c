#include "MIC_BSP\mic_bsp.h"
#include "LOG_SYSTEM\LOG.h"
#include "i2s.h"

/**
 * @brief  麦克风初始化
 * @param  callback:中断回调函数
 * @param  sample_rate:采样率
 * @retval None
 */
void MIC_Init(plic_irq_callback_t callback,uint32_t sample_rate){
    i2s_init(MIC_I2S_DEVICE,I2S_RECEIVER,I2S_CHANNE0_MASK << (MIC_I2S_CHANNEL*2));
    i2s_rx_channel_config(
        MIC_I2S_DEVICE   ,   /*设备号*/
        MIC_I2S_CHANNEL  ,   /*通道号*/
        RESOLUTION_16_BIT,   /*单次接收数据位数  (位深)*/
        SCLK_CYCLES_32   ,   /*单次数据时钟数    (位宽)*/
        TRIGGER_LEVEL_4  ,   /*DMA的FIFO触发深度*/
        STANDARD_MODE    );  /*I2S工作模式       (标准模式)*/

    i2s_set_sample_rate(MIC_I2S_DEVICE,sample_rate);//设置采样率
    //注册DMA中断函数
    if(callback == NULL){
        EMLOG(LOG_INFO,"MIC Init Success,");
        printf_red("But don't set callback!!!\n");
        return ;
    }
    dmac_irq_register(MIC_DMAC_CHANNE,callback,NULL,5);
    //i2s_receive_data_dma(MIC_I2S_DEVICE,buff, 100, MIC_DMAC_CHANNE);
    EMLOG(LOG_INFO,"MIC I2S Init Success\n");
}


/**
 * @brief  注销麦克风设备，释放资源
 * @param  None
*/
void MIC_deinit(void){
    dmac_wait_done(MIC_DMAC_CHANNE);
    dmac_channel_disable(MIC_DMAC_CHANNE);
    dmac_free_irq(MIC_DMAC_CHANNE);
    EMLOG(LOG_INFO,"MIC Deinit\n");
}

/**
 * @brief  初始化接收缓冲区
 * @param  rx_buff:最终接收缓冲区
 * @param  g_rx_dma_buf:DMA接收缓冲区
 * @return 1:失败 0:成功
*/
int8_t MIC_Buff_Init(int16_t** rx_buff,uint32_t** g_rx_dma_buf){
    if(*rx_buff == NULL || *g_rx_dma_buf == NULL){
        EMLOG(LOG_INFO,"rx_buff or g_rx_dma_buf is NULL\n");
        //分配内存
        *rx_buff = (int16_t*)malloc(MIC_FRAME_LEN*2*2*4);//双声道，16位传输，双缓存
        if(*rx_buff == NULL){
            EMLOG(LOG_ERROR,"rx_buff malloc fail\n");
            return 1;
        }
        *g_rx_dma_buf = (uint32_t*)malloc(MIC_FRAME_LEN*2*2*8);//双声道，32位传输，双缓存
        if(*g_rx_dma_buf == NULL){
            free(*rx_buff);*rx_buff = NULL;
            EMLOG(LOG_ERROR,"g_rx_dma_buf malloc fail\n");
            return 1;
        }
    }
    //初始化内存
    memset((int16_t *)(*rx_buff),1,MIC_FRAME_LEN*2*2*2);
    memset((uint32_t *)(*g_rx_dma_buf),1,MIC_FRAME_LEN*2*2*4);
    EMLOG(LOG_DEBUG,"buff memset 0 ok!!\n");
    return 0;
}