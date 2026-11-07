#include "stm32f4xx.h"                  // Device header
#include "ImageElement.h"
#include "stdlib.h"

uint8_t Image[Image_Wide][Image_Length] = {0};
uint8_t LeftLine[Image_Wide],RightLine[Image_Wide],MidLine[Image_Wide];

//灰度数组一维转二维
void Image_DimensionChange(uint8_t *GrayImage)
{
    uint16_t i;

    for (i = 0;i < Image_Wide * Image_Length;i++)
    {
        Image[i / Image_Length][i % Image_Length] = GrayImage[i];
    }
}

//从图像中心点向两侧找中心线
void FindLine()
{
    uint16_t i,j;
    
    for (i = 0;i < Image_Wide;i++)
    {
        LeftLine[i] = 0;
        RightLine[i] = Image_Length - 1;
    }

    for (i = Image_Wide - 1;i > 0;i--)
    {
        //从中间向左边找线
        for (j = Image_Length / 2;j >= 1;j--)
        {
            if (abs(Image[i][j] - Image[i][j - 1]) == 255)
            {
                LeftLine[i] = j;
                break;
            }
        }

        //从中间向右边找线
        for (j = Image_Length / 2;j <= Image_Length - 1;j++)
        {
            if (abs(Image[i][j] - Image[i][j + 1]) == 255)
            {
                RightLine[i] = j;
                break;
            }
        }

        //取中线
        MidLine[i] = (LeftLine[i] + RightLine[i]) / 2;
    }

    for (i = 0; i < Image_Wide;i++)
    {
        //Image[i][LeftLine[i]] = 0;
        //Image[i][RightLine[i]] = 0;

        if (MidLine[i] > 0 && MidLine[i] < Image_Length && !(LeftLine[i] == 0 && RightLine[i] == Image_Length - 1))
            Image[i][MidLine[i]] = 0;
    }
}
