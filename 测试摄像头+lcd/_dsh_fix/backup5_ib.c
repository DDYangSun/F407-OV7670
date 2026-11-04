#include "stm32f4xx.h"                  // Device header
#include "stdlib.h"

uint8_t TopPosition,Top2Position,ValleyPos;
uint16_t TopNumber,Top2Number,ValleyNum;

//RGB565（16bit色彩）转8位灰度（亮度）
void ChangeGray(uint16_t *RGB565Image,uint16_t Length,uint8_t *GrayImage)
{
	uint8_t r,g,b;

	uint16_t i;
	for (i = 0;i < Length;i++)
	{
		r = (RGB565Image[i] >> 11) & 0x1F;
		g = (RGB565Image[i] >> 5) & 0x3F;
		b = (RGB565Image[i]) & 0x1F;

		GrayImage[i] = (uint8_t)((((r << 3) | (r >> 2)) * 77 + ((g << 2) | (g >> 4)) * 150 + ((b << 3) | (b >> 2)) * 29) >> 8);
	}
}

//制作像素值数量图表
//PixelValueNum需要是根据长度制定的空数组
void DrawPixelChart(uint8_t *GrayImage,uint16_t Length,uint16_t *PixelValueNum)		//形参分别是灰度值，像素值数量图表
{
	uint16_t i;	


	for(i = 0;i < 256;i++)		//每帧统计前必须先清零，否则直方图会一帧帧累加，uint16 几帧就溢出
	{
		PixelValueNum[i] = 0;
	}

	for(i = 0;i < Length;i++)
	{
		PixelValueNum[*GrayImage]++;
		GrayImage++;
	}
}

//计算最高峰的位置和数量
void CalTop_PosNum(uint16_t *PixelValueNum,uint16_t Length,uint8_t *TopPosition,uint16_t *TopNumber)
{
	uint8_t TopPosition_Temp = 0;
	uint16_t TopNumber_Temp = 0;

	uint16_t i;
	for (i = 0;i < Length;i++)
	{
		if (PixelValueNum[i] > TopNumber_Temp)
		{
			TopPosition_Temp = i;
			TopNumber_Temp = PixelValueNum[i];
		}
	}

	*TopPosition = TopPosition_Temp;
	*TopNumber = TopNumber_Temp;
}

//计算第二高峰的位置和数量
void CalTop2_PosNum(uint16_t *PixelValueNum,uint16_t Length,uint8_t *Top2Position,uint16_t *Top2Number,uint16_t *TopNumber)
{
	uint8_t Top2Position_Temp;
	uint16_t Top2Number_Temp;
	uint16_t DifValue = *TopNumber;		//与最高峰的差值

	uint16_t PeakNum = 0;		//山峰数量
	uint16_t PeakPos[256];

	uint16_t i;
	//找到几个山峰
	for (i = 0;i < Length;i++)
	{
		if (PixelValueNum[i] >= PixelValueNum[i+1] && PixelValueNum[i] >= PixelValueNum[i-1])
		{
			PeakPos[PeakNum] = i;
			PeakNum++;
		}
	}

	//从山峰中找出第二高峰
	for (i = 0;i < PeakNum;i++)
	{
		if (abs(PixelValueNum[PeakPos[i]] - *TopNumber) < DifValue && abs(PixelValueNum[PeakPos[i]] - *TopNumber) != 0)
		{
			DifValue =  PixelValueNum[PeakPos[i]] - *TopNumber;
			Top2Position_Temp = PeakPos[i];
			Top2Number_Temp = PixelValueNum[PeakPos[i]];
		}
	}

	*Top2Position = Top2Position_Temp;
	*Top2Number	= Top2Number_Temp;
}


//从两个山峰之间找到山谷的位置，要注意最高峰和第二高峰有两种位置关系
void CalValley_PosNum(uint16_t *PixelValueNum,uint16_t Length,uint8_t *ValleyPos,uint16_t *ValleyNum,
						uint8_t *TopPosition,uint8_t *Top2Position)
{
	uint8_t ValleyPos_Temp = 0;
	uint16_t ValleyNum_Temp = PixelValueNum[*TopPosition];
	uint16_t i;

	if (*TopPosition > *Top2Position)
	{
		for (i = *Top2Position;i <= *TopPosition;i++)
		{
			if (PixelValueNum[i] < ValleyNum_Temp)
			{
				ValleyNum_Temp = PixelValueNum[i];
				ValleyPos_Temp = i;
			}
		}
	}
	else
	{
		for (i = *TopPosition;i <= *Top2Position;i++)
		{
			if (PixelValueNum[i] < ValleyNum_Temp)
			{
				ValleyNum_Temp = PixelValueNum[i];
				ValleyPos_Temp = i;
			}
		}		
	}

	*ValleyPos = ValleyPos_Temp;
	*ValleyNum = ValleyNum_Temp;
}

//二值化
void ImageBinaryzation(uint8_t *GrayImage,uint16_t Length,uint8_t *ValleyPos)
{
	uint16_t i;
	for (i = 0;i < Length;i++)
	{
		if (GrayImage[i] < *ValleyPos)
			GrayImage[i] = 0;		//黑色
		else
			GrayImage[i] = 255;		//白色
	}
}

/*
	param0:RGB565数组
	param1:灰度值数组
	param2:RGB565数组和灰度值数组长度，与分辨率有关
	param3:像素值数量图表数组
	param4:像素值数量图表数组长度
*/
void ImageProcess(uint16_t *RGB565Image,uint8_t *GrayImage,uint16_t Length,uint16_t *PixelValueNum,uint16_t Length1)
{
	ChangeGray(RGB565Image,Length,GrayImage);
	DrawPixelChart(GrayImage,Length,PixelValueNum);
	CalTop_PosNum(PixelValueNum,Length1,&TopPosition,&TopNumber);
	CalTop2_PosNum(PixelValueNum,Length1,&Top2Position,&Top2Number,&TopNumber);
	CalValley_PosNum(PixelValueNum,Length1,&ValleyPos,&ValleyNum,&TopPosition,&Top2Position);
	ImageBinaryzation(GrayImage,Length,&ValleyPos);					
}
