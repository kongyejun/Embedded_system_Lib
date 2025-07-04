#include <TM1637Display.h>
#include "OLED.h"
// 红外传感器数字输入IO端口定义
// 数码管引脚配置
#define CLK 6
#define DIO 7
// 双向站点相关传感器IO定义
#define IRSensor1 2   // 1号(内部)
#define IRSensor2 3   // 2号(外部)
// 单向站点相关传感器IO定义
#define IRSensor3 4   // 3号(进)
#define IRSensor4 5   // 4号(出)
// LED灯IO端口定义
#define LED 13  // LED指示灯IO口定义
// 日志函数宏定义
#define EMLOG(Str) Serial.print(Str); 
// 断言检测函数
#define assert(x) do{\
                      if(x==0){\
                        digitalWrite(LED,HIGH);\
                        while(1);\
                      }\
                    }while(0)
// 系统核心参数
int Position_Num = 100;// 剩余空位(数值信号量)
uint8_t cur_value;// 当前时刻信号状态
uint8_t per_value = 0x00;// 上一时刻信号状态
uint8_t temp_flag = 0;// 辅助标志组
/*******************************************
*             双向站点检测相关函数
*   这种系统一般设立在可以双向行驶的关卡,传感器之间距离比较近
**********************************************/
// 1号双向系统检测状态枚举
typedef enum {
  None,
  Is_IN,
  IN,
  Is_OUt,
  OUT,
}DEtECT_SYSTEM_STATE;
DEtECT_SYSTEM_STATE State;// 传感器检测系统状态
// 1号站点出入检测函数
void BilateralSite_Detect(){
  switch(State){
    case None:{
      if(cur_value == 0x01 ){// 内部检测到信号
        State = Is_OUt;
        EMLOG("内部检测到信号\n");
      }else if(cur_value == 0x02){// 外部检测到信号
        State = Is_IN;
        EMLOG("外部检测到信号\n");
      }
    }
    break;
    case Is_IN:{
      if(!(temp_flag & 0x01)){// 如果小车还没碰到内部检测传感器
        if(cur_value == 0x03){// 内部检测传感器也检测到
          temp_flag |= 0x01;// 设立确保有效标志位
          EMLOG("确认是否进入检测\n");
        }else if(cur_value == 0x00||cur_value == 0x01){// 0x01表示内部有车想要外出,外部车在避让出现的数值
          State = None;// 表示小车取消进入计划
          EMLOG("取消进入\n");
        }
      }else{
        if(cur_value == 0x01){// 外部先没有检测到信号,表示小车的确是要进入
          State = IN;
          temp_flag ^= 0x01;// 清除标志位 
          EMLOG("进入检测成功\n");
        }else if(cur_value == 0x02){// 内部先没有检测到信号,表示小车又不想出去了
          temp_flag ^= 0x01;// 清除标志位
          EMLOG("不确定进入\n");
        }
      }
    }
    break;
    case Is_OUt:{
      if(!(temp_flag & 0x01)){// 如果小车还没碰到外部检测传感器
        if(cur_value == 0x03){// 外部检测传感器也检测到
          temp_flag |= 0x01;// 设立确保有效标志位
          EMLOG("确认是否出去检测\n");
        }else if(cur_value == 0x00||cur_value == 0x02){// 0x02表示外部有车想要进入,内部车在避让出现的数值
          State = None;// 表示小车取消外出计划
          EMLOG("取消外出\n");
        }
      }else{
        if(cur_value == 0x02){// 内部先没有检测到信号,表示小车的确是要外出
          State = OUT;
          temp_flag ^= 0x01;// 清除标志位 
          EMLOG("外出检测成功\n");
        }else if(cur_value == 0x01){// 外部先没有检测到信号,表示小车又不想外出了
          temp_flag ^= 0x01;// 清除标志位
          EMLOG("不确定出去\n");
        }
      }
    }
    break;
    case IN:{
      Position_Num -= 1;
      EMLOG("进入一辆车,剩余空位:");
      EMLOG(Position_Num);
      EMLOG("\n");
      State = None;
    }
    break;
    case OUT:{
      Position_Num += 1;
      EMLOG("出去一辆车,剩余空位:");
      EMLOG(Position_Num);
      EMLOG("\n");
      State = None;
    }
    break;
  }
  per_value = cur_value;// 更新信号
}

