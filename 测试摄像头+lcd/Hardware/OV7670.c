#include "stm32f4xx.h"                  // Device header
#include <stm32f4xx_conf.h>
#include <stm32f4xx_dcmi.h>
#include "SCCB.h"
#include "Delay.h"

uint16_t QQVGA_Frame[120 * 160]={0};//??一??缓冲??
uint16_t QQVGA_Frame1[120 * 160] = {0};//??二个缓冲区，用于存放DCMI接收到的数据，???置为QQVGA的大??

void OV7670_DCMI_Init(void)
{
	RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOA ,ENABLE );//控制信号引脚
	RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOC ,ENABLE );//DCMI数据
	RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOE ,ENABLE );//DCMI数据
	RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOD ,ENABLE );//DCMI数据
	RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOB ,ENABLE );//DCMI数据
	RCC_AHB2PeriphClockCmd (RCC_AHB2Periph_DCMI ,ENABLE );//开启DCMI时钟
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	//MCLK
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_MCO); // AF0: MCO
	/*==================对图像进行灰度转??====================*/
	//PWDN
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT ;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//RST
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT ;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP ;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz ;
	GPIO_Init (GPIOA ,&GPIO_InitStructure);
	
	//VSYNC
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_DCMI ); 
	
	//HSYNC
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_DCMI); 
	
	//PIXCLK
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_DCMI ); 
	
	//D0,D1
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7 ;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_DCMI ); 
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_DCMI ); 
	
	//D2,D3,D4
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1 |GPIO_Pin_4  ;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource0, GPIO_AF_DCMI ); 
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource1, GPIO_AF_DCMI );
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource4, GPIO_AF_DCMI );
	
	//D5
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_DCMI ); 

	//D6,D7
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5 | GPIO_Pin_6  ;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource5, GPIO_AF_DCMI ); 
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource6, GPIO_AF_DCMI );

	
	DCMI_InitTypeDef DCMI_InitStructure;
	DCMI_InitStructure.DCMI_CaptureMode = DCMI_CaptureMode_Continuous ;//捕获模式
	DCMI_InitStructure.DCMI_CaptureRate = DCMI_CaptureRate_All_Frame ;//
	DCMI_InitStructure.DCMI_ExtendedDataMode = DCMI_ExtendedDataMode_8b;
	DCMI_InitStructure.DCMI_HSPolarity = DCMI_HSPolarity_Low ;
	DCMI_InitStructure.DCMI_VSPolarity = DCMI_VSPolarity_High ;
	DCMI_InitStructure.DCMI_PCKPolarity = DCMI_PCKPolarity_Rising ;//OV7670在PCLK下降沿更新数??，上升沿采样
	DCMI_InitStructure.DCMI_SynchroMode =DCMI_SynchroMode_Hardware ;
	DCMI_Init (&DCMI_InitStructure);
	
		/*--------------------------------------------------------------------------
		注意：这里绝对不能打开 DCMI 的中????
		一?? DCMI_ITConfig(DCMI_IT_FRAME / DCMI_IT_OVF) ??使能，摄像头一开??
		输出数据，DCMI 就会进中??；而工程里没有实现 DCMI_IRQHandler，CPU ??
		直接跳到 startup_stm32f40_41xxx.s ?? weak 的默认???理函数 "B ."??
		整个程序?? LCD_Init() 之前就???在那里，屏幕永远停在上电未初???化的样子??
		??工程真???需要的???? DMA2_Stream1 的传输完成中??(TC)，在
		OV7670DMA_DCMItoSRAM() 里已经单??开了??
	--------------------------------------------------------------------------*/
	/*	DCMI_Cmd(ENABLE) 也挪?? OV7670_GetPicture() 里，
		保证先把 DMA 准???好，再?? DCMI 开始采数据，避免上??(OVR)??*/


}

