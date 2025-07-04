#include "PIN_Config.h"
#include "fpioa.h"
#include "sysctl.h"
#include "uarths.h"
#include "PCM_MIC.h"
#include "LOG_SYSTEM/LOG.h"

extern uint8_t i2s_rec_flag;
extern int16_t rx_buf[(FRAME_LEN * 2) * 2];

void Hardware_Init(void);
void System_Clock_Init(void);

int main(){
    Hardware_Init();
    System_Clock_Init();

    /* 初始化中断，使能全局中断，初始化dmac */
    plic_init();
    sysctl_enable_irq();
    dmac_init();//一定要在注册函数之前,该函数会重置一切回调函数

    PCM_I2S_Init();
    MIC_I2S_Init();
    while (1){
        if(i2s_rec_flag == 1){
            i2s_play(
                PCM_I2S_DEVICE,  /* I2S设备号 */
                PCM_DMAC_CHANNE, /* DMA通道号 */ 
                (uint8_t *)(&rx_buf[0]), /* 播放的PCM数据 */
                FRAME_LEN * 4, /* PCM数据的长度 */
                1024, /* 单次发送数量 */
                16,   /* 单次采样位宽 */
                2);   /* 声道数 */
            i2s_rec_flag = 0;
        }else if(i2s_rec_flag == 2){
            i2s_play(
                PCM_I2S_DEVICE, /* I2S设备号 */
                PCM_DMAC_CHANNE, /* DMA通道号 */ 
                (uint8_t *)(&rx_buf[FRAME_LEN * 2]), /* 播放的PCM数据 */
                FRAME_LEN * 4, /* PCM数据的长度(总字节数) */
                1024, /* 单次发送数量 */
                16, /* 单次采样位宽 */
                2); /* 声道数 */
            i2s_rec_flag = 0;
        }
    }
    return 0;
}

/**
 * @brief  硬件初始化
 * @param  无
 * @retval None
 */
void Hardware_Init(void){
    /* mic */
    fpioa_set_function(PIN_MIC_WS,   FUNC_MIC_WS);
    fpioa_set_function(PIN_MIC_DATA, FUNC_MIC_DATA);
    fpioa_set_function(PIN_MIC_SCK,  FUNC_MIC_SCK);

    /* speak dac */
    fpioa_set_function(PIN_SPK_WS,   FUNC_SPK_WS);
    fpioa_set_function(PIN_SPK_DATA, FUNC_SPK_DATA);
    fpioa_set_function(PIN_SPK_SCK,  FUNC_SPK_SCK);
}

/**
 * @brief  设置PLL频率（系统时钟频率）
 * @param  无
 * @retval None
 */
void System_Clock_Init(void){
    sysctl_pll_set_freq(SYSCTL_PLL0, 320000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 160000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
    uarths_init();//因为uraths时钟受PLL0分频影响,所以当PLL0被修改后,需要使用重新初始化
}