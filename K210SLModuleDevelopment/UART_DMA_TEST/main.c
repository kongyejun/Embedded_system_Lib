#include "IO_SET.h"
#include "uart.h"
#include "dmac.h"
#include "gpio.h"
#include "sleep.h"

#define BAUD_RATE 115200                    //波特率
#define STOPBIT UART_STOP_1                 //一位停止位
#define DATA_WIDTH UART_BITWIDTH_8BIT       //8为数据长度
#define PARITY UART_PARITY_NONE             //不使用奇偶校验

uint8_t send_buff[25];
uint8_t receive_buff[25];

/*
@name 软硬件映射函数
@param
@return
@des 将硬件IO口复用为对应功能的IO口
*/
void Hardware_Init(void){
    fpioa_set_function(UART_USB_RX_PIN,FUNC_UART_USB_RX);
    fpioa_set_function(UART_USB_TX_PIN,FUNC_UART_USB_TX);
    
    fpioa_set_function(RGB_R_PIN , FUNC_RGB_R);
    fpioa_set_function(RGB_G_PIN , FUNC_RGB_G);
    fpioa_set_function(RGB_B_PIN , FUNC_RGB_B);
}

void OFF_RGB(void){
    gpio_set_pin(RGB_R_GPIONUM,GPIO_PV_HIGH);
    gpio_set_pin(RGB_G_GPIONUM,GPIO_PV_HIGH);
    gpio_set_pin(RGB_B_GPIONUM,GPIO_PV_HIGH);
}

void RGB_Init(void){
    gpio_set_drive_mode(RGB_R_GPIONUM,GPIO_DM_OUTPUT);
    gpio_set_drive_mode(RGB_G_GPIONUM,GPIO_DM_OUTPUT);
    gpio_set_drive_mode(RGB_B_GPIONUM,GPIO_DM_OUTPUT);
    OFF_RGB();
}

int Strcmp (const uint8_t* src,const char* dst){
    //与kendryte-toolchain\riscv64-unknown-elf\include\string.h中的strcmp撞名,故改为大写
    for(;!((*src)-(*dst));++src,++dst){
        if((*src)=='\0')return 1;
    }
    return 0;
}

// int strcmp (const char * src,const char * dst){
//     int ret = 0 ;
//     while((ret = *(unsigned char *)src - *(unsigned char *)dst) == 0 && *dst && *src){
//         ++src, ++dst;
//     }
//     return ((-ret) < 0) - (ret < 0); 
//     //C语言库string.h中的strcmp函数如果两个字符串相同返回0,不相同返回1;注意!!!!!!!!!!!!!!!!!!
// }

// char Process_UARTData(char *UART_BUFF){
//     OFF_RGB();
//     if(Strcmp(UART_BUFF,"ON_RLIGHT")){
//         gpio_set_pin(RGB_R_GPIONUM,GPIO_PV_LOW);
//         return 1;
//     }else if(Strcmp(UART_BUFF,"ON_GLIGHT")){
//         gpio_set_pin(RGB_G_GPIONUM,GPIO_PV_LOW);
//         return 2;
//     }else if(Strcmp(UART_BUFF,"ON_BLIGHT")){
//         gpio_set_pin(RGB_B_GPIONUM,GPIO_PV_LOW);
//         return 3;
//     }else{
//         return 0;
//     }
// }

// int main(){
//     Hardware_Init();
//     RGB_Init();
//     uart_init(UART_DEVICENUM);//初始化uart
//     uart_config(UART_DEVICENUM,BAUD_RATE,DATA_WIDTH,STOPBIT,PARITY);//UART配置
//     char UART_BUFF[15]={0};//创建串口缓冲区
//     int8_t state=0;

//     uart_send_data(UART_DEVICENUM,"Init is complete!\n",18);//初始化完成发送信息
//     while(1){
//         //uart_receive_data()返回接收到数据的长度;未接收到则返回0
//         while(uart_receive_data(UART_DEVICENUM,(char *)UART_BUFF,15)){
//         //知识回顾:一个地址拥有8个bit(1字节),strlen是采用当前字符串'\0'的地址与起始地址相减得到字符串长度
//             switch(Process_UARTData(UART_BUFF)){
//                 case 0:uart_send_data(UART_DEVICENUM,"Invalid data!\n",14);break;//注意这里的长度是决定发送字符的长度
//                 //uart_send_data()内部实现中会自动使CPU等待旧的数据发送完毕才像UART的发送寄存器覆盖新的值
//                 case 1:uart_send_data(UART_DEVICENUM,"OPEN RED!\n",10);break;
//                 case 2:uart_send_data(UART_DEVICENUM,"OPEN GREEN!\n",12);break;
//                 case 3:uart_send_data(UART_DEVICENUM,"OPEN BLUE!\n",11);break;
//             }
//             state = 0;
//         }
//         if(!state){
//             uart_send_data(UART_DEVICENUM,"Wait you send data!\n",20);
//             state = 1;
//         }
//     }
//     return 0;
// }

// int strlen(char* src){
//     int cont=0;
//     if(src==NULL)return -1;
//     while(*src){++src;++cont;}
//     return cont;
// }
volatile uint8_t state=4;//编译器容易优化掉变量,所以需要加上volatile关键字
//表明每次使用它的时候必须从 i的地址中读取,常用在中断程序中改变外部变量!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
extern volatile uart_t* const  uart[3];
int Process_RData(void* ctx){
    //uart_receive_data_dma_irq(UART_DEVICENUM,DMAC_CHANNEL1,receive_buff,4,Process_RData,&state,1);
    //DMA快速重启通道函数，在DMA中断函数中使用，可以除去一些非必要的配置过程
    dmac_set_src_dest_length(DMAC_CHANNEL1,(void *)(&uart[UART_DEVICENUM]->RBR),receive_buff,4);
    if(Strcmp(receive_buff,"ON_R")){
        state='1';
    }else if(Strcmp(receive_buff,"ON_G")){
        state='2';
    }else if(Strcmp(receive_buff,"ON_B")){
        state='3';
    }else{
        state='0';
    }
    return 0;
}

int main(){
    Hardware_Init();
    gpio_init();uart_init(UART_DEVICENUM);
    RGB_Init();
    uart_config(UART_DEVICENUM,BAUD_RATE,DATA_WIDTH,STOPBIT,PARITY);//UART配置
    //该函数内部实现会调用dmac_wait_done(dmac_channel);等待DMA空闲时,CPU才会继续执行,即等待DMA配置好任务
    uart_send_data_dma(UART_DEVICENUM,DMAC_CHANNEL0,"Init is conplete!\n",19);//通过DMA发送数据
    //如果使用uart_receive_data_dma()CPU会等待DMA空闲时继续执行,即防止在DMA搬运途中读取未完全覆盖的数据
    /*
        不建议使用SDK中uart_receive_data_dma(),
        因为在内部实现中会使CPU等待DMA空闲才会释放CPU,
        并不能体现出,在DMA搬运期间CPU可以执行其他功能的特点
    */
    uart_receive_data_dma_irq(UART_DEVICENUM,DMAC_CHANNEL1,receive_buff,4,Process_RData,&state,1);
    //字符长度,在未设置DMA FIFO深度时,会需要当缓冲栈达到满栈时才会触发中断
    //惨痛的教训,优先级从1开始,好像没有0
    while(1){
        switch(state){
            case '0':uart_send_data_dma(UART_DEVICENUM,DMAC_CHANNEL0,"ERROR Command",14);state = 0;break;
            case '1':
            OFF_RGB();
            gpio_set_pin(RGB_R_GPIONUM,GPIO_PV_LOW);
            uart_send_data_dma(UART_DEVICENUM,DMAC_CHANNEL0,"Opened Red!",12);
            state = 0;break;
            case '2':
            OFF_RGB();
            gpio_set_pin(RGB_G_GPIONUM,GPIO_PV_LOW);
            uart_send_data_dma(UART_DEVICENUM,DMAC_CHANNEL0,"Opened Green!",14);
            state = 0;break;
            case '3':
            OFF_RGB();
            gpio_set_pin(RGB_B_GPIONUM,GPIO_PV_LOW);
            uart_send_data_dma(UART_DEVICENUM,DMAC_CHANNEL0,"Opened Blue!",13);
            state = 0;break;
            default:break;
        }
        msleep(100);
    }
}