/*****************************************
*           单向站点检测相关函数
*   这种系统一般设立在路口只能单向同行的地方
********************************************/
// 进入站点检测
void InSite_Detect(){
  if(temp_flag & 0x10){
    if((cur_value & 0x10) == 0x00){
        Position_Num -= 1;
        EMLOG("进入一辆车,剩余空位:");
        EMLOG(Position_Num);
        EMLOG("\n");
        temp_flag ^= 0x10;// 清除等待进入动作完成标志位
      }
  }else if(cur_value & 0x10){// 单向进站点检测
    temp_flag |= 0x10;// 设置等待进入动作完成标志位
    delay(20);
  }
}
// 外出站点检测
void OUTSite_Detect(){
  if(temp_flag & 0x20){
    if((cur_value & 0x20) == 0x00){
        Position_Num += 1;
        temp_flag ^= 0x20;// 清除等待出去动作完成标志位
        EMLOG("出去一辆车,剩余空位:");
        EMLOG(Position_Num);
        EMLOG("\n");
      }
  }else if(cur_value & 0x20){// 单向出站点检测
    temp_flag |= 0x20;// 设置等待出去动作完成标志位
    delay(20);
  }
  
}
// 单向站点组检测主函数
void UnilateralSite_Detect(){
  OUTSite_Detect();
  InSite_Detect();
}
/************************************
*         传感器检测定时器
**************************************/
// 定时器1的中断函数25ms
ISR(TIMER1_COMPA_vect) {
  cur_value  = ((!digitalRead(IRSensor2)) << 1 | (!digitalRead(IRSensor1)));// 获取检测信号(因为红外传感器默认高电平)
  cur_value |= ((!digitalRead(IRSensor4)) << 1 | (!digitalRead(IRSensor3))) << 4;// 获取检测信号
  BilateralSite_Detect();
  UnilateralSite_Detect();
  // EMLOG(cur_value);
  // EMLOG("\n");
}
// 1号16位定时器初始化
void Init_Timer1(){
  // 配置Timer1（TCNT1是16位寄存器)
  noInterrupts();           // 暂时禁用中断
  TCCR1A = 0;               // 设置Timer1为普通模式
  TCCR1B = 0;
  TCNT1 = 0;                // 计数器初始值
  
  // 设置比较匹配值 (16MHz / 256预分频 = 62500Hz，计数1560次为25ms)
  OCR1A = 1560;
  // 配置为CTC模式，选择256预分频
  TCCR1B |= (1 << WGM12);  // CTC模式 (WGM13:0 = 0100)
  TCCR1B |= (1 << CS12);   // 256预分频 (CS12:0 = 100)
  // 启用比较匹配中断
  TIMSK1 |= (1 << OCIE1A); // 使能OCR1A匹配中断
  interrupts();             // 重新启用中断
}
/************************************
*         数字转字符串函数
**************************************/
char* NumberToStr(int num){
  static char str[20] = {0};
  sprintf(str, "%d", num);
  return str;
}
/************************************
*         各模块初始化函数
**************************************/
// 初始化红外传感器
void Init_IRSensor() {
  // 电源端口
  pinMode(8,OUTPUT);
  pinMode(9,OUTPUT);
  digitalWrite(8,HIGH);// 输出5V电压供电
  digitalWrite(9,HIGH);// 输出5V电压供电
  // 信号端口
  pinMode(IRSensor1, INPUT_PULLUP);  // 数字输入模式
  pinMode(IRSensor2, INPUT_PULLUP);  // 数字输入模式
  pinMode(IRSensor3, INPUT_PULLUP);  // 数字输入模式
  pinMode(IRSensor4, INPUT_PULLUP);  // 数字输入模式
  // 端口定时检测器初始化
  Init_Timer1();
  // 状态机初始化
  State = None;
}
// 串口初始化函数
void Init_UART() {
  Serial.begin(9600);  // 串口波特率设置
  EMLOG("串口初始化完成\n");  
}
// 系统状态指示灯初始化
void Init_System_LED(){
  pinMode(LED, OUTPUT);  // LED端口初始化
  digitalWrite(LED,LOW); // 默认不亮
}
// OLED初始化
void Init_OLED(){
 OLED_Init();
}
// 数码管初始化
TM1637Display display(CLK, DIO);
void Init_NumShowDevice(){
  // 电源端口初始化
  display.setBrightness(0x07);
  display.showNumberDec(8888);
}
// 系统初始化函数
void setup() {
  temp_flag = 0x00;
  Init_UART();// 串口初始化
  Init_IRSensor();// 传感器初始化
  EMLOG("传感器初始化完成\n");
  Init_System_LED();// 指示灯初始化
  EMLOG("指示灯初始化完成\n");
  // Init_OLED();// 初始化OELD
  // EMLOG("OELD初始化完成\n");
  Init_NumShowDevice();// 初始化数码管
  EMLOG("数码管驱动初始化完成\n");
  delay(1000); // 暂停1秒
  EMLOG("系统初始化完毕\n");
}

/************************************
*         主循环函数
**************************************/
void loop() {
  display.showNumberDec(Position_Num);
  // delay(1000); // 暂停1秒,隔1S刷新一次空闲位显示
}



