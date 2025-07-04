#include "stm32f10x.h"
#define I2C1_GPIO_SCL_Pin  GPIO_Pin_6
#define I2C1_GPIO_SDA_Pin  GPIO_Pin_7

void I2C1_GPIO_Config(void){
	GPIO_InitTypeDef GPIO_InitStruct;
	//初始化 IIC_GPIO 时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	//初始化IIC_SCL
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStruct.GPIO_Pin = I2C1_GPIO_SCL_Pin|I2C1_GPIO_SDA_Pin;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
}

void I2C1_Init(void){
	I2C1_GPIO_Config();//配置SDA和SCL GPIO
	//初始化IIC外设时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,ENABLE);
    I2C_InitTypeDef I2C_InitStructure;
	I2C_InitStructure.I2C_Ack                 = I2C_Ack_Enable;                //接收一个字节后是否给从机应答
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;  //stm32从机可以响应几位的地址(stm32作为从机才会用到)
	I2C_InitStructure.I2C_ClockSpeed          = 400000;                        // IIC时钟速率
	I2C_InitStructure.I2C_DutyCycle           = I2C_DutyCycle_2;
    //占空比：这里指的是SCL低电平与高电平的比，因为SCL是弱上拉，所以在快速状态下，SDA波形翻转需要一些时间（低电平需要更多的时间）
	I2C_InitStructure.I2C_Mode                = I2C_Mode_I2C;
	I2C_InitStructure.I2C_OwnAddress1         = 0x00;      //自身地址1(stm32作为从机才会用到)，指定stm32的自身地址，方便别的主机呼叫。
	I2C_Init(I2C1,&I2C_InitStructure);
    //使能IIC
	I2C_Cmd(I2C1, ENABLE);
}

void I2C1_Write(uint8_t *pBuffer,uint8_t SlaveADDR ,uint8_t WriteAddr ,uint16_t Len){
	//确保总线处于空闲状态,确保当前没有其他传输正在进行,以避免发送开始信号时发生冲突。
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
	//发送Start信号
	I2C_GenerateSTART(I2C1,ENABLE);
	//等待EV5事件：IIC开始信号已经发出 （I2C_SR1内SB位置1）
	while(I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_MODE_SELECT)==ERROR);
	//发送7位“从机地址”
	I2C_Send7bitAddress(I2C1,SlaveADDR,I2C_Direction_Transmitter);
	//等待EV6事件：表示地址已经发送
	while(I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)==ERROR);
	//写入从机内将要写入的地址数据
	I2C_SendData(I2C1,WriteAddr);
    //等待EV8事件：返回SET则数据寄存器DR为空	
    while(I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_TRANSMITTED)==ERROR);
	//连续发送指定长度数据
	while(Len--){
        //写入数据
        I2C_SendData(I2C1,*pBuffer);
        //指针后移
        pBuffer++;
        //等待EV8事件：返回SET则数据寄存器DR为空	
        while(I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_TRANSMITTED)==ERROR);
	}
	//一个字节发送完成，发送Stop信号
    I2C_GenerateSTOP(I2C1, ENABLE);
}

void I2C1_Fill(uint8_t *pBuffer,uint8_t SlaveADDR ,uint8_t WriteAddr ,uint8_t Len){
	//确保总线处于空闲状态,确保当前没有其他传输正在进行,以避免发送开始信号时发生冲突。
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
	//发送Start信号
	I2C_GenerateSTART(I2C1,ENABLE);
	//等待EV5事件：IIC开始信号已经发出 （I2C_SR1内SB位置1）
	while(I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_MODE_SELECT)==ERROR);
	//发送7位“从机地址”
	I2C_Send7bitAddress(I2C1,SlaveADDR,I2C_Direction_Transmitter);
	//等待EV6事件：表示地址已经发送
	while(I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)==ERROR);
	//写入从机内将要写入的地址数据
	I2C_SendData(I2C1,WriteAddr);
    //等待EV8事件：返回SET则数据寄存器DR为空	
    while(I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_TRANSMITTED)==ERROR);
	//连续发送指定长度数据
	while(Len--){
        //写入数据
        I2C_SendData(I2C1,*pBuffer);
        //等待EV8事件：返回SET则数据寄存器DR为空	
        while(I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_TRANSMITTED)==ERROR);
	}
	//一个字节发送完成，发送Stop信号
    I2C_GenerateSTOP(I2C1, ENABLE);
}

void I2C1_Read(uint8_t *pBuffer,uint8_t SlaveADDR ,uint8_t ReadAddr,uint8_t Len){
                        /***************设置从机发送地址******************/
	//发送Start信号
	I2C_GenerateSTART(I2C1,ENABLE);
	//等待EV5事件：IIC开始信号已经发出 （I2C_SR1内SB位置1）
	while(I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_MODE_SELECT)==ERROR);
	//发送7位“从机地址”
	I2C_Send7bitAddress(I2C1,SlaveADDR,I2C_Direction_Transmitter);
	//等待EV6事件：表示地址已经发送
	while(I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)==ERROR);
	//写入从机内存“单元地址”
	I2C_SendData(I2C1,ReadAddr);
	//等待EV8事件：数据寄存器DR为空	,地址数据已经发送
	while(I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_TRANSMITTED)==ERROR);
	                    /***************设置主机接收******************/
    //重新发送Start信号
	I2C_GenerateSTART(I2C1,ENABLE);
	//等待EV5事件
	while(I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_MODE_SELECT)==ERROR);
	//发送7位“从机地址”
	I2C_Send7bitAddress(I2C1,SlaveADDR,I2C_Direction_Receiver);//7位从机地址以及1位接收位
	//等待EV6事件（接收）：表示地址已经发送
	while(I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)==ERROR);//注意方向
	// 读取指定量的数据
	while((--Len)!=0){
			//等待EV7事件， BUSY, MSL and RXNE flags
			while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED)==ERROR);
			*pBuffer = I2C_ReceiveData(I2C1);// 读取数据
			++pBuffer;// 指针后移
	}
                    /***************接收最后一个字节,使能NACK,并准备发送stop******************/
	//禁用应答功能，通常用于在接收最后一个字节时，以便主设备不会发送应答，从而通知从设备停止发送数据。
	I2C_AcknowledgeConfig(I2C1, DISABLE);
	//发送Stop信号（注意：停止信号的发送将会在当前传输完成后进行）
	I2C_GenerateSTOP(I2C1, ENABLE);
	//等待EV7事件， BUSY, MSL and RXNE flags
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED)==ERROR);
	*pBuffer = I2C_ReceiveData(I2C1);// 读取数据
	//重新初始化 为下次做准备
	I2C_AcknowledgeConfig(I2C1, ENABLE);
}
