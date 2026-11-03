#include "stm32f4xx.h"
#include "Delay.h"
#include "lcd.h"
#include "pic.h"
#include "ov7670.h"
#include "SCCB.h"
#include "TIMER3.h"
#include "stm32f4xx_conf.h"

uint16_t Camera_FPS = 0;

extern uint16_t QQVGA_Frame[];		//储存原始画面0
extern uint16_t QQVGA_Frame1[];		//原始画面的位置1

uint16_t *Read_Frame = QQVGA_Frame;
volatile uint8_t LCD_ShowReady = 0;	//	等第一帧到了再显示

int main(void)
{
	Delay_Init((uint8_t)(SystemCoreClock / 1000000));	//	168MHz -> 168		延时函数初始化

	LCD_Init();				//点亮LCD
	LCD_Fill(0, 0, LCD_W, LCD_H, WHITE);

	OV7670_DCMI_Init();				//配置 DCMI 用到的 GPIO

	//给时钟 + 硬件复位
	OV7670_MCLK_start();		//	PA8 输出 8MHz XCLK
	Delay_ms(10);
	OV7670_Rest();				//	PA1=PWDN(低=工作)，PA3=RST
	Delay_ms(20);				//	复位后等摄像头内部初始化

	SCCB_Init();				//	SCL=PB10，SDA=PB11
	Delay_ms(10);
	OV7670_RegConfig();

	OV7670_Window_Set(160, 120, 160, 120);			//设置采集窗口

	//DMA + DCMI 开始采集
	OV7670DMA_DCMItoSRAM();		//	先把 DMA2_Stream1 配好：DCMI -> QQVGA_Frame
	OV7670_GetPicture();		//	再同时打开 DMA 和 DCMI

	Timer_init();
	while(1)
	{
		if (LCD_ShowReady)
		{
			LCD_ShowReady = 0;
			LCD_ShowOV7670PIC(Read_Frame);
		}
	}
}


void DMA2_Stream1_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA2_Stream1, DMA_IT_TCIF1) == SET)	//	DMA 传输完成
	{
		if (DMA_GetCurrentMemoryTarget(DMA2_Stream1) == SET)	//	CT=1：接下来写缓冲区1，说明缓冲区0已满
		{
			Read_Frame = QQVGA_Frame;		//	缓冲区0 可以读了
		}
		else
		{
			Read_Frame = QQVGA_Frame1;		//	缓冲区1 可以读了
		}
		LCD_ShowReady = 1;
		Camera_FPS++;
		DMA_ClearITPendingBit(DMA2_Stream1, DMA_IT_TCIF1);	//	清中断标志
	}
}


void TIM3_IRQHandler(void)			//1ms中断
{
	static int16_t TIM3_Count = 0;
	
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
	{
		if (++TIM3_Count >= 1000)
		{
			LCD_ShowIntNum(0,0,Camera_FPS,3,RED,WHITE,24);
			TIM3_Count = 0;
			Camera_FPS = 0;
		}
		
	}
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
}

