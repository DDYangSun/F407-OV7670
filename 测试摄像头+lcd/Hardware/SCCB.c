#include "stm32f4xx.h"
#include <stm32f4xx_conf.h>
#include "Delay.h"
#include "SCCB.h"

/*====================== SCCB 引脚定义（按实际接线修改这两个宏）======================
	SCCB 本质就是"只支持单主机的 I2C"，两根线：
		SCL —— 主机输出，推挽
		SDA —— 双向，开漏输出 + 上拉，主从都能拉低
	本工程实测接线：SCL=PB8，SDA=PB9（main.c 里的注释也是这么写的）。
	注意：
		PB1  已经被 LCD 的 DC 占用；
		PB13/PB15 是 LCD 的 SCL/SDA；
		PB12 是 LCD 的 CS，PB5 是 LCD 的 RST。
	 所以 SCCB 千万不要再挑上面这几个脚，原来的代码用 PB10+PB1 是错的，
	 PB1 会去抢 LCD 的 DC，SCCB 也读不回从机的应答。
===================================================================================*/
#define SCCB_GPIO_PORT		GPIOB
#define SCCB_SCL_PIN		GPIO_Pin_10
#define SCCB_SDA_PIN		GPIO_Pin_11

/*	OV7670 的 7 位地址是 0x21，写地址 0x42，读地址 0x43（本文件里直接写死）	*/

static void SCCB_W_SCL(uint8_t BitValue)
{
	GPIO_WriteBit(SCCB_GPIO_PORT, SCCB_SCL_PIN, (BitAction)BitValue);
	Delay_us(10);			//	SCCB 时钟约 50kHz，太快读不回来就把这里加大
}

static void SCCB_W_SDA(uint8_t BitValue)
{
	GPIO_WriteBit(SCCB_GPIO_PORT, SCCB_SDA_PIN, (BitAction)BitValue);
	Delay_us(10);
}

static uint8_t SCCB_R_SDA(void)
{
	uint8_t BitValue;

	BitValue = GPIO_ReadInputDataBit(SCCB_GPIO_PORT, SCCB_SDA_PIN);
	Delay_us(10);
	return BitValue;
}

void SCCB_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	/*	SCL：推挽输出	*/
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin   = SCCB_SCL_PIN;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(SCCB_GPIO_PORT, &GPIO_InitStructure);

	/*	SDA：开漏输出 + 上拉，既能输出，也能读到从机拉低的应答	*/
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_Pin   = SCCB_SDA_PIN;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(SCCB_GPIO_PORT, &GPIO_InitStructure);

	GPIO_SetBits(SCCB_GPIO_PORT, SCCB_SCL_PIN);
	GPIO_SetBits(SCCB_GPIO_PORT, SCCB_SDA_PIN);
}

/*	起始位：SCL 高电平期间，SDA 由高变低	*/
void SCCB_Start(void)
{
	SCCB_W_SDA(1);
	SCCB_W_SCL(1);
	SCCB_W_SDA(0);
	SCCB_W_SCL(0);
}

/*	停止位：SCL 高电平期间，SDA 由低变高	*/
void SCCB_Stop(void)
{
	SCCB_W_SDA(0);
	SCCB_W_SCL(1);
	SCCB_W_SDA(1);
}

void SCCB_SendByte(uint8_t Byte)
{
	uint8_t i;

	for (i = 0; i < 8; i++)
	{
		SCCB_W_SDA(!!(Byte & (0x80 >> i)));
		SCCB_W_SCL(1);			//	释放 SCL 后从机立刻取走数据
		SCCB_W_SCL(0);
	}
}

uint8_t SCCB_ReceiveByte(void)
{
	uint8_t i, Byte = 0x00;

	SCCB_W_SDA(1);				//	主机释放 SDA，从机把数据放到 SDA 上
	for (i = 0; i < 8; i++)
	{
		SCCB_W_SCL(1);
		if (SCCB_R_SDA())
		{
			Byte |= (0x80 >> i);
		}
		SCCB_W_SCL(0);
	}
	return Byte;
}

/*	第 9 个时钟：主机回 ACK(0) / NACK(1)	*/
void SCCB_SendAck(uint8_t AckBit)
{
	SCCB_W_SDA(AckBit);
	SCCB_W_SCL(1);
	SCCB_W_SCL(0);
}

/*	第 9 个时钟：读从机应答，0=NACK 1=ACK(OV7670 用 0 表示应答)	*/
uint8_t SCCB_ReceiveAck(void)
{
	uint8_t AckBit;

	SCCB_W_SDA(1);				//	主机释放 SDA，让从机来应答
	SCCB_W_SCL(1);
	AckBit = SCCB_R_SDA();
	SCCB_W_SCL(0);
	return AckBit;
}

void SCCB_WriteReg(uint8_t RegAddress, uint8_t Data)
{
	SCCB_Start();
	SCCB_SendByte(0x42);		//	0x21<<1 | 0 = 写
	SCCB_ReceiveAck();
	SCCB_SendByte(RegAddress);
	SCCB_ReceiveAck();
	SCCB_SendByte(Data);
	SCCB_ReceiveAck();
	SCCB_Stop();
}

uint8_t SCCB_ReadReg(uint8_t RegAddress)
{
	uint8_t Byte = 0x00;

	/*	第一阶段：写寄存器地址（dummy write）	*/
	SCCB_Start();
	SCCB_SendByte(0x42);
	SCCB_ReceiveAck();
	SCCB_SendByte(RegAddress);
	SCCB_ReceiveAck();
	SCCB_Stop();

	/*	第二阶段：读数据	*/
	SCCB_Start();
	SCCB_SendByte(0x43);		//	0x21<<1 | 1 = 读
	SCCB_ReceiveAck();
	Byte = SCCB_ReceiveByte();
	SCCB_SendAck(1);			//	NACK，告诉从机不再读了
	SCCB_Stop();

	return Byte;
}

/*	SCCB 连线自检：
	返回 0 = 正常（释放 SDA 时为高，主动拉低时为低）
	     1 = 释放 SDA 后仍然是低电平 —— 没有上拉电阻 / 线短路到地 / 摄像头模块没供电
	     2 = 主动拉低后仍然是高电平 —— 引脚接错 / 焊点没通 / 没和板子共地
	在上电读 ID 之前先跑一遍，可以把"是通信时序问题"还是"是硬件连线问题"分开。	*/
uint8_t SCCB_TestLine(void)
{
	uint8_t a, b;

	SCCB_W_SDA(1);
	Delay_us(50);
	a = SCCB_R_SDA();

	SCCB_W_SDA(0);
	Delay_us(50);
	b = SCCB_R_SDA();

	SCCB_W_SDA(1);
	Delay_us(50);

	if (a == 0)
	{
		return 1;
	}
	if (b == 1)
	{
		return 2;
	}
	return 0;
}
