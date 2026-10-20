#include "stm32f1xx_hal.h"     
#include "OLED.h"
#include "OLED_Font.h"

/**
 * @brief 宏定义封装GPIO口的电平变化函数 
 * 
 */
#define OLED_W_SCL(x) HAL_GPIO_WritePin(GPIOB,GPIO_PIN_8,(x) ? GPIO_PIN_SET : GPIO_PIN_RESET)
#define OLED_W_SDA(x) HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9,(x) ? GPIO_PIN_SET : GPIO_PIN_RESET)

/**
 * @brief 用软件模拟IIC通信的时候的引脚的初始化，
 * 
 */
void OLED_I2C_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure = {0};
	
	__HAL_RCC_GPIOB_CLK_ENABLE();
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	
	GPIO_InitStructure.Pin = GPIO_PIN_8;
	HAL_GPIO_Init(GPIOB,&GPIO_InitStructure);
	GPIO_InitStructure.Pin = GPIO_PIN_9;
	HAL_GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	OLED_W_SCL(1);
	OLED_W_SDA(1);
	
}

/**
 * @brief IIC的开始信号
 * 
 */
void OLED_Start(void)
{
	OLED_W_SCL(1);
	OLED_W_SDA(1);
	OLED_W_SDA(0);
	OLED_W_SCL(0);
}

/**
 * @brief IIC的结束信号 
 * 
 */
void OLED_End(void)
{
	OLED_W_SDA(0);
	OLED_W_SCL(1);
	OLED_W_SDA(1);
}

/**
 * @brief 用IIC发送一个字符
 * 
 */
void OLED_Sendbyte(uint8_t Byte)
{
	uint8_t i;
	for(i = 0;i<8;i++)
	{
		OLED_W_SDA(!!(Byte & (0x80 >> i)));
		OLED_W_SCL(1);
		OLED_W_SCL(0);
	}
	OLED_W_SCL(1);
	OLED_W_SCL(0);

}

/**
 * @brief 用IIC给OLED发送命令
 * 
 */
void OLED_WriteCommand(uint8_t Command)
{
	OLED_Start();
	OLED_Sendbyte(0x78);
	OLED_Sendbyte(0x00);
	OLED_Sendbyte(Command);
	OLED_End();
}

/**
 * @brief 用IIC给OLED发送一个数据
 * 
 */
void OLED_WriteData(uint8_t Data)
{
	OLED_Start();
	OLED_Sendbyte(0x78);
	OLED_Sendbyte(0x40);
	OLED_Sendbyte(Data);
	OLED_End();
}

/**
 * @brief 指定OLED的写入数据的位置
 * 
 */
void OLED_SetCursor(uint8_t Y, uint8_t X)
{
    OLED_WriteCommand(0xB0 | Y);
    OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));
    OLED_WriteCommand(0x00 | (X & 0x0F));
}

/**
 * @brief 将OLED上的东西清空
 * 
 */
void OLED_Clear(void)
{
    uint8_t i, j;
	// 遍历行，这个每次写都是竖着写进去的128x64
    for (j = 0; j < 8; j++)
    {
        OLED_SetCursor(j, 0);
        for (i = 0; i < 128; i++)
        {
            OLED_WriteData(0x00);
        }
    }
}

/**
 * @brief OLED在指定位置显示字符
 * 
 */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{
    uint8_t i;
    OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);
    for (i = 0; i < 8; i++)
    {
        OLED_WriteData(OLED_F8x16[Char - ' '][i]);
    }
    OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);
    for (i = 0; i < 8; i++)
    {
        OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);
    }
}

/**
 * @brief OLED在指定位置显示字符串
 * 
 */
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
    uint8_t i;
    for (i = 0; String[i] != '\0'; i++)
    {
        OLED_ShowChar(Line, Column + i, String[i]);
    }
}

/**
 * @brief  求X的Y次方的函数
 * 
 */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;
    while (Y--)
    {
        Result *= X;
    }
    return Result;
}

