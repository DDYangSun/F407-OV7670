#include "stm32f4xx.h"
#include "Delay.h"
#include "lcd.h"
#include "pic.h"
#include "ov7670.h"
#include "SCCB.h"
#include "stm32f4xx_conf.h"

extern uint16_t QQVGA_Frame[];		//储存原始画面0
extern uint16_t QQVGA_Frame1[];   //原始画面的位置1

uint16_t *Read_Frame = QQVGA_Frame;
volatile uint8_t LCD_ShowReady = 1;
 
int main(void)
{ 	
//		LCD_Init();
//		LCD_Fill(0,0,LCD_W,LCD_H,WHITE);
//		LCD_ShowChinese(0,0,"你好",BLACK,WHITE,32,0);
//		LCD_ShowPicture(0,34,100,100,gImage_pic);//显示图片
	
		OV7670_DCMI_Init();
		OV7670_MCLK_start();   // PA8输出8MHz XCLK给OV7670
		Delay_ms(10);          // 等待时钟稳定
		OV7670_Rest();         // 硬件复位
		SCCB_Init();           // PB8(SCL)+PB9(SDA) 
		Delay_ms(10);
		OV7670_RegConfig();    // 写入QQVGA RGB565默认寄存器表	
	
		LCD_Init();
		LCD_Fill(0,0,LCD_W,LCD_H,WHITE);
	
		OV7670_Window_Set(160, 120, 160, 120);
		OV7670DMA_DCMItoSRAM();    // DMA2_Stream1: DCMI→QQVGA_Frame		
		OV7670_GetPicture();       // 使能DMA + DCMI捕获

		
		
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
    if(DMA_GetITStatus(DMA2_Stream1, DMA_IT_TCIF1) == SET)//查询Dma通道一中的数据是否传输完成
    {
			
			if(DMA_GetCurrentMemoryTarget(DMA2_Stream1) == SET)//检查CT标志位，假如置1，说明接下来写地址1，地址0写满
			{
				Read_Frame = QQVGA_Frame; //地址0可以被读取
			}
			else
			{
				Read_Frame = QQVGA_Frame1;//地址1可以被读取
			}
			LCD_ShowReady = 1;
			
			DMA_ClearITPendingBit(DMA2_Stream1, DMA_IT_TCIF1);  // 清除中断标志
    }
}

