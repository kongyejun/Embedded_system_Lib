#include "I2C_BASE.h"
#include "OLED_Font.h"

#define OLED_ADDR 0x78  //OLED I2C 地址

/*
    对于 SSD1306 OLED，I2C 通信协议中，在传输完从机地址后，还需要传输控制字节，以使 OLED 进入对应的状态。数据格式如下：

        |   b7    |   b6    |   b5    |   b4    |   b3    |   b2    |   b1    |   b0    |
        |---------|---------|---------|---------|---------|---------|---------|---------|
        |  Co     |  D/C#   |  保留   |  保留    |  保留   |  保留   |  保留    |  保留   |

    a. 如果 Co 位设置为逻辑“0”，则后续信息的传输将仅包含数据字节。
    - 此时，OLED 显示器将把接收到的字节视为图形数据，直接存储在其图形数据 RAM（GDDRAM）中。这通常用于更新显示内容，例如绘制图像或文本。
    b. D/C# 位决定下一个数据字节作为命令还是数据。
    - 如果 D/C# 位设置为逻辑“0”，则定义后续数据字节为命令：
        - 这些命令用于配置显示器的状态或操作，例如设置对比度、开启/关闭显示、清屏等。
    - 如果 D/C# 位设置为逻辑“1”，则定义后续数据字节为数据：
        - 这些数据将被写入 GDDRAM 中，并且每次写入数据后，GDDRAM 的列地址指针将自动增加一个，以便下一个数据字节可以写入正确的位置。
        - 这使得可以连续发送多个数据字节，从而形成完整的图形或字符。
    通过正确设置 Co 位和 D/C# 位，可以灵活地控制 OLED 显示器的操作和显示内容，从而实现丰富的视觉效果。
*/

/**
  * @brief  OLED写命令
  * @param  Command 要写入的命令
  * @retval 无
  */
void OLED_WriteCommand(uint8_t Command){
    I2C1_Write(&Command ,OLED_ADDR ,0x00 ,1);
}

/**
  * @brief  OLED写数据
  * @param  Data 要写入的数据
  * @retval 无
  */
void OLED_WriteData(uint8_t Data){
    I2C1_Write(&Data ,OLED_ADDR ,0x40 ,1);
}

/**
  * @brief  OLED设置光标位置
  * @param  Y 以左上角为原点，向下方向的坐标，范围：0~7
  * @param  X 以左上角为原点，向右方向的坐标，范围：0~127
  * @retval 无
  */
void OLED_SetCursor(uint8_t Y, uint8_t X){
    uint8_t Data[] = {
      0xB0 | Y,//设置页地址
      0x10 | ((X & 0xF0) >> 4),//列地址的命令的高四位
      0x00 | (X & 0x0F)};//设置列地址的命令的低四位
    I2C1_Write(Data ,OLED_ADDR ,0x00 ,3);
}

/**
  * @brief  OLED清屏
  * @param  无
  * @retval 无
  */
void OLED_Clear(void){
	uint8_t i, j;
	for (j = 0; j < 8; j++){
		OLED_SetCursor(j, 0);
		for(i = 0; i < 128; i++){
			OLED_WriteData(0x00);
		}
	}
}

/**
  * @brief  OLED清除一页数据
  * @param  Line 行位置，范围：1~8
  * @retval 无
  */
void OLED_Clear_8Line(uint8_t Line, uint8_t Line_Num){
  uint8_t Data = 0x00;
  while(Line_Num--){
    OLED_SetCursor((Line++)-1, 0);
    I2C1_Fill(&Data,OLED_ADDR,0x40,128);
  }
}

/**
  * @brief  OLED显示一个字符
  * @param  Line 行位置，范围：1~4
  * @param  Column 列位置，范围：1~16
  * @param  Char 要显示的一个字符，范围：ASCII可见字符
  * @retval 无
  */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char){      	
	uint8_t i;
	OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);		//设置光标位置在上半部分
	for (i = 0; i < 8; i++){
		OLED_WriteData(OLED_F8x16[Char - ' '][i]);			//显示上半部分内容
	}
	OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);	//设置光标位置在下半部分
	for (i = 0; i < 8; i++){
		OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);		//显示下半部分内容
	}
}

/**
  * @brief  OLED显示字符串
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  String 要显示的字符串，范围：ASCII可见字符
  * @retval 无
  */
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String){
	uint8_t i;
	for (i = 0; String[i] != '\0'; i++){
		OLED_ShowChar(Line, Column + i, String[i]);
	}
}

/**
  * @brief  OLED次方函数
  * @retval 返回值等于X的Y次方
  */
uint32_t OLED_Pow(uint32_t X, uint32_t Y){
	uint32_t Result = 1;
	while (Y--){
		Result *= X;
	}
	return Result;
}

