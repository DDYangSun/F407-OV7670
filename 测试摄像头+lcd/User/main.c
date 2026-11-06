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

/*	1 = 显示 8 位灰度图（能看清画面明暗、判断阈值切得合不合适）
	0 = 显示二值图（纯黑/纯白）
	两种都会先走一遍 ImageAnalyze()，阈值照常算并打印到串口。		*/
#define SHOW_GRAY_IMAGE		1

uint16_t Camera_FPS = 0;

extern uint16_t QQVGA_Frame[];		//储存原始画面0
extern uint16_t QQVGA_Frame1[];		//原始画面的位置1

uint8_t GrayImage[120 * 160] = {0};
uint16_t PixelValueNum[256] = {0};

uint16_t *Read_Frame = QQVGA_Frame;
volatile uint8_t LCD_ShowReady = 0;	//等第一帧到了再显示

uint8_t NowReadNum;			//现在读取哪一个RGB565的数组

int main(void)
{
	uint8_t threshold;

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

			/*	只做分析：灰度化 + 直方图 + 峰谷 + 大津，返回这一帧的阈值。
				注意 ImageAnalyze() 不会改 GrayImage 的像素值，
				所以这时缓冲区里还是完整的 8 位灰度图。				*/
			if (NowReadNum == 0)
				threshold = ImageAnalyze(QQVGA_Frame,GrayImage,120 * 160,PixelValueNum,256);
			else
				threshold = ImageAnalyze(QQVGA_Frame1,GrayImage,120 * 160,PixelValueNum,256);

#if SHOW_GRAY_IMAGE
			LCD_ShowOV7670GrayPIC(GrayImage);					//先显示灰度图
			ImageBinarize(GrayImage,120 * 160,threshold);		//需要二值结果时再就地二值化
#else
			ImageBinarize(GrayImage,120 * 160,threshold);		//先二值化
			LCD_ShowOV7670GrayPIC(GrayImage);					//再显示二值图
#endif
			Camera_FPS++;
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
			NowReadNum = 0;
		}
		else
		{
			Read_Frame = QQVGA_Frame1;		//	缓冲区1 可以读了
			NowReadNum = 1;
		}
		LCD_ShowReady = 1;
		DMA_ClearITPendingBit(DMA2_Stream1, DMA_IT_TCIF1);	//	清中断标志
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
