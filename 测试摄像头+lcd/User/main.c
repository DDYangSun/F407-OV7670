#include "stm32f4xx.h"
#include "Delay.h"
#include "lcd.h"
#include "pic.h"
#include "ov7670.h"
#include "SCCB.h"
#include "stm32f4xx_conf.h"

extern uint16_t QQVGA_Frame[];		//储存原始画面0
extern uint16_t QQVGA_Frame1[];		//原始画面的位置1

uint16_t *Read_Frame = QQVGA_Frame;
volatile uint8_t LCD_ShowReady = 0;	//	等第一帧到了再显示

int main(void)
{								
	Delay_Init((uint8_t)(SystemCoreClock / 1000000));

	/*--------------------------- 第1步：先把 LCD 点亮 ---------------------------*/
	LCD_Init();
	LCD_Fill(0, 0, LCD_W, LCD_H, WHITE);

	/*--------------------------- 第2步：配置 DCMI 用到的 GPIO ------------------*/
	OV7670_DCMI_Init();

	/*--------------------------- 第3步：给时钟 + 硬件复位 ----------------------*/
	OV7670_MCLK_start();		//	PA8 输出 8MHz XCLK
	Delay_ms(10);
	OV7670_Rest();				//	PA1=PWDN(低=工作)，PA3=RST
	Delay_ms(20);

	/*--------------------------- 第4步：SCCB 自检 ------------------------------*/
	SCCB_Init();				//	SCL=PB10，SDA=PB11
	Delay_ms(10);


	/*--------------------------- 第5步：写寄存器表（RGB565 + QVGA）-------------*/
	OV7670_RegConfig();

	/*--------------------------- 第6步：设置采集窗口 ---------------------------*/
	OV7670_Window_Set(160, 120, 160, 120);

	/*--------------------------- 第7步：DMA + DCMI 开始采集 -------------------*/
	OV7670DMA_DCMItoSRAM();
	OV7670_GetPicture();

	LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);

	while(1)
	{
		/*	1. 有新帧就刷屏	*/
		if (LCD_ShowReady)
		{
			LCD_ShowReady = 0;
			LCD_ShowOV7670PIC(Read_Frame);
		}

//		/*	2. 统计 DCMI 上溢(OVR)：一旦上溢，DCMI 硬件会自动停掉 CAPTURE，
//			   表现就是"画面定格、再也不刷新"。这里把标志清掉并计数	*/
//		if (DCMI_GetFlagStatus(DCMI_FLAG_OVFRI) != RESET)
//		{
//			DCMI_ClearFlag(DCMI_FLAG_OVFRI);
//			ovr_cnt++;
//		}

		/*	3. 兜底：连续约 500ms 没有新帧，就重新武装 DCMI + DMA，
			   不管是上溢、DMA 传输错误还是别的原因，都能自己爬起来	*/
//		if (frame_cnt != last_frame)
//		{
//			last_frame = frame_cnt;
//			idle = 0;
//		}
//		else
//		{
//			Delay_ms(10);
//			if (++idle >= 50)
//			{
//				idle = 0;
//				rearm_cnt++;

//				OV7670_StopGetPicture();
//				OV7670DMA_DCMItoSRAM();
//				Read_Frame = QQVGA_Frame;
//				LCD_ShowReady = 0;
//				OV7670_GetPicture();
//			}
//		}
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

		DMA_ClearITPendingBit(DMA2_Stream1, DMA_IT_TCIF1);	//	清中断标志
	}
}
