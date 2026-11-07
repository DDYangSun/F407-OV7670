#include "stm32f4xx.h"
#include "stdlib.h"
#include "ImageBinaryzation.h"
#include <math.h>

/*	二值化阈值的来源：
		0 = 峰谷法（你自己写的那套：找最高峰、第二高峰，取中间的谷）
		1 = 大津法（Otsu，按"类间方差最大"自动定阈值，图像不是明显双峰时更稳）
	两种方法每帧都会算出来，串口每秒打印一次，可以亲眼对比后再选。		*/
#define USE_OTSU_THRESHOLD	1

uint8_t  TopPosition,Top2Position,ValleyPos,OtsuPos;
uint16_t TopNumber,Top2Number,ValleyNum;

/*******************************求阈值前图像处理******************************* */
//RGB565（16bit色彩）转8位灰度（亮度）
void ChangeGray(uint16_t *RGB565Image,uint16_t Length,uint8_t *GrayImage)
{
	uint8_t r,g,b;

	uint16_t i;
	for (i = 0;i < Length;i++)
	{
		r = (RGB565Image[i] >> 11) & 0x1F;		//	0~31
		g = (RGB565Image[i] >> 5) & 0x3F;		//	0~63
		b = (RGB565Image[i]) & 0x1F;			//	0~31

		/*	5/6 位先扩展成 8 位（低位复制高位），再套 0.299/0.587/0.114 的整数系数，
			否则灰度只能到 49，直方图分辨率浪费一大半						*/
		GrayImage[i] = (uint8_t)((((r << 3) | (r >> 2)) * 77
								+ ((g << 2) | (g >> 4)) * 150
								+ ((b << 3) | (b >> 2)) * 29) >> 8);
	}
}

//制作像素值数量图表
void DrawPixelChart(uint8_t *GrayImage,uint16_t Length,uint16_t *PixelValueNum)
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

/************************************峰谷法*************************************** */
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
			TopPosition_Temp = (uint8_t)i;
			TopNumber_Temp = PixelValueNum[i];
		}
	}

	*TopPosition = TopPosition_Temp;
	*TopNumber = TopNumber_Temp;
}

/*	计算第二高峰的位置和数量：
	先把直方图做 5 点平滑（主峰很宽时，不平滑会把主峰的肩膀也当成峰，
	第二高峰就会紧贴着主峰）；再在平滑后的曲线上找"低于最高峰的局部极大值中最高者"，
	并且跳过人数少于总像素 0.5% 的桶。										*/
void CalTop2_PosNum(uint16_t *PixelValueNum,uint16_t Length,uint8_t *Top2Position,uint16_t *Top2Number)
{
	uint8_t  Top2Position_Temp = 0;		//必须初始化，否则找不到第二峰时是栈上的随机值
	uint16_t Top2Smooth_Temp   = 0;
	uint16_t TopSmooth         = 0;
	uint16_t MinPeak;
	uint32_t total = 0;
	uint16_t i;
	int16_t  k;
	static uint16_t Smooth[256];

	//求像素和，这里不导入新的像素总数形参是为了方便，只从直方图里面读总数即可
	for (i = 0;i < Length;i++)			//直方图总和就是这一帧的像素总数
	{
		total += PixelValueNum[i];
	}
	MinPeak = (uint16_t)(total / 200);	//低于总像素 0.5% 的不算峰

	//在原来的灰度数组基础上将附近5个点求平均数放进新的平滑数组
	for (i = 0;i < Length;i++)			//5 点平滑，消掉平台和毛刺
	{
		uint32_t s = 0;
		uint16_t n = 0;
		for (k = -2;k <= 2;k++)
		{
			int16_t j = (int16_t)i + k;
			if ((j >= 0) && (j < (int16_t)Length))
			{
				s += PixelValueNum[j];
				n++;
			}
		}
		Smooth[i] = (uint16_t)(s / n);
		if (Smooth[i] > TopSmooth)
		{
			TopSmooth = Smooth[i];
		}
	}

	//跟找最高峰一样，这里小于最高峰之中最高的就是第二高峰
	for (i = 2;i < Length - 2;i++)		//i-2、i+2 都不能越界
	{
		if ((Smooth[i] < TopSmooth)			//不是最高峰本身
			&& (Smooth[i] > MinPeak)		//不是空桶/噪点
			&& (Smooth[i] >= Smooth[i-1])
			&& (Smooth[i] >= Smooth[i+1])
			&& (Smooth[i] > Top2Smooth_Temp))
		{
			Top2Smooth_Temp   = Smooth[i];
			Top2Position_Temp = (uint8_t)i;
		}
	}

	*Top2Position = Top2Position_Temp;
	*Top2Number   = PixelValueNum[Top2Position_Temp];
}

