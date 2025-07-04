#include "PCM_MIC.h"
#include "LOG_SYSTEM\LOG.h"
#include "sysctl.h"

int16_t rx_buf[FRAME_LEN * 2 * 2];// *2  因为32位采样中只有16位有效,所以存储在uint16_t中就需要2倍空间
uint32_t g_rx_dma_buf[FRAME_LEN * 2 * 2];//(FRAME_LEN * 2)表示每个帧（FRAME_LEN）的两个通道数据 
                                                  // *2  构建双缓冲区域
uint8_t g_index = 0;//用于指示当前接收到的数据在 g_rx_dma_buf 中的位置
uint8_t i2s_rec_flag = 0;

/**
 * @brief  扬声器I2S初始化
 * @param  无
 * @retval --32位宽16位深
 *         --I2S0的0号通道
 *         --右对齐模式
 */
void PCM_I2S_Init(){
    //初始化为 发送模式 通道掩码：
    i2s_init(PCM_I2S_DEVICE,I2S_TRANSMITTER,I2S_CHANNE0_MASK << (PCM_I2S_CHANNE*2));
    //设置I2S发送通道
    i2s_tx_channel_config(
        PCM_I2S_DEVICE,         /*设备号*/
        PCM_I2S_CHANNE,         /*通道号*/
        RESOLUTION_16_BIT,      /*单次接收数据位数  (位深)*/
        SCLK_CYCLES_32,         /*单次数据时钟数    (位宽)*/
        TRIGGER_LEVEL_4,        /*DMA的FIFO触发深度*/
        RIGHT_JUSTIFYING_MODE); /*I2S工作模式       (右对齐模式)*/
    
    i2s_set_sample_rate(PCM_I2S_DEVICE,16000);//设置采样率
    /*虽然音频样本的有效位数是16位，但每个样本占据32个时钟周期，剩余的位可以用作填充或对齐*/
    EMLOG(LOG_INFO,"PCM I2S Init Success\n");
}

/**
 * @brief  扬声器播放
 * @param  PCM_DATA: 播放数据
 * @param  DATA_size:数据内存大小(字节数)
 * @retval None
 */
void PCM_Play(void *PCM_DATA,uint64_t DATA_size){
    if(!PCM_DATA){EMLOG(LOG_ERROR,"PCM_DATA is NULL\n");return ;}
    /*sizeof()  的本质是: 直接返回编译器在编译时已知的类型或变量的内存大小
                对于数组,在定义时编译器就将它的内存大小记录下来了,在使用sizeof时会直接返回整体占用内存大小
                对于一般变量或指针,这些在定义时也被编译器记录下来了
    */                    
    i2s_play(
        PCM_I2S_DEVICE,             /* I2S设备号 */
        PCM_DMAC_CHANNE,            /* DMA通道号 */ 
        (uint8_t *)PCM_DATA,        /* 播放的PCM数据 */
        DATA_size,                  /* PCM数据的长度 */
        1024,                       /* 单次发送数量 */
        16,                         /* 单次采样位宽 */
        2);                         /* 声道数 */
    EMLOG(LOG_INFO,"PCM Play Success\n");
}

/**
* @brief         I2S0接收麦克风数据中断回调函数
* @param[in]     void
* @param[out]    void
* @retval        0
*/
int __attribute__((optimize("O0")))I2S_RECEIVE_DMA_CB(void *ctx){
    uint32_t i;
    if(g_index){                                /*双缓冲区写法*/
        i2s_receive_data_dma(MIC_I2S_DEVICE,(uint32_t*)&g_rx_dma_buf[g_index], FRAME_LEN * 2, MIC_DMAC_CHANNE);/* 接收DMA数据 */
        g_index = 0;//表示下一次触发中断时,数据存放的起始位置 
        for(i = 0; i < FRAME_LEN; i++){
            /* 保存数据 */
            rx_buf[2 * i] = (int16_t)((g_rx_dma_buf[2 * i + 1] * MIC_GAIN) & 0xffff);
            rx_buf[2 * i + 1] = (int16_t)((g_rx_dma_buf[2 * i + 1] * MIC_GAIN) & 0xffff);
        }
        i2s_rec_flag = 1;
    }else{
        i2s_receive_data_dma(MIC_I2S_DEVICE, (uint32_t*)&g_rx_dma_buf[0], FRAME_LEN * 2, MIC_DMAC_CHANNE);
        g_index = FRAME_LEN * 2;
        for(i = FRAME_LEN; i < FRAME_LEN * 2; i++){//只获取单个声道的采集数据,所以接收区内奇数才是有效的数据    (高右低左)[先低后高]
            rx_buf[2 * i] = (int16_t)((g_rx_dma_buf[2 * i + 1] * MIC_GAIN) & 0xffff);
            //因为32位采样16位有效的原因,为此取低16位
            rx_buf[2 * i + 1] = (int16_t)((g_rx_dma_buf[2 * i + 1] * MIC_GAIN) & 0xffff);
            //设置左右声道发出声音一样
            // EMLOG(LOG_INFO,"%lx  %lx\n",rx_buf[2*i],rx_buf[2*i+1]);
        }
        i2s_rec_flag = 2;
    }
    return 0;
}

/**
 * @brief  麦克风初始化
 * @param  无
 * @retval None
 */
void MIC_I2S_Init(void){
    i2s_init(MIC_I2S_DEVICE,I2S_RECEIVER,I2S_CHANNE0_MASK << (MIC_I2S_CHANNE*2));
    i2s_rx_channel_config(
        MIC_I2S_DEVICE,         /*设备号*/
        MIC_I2S_CHANNE,         /*通道号*/
        RESOLUTION_16_BIT,      /*单次接收数据位数  (位深)*/
        SCLK_CYCLES_32,         /*单次数据时钟数    (位宽)*/
        TRIGGER_LEVEL_4,        /*DMA的FIFO触发深度*/
        STANDARD_MODE); /*I2S工作模式       (标准模式)*/

    i2s_set_sample_rate(MIC_I2S_DEVICE,16000);//设置采样率
    EMLOG(LOG_INFO,"MIC I2S Init Success\n");
    //注册DMA中断函数
    dmac_irq_register(MIC_DMAC_CHANNE,I2S_RECEIVE_DMA_CB,NULL,4);
    //开启接收
    i2s_receive_data_dma(MIC_I2S_DEVICE,(uint32_t*)&g_rx_dma_buf[FRAME_LEN*2],FRAME_LEN*2,MIC_DMAC_CHANNE);
    EMLOG(LOG_INFO,"MIC DMA IRQ Set Success\n");
}