/**
  * @brief  OLED显示数字（十进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~4294967295
  * @param  Length 要显示数字的长度，范围：1~10
  * @retval 无
  */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length){
	uint8_t i;
	for (i = 0; i < Length; i++){
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}

/**
  * @brief  OLED显示数字（十进制，带符号数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：-2147483648~2147483647
  * @param  Length 要显示数字的长度，范围：1~10
  * @retval 无
  */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length){
	uint8_t i;
	uint32_t Number1;
	if (Number >= 0){
		OLED_ShowChar(Line, Column, '+');
		Number1 = Number;
	}else{
		OLED_ShowChar(Line, Column, '-');
		Number1 = -Number;
	}
	for (i = 0; i < Length; i++){
		OLED_ShowChar(Line, Column + i + 1, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}

/**
  * @brief  OLED显示数字（十六进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~0xFFFFFFFF
  * @param  Length 要显示数字的长度，范围：1~8
  * @retval 无
  */
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length){
	uint8_t i, SingleNumber;
	for (i = 0; i < Length; i++){
		SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
		if (SingleNumber < 10){
			OLED_ShowChar(Line, Column + i, SingleNumber + '0');
		}else{
			OLED_ShowChar(Line, Column + i, SingleNumber - 10 + 'A');
		}
	}
}

/**
  * @brief  OLED显示数字（二进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~1111 1111 1111 1111
  * @param  Length 要显示数字的长度，范围：1~16
  * @retval 无
  */
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length){
	uint8_t i;
	for (i = 0; i < Length; i++){
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
	}
}

/**
  * @brief  OLED显示固定文字（二进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @retval 无
  */
void OLED_ShowNOWChar16X16(uint8_t Line, uint8_t Column){
  uint8_t i,j=0;
  while(j<8){
	  OLED_SetCursor((Line - 1) * 2, (Column - 1) * 16);		//设置光标位置在上半部分
    for (i = 0; i < 16; i++){
      OLED_WriteData(OLED_DTNOW16x16[j][i]);			//显示上半部分内容
    }
    OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 16);	//设置光标位置在下半部分
    for (i = 0; i < 16; i++){
      OLED_WriteData(OLED_DTNOW16x16[j+1][i]);		//显示下半部分内容
    }
    j+=2;Column+=1;
    if(Column>16){
      Line+=1;
      Column=1;
    }
  }
}
void OLED_ShowTargetChar16X16(uint8_t Line, uint8_t Column){
  uint8_t i,j=0;
  while(j<8){
	  OLED_SetCursor((Line - 1) * 2, (Column - 1) * 16);		//设置光标位置在上半部分
    for (i = 0; i < 16; i++){
      OLED_WriteData(OLED_DTNEXT16x16[j][i]);			//显示上半部分内容
    }
    OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 16);	//设置光标位置在下半部分
    for (i = 0; i < 16; i++){
      OLED_WriteData(OLED_DTNEXT16x16[j+1][i]);		//显示下半部分内容
    }
    j+=2;Column+=1;
    if(Column>16){
      Line+=1;
      Column=1;
    }
  }
}

/**
  * @brief  OLED初始化
  * @param  无
  * @retval 无
  */
void OLED_Init(void){
	uint32_t i, j;
	for (i = 0; i < 1000; i++){			//上电延时
		for (j = 0; j < 1000; j++);
	}
	I2C1_Init();			//端口初始化
  uint8_t Command[] = {
    0xAE, // 关闭显示（Display Off）
    0xD5, // 设置时钟分频因子（Set Display Clock Divide Ratio / Oscillator Frequency）
    0x80, // 时钟分频因子设置（设置为默认值）
    0xA8, // 设置多路复用比（Set Multiplex Ratio）
    0x3F, // 多路复用比设置为 64（0x3F 表示 64 行）
    0xD3, // 设置显示偏移（Set Display Offset）
    0x00, // 偏移设置为 0
    0x40, // 设置起始行（Set Display Start Line）为 0
    0xA1, // 设置段重定向（Set Segment Re-map）为正向
    0xC8, // 设置 COM 输出扫描方向（Set COM Output Scan Direction）为反向
    0xDA, // 设置 COM 引脚硬件配置（Set COM Pins Hardware Configuration）
    0x12, // 配置为 1/2 引脚模式
    0x81, // 设置对比度控制（Set Contrast Control）
    0xCF, // 对比度值设置为 207
    0xD9, // 设置预充电周期（Set Pre-charge Period）
    0xF1, // 预充电周期设置为 15
    0xDB, // 设置 VCOMH 电压倍率（Set VCOMH Deselect Level）
    0x30, // VCOMH 设置为 0.3 × VCC
    0xA4, // 使显示恢复到正常模式（Set Display to Normal Mode）
    0xA6, // 设置显示模式为正常（Set Display Mode to Normal）
    0x8D, // 设置充电泵（Charge Pump Setting）
    0x14, // 使能充电泵
    0xAF  // 打开显示（Display On）
  };
	I2C1_Write(Command,OLED_ADDR,0x00,23);	//发送初始化指令
	OLED_Clear();				//OLED清屏
}
