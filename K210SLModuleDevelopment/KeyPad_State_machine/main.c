#include "LOG_SYSTEM/LOG.h"
#include "PIN_Config.h"
#include "uarths.h"
#include "Key_SM.h"

/**
 * @brief  硬件初始化
 * @param  无
 * @retval None
 */
//软硬件映射函数
void Hardware_Init(void){
    fpioa_set_function(PIN_KEYM, FUNC_KEYM);
    fpioa_set_function(PIN_KEYL, FUNC_KEYL);
    fpioa_set_function(PIN_KEYR, FUNC_KEYR);
}


int main(){
    Hardware_Init();
    uarths_init();
    Key_SM_Init();
    EMLOG(LOG_INFO,"Init Success\n");
    while(1){
        switch(KeyFIFO_read()){
            // 向左按键事件
            case KEY_LEFT_DOWN: EMLOG(LOG_INFO,"KEY_LEFT_DOWN\n");break;
            case KEY_LEFT_UP: EMLOG(LOG_INFO,"KEY_LEFT_UP\n");break;
            case KEY_LEFT_LONG: EMLOG(LOG_INFO,"KEY_LEFT_LONG\n");break;
            case KEY_LEFT_BURSTS: EMLOG(LOG_INFO,"KEY_LEFT_BURSTS\n");break;
            case KEY_LEFT_BATTER_1:EMLOG(LOG_INFO,"KEY_LEFT_BATTER_1\n");break;
            case KEY_LEFT_BATTER_2:EMLOG(LOG_INFO,"KEY_LEFT_BATTER_2\n");break;
            case KEY_LEFT_BATTER_3:EMLOG(LOG_INFO,"KEY_LEFT_BATTER_3\n");break;
            // 向右按键事件
            case KEY_RIGHT_DOWN:EMLOG(LOG_INFO,"KEY_RIGHT_DOWN\n");break;
            case KEY_RIGHT_UP:EMLOG(LOG_INFO,"KEY_RIGHT_UP\n");break;
            case KEY_RIGHT_LONG:EMLOG(LOG_INFO,"KEY_RIGHT_LONG\n");break;
            case KEY_RIGHT_BURSTS:EMLOG(LOG_INFO,"KEY_RIGHT_BURSTS\n");break;
            case KEY_RIGHT_BATTER_1:EMLOG(LOG_INFO,"KEY_RIGHT_BATTER_1\n");break;
            case KEY_RIGHT_BATTER_2:EMLOG(LOG_INFO,"KEY_RIGHT_BATTER_2\n");break;
            case KEY_RIGHT_BATTER_3:EMLOG(LOG_INFO,"KEY_RIGHT_BATTER_3\n");break;
            // 中间按键事件
            case KEY_MIDDLE_DOWN:EMLOG(LOG_INFO,"KEY_MIDDLE_DOWN\n");break;
            case KEY_MIDDLE_UP:EMLOG(LOG_INFO,"KEY_MIDDLE_UP\n");break;
            case KEY_MIDDLE_LONG:EMLOG(LOG_INFO,"KEY_MIDDLE_LONG\n");break;
            case KEY_MIDDLE_BURSTS:EMLOG(LOG_INFO,"KEY_MIDDLE_BURSTS\n");break;
            case KEY_MIDDLE_BATTER_1:EMLOG(LOG_INFO,"KEY_MIDDLE_BATTER_1\n");break;
            case KEY_MIDDLE_BATTER_2:EMLOG(LOG_INFO,"KEY_MIDDLE_BATTER_2\n");break;
            case KEY_MIDDLE_BATTER_3:EMLOG(LOG_INFO,"KEY_MIDDLE_BATTER_3\n");break;
            default:break;
        }
    }
    return 0;
}