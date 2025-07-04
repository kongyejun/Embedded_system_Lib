#include <stdio.h>
#include "dvp_cam.h"
#include "dvp.h"
#include "plic.h"
#include "sysctl.h"
#include "stdlib.h"
#include "kpu.h"
// 图像传递到AI的相关缓冲区
uint8_t *g_ai_buf_in = NULL;
uint32_t g_ai_red_buf_addr, g_ai_green_buf_addr, g_ai_blue_buf_addr;

uint32_t *display_buf1 = NULL;
uint32_t display_buf_addr1 = 0;
uint32_t *display_buf2 = NULL;
uint32_t display_buf_addr2 = 0;
volatile uint8_t g_dvp_finish_flag;
uint8_t gindx_i = 0;

#define OVXXXX_ADDR    0x60 
#define OV9655_PID_1   0x9657   
#define OV9655_PID_2   0x9656  
#define OV2640_PID     0x2642

volatile uint32_t g_ai_done_flag;
kpu_model_context_t face_detect_task;//KPU任务句柄

void Kpu_Callback(void* userdata){
    g_ai_done_flag = 1;// 标记AI运算完成
    return ;
}

/* dvp中断回调函数 */
static int on_dvp_irq_cb(void *ctx){
    /* 读取DVP中断状态，如果完成则刷新显示地址的数据，并清除中断标志，否则读取摄像头数据*/
    if (dvp_get_interrupt(DVP_STS_FRAME_FINISH)){
        if(gindx_i==0)
            dvp_set_display_addr((uint32_t)display_buf_addr1);
        else 
            dvp_set_display_addr((uint32_t)display_buf_addr2);
        // 运行 kmodel   KPU任务句柄                     源数据                 DMA通道     完成后回调函数  回调的参数 
        kpu_run_kmodel(&face_detect_task, (const uint8_t*)g_ai_red_buf_addr, DMAC_CHANNEL5, Kpu_Callback, NULL);
        dvp_clear_interrupt(DVP_STS_FRAME_FINISH);
        g_dvp_finish_flag = 1;
    }
    else{
        if (g_dvp_finish_flag == 0){
            gindx_i+=1;gindx_i%=2;
            dvp_start_convert();// 开始新的图像数据采集
        }
        dvp_clear_interrupt(DVP_STS_FRAME_START);
    }
    return 0;
}

/* dvp初始化 */
void dvp_cam_init(void){
    /* DVP初始化，设置sccb的寄存器长度为8bit */
    dvp_init(8);

    dvp_sccb_set_clk_rate(1000000);//为了兼容GC2145不能太高的速率

    /* 设置输入时钟为24000000*/
    dvp_set_xclk_rate(24000000); //20000000
    /* 使能突发传输模式 */
    dvp_enable_burst();

    /* 关闭AI输出模式，使能显示模式 */
    dvp_set_output_enable(DVP_OUTPUT_AI, 0);
    dvp_set_output_enable(DVP_OUTPUT_DISPLAY, 1);
    /* 设置输出格式为RGB */
    dvp_set_image_format(DVP_CFG_RGB_FORMAT);
    /* 设置输出像素大小为320*240 */
    dvp_set_image_size(CAM_WIDTH_PIXEL, CAM_HIGHT_PIXEL);

    /* 设置DVP的显示地址参数和中断 */
    display_buf1 = (uint32_t*)malloc(CAM_WIDTH_PIXEL * CAM_HIGHT_PIXEL * 2);
    display_buf_addr1 = (uint32_t)display_buf1;
    display_buf2 = (uint32_t*)malloc(CAM_WIDTH_PIXEL * CAM_HIGHT_PIXEL * 2);
    display_buf_addr2 = (uint32_t)display_buf2;
    dvp_set_display_addr(display_buf_addr1);
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 0);
    dvp_disable_auto();
}

