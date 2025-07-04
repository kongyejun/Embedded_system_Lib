#include "PIN_Config.h"
#include "fpioa.h"
#include "sysctl.h"
#include "uarths.h"
#include "FLASH\FLASH_W25.h"
#include "kpu.h"
#include "Carmre\dvp_cam.h"
#include "bsp.h"
#include "LCD\lcd.h"
#include "incbin.h"
#include "Carmre\gc2145.h"
#include "LOG_SYSTEM\LOG.h"
#include "region_layer.h"
#include "font.h"

#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX

void HardwareSystem_Init(void);

#define  LOAD_KMODEL_FROM_FLASH    0
#if LOAD_KMODEL_FROM_FLASH
#define KMODEL_SIZE (380 * 1024)
uint8_t* model_data;
#else
INCBIN(model, "detect.kmodel");
#endif

volatile uint32_t* LCD_PlayADD,*playadd;
volatile uint8_t pubilc_value_lock=0; 
extern volatile uint32_t g_ai_done_flag;
extern kpu_model_context_t face_detect_task;//KPU任务句柄
static region_layer_t face_detect_rl;
static obj_info_t face_detect_info;// 人脸检测最终信息
/* 人脸层参数配置 */
#define ANCHOR_NUM 5
float g_anchor[ANCHOR_NUM * 2] = {0.57273, 0.677385, 1.87446, 2.06253, 3.33843, 5.47434, 7.88282, 3.52778, 9.77052, 9.16828};

//画人脸识别框
static void draw_edge(uint32_t *gram, obj_info_t *obj_info, uint32_t index, uint16_t color){
    uint32_t data = ((uint32_t)color << 16) | (uint32_t)color;// 一次性设置两个相连的像素颜色
    uint32_t *addr1, *addr2, *addr3, *addr4, x1, y1, x2, y2;
    // 保存相关值
    x1 = obj_info->obj[index].x1;
    y1 = obj_info->obj[index].y1;
    x2 = obj_info->obj[index].x2;
    y2 = obj_info->obj[index].y2;
    // 识别超出边界处理
    if (x1 <= 0)
        x1 = 1;
    if (x2 >= LCD_X_MAX-1)
        x2 = LCD_X_MAX-2;
    if (y1 <= 0)
        y1 = 1;
    if (y2 >= LCD_Y_MAX-1)
        y2 = LCD_Y_MAX-2;
    // 计算方框的角点坐标
    addr1 = gram + (LCD_X_MAX * y1 + x1) / 2;// 因为一次性设置两个相邻的像素,所有需要除以2
    addr2 = gram + (LCD_X_MAX * y1 + x2 - 8) / 2;
    addr3 = gram + (LCD_X_MAX * (y2 - 1) + x1) / 2;
    addr4 = gram + (LCD_X_MAX * (y2 - 1) + x2 - 8) / 2;
    // 绘制竖直的线 宽8个像素
    for (uint32_t i = 0; i < 4; i++){
        *addr1 = data;
        *(addr1 + LCD_X_MAX/2) = data;// 相当于换行,因为此时是一次性控制两个像素
        *addr2 = data;
        *(addr2 + LCD_X_MAX/2) = data;
        *addr3 = data;
        *(addr3 + LCD_X_MAX/2) = data;
        *addr4 = data;
        *(addr4 + LCD_X_MAX/2) = data;
        addr1++;
        addr2++;
        addr3++;
        addr4++;
    }

    addr1 = gram + (320 * y1 + x1) / 2;
    addr2 = gram + (320 * y1 + x2 - 2) / 2;
    addr3 = gram + (320 * (y2 - 8) + x1) / 2;
    addr4 = gram + (320 * (y2 - 8) + x2 - 2) / 2;
    // 绘制横向的线 长为8个像素
    for (uint32_t i = 0; i < 8; i++){
        *addr1 = data;
        *addr2 = data;
        *addr3 = data;
        *addr4 = data;
        addr1 += 160;
        addr2 += 160;
        addr3 += 160;
        addr4 += 160;
    }
}

int core1_main(void* ctx){
    float *output;
    size_t output_size;
    while(1){
        while (g_dvp_finish_flag == 0);   //等待采集中断 使能
        LCD_PlayADD = gindx_i?(uint32_t*)display_buf_addr2:(uint32_t*)display_buf_addr1;
        g_dvp_finish_flag = 0;// 切换采集
        while(!g_ai_done_flag);//等待KPU处理完成
        // EMLOG(LOG_DEBUG,"OK\n");
                        // 获取 KPU 最终处理的结果  
        //               KPU任务句柄  结果的索引值          结果        大小（字节）
        kpu_get_output(&face_detect_task, 0, (uint8_t **)&output, &output_size);
        /*算法检测人脸*/
        face_detect_rl.input = output;
        region_layer_run(&face_detect_rl, &face_detect_info);
        /*根据返回值进行人脸圈住 */
        for (uint32_t face_cnt = 0; face_cnt < face_detect_info.obj_number; face_cnt++){
            draw_edge(LCD_PlayADD, &face_detect_info, face_cnt, RED);
        }
        // EMLOG(LOG_DEBUG,"OK,pubilc_value_lock:%d\n",pubilc_value_lock);
        while(pubilc_value_lock);// 等待LCD显示完成
        playadd = LCD_PlayADD;
        pubilc_value_lock=1;
    }
}

int main(void){
    HardwareSystem_Init();
    
    // FLASH初始化
    // FLASH_W25_Init();

    /*kmodel加载方式： 1：分开烧录模式  2：直接与代码合并编译*/
#if LOAD_KMODEL_FROM_FLASH
    model_data = (uint8_t*)malloc(KMODEL_SIZE + 255);
    uint8_t *model_data_align = (uint8_t*)(((uintptr_t)model_data+255)&(~255));
    FLASH_W25_Read(0xA00000, model_data_align, KMODEL_SIZE, W25QXX_QUAD_FAST);
#else
    uint8_t *model_data_align = model_data;
#endif

    // LCD初始化
    lcd_init();
    lcd_draw_string(100, 40, "Hello Yahboom!", RED);
    lcd_draw_string(100, 60, "Demo: Face Detect!", BLUE);
    msleep(100);

    // 摄像头初始化
    dvp_camAI_init();
    dvp_cam_set_irq();
    //读取gc2145摄像头
    uint16_t device_id;
    gc2145_read_id(&device_id);
    EMLOG(LOG_DEBUG,"device_id:0x%04x\n", device_id);
    if(device_id != GC2145_ID){
        EMLOG(LOG_ERROR,"Camera failure\n");
        return 0;//打不开摄像头，结束
    }
    EMLOG(LOG_INFO,"This is the GC2145 camera\n");
    gc2145_init();//初始化

    // 模型加载与初始化
    /* init face detect model  KPU任务句柄 kmodel数据*/
    // 加载 kmodel，需要与 nncase 配合使用
    if (kpu_load_kmodel(&face_detect_task, model_data_align) != 0)  {
        EMLOG(LOG_ERROR,"\n模型初始化错误\n");
        while (1);
    }
    // 人脸层配置参数
    face_detect_rl.anchor_number = ANCHOR_NUM; // 设置锚框的数量，ANCHOR_NUM是一个预定义的常量，表示用于人脸检测的锚框数量
    face_detect_rl.anchor = g_anchor; 
    // 将锚框的具体值（坐标和尺寸）赋值给人脸检测层的锚框，g_anchor 是一个预定义的数组或结构体，包含锚框信息
    face_detect_rl.threshold = 0.7; // 设置检测阈值，当检测到的置信度超过时，认为是有效的人脸检测
    face_detect_rl.nms_value = 0.2; 
    // 设置非极大值抑制（NMS）的阈值，用于过滤重叠的检测框，0.3表示重叠度的阈值
    region_layer_init(&face_detect_rl, 20, 15, 30, 320, 240); 
    // 初始化区域层，传入人脸检测层配置以及其他参数：
    // 20（网格高度），15（网格宽度），30（类别数），320（输入图像宽度），240（输入图像高度）
    EMLOG(LOG_WARN,"区域层初始化后,系统可用空间还剩: %ld\r\n", (long)get_free_heap_size());
    sysctl_enable_irq(); // 使能系统中断
    while (g_dvp_finish_flag == 0);   //等待采集中断 使能  先采集一次数据
    register_core1(core1_main,NULL); // 注册核心1任务
    uint64_t time[2]={0};uint8_t str[20]="\n";
    while (1){
        time[0] = sysctl_get_time_us();
        while(pubilc_value_lock==0); // 等待图片处理完成
        /* 显示画面 */
        lcd_draw_picture(0, 0, 320, 240, playadd);
        pubilc_value_lock = 0;
        time[1]=sysctl_get_time_us();
        sprintf((char *)str, "FPS %ld", 1000000/(time[1] - time[0]));
        lcd_draw_string(0,0,(char*)str,BLUE);
    }
    return 0;
}

/* 端口电压设置 */
void io_set_power(void){
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
}

/*系统时钟初始化*/
void sysclock_init(void)
{
    /* 设置系统时钟和DVP时钟 */
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 400000000UL);
    //sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
    sysctl_clock_enable(SYSCTL_CLOCK_AI);
    uarths_init();
}

void HardwareSystem_Init(void){
    // 系统时钟设置
    sysclock_init();
    // LCD
    fpioa_set_function(PIN_LCD_CS, FUNC_LCD_CS);
    fpioa_set_function(PIN_LCD_RST, FUNC_LCD_RST);
    fpioa_set_function(PIN_LCD_RS, FUNC_LCD_RS);
    fpioa_set_function(PIN_LCD_WR, FUNC_LCD_WR);
    // DVP camera
    fpioa_set_function(PIN_DVP_RST, FUNC_CMOS_RST);
    fpioa_set_function(PIN_DVP_PWDN, FUNC_CMOS_PWDN);
    fpioa_set_function(PIN_DVP_XCLK, FUNC_CMOS_XCLK);
    fpioa_set_function(PIN_DVP_VSYNC, FUNC_CMOS_VSYNC);
    fpioa_set_function(PIN_DVP_HSYNC, FUNC_CMOS_HREF);
    fpioa_set_function(PIN_DVP_PCLK, FUNC_CMOS_PCLK);
    fpioa_set_function(PIN_DVP_SCL, FUNC_SCCB_SCLK);
    fpioa_set_function(PIN_DVP_SDA, FUNC_SCCB_SDA);
    // 使能SPI0和DVP
    sysctl_set_spi0_dvp_data(1);
    // IO口电压设置
    io_set_power();
    // 系统中断设置
    plic_init();            /* 系统中断初始化 */
}
