#include "i2c_ctl.h"
#include "i2c.h"
#include "stdio.h"
#include "stdlib.h"

static uint16_t _current_addr = 0x00;

/* 硬件初始化I2C，设置从机地址，数据位宽度，I2C通讯速率 */
void I2C_HardWare_Init(uint16_t addr)
{
    i2c_init(I2C_DEVICE_0,addr, ADDRESS_WIDTH, I2C_CLK_SPEED);
    _current_addr = addr;
}

/* 向寄存器reg写入一个数据data ，写入成功返回0，失败则返回非0*/
uint16_t I2C_Write_Byte(uint8_t addr, uint8_t reg, uint8_t data){
    if (_current_addr != addr){I2C_HardWare_Init(addr);}
    uint8_t cmd[2];
    cmd[0] = reg;cmd[1] = data;
    return i2c_send_data(I2C_DEVICE_0, cmd, 2);
}
/* 使用I2C传输一段数据，写入成功返回0，失败则返回非0*/
uint16_t I2C_Write(uint8_t addr, uint8_t reg, uint8_t* data,uint8_t Length){
    if (_current_addr != addr){I2C_HardWare_Init(addr);}
    uint8_t* cmd = malloc((Length+1)*sizeof(uint8_t));
    int i=1;
    cmd[0]=reg;
    while(i <= Length){
        cmd[i]=data[i-1];++i;
    }
    return i2c_send_data(I2C_DEVICE_0,cmd,Length+1);
}

/* 从寄存器reg读取length个数据保存到data_buf，读取成功返回0，失败则返回非0 */
uint16_t I2C_Read(uint8_t addr, uint8_t reg, uint8_t *data_buf, uint16_t length){
    if (_current_addr != addr){I2C_HardWare_Init(addr);}
    return i2c_recv_data(I2C_DEVICE_0, &reg, 1, data_buf, length);
}

/* 打印当前I2C总线上的I2C设备地址 */
void I2C_Read_ADDR(void){
    uint16_t error = 1;
    uint8_t cmd[2];cmd[0] = 0x00;cmd[1] = 0x01;
    for (int i = 0; i < 255; i++){
        i2c_init(I2C_DEVICE_0, i, ADDRESS_WIDTH, I2C_CLK_SPEED);
        error = i2c_send_data(I2C_DEVICE_0, cmd, 2);
        if (error == 0){printf("0x");printf("%x", i);}
        else{printf(".");}
    }
    printf(" \n");
}