/* dvp初始化开启AI输出模式 */
void dvp_camAI_init(void){
    /* DVP初始化，设置sccb的寄存器长度为8bit */
    dvp_init(8);

    dvp_sccb_set_clk_rate(1000000);//为了兼容GC2145不能太高的速率

    /* 设置输入时钟为24000000*/
    dvp_set_xclk_rate(24000000); //20000000
    /* 使能突发传输模式 */
    dvp_enable_burst();

    /* 关闭AI输出模式，使能显示模式 */
    dvp_set_output_enable(DVP_OUTPUT_AI, 1);
    dvp_set_output_enable(DVP_OUTPUT_DISPLAY, 1);
    /* 设置输出格式为RGB */
    dvp_set_image_format(DVP_CFG_RGB_FORMAT);
    /* 设置输出像素大小为320*240 */
    dvp_set_image_size(CAM_WIDTH_PIXEL, CAM_HIGHT_PIXEL);

    /* 设置DVP的显示地址参数和中断 */
    display_buf1 = (uint32_t*)malloc(CAM_WIDTH_PIXEL * CAM_HIGHT_PIXEL * 2);
    display_buf_addr1 = (uint32_t)display_buf1;
    display_buf2 = (uint32_t*)malloc(CAM_WIDTH_PIXEL * CAM_HIGHT_PIXEL * 2);
    display_buf_addr2 = (uint32_t)display_buf2;
    dvp_set_display_addr(display_buf_addr1);
    /* 设置Ai相关地址 */
    g_ai_buf_in = (uint8_t*)malloc(CAM_WIDTH_PIXEL * CAM_HIGHT_PIXEL * 3);
    g_ai_red_buf_addr =  (uint32_t)&g_ai_buf_in[0];                                     // 保存AI处理图片R通道的地址,十进制值
    g_ai_green_buf_addr = (uint32_t)&g_ai_buf_in[CAM_WIDTH_PIXEL * CAM_HIGHT_PIXEL];    // 保存AI处理图片G通道的地址
    g_ai_blue_buf_addr = (uint32_t)&g_ai_buf_in[CAM_WIDTH_PIXEL * CAM_HIGHT_PIXEL * 2]; // 保存AI处理图片B通道的地址
    dvp_set_ai_addr(g_ai_red_buf_addr, g_ai_green_buf_addr, g_ai_blue_buf_addr);        // 设置AI处理图片的地址

    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 0);
    dvp_disable_auto();
}

void dvp_cam_set_irq(void){
    /* DVP 中断配置：中断优先级，中断回调，使能DVP中断 */
    printf("DVP interrupt config\r\n");
    plic_set_priority(IRQN_DVP_INTERRUPT, 1);
    plic_irq_register(IRQN_DVP_INTERRUPT, on_dvp_irq_cb, NULL);
    plic_irq_enable(IRQN_DVP_INTERRUPT);

    // /* 使能系统全局中断 */
    // sysctl_enable_irq();

    /* 清除DVP中断位 */
    g_dvp_finish_flag = 0;
    dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);
}


int OVxxxx_read_id(void) //只适用OV9655和2640
{
    uint16_t manuf_id = 0;
    uint16_t device_id = 0;
    uint16_t *manuf_id_P = &manuf_id;
    uint16_t *device_id_P = &device_id;
    dvp_cam_init();
    dvp_cam_set_irq();

    *manuf_id_P = (dvp_sccb_receive_data(OVXXXX_ADDR, 0x1C) << 8) | dvp_sccb_receive_data(OVXXXX_ADDR, 0x1D); //读制造商标识符
    *device_id_P = (dvp_sccb_receive_data(OVXXXX_ADDR, 0x0A) << 8) | dvp_sccb_receive_data(OVXXXX_ADDR, 0x0B); //读PID和VER
    

    if(device_id==OV9655_PID_1 ||device_id==OV9655_PID_2)//OV9655
    {
        printf("This is the OV9655 camera\n");
        printf("manuf_id:0x%04x,device_id:0x%04x\n", manuf_id, device_id);
        return OV_9655;
    }
    else if(device_id==OV2640_PID)//OV2640
    {
        printf("This is the OV2640 camera\n");
        printf("manuf_id:0x%04x,device_id:0x%04x\n", manuf_id, device_id);
        return OV_2640;
    }
    else
    {
        // printf("manuf_id:0x%04x,device_id:0x%04x\n", manuf_id, device_id);
        // printf("Camera failure\n");
        return OV_error;
    }
    return 0;
}