#include "GUI\GUI_APP\MY_GUI.h"
#include "LOG_SYSTEM\LOG.h"
#include "System_Bsp.h"
#include "GUI\LVGL\porting\lv_port_disp.h"
#include "GUI\LVGL\porting\lv_port_indev.h"
#include "GUI\LCD_Button\Button.h"

lv_coord_t SCR_WIDTH;
lv_coord_t SCR_HEIGHT;
/***************************************
 *              全局对象区              *
 ***************************************/
// 按键文字
char* ReturnBtn_text = "Return";
// 主界面对象
lv_obj_t* Scr_MainMenu;
uint8_t* MainMenu_Count = 0;
static lv_style_t btn_style;
lv_obj_t* GameList_butten;
lv_obj_t* GameList_label;
lv_obj_t* MusicList_butten;
lv_obj_t* MusicList_label;
// 游戏列表界面
lv_obj_t* Scr_GameList;
lv_obj_t* GLReturn_butten;
lv_obj_t* GLRB_label;

// 音乐列表界面
lv_obj_t* Scr_MusicList;
lv_obj_t* MLReturn_butten;
lv_obj_t* MLRB_label;

int Time0_channel0_ISR(void* ctx){
    lv_tick_inc(LVGL_TICK_TIME);
    return 0;
}

void lvgl_bsp_init(){
    lv_init();// LVGL内核初始化函数
    lv_port_disp_init();// LVGL显示接口初始化函数
    lv_port_indev_init();// LVGL输入接口初始化函数
    Timer_Init(LVGL_TICK_TIMERNUM,LVGL_TICK_CHANNEL,LVGL_TICK_TIME,0,1,
                Time0_channel0_ISR,NULL,5);// LVGL心跳定时器初始化
}

void SwitchScreen_Callback(lv_event_t* e){
    // 获取用户传递的参数
    lv_obj_t* obj = lv_event_get_user_data(e);
    if(obj==NULL){
        EMLOG(LOG_ERROR,"obj is NULL\n");
        return ;
    }
    if((void*)obj == (void*)Scr_MainMenu){// 主界面切换次数计算
        MainMenu_Count+=1;
    }
    // 加载主界面
    lv_screen_load(obj); 
    EMLOG(LOG_INFO,"%#x\n",obj);
}

