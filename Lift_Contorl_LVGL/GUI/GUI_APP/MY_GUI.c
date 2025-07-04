#include "GUI\GUI_APP\MY_GUI.h"
#include "LOG_SYSTEM\LOG.h"
#include "System_Bsp.h"
#include "GUI\LVGL\porting\lv_port_disp.h"
#include "GUI\LVGL\porting\lv_port_indev.h"
#include "GUI\LCD_Button\Button.h"

lv_coord_t SCR_WIDTH;
lv_coord_t SCR_HEIGHT;
/***************************************
 *              ȫ�ֶ�����              *
 ***************************************/
// ��������
char* ReturnBtn_text = "Return";
// ���������
lv_obj_t* Scr_MainMenu;
uint8_t* MainMenu_Count = 0;
static lv_style_t btn_style;
lv_obj_t* GameList_butten;
lv_obj_t* GameList_label;
lv_obj_t* MusicList_butten;
lv_obj_t* MusicList_label;
// ��Ϸ�б����
lv_obj_t* Scr_GameList;
lv_obj_t* GLReturn_butten;
lv_obj_t* GLRB_label;

// �����б����
lv_obj_t* Scr_MusicList;
lv_obj_t* MLReturn_butten;
lv_obj_t* MLRB_label;

int Time0_channel0_ISR(void* ctx){
    lv_tick_inc(LVGL_TICK_TIME);
    return 0;
}

void lvgl_bsp_init(){
    lv_init();// LVGL�ں˳�ʼ������
    lv_port_disp_init();// LVGL��ʾ�ӿڳ�ʼ������
    lv_port_indev_init();// LVGL����ӿڳ�ʼ������
    Timer_Init(LVGL_TICK_TIMERNUM,LVGL_TICK_CHANNEL,LVGL_TICK_TIME,0,1,
                Time0_channel0_ISR,NULL,5);// LVGL������ʱ����ʼ��
}

void SwitchScreen_Callback(lv_event_t* e){
    // ��ȡ�û����ݵĲ���
    lv_obj_t* obj = lv_event_get_user_data(e);
    if(obj==NULL){
        EMLOG(LOG_ERROR,"obj is NULL\n");
        return ;
    }
    if((void*)obj == (void*)Scr_MainMenu){// �������л���������
        MainMenu_Count+=1;
    }
    // ����������
    lv_screen_load(obj); 
    EMLOG(LOG_INFO,"%#x\n",obj);
}

void Create_MainMenu(void){
    // ����������
    Scr_MainMenu = lv_obj_create(NULL);
    lv_obj_remove_style_all(Scr_MainMenu);
    lv_obj_set_pos(Scr_MainMenu, 0, 0);
    lv_obj_set_size(Scr_MainMenu, SCR_WIDTH, SCR_HEIGHT);
    lv_obj_set_style_bg_color(Scr_MainMenu,lv_color_black(),LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(Scr_MainMenu,255,LV_STATE_DEFAULT);// ���ñ���͸����Ϊ255
    lv_scr_load(Scr_MainMenu);// ����Ϊ������
    // ������Ϸ�б�ť
    GameList_butten = lv_button_create(Scr_MainMenu);
    lv_obj_align(GameList_butten, LV_ALIGN_TOP_LEFT,0,0);
    // �ڰ�ť�ϴ�����ǩ
    GameList_label = lv_label_create(GameList_butten);
    // ���ñ�ǩ���ı�Ϊ��ť�ı�
    lv_label_set_text(GameList_label,"Game");
    // ����ǩ���뵽��ť������
    lv_obj_set_align(GameList_label, LV_ALIGN_CENTER);

    // ���������б�ť
    MusicList_butten = lv_button_create(Scr_MainMenu);
    // ����ť���뵽����������ģ���ƫ��
    lv_obj_align(MusicList_butten, LV_ALIGN_TOP_LEFT,+58,0);
    // �ڰ�ť�ϴ�����ǩ
    MusicList_label = lv_label_create(MusicList_butten);
    // ���ñ�ǩ���ı�Ϊ��ť�ı�
    lv_label_set_text(MusicList_label,"Music");
    // ����ǩ���뵽��ť������
    lv_obj_set_align(MusicList_label, LV_ALIGN_CENTER);

    // ��ʼ����ť��ʽ,�����ð�ť��ʽ
    lv_style_init(&btn_style);
    lv_style_set_bg_color(&btn_style,lv_color_hex(0x00FF00));
    lv_style_set_radius(&btn_style,2);
    lv_style_set_border_width(&btn_style,0);
    lv_style_set_width(&btn_style,48);lv_style_set_height(&btn_style,28);

    // ����ť��ʽӦ�õ���ť��
    lv_obj_add_style(GameList_butten,&btn_style,LV_STATE_DEFAULT);
    lv_obj_add_style(MusicList_butten,&btn_style,LV_STATE_DEFAULT);
}

void Create_GameList(void){
    Scr_GameList = lv_obj_create(NULL);
    lv_obj_remove_style_all(Scr_GameList);// �Ƴ���Ϸ�б�����������ʽ
    lv_obj_set_style_bg_opa(Scr_GameList,255,LV_STATE_DEFAULT);// ������Ϸ�б�����͸����Ϊ0
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
    lv_obj_remove_style_all(Scr_MusicList);// �Ƴ������б�����������ʽ
    lv_obj_set_style_bg_opa(Scr_MusicList,255,LV_STATE_DEFAULT);// ���������б�����͸����Ϊ0
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
    // ��̬��ȡ��Ļ��С
    SCR_HEIGHT = lv_obj_get_height(lv_scr_act());
    SCR_WIDTH = lv_obj_get_width(lv_scr_act());

    Create_MusicList();
    Create_GameList();
    Create_MainMenu();
    // ����¼�(���ڻص�������Ҫ���ݵ�ַ������������Ҫ�ڵ�ַ����ȷ�Ϻ��ڵ������)
    lv_obj_add_event_cb(GLReturn_butten,SwitchScreen_Callback, LV_EVENT_CLICKED, Scr_MainMenu);//�����л�λ������ĺ������ݲ���
    lv_obj_add_event_cb(MLReturn_butten,SwitchScreen_Callback, LV_EVENT_CLICKED, Scr_MainMenu);
    lv_obj_add_event_cb(GameList_butten,SwitchScreen_Callback, LV_EVENT_CLICKED, Scr_GameList);
    lv_obj_add_event_cb(MusicList_butten,SwitchScreen_Callback, LV_EVENT_CLICKED, Scr_MusicList);

    EMLOG(LOG_INFO,"ADD\nMainMenu:%#x\nGameList:%#x\nMusicList:%#x\n",Scr_MainMenu,Scr_GameList,Scr_MusicList);
    return 0;
}
