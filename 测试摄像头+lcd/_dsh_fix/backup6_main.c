#include "stm32f4xx.h"
#include "Delay.h"
#include "lcd.h"
#include "pic.h"
#include "ov7670.h"
#include "SCCB.h"
#include "Serial.h"
#include "Timer3.h"
#include "ImageBinaryzation.h"
#include "stm32f4xx_conf.h"

uint16_t Camera_FPS = 0;

extern uint16_t QQVGA_Frame[];		//锟斤拷锟斤拷原始锟斤拷锟斤拷0
extern uint16_t QQVGA_Frame1[];		//原始锟斤拷锟斤拷锟轿伙拷锟?

uint8_t GrayImage[120 * 160] = {0};
uint16_t PixelValueNum[256] = {0};

uint16_t *Read_Frame = QQVGA_Frame;
volatile uint8_t LCD_ShowReady = 0;	//锟饺碉拷一帧锟斤拷锟斤拷锟斤拷锟斤拷示

uint8_t NowReadNum;			//锟斤拷锟节讹拷取锟斤拷一锟斤拷RGB565锟斤拷锟斤拷锟斤拷

int main(void)
{								
	Delay_Init((uint8_t)(SystemCoreClock / 1000000));
	uart1_init(115200U);
	Timer_init();

	LCD_Proc();
	OV7670_Proc();

	while(1)
	{
		if (LCD_ShowReady)
		{
			LCD_ShowReady = 0;

			if (NowReadNum == 0)
				ImageProcess(QQVGA_Frame,GrayImage,120 * 160,PixelValueNum,256);
			else
				ImageProcess(QQVGA_Frame1,GrayImage,120 * 160,PixelValueNum,256);

			LCD_ShowOV7670GrayPIC(GrayImage);
			Camera_FPS++;
		}
	}
}


void DMA2_Stream1_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA2_Stream1, DMA_IT_TCIF1) == SET)	//	DMA 锟斤拷锟斤拷锟斤拷锟?
	{
		if (DMA_GetCurrentMemoryTarget(DMA2_Stream1) == SET)	//	CT=1锟斤拷锟斤拷锟斤拷锟斤拷写锟斤拷锟斤拷锟斤拷1锟斤拷说锟斤拷锟斤拷锟斤拷锟斤拷0锟斤拷锟斤拷
		{
			Read_Frame = QQVGA_Frame;		//	锟斤拷锟斤拷锟斤拷0 锟斤拷锟皆讹拷锟斤拷
			NowReadNum = 0;
		}
		else
		{
			Read_Frame = QQVGA_Frame1;		//	锟斤拷锟斤拷锟斤拷1 锟斤拷锟皆讹拷锟斤拷
			NowReadNum = 1;
		}
		LCD_ShowReady = 1;
		DMA_ClearITPendingBit(DMA2_Stream1, DMA_IT_TCIF1);	//	锟斤拷锟叫断憋拷志
	}
}

void TIM3_IRQHandler(void)
{
	static uint16_t TIM3_Count = 0;
	
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
	{
		if (++TIM3_Count >= 1000)
		{
			printf("FPS=%d  Top=%d(%d)  Top2=%d(%d)  Valley=%d  Otsu=%d\r\n",Camera_FPS,TopPosition,TopNumber,Top2Position,Top2Number,ValleyPos,OtsuPos);
			Camera_FPS = 0;
			TIM3_Count = 0;
		}
	}
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
}