/*==================OV7670寄存器配??============================*/
static const uint8_t OV7670_DefaultRegConfig[][2]= 
{   
	{0x3a, 0x0C},//
	{0x40, 0xd0},//
	{0x12, 0x14},//COM7，???位或者RGB+QVGA格式配置
	
	{0x32, 0x80},//
	{0x17, 0x17},
	{0x18, 0x05},
	{0x19, 0x02},
	{0x1a, 0x7b},
	{0x03, 0x0a},
	
	{0x0c, 0x1C},//
	{0x3e, 0x00},//
	{0x70, 0x00},//测试1
	{0x71, 0x01},//测试2
	{0x72, 0x11},//
	{0x73, 0x09},
	
	{0xa2, 0x02},
	{0x11, 0x00},//clkin_div
	{0x7a, 0x20}, 
	{0x7b, 0x1c}, 
	{0x7c, 0x28},
	
	{0x7d, 0x3c}, 
	{0x7e, 0x55}, 
	{0x7f, 0x68}, 
	{0x80, 0x76}, 
	{0x81, 0x80},
	
	{0x82, 0x88}, 
	{0x83, 0x8f}, 
	{0x84, 0x96}, 
	{0x85, 0xa3}, 
	{0x86, 0xaf},
	
	{0x87, 0xc4}, 
	{0x88, 0xd7}, 
	{0x89, 0xe8}, 
	{0x13, 0xe0},//
	{0x00, 0x00},//AGC
	
	{0x10, 0x00},
	{0x0d, 0x11},//
	{0x14, 0x38},//
	{0xa5, 0x05},//
	{0xab, 0x07},//
	
	{0x24, 0x75},//
	{0x25, 0x63},//
	{0x26, 0xA5},//
	{0x9f, 0x78},//
	{0xa0, 0x68},//
	
	{0xa1, 0x03},//
	{0xa6, 0xdf},//
	{0xa7, 0xdf},//
	{0xa8, 0xf0},//
	{0xa9, 0x90},//
	
	{0xaa, 0x94},//
	{0x13, 0xe7},//??动白平衡
	{0x0e, 0x61},//
	{0x0f, 0x4b},//
	{0x16, 0x02},//
	
	{0x1e, 0x30},//MVFP，镜像翻??
	{0x21, 0x02},//
	{0x22, 0x91},//
	{0x29, 0x07},//
	{0x33, 0x0b},//
	
	{0x35, 0x0b},//
	{0x37, 0x1d},//
	{0x38, 0x71},//
	{0x39, 0x2a},//
	{0x3c, 0x78},//
	
	{0x4d, 0x40},//
	{0x4e, 0x20},//
	{0x69, 0x00},//固定增益控制g
	{0x6b, 0x4A},//PLL*4=32Mhz
	{0x74, 0x19},//
	{0x8d, 0x4f},//
	
	{0x8e, 0x00},//
	{0x8f, 0x00},//
	{0x90, 0x00},//
	{0x91, 0x00},//
	{0x92, 0x00},//
	
	{0x96, 0x00},//
	{0x9a, 0x80},//
	{0xb0, 0x84},//
	{0xb1, 0x0c},//
	{0xb2, 0x0e},//
	
	{0xb3, 0x82},//
	{0xb8, 0x0a},//
	{0x43, 0x14},//
	{0x44, 0xf0},//
	{0x45, 0x34},//
	
	{0x46, 0x58},//
	{0x47, 0x28},//
	{0x48, 0x3a},//
	{0x59, 0x88},//
	{0x5a, 0x88},//
	
	{0x5b, 0x44},//
	{0x5c, 0x67},//
	{0x5d, 0x49},//
	{0x5e, 0x0e},//
	{0x64, 0x00},//G通道补偿g
	{0x65, 0x20},//
	
	{0x66, 0x05},//
	{0x94, 0x04},//
	{0x95, 0x08},//
	{0x6c, 0x0a},//
	{0x6d, 0x55},//
	
	{0x4f, 0x80},//g
	{0x50, 0x80},//g
	{0x51, 0x00},//g
	{0x52, 0x22},//g
	{0x53, 0x5e},//g
	{0x54, 0x80},//g
	
	{0x6e, 0x11},//
	{0x6f, 0x9f},//
	
	{0x6a, 0x00},//G通道增益g
	{0x01, 0x80},//B通道增益g
	{0x02, 0x80},//R通道增益g
	{0x13, 0xe7},//
	{0x15, 0x00},//
 
	{0x55, 0x00},//??度控制g
	{0x56, 0x40},//对比度控制g
	{0x58, 0x9e},//色彩矩阵g
	{0x41, 0x38},//
	{0x3f, 0x05},//边缘增强
	{0x75, 0x05},//
	{0x76, 0xe1},//
	{0x4c, 0x0F},//??声抑??
	{0x77, 0x0a},//
	{0x3d, 0xc2},//
	{0x4b, 0x09},//
	{0xc9, 0xc8},//饱和度控??
	{0x41, 0x38},//
	{0x34, 0x11},//
	{0x3b, 0x02},//夜晚控制 
	{0xa4, 0x89},//
	{0x96, 0x00},//
	{0x97, 0x30},//
	{0x98, 0x20},//
	{0x99, 0x30},//
	{0x9a, 0x84},//
	{0x9b, 0x29},//
	{0x9c, 0x03},//
	{0x9d, 0x4c},//
	{0x9e, 0x3f},//
	{0x78, 0x04},//
	{0x79, 0x01},//
	{0xc8, 0xf0},//
	{0x79, 0x0f},//
	{0xc8, 0x00},//
	{0x79, 0x10},//
	{0xc8, 0x7e},//
	{0x79, 0x0a},//
	{0xc8, 0x80},//
	{0x79, 0x0b},//
	{0xc8, 0x01},//
	{0x79, 0x0c},//
	{0xc8, 0x0f},//
	{0x79, 0x0d},//
	{0xc8, 0x20},//
	{0x79, 0x09},//
	{0xc8, 0x80},//
	{0x79, 0x02},//
	{0xc8, 0xc0},//
	{0x79, 0x03},//
	{0xc8, 0x40},//
	{0x79, 0x05},//
	{0xc8, 0x30},//
	{0x79, 0x26},//
	{0x09, 0x00},//
	{0x3b, 0x42},//night mode 
	{0x2d, 0x01}//
}; 

void OV7670_RegConfig(void)
{
	uint32_t i = 0;
	for (i = 0 ; i< (sizeof(OV7670_DefaultRegConfig ) / sizeof( OV7670_DefaultRegConfig[0]) ); i++)
	{
		SCCB_WriteReg (OV7670_DefaultRegConfig[i][0] , OV7670_DefaultRegConfig[i][1]);
	}

}

/*===========================================================================*/


void OV7670_Rest(void)//上电复位
{
	GPIO_ResetBits (GPIOA ,GPIO_Pin_1);//拉低PWDM
	Delay_ms (100);
	GPIO_ResetBits (GPIOA ,GPIO_Pin_3);//拉低RST
	Delay_ms (100);
	GPIO_SetBits (GPIOA ,GPIO_Pin_3 );//拉高RST
	
}

/*===============开??开OV7670的时钟的输出=========*/
void OV7670_MCLK_start(void)
{
	// 注意: F407的HSI??16MHz(不是4MHz)，原??Div_1实际输出16MHz
	// XCLK过快且配合寄存器表里0x6b PLLx4会???致PCLK过快，DCMI收不到稳定数??
	// Div_2 = 8MHz，OV7670??8~16MHz都能工作??8MHz最稳妥
	RCC_MCO1Config(RCC_MCO1Source_HSI, RCC_MCO1Div_2 );//8Mhz
}

/*===============摄像头窗口???置======================*/
void OV7670_Window_Set(uint16_t sx,uint16_t sy,uint16_t width,uint16_t height)
{
	uint16_t endx;
	uint16_t endy;
	uint8_t temp; 
	
	if ((sx+width) > 320)
	{
		width = 320 - sx;
	}
	
	if ((sy+height) > 240)
	{
		height = 240 - sy;
	}
	
	sx += 176;
	sy += 12;
	
	endx=sx+width*2;	//HREF
 	endy=sy+height*2;	//VREF
	if(endx>784)
	{
		endx-=784;
	}
	
	
	/*
		这里举endx(HREF)的例子，VREF同理
		sx,endx分别??在一帧画面时序中，消影后的画面图像的开始和结束，然后重复消??+画面的过程，hstart和hstop??11位（垂直的是10位）
		高八位分??存储在寄存器0x17??0x18里面，低三位分别存放??0x32??345??012里面??67??保留字。保?? 保留 HSTOP[2] HSTOP[1] HSTOP[0] HSTART[2] HSTART[1] HSTART[0]
		然后进???寄存器操作即可
	*/
	temp=SCCB_ReadReg(0X32);				//读取Href之前的??
	temp&=0XC0;
	temp|=((endx&0X07)<<3)|(sx&0X07);
	SCCB_WriteReg(0X32,temp);
	SCCB_WriteReg(0X17,sx>>3);			//设置Href的start??8??
	SCCB_WriteReg(0X18,endx>>3);			//设置Href的end的高8??
	
	temp=SCCB_ReadReg(0X03);				//读取Vref之前的??
	temp&=0XF0;
	temp|=((endy&0X03)<<2)|(sy&0X03);
	SCCB_WriteReg(0X03,temp);				//设置Vref的start和end的最??2??
	SCCB_WriteReg(0X19,sy>>2);			//设置Vref的start??8??
	SCCB_WriteReg(0X1A,endy>>2);			//设置Vref的end的高8??

}


void OV7670_GetPicture(void)
{
	//??动DMA
	DMA_Cmd(DMA2_Stream1 ,ENABLE );
	DCMI_Cmd (ENABLE );
	DCMI_CaptureCmd (ENABLE );
}


void OV7670_StopGetPicture(void)
{
	DCMI_CaptureCmd (DISABLE );
	DCMI_Cmd (DISABLE );
	//关闭DMA
	DMA_Cmd(DMA2_Stream1 ,DISABLE );
}


void OV7670DMA_DCMItoSRAM(void)
{
	RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_DMA2 ,ENABLE );
	
	DMA_DeInit (DMA2_Stream1);
	
	DMA_InitTypeDef DMA_InitStructure;
	DMA_InitStructure.DMA_BufferSize = (160 * 120)/2 ;//传输计数??
	DMA_InitStructure.DMA_Channel =DMA_Channel_1 ; //通道
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory ; //传输方向
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable ;//FIFIO模式，开??/关闭，FIFO??以理解为一??缓冲??	
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull ;//FIFO半满才进行一次存储器突发
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)QQVGA_Frame;//??标???地址
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_INC4 ;  //意味一次传输中，是不是要连????运???个数据，单位是DMA_MemoryDataSize，只在地址??增时????
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord ; //??标???数据宽度
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable ; // ??标???数据地址??否自??
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;//开??双缓冲的条件
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&DCMI-> DR;//源???地址
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single ;     
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word ;//源???数据宽度
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable ;//源???地址??否自??
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_Init(DMA2_Stream1,&DMA_InitStructure);
	
	//开??双缓??
	DMA_DoubleBufferModeConfig(DMA2_Stream1,(uint32_t)QQVGA_Frame1, DMA_Memory_0);//??三个参数意味，指定哪??内存地址为???一??缓冲??
	DMA_DoubleBufferModeCmd(DMA2_Stream1,ENABLE);
	
	DMA_ITConfig (DMA2_Stream1 ,DMA_IT_TC ,ENABLE );
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStructure);
	
	DMA_Cmd (DMA2_Stream1 ,DISABLE );
}


/*===================读取器件ID，用来验证SCCB??否真的通了===================
	OV7670 ?? PID(0x0A) 读出来固定是 0x76，VER(0x0B) 读出来固定是 0x73??
	如果这两??值???不对，说明 SCCB 根本没写进摄像头，后面写的整张寄存器??
	等于全部作废，摄像头还停在上电默认的 YUV 输出，画面必然是乱的??
	返回值：0 = SCCB正常??1 = SCCB不??
========================================================================*/
uint8_t OV7670_CheckID(void)
{
	uint8_t pid;
	uint8_t ver;

	pid = SCCB_ReadReg(0x0A);
	ver = SCCB_ReadReg(0x0B);

	if ((pid == 0x76) && (ver == 0x73))
	{
		return 0;
	}
	return 1;
}


void OV7670_Proc()
{
	/*--------------------------- ??2???????? DCMI ????? GPIO ------------------*/
	OV7670_DCMI_Init();

	/*--------------------------- ??3????????? + ?????λ ----------------------*/
	OV7670_MCLK_start();		//	PA8 ??? 8MHz XCLK
	Delay_ms(10);
	OV7670_Rest();				//	PA1=PWDN(??=????)??PA3=RST
	Delay_ms(20);

	/*--------------------------- ??4????SCCB ??? ------------------------------*/
	SCCB_Init();				//	SCL=PB10??SDA=PB11
	Delay_ms(10);

	/*--------------------------- ??5????д?????????RGB565 + QVGA??-------------*/
	OV7670_RegConfig();

	/*--------------------------- ??6???????ò?????? ---------------------------*/
	OV7670_Window_Set(160, 120, 160, 120);

	/*--------------------------- ??7????DMA + DCMI ?????? -------------------*/
	OV7670DMA_DCMItoSRAM();
	OV7670_GetPicture();
	
}