/*	从两个山峰之间找到山谷的位置
	原版：当第二高峰和最高峰重合/相邻时（例如全图只有一种灰度），
	循环只走一次，ValleyPos_Temp 保持初值 0，阈值直接变成 0，画面全白。
	这里先把 lo/hi 规范化，并在"两峰太近、根本没有谷"时给出兜底值。		*/
uint8_t CalValley_PosNum(uint16_t *PixelValueNum,uint16_t Length,uint8_t *ValleyPos,uint16_t *ValleyNum,
						uint8_t *TopPosition,uint8_t *Top2Position)
{
	uint8_t  lo,hi,ValleyPos_Temp;
	uint16_t ValleyNum_Temp;
	uint16_t i;

	if (*TopPosition < *Top2Position)	{ lo = *TopPosition; hi = *Top2Position; }
	else								{ lo = *Top2Position; hi = *TopPosition; }

	/*	先找两峰之间的最小值，再把「取到最小值的这段区间」的中点当作谷底。
	如果只取第一个最小值，阈值通常会紧贴着最高的那个峰（空桶从峰旁边就开始了），
	二值化结果就会严重偏向一侧，几乎全白或几乎全黑。			*/
{
	uint8_t  first = lo,last = lo;
	uint16_t minv = 0xFFFF;

	for (i = lo;i <= hi;i++)
	{
		if (PixelValueNum[i] < minv)		{ minv = PixelValueNum[i]; first = (uint8_t)i; last = (uint8_t)i; }
		else if (PixelValueNum[i] == minv)	{ last = (uint8_t)i; }
	}
	ValleyNum_Temp = minv;
	ValleyPos_Temp = (uint8_t)(((uint16_t)first + (uint16_t)last) / 2);
}

	if ((uint8_t)(hi - lo) < 2)			//两峰重合或紧挨着，没有真正的谷
	{
		ValleyPos_Temp = (uint8_t)((lo + hi) / 2);
		ValleyNum_Temp = PixelValueNum[ValleyPos_Temp];
	}

	*ValleyPos = ValleyPos_Temp;
	*ValleyNum = ValleyNum_Temp;

	return *ValleyPos;
}


/*******************************大津法****************************************** */
/*	大津法（Otsu）：遍历 0~255 每个阈值，算"类间方差"，
	方差最大的那个阈值就是最能把画面分成两堆的位置。
	直接用已经统计好的直方图，只有 256 次循环，开销可以忽略。		*/
