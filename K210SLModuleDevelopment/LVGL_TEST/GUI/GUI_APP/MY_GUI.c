#include "GUI\LVGL\lvgl.h"
#include "MY_GUI.h"
#include "LOG_SYSTEM\LOG.h"
#include "GUI\LVGL\src\core\lv_obj_private.h"
#include "ICM2060\ICM2060.h"

lv_coord_t SCR_WIDTH;
lv_coord_t SCR_HEIGHT;

void event_cb(lv_event_t * e){
    ;
}

LV_IMG_DECLARE(img_src)
lv_obj_t* img;
void Img_Create(void){
    img = lv_img_create(lv_scr_act());
    lv_obj_set_size(img,SCR_WIDTH/3,SCR_HEIGHT/3);
    lv_obj_set_style_align(img,LV_ALIGN_CENTER,LV_STATE_DEFAULT);
    lv_img_set_src(img,&img_src);
}

int My_GUI(void){
    //动态获取屏幕大小
    SCR_HEIGHT = lv_obj_get_height(lv_scr_act());
    SCR_WIDTH = lv_obj_get_width(lv_scr_act());
    //创建一个图片对象
    Img_Create();
    return 0;
}

void lvgl_move_image(int16_t x,int16_t y){
    lv_obj_set_pos(img,x,y);
}

void lvgl_Horizontal_plates(){
    ICM2060_PosSlove();
    int16_t x = convertAngle(ICM_EndData.roll);
    int16_t y = convertAngle(ICM_EndData.pitch);
    lvgl_move_image(x,y);
    EMLOG(LOG_INFO,"ROLL:%d PITCH:%d\n",x,y);
}