void Create_MainMenu(void){
    // 创建主界面
    Scr_MainMenu = lv_obj_create(NULL);
    lv_obj_remove_style_all(Scr_MainMenu);
    lv_obj_set_pos(Scr_MainMenu, 0, 0);
    lv_obj_set_size(Scr_MainMenu, SCR_WIDTH, SCR_HEIGHT);
    lv_obj_set_style_bg_color(Scr_MainMenu,lv_color_black(),LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(Scr_MainMenu,255,LV_STATE_DEFAULT);// 设置背景透明度为255
    lv_scr_load(Scr_MainMenu);// 加载为主界面
    // 创建游戏列表按钮
    GameList_butten = lv_button_create(Scr_MainMenu);
    lv_obj_align(GameList_butten, LV_ALIGN_TOP_LEFT,0,0);
    // 在按钮上创建标签
    GameList_label = lv_label_create(GameList_butten);
    // 设置标签的文本为按钮文本
    lv_label_set_text(GameList_label,"Game");
    // 将标签对齐到按钮的中心
    lv_obj_set_align(GameList_label, LV_ALIGN_CENTER);

    // 创建音乐列表按钮
    MusicList_butten = lv_button_create(Scr_MainMenu);
    // 将按钮对齐到主界面的中心，并偏移
    lv_obj_align(MusicList_butten, LV_ALIGN_TOP_LEFT,+58,0);
    // 在按钮上创建标签
    MusicList_label = lv_label_create(MusicList_butten);
    // 设置标签的文本为按钮文本
    lv_label_set_text(MusicList_label,"Music");
    // 将标签对齐到按钮的中心
    lv_obj_set_align(MusicList_label, LV_ALIGN_CENTER);

    // 初始化按钮样式,并设置按钮样式
    lv_style_init(&btn_style);
    lv_style_set_bg_color(&btn_style,lv_color_hex(0x00FF00));
    lv_style_set_radius(&btn_style,2);
    lv_style_set_border_width(&btn_style,0);
    lv_style_set_width(&btn_style,48);lv_style_set_height(&btn_style,28);

    // 将按钮样式应用到按钮上
    lv_obj_add_style(GameList_butten,&btn_style,LV_STATE_DEFAULT);
    lv_obj_add_style(MusicList_butten,&btn_style,LV_STATE_DEFAULT);
}

void Create_GameList(void){
    Scr_GameList = lv_obj_create(NULL);
    lv_obj_remove_style_all(Scr_GameList);// 移除游戏列表界面的所有样式
    lv_obj_set_style_bg_opa(Scr_GameList,255,LV_STATE_DEFAULT);// 设置游戏列表界面的透明度为0
    lv_obj_set_style_bg_color(Scr_GameList,lv_color_hex(0x005555),LV_STATE_DEFAULT);
    lv_obj_set_size(Scr_GameList,SCR_WIDTH,SCR_HEIGHT);

    GLReturn_butten = lv_btn_create(Scr_GameList);
    lv_obj_set_size(GLReturn_butten,48,28);
    lv_obj_align(GLReturn_butten, LV_ALIGN_TOP_LEFT,0,0);

    GLRB_label = lv_label_create(GLReturn_butten);
    lv_label_set_text(GLRB_label,ReturnBtn_text);
    lv_obj_set_align(GLRB_label, LV_ALIGN_CENTER);
}

void Create_MusicList(void){
    Scr_MusicList = lv_obj_create(NULL);
    lv_obj_remove_style_all(Scr_MusicList);// 移除音乐列表界面的所有样式
    lv_obj_set_style_bg_opa(Scr_MusicList,255,LV_STATE_DEFAULT);// 设置音乐列表界面的透明度为0
    lv_obj_set_style_bg_color(Scr_MusicList,lv_color_hex(0x550055),LV_STATE_DEFAULT);
    lv_obj_set_size(Scr_MusicList,SCR_WIDTH,SCR_HEIGHT);

    MLReturn_butten = lv_btn_create(Scr_MusicList);
    lv_obj_set_size(MLReturn_butten,48,28);
    lv_obj_align(MLReturn_butten, LV_ALIGN_TOP_LEFT,0,0);

    MLRB_label = lv_label_create(MLReturn_butten);
    lv_label_set_text(MLRB_label,ReturnBtn_text);
    lv_obj_set_align(MLRB_label, LV_ALIGN_CENTER);
}

int GUI_Init(void){
    lvgl_bsp_init();
    // 动态获取屏幕大小
    SCR_HEIGHT = lv_obj_get_height(lv_scr_act());
    SCR_WIDTH = lv_obj_get_width(lv_scr_act());

    Create_MusicList();
    Create_GameList();
    Create_MainMenu();
    // 添加事件(由于回调函数需要传递地址参数，所以需要在地址参数确认后在调用添加)
    lv_obj_add_event_cb(GLReturn_butten,SwitchScreen_Callback, LV_EVENT_CLICKED, Scr_MainMenu);//设置切换位主界面的函数传递参数
    lv_obj_add_event_cb(MLReturn_butten,SwitchScreen_Callback, LV_EVENT_CLICKED, Scr_MainMenu);
    lv_obj_add_event_cb(GameList_butten,SwitchScreen_Callback, LV_EVENT_CLICKED, Scr_GameList);
    lv_obj_add_event_cb(MusicList_butten,SwitchScreen_Callback, LV_EVENT_CLICKED, Scr_MusicList);

    EMLOG(LOG_INFO,"ADD\nMainMenu:%#x\nGameList:%#x\nMusicList:%#x\n",Scr_MainMenu,Scr_GameList,Scr_MusicList);
    return 0;
}