/**
 * @brief OLED在指定位置显示数字(没有正负号)
 * 
 */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
    }
}

/**
 * @brief OLED在指定位置显示数字，有正负号
 * 
 */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
    uint8_t i;
    uint32_t Number1;
    if (Number >= 0)
    {
        OLED_ShowChar(Line, Column, '+');
        Number1 = Number;
    }
    else
    {
        OLED_ShowChar(Line, Column, '-');
        Number1 = -Number;
    }
    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(Line, Column + i + 1, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
    }
}

/**
 * @brief OLED在指定位置显示原始数字(16位)
 * 
 */
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i, SingleNumber;
    for (i = 0; i < Length; i++)
    {
        SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
        if (SingleNumber < 10)
        {
            OLED_ShowChar(Line, Column + i, SingleNumber + '0');
        }
        else
        {
            OLED_ShowChar(Line, Column + i, SingleNumber - 10 + 'A');
        }
    }
}

/**
 * @brief OLED在指定位置显示浮点数
 * 
 */
void OLED_ShowFloatNum(uint8_t Line,uint8_t Col,float Number,uint8_t len)
{
	if(Number >= 0)
	{
		uint32_t Re = (uint32_t)Number;
		uint32_t Im = Number*1000 - (uint32_t)Number * 1000;
		OLED_ShowChar(Line,Col,'+');
		OLED_ShowNum(Line,Col+1,Re,len);
		OLED_ShowChar(Line,Col+len+1,'.');
		OLED_ShowNum(Line,Col+len+2,Im,3);
	}
	else{
		Number = -Number;
		uint32_t Re = (uint32_t)Number;
		uint32_t Im = Number*1000 - (uint32_t)Number * 1000;
		OLED_ShowChar(Line,Col,'-');
		OLED_ShowNum(Line,Col+1,Re,len);
		OLED_ShowChar(Line,Col+len+1,'.');
		OLED_ShowNum(Line,Col+len+2,Im,3);
	}
}

/**
 * @brief OLED在指定位置显示二进制数字
 * 
 */
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
    }
}

/**
 * @brief OLED的初始化指令
 * 
 */
void OLED_Init(void)
{
    uint32_t i, j;

    for (i = 0; i < 1000; i++)			//上电延时
    {
        for (j = 0; j < 1000; j++);
    }

    OLED_I2C_Init();			//端口初始化

    OLED_WriteCommand(0xAE);	//关闭显示

    OLED_WriteCommand(0xD5);	//设置显示时钟分频比/振荡器频率
    OLED_WriteCommand(0x80);

    OLED_WriteCommand(0xA8);	//设置多路复用率
    OLED_WriteCommand(0x3F);

    OLED_WriteCommand(0xD3);	//设置显示偏移
    OLED_WriteCommand(0x00);

    OLED_WriteCommand(0x40);	//设置显示开始行

    OLED_WriteCommand(0xA1);	//设置左右方向
    OLED_WriteCommand(0xC8);	//设置上下方向

    OLED_WriteCommand(0xDA);	//设置COM引脚硬件配置
    OLED_WriteCommand(0x12);

    OLED_WriteCommand(0x81);	//设置对比度控制
    OLED_WriteCommand(0xCF);

    OLED_WriteCommand(0xD9);	//设置预充电周期
    OLED_WriteCommand(0xF1);

    OLED_WriteCommand(0xDB);	//设置VCOMH取消选择级别
    OLED_WriteCommand(0x30);

    OLED_WriteCommand(0xA4);	//设置整个显示打开/关闭
    OLED_WriteCommand(0xA6);	//设置正常/倒转显示

    OLED_WriteCommand(0x8D);	//设置充电泵
    OLED_WriteCommand(0x14);

    OLED_WriteCommand(0xAF);	//开启显示

    OLED_Clear();				//OLED清屏
}


