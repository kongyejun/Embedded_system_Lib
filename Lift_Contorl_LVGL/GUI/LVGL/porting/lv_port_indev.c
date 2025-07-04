/*********************
 *      INCLUDES
 *********************/
#include "GUI\LVGL\porting\lv_port_indev.h"
#include "GUI\LCD_Button\Button.h"
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void button_init(void);
static void button_read(lv_indev_t * indev, lv_indev_data_t * data);

/**********************
 *  STATIC VARIABLES
 **********************/
lv_indev_t * indev_button;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_indev_init(void){
    // 初始化按键外设
    button_init();// LVGL系统初始化按键接口，若已在之前初始化可以省略

    // 初始化LVGL按键管理句柄
    indev_button = lv_indev_create();// 创建输入设备句柄
    lv_indev_set_type(indev_button, LV_INDEV_TYPE_BUTTON);// 设定句柄管理类型为按键
    lv_indev_set_read_cb(indev_button, button_read);// 设定按键回调函数
    static const lv_point_t btn_points[Key_Num_Max]={
        {0 , 0},
        {82 , 0},
        {0, 82},
    };

    lv_indev_set_button_points(indev_button, btn_points);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
// LVGL按键初始化
static void button_init(void){
    LCD_Button_Init();
}

/*Will be called by the library to read the button*/
static void button_read(lv_indev_t * indev_drv, lv_indev_data_t * data){
    static uint8_t last_btn = 0;
    int8_t btn_act = Sreach_DownKey();
    if(btn_act >= 0){
        data->state = LV_INDEV_STATE_PRESSED;
        last_btn = btn_act;
    }else{
        data->state = LV_INDEV_STATE_RELEASED;
    }
    // 如果没有按键按下，则不更新按键ID
    data->btn_id = last_btn;
}
