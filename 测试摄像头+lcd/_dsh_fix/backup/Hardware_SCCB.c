#include "stm32f4xx.h"                  // Device header
#include <stm32f4xx_conf.h>
#include "Delay.h"
#define OV7670_ADDRESS 0x21 


/* 
解释：void MyI2C_W_SCL(uint8_t BitValue)这个函数可以指定一个值，并且这个指定的值 
GPIO_WriteBit(GPIOB, GPIO_Pin_10, (BitAction)BitValue)会被填入这行代码中BitValue中，
含义为，0为低电平，1为高电平
*/
void SCCB_W_SCL(uint8_t BitValue)//配置SCL 的电平状态
{
	GPIO_WriteBit(GPIOB, GPIO_Pin_10, (BitAction)BitValue);
	Delay_us(10);
}

void SCCB_W_SDA(uint8_t BitValue)
{
	GPIO_WriteBit(GPIOB, GPIO_Pin_1, (BitAction)BitValue);
	Delay_us(10);
}

uint8_t SCCB_R_SDA(void)//读取SDA
{
	uint8_t BitValue;
	BitValue = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1);
	Delay_us(10);
	return BitValue;//返回读到SDA线的电平
}

void SCCB_Init(void)
{
	RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOB ,ENABLE );

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT ;
	GPIO_InitStructure.GPIO_OType =GPIO_OType_PP ;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 ;//SCL
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init (GPIOB ,&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;//SDA
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB ,&GPIO_InitStructure);

	GPIO_SetBits (GPIOB ,GPIO_Pin_10 );
	GPIO_SetBits (GPIOB ,GPIO_Pin_1 );
}

void SCCB_Start(void)//起始函数，
{
	SCCB_W_SDA (1);
	SCCB_W_SCL (1);
	SCCB_W_SDA (0);
	SCCB_W_SCL (0);
}

void SCCB_Stop(void)//结束函数
{
	SCCB_W_SDA (0);
	SCCB_W_SCL (1);
	SCCB_W_SDA (1);
}

void SCCB_SendByte(uint8_t Byte)
{
	uint8_t i;
	for (i = 0; i < 8; i ++)
	{
		//(0x80 >> i)的作用是每次右移i位，>>是右移运算符，0x80的二进制为10000000
		SCCB_W_SDA(!!(Byte & (0x80 >> i)));
		SCCB_W_SCL(1);//释放SCL后，从机会立刻将数据取走
		SCCB_W_SCL(0);
	}
}

uint8_t SCCB_ReceiveByte(void)
{
	uint8_t i, Byte = 0x00;//定义一个变量来储存数据，初始值全为0
	SCCB_W_SDA(1);//由之前的代码，初始时SCL为低电平，当主机释放了SDA，此时可以把数据放到SDA，这里就是释放SDA
	//这时候从机就要把数据放到SDA上
	for (i = 0; i < 8; i ++)
	{
		SCCB_W_SCL(1);
		if (SCCB_R_SDA()){Byte |= (0x80 >> i);}
		//Byte |= 按位或赋值操作，读到是一，将Byte赋值为1
		SCCB_W_SCL(0);
	}
	return Byte;
}

void SCCB_SendAck(uint8_t AckBit)//发送应答
{
//在sendbyte函数的最后，SCL为低电平，可以将AckBit放到SDA上
	SCCB_W_SDA(AckBit);
	SCCB_W_SCL(1);//读取应答
	SCCB_W_SCL(0);
}

uint8_t SCCB_ReceiveAck(void)//接受应答，0表示接收到
{
	uint8_t AckBit;
	SCCB_W_SDA(1);//这一步，主机释放SDA的同时，从机就把数据放在SDA上
	SCCB_W_SCL(1);
	AckBit = SCCB_R_SDA();
	SCCB_W_SCL(0);
	return AckBit;
}

void SCCB_WriteReg(uint8_t Reg ,uint8_t Data)
{
	SCCB_Start();
	SCCB_SendByte(0x42);
	SCCB_ReceiveAck();
	SCCB_SendByte(Reg);
	SCCB_ReceiveAck();
	SCCB_SendByte(Data);
	SCCB_ReceiveAck();
	SCCB_Stop();
}

uint8_t SCCB_ReadReg(uint8_t Reg)
{
	uint8_t Byte = 0x00;
  SCCB_Start();
  SCCB_SendByte(0x42);
	SCCB_ReceiveAck();
  SCCB_SendByte(Reg);
  SCCB_ReceiveAck();
  SCCB_Stop();
	
  SCCB_Start();
  SCCB_SendByte(0x43);
	SCCB_ReceiveAck();
  Byte = SCCB_ReceiveByte();
  SCCB_SendAck (1);  // 发送NACK
  SCCB_Stop();
	
  return Byte;
}