uint8_t Otsu_GetThreshold(uint8_t *GrayImage)
{
	uint8_t Otsu_Threshold = 0;
	float Threshold_BackgroundNum = 0,Thereshold_ForegroundNum = 0;				//前景背景的像素数量
	float Threshold_BackgroundSum = 0,Threshold_ForegroundSum,GraySum = 0;		//灰度值
	float w0,w1,u0,u1,ut;
	uint16_t PixelNum = 0;										//像素总数量
	float Variance = 0,Variance_Temp = 0;						//方差

	uint16_t i,j;
	for (i = 0;i < 256;i++)
	{
		GraySum += GrayImage[i] * i;				//计算像素灰度值之和，用于求全局平均灰度
		PixelNum += GrayImage[i];		//计算像素数量之和
	}

	for (i = 0;i < 256;i++)
	{
		for (j = 0;j < 256;j++)
		{
			if (j < i)
			{
				Threshold_BackgroundNum += GrayImage[j];			//计算背景灰度值数量之和，用于计算背景权重
				Threshold_BackgroundSum += GrayImage[j] * j;
			}
			else
			{
				Thereshold_ForegroundNum += GrayImage[j];			//计算前景灰度值数量之和，用于计算前景权重				
				Threshold_ForegroundSum += GrayImage[j] * j;
			}

		}
		w0 = Threshold_BackgroundNum / PixelNum;
		w1 = 1 - w0;	

		u0 = Threshold_BackgroundSum / Threshold_BackgroundNum;
		u1 = Threshold_ForegroundSum / Thereshold_ForegroundNum;
		ut = GraySum / (Threshold_BackgroundNum + Thereshold_ForegroundNum);

		Variance_Temp = w0 * pow((u0 - ut) , 2) + w1 * pow((u1 - ut) , 2);

		if (Variance_Temp > Variance)
		{
			Otsu_Threshold = i;
			Variance = Variance_Temp;
		}

		Threshold_BackgroundNum = Thereshold_ForegroundNum = Threshold_BackgroundSum = Threshold_ForegroundSum = 0;
	}

	return Otsu_Threshold;
}

/*
uint8_t Otsu_Threshold(uint16_t *PixelValueNum,uint16_t Length)
{
	uint32_t total = 0,wB = 0,wF;
	uint64_t sum = 0,sumB = 0;
	int64_t  mB,mF,d,var,maxVar = -1;
	uint8_t  best = 0;
	uint16_t i;

	for (i = 0;i < Length;i++)
	{
		total += PixelValueNum[i];
		sum   += (uint64_t)i * PixelValueNum[i];
	}
	if (total == 0)
	{
		return 128;
	}

	for (i = 0;i < Length;i++)
	{
		wB += PixelValueNum[i];
		if (wB == 0)	continue;

		wF = total - wB;
		if (wF == 0)	break;

		sumB += (uint64_t)i * PixelValueNum[i];
		mB = (int64_t)(sumB / wB);
		mF = (int64_t)((sum - sumB) / wF);
		d  = mB - mF;
		var = (int64_t)wB * (int64_t)wF * d * d;		//必须 64 位，int32 会溢出

		if (var > maxVar)
		{
			maxVar = var;
			best = (uint8_t)i;
		}
	}
	return best;
}
*/
//二值化
void ImageBinaryzation(uint8_t *GrayImage,uint16_t Length,uint8_t *Threshold)
{
	uint16_t i;
	for (i = 0;i < Length;i++)
	{
		if (GrayImage[i] < *Threshold)
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

uint8_t Image_GetThereshold(uint16_t *RGB565Image,uint8_t *GrayImage,uint16_t Length,uint16_t *PixelValueNum,uint16_t Length1)
{
	uint8_t threshold;

	#define BeforeTheresholdImageDeal		1	//0：关闭求阈值前图像处理		1：开启求阈值前图像处理
	#define Method		1						//0：峰谷法					   1：大津法

	#if BeforeTheresholdImageDeal
	//求阈值前图像处理，若不需要将BeforeTheresholdImageDeal置0
		ChangeGray(RGB565Image,Length,GrayImage);
	#endif
	
	DrawPixelChart(GrayImage,Length,PixelValueNum);

	#if Method
		threshold = Otsu_GetThreshold(GrayImage);
	#else
		CalTop_PosNum(PixelValueNum,Length1,&TopPosition ,&TopNumber);
		CalTop2_PosNum(PixelValueNum,Length1,&Top2Position,&Top2Number);
		threshold = CalValley_PosNum(PixelValueNum,Length1,&ValleyPos,&ValleyNum,&TopPosition,&Top2Position);
	#endif

	return threshold;
}
