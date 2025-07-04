/**
 * @file lv_port_disp_templ.c
 *
 */

/*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp.h"
#include <stdbool.h>
#include "GUI\LCD_SHOW\lcd.h"

#define MY_DISP_HOR_RES LCD_X_MAX
#define MY_DISP_VER_RES LCD_Y_MAX

/*********************
 *      DEFINES
 *********************/
#ifndef MY_DISP_HOR_RES
    #warning Please define or replace the macro MY_DISP_HOR_RES with the actual screen width, default value 320 is used for now.
    #define MY_DISP_HOR_RES    320
#endif

#ifndef MY_DISP_VER_RES
    #warning Please define or replace the macro MY_DISP_VER_RES with the actual screen height, default value 240 is used for now.
    #define MY_DISP_VER_RES    240
#endif

#define BYTE_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565)) /*will be 2 for RGB565 */

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

static void disp_flush(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map);
static void disp_flush_irq(lv_display_t * disp_drv, const lv_area_t * area, uint8_t * px_map);
/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();

    /*------------------------------------
     * Create a display and set a flush_cb
     * -----------------------------------*/
    lv_display_t * disp = lv_display_create(MY_DISP_HOR_RES, MY_DISP_VER_RES);
    lv_display_set_flush_cb(disp, disp_flush_irq);

    /* Example 1
     * One buffer for partial rendering*/
    LV_ATTRIBUTE_MEM_ALIGN
    static uint8_t buf_1_1[MY_DISP_HOR_RES * 10 * BYTE_PER_PIXEL];            /*A buffer for 10 rows*/
    static uint8_t buf_1_2[MY_DISP_HOR_RES * 10 * BYTE_PER_PIXEL];            /*A buffer for 10 rows*/
    lv_display_set_buffers(disp, buf_1_1, buf_1_2, sizeof(buf_1_1), LV_DISPLAY_RENDER_MODE_PARTIAL);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init(void){
    lcd_init();
}

volatile bool disp_flush_enabled = true;

void disp_enable_update(void){
    disp_flush_enabled = true;
}

void disp_disable_update(void){
    disp_flush_enabled = false;
}

static lv_display_t* disp_success_ctx;
int disp_success_callback(void* ctx){
    lv_display_flush_ready(disp_success_ctx);
    return 0;
}

static void disp_flush_irq(lv_display_t * disp_drv, const lv_area_t * area, uint8_t * px_map){
    disp_success_ctx = disp_drv;
    lcd_draw_picture_half_irq(area->x1,area->y1,area->x2-area->x1+1,area->y2-area->y1+1,(uint16_t*)px_map,disp_success_callback);
}

// static void disp_flush(lv_display_t * disp_drv, const lv_area_t * area, uint8_t * px_map){
//     lcd_draw_picture_half(area->x1,area->y1,area->x2-area->x1+1,area->y2-area->y1+1,(uint16_t*)px_map);
//     lv_display_flush_ready(disp_drv);
// }
#else 
#endif
