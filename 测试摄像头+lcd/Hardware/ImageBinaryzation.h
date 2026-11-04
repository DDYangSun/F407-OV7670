#ifndef __IMAGEBINARYZATION_H__
#define __IMAGEBINARYZATION_H__

void ChangeGray(uint16_t *RGB565Image,uint16_t Length,uint8_t *GrayImage);
void DrawPixelChart(uint8_t *GrayImage,uint16_t Length,uint16_t *PixelValueNum);		//形参分别是位置像素值，像素值数量图表
void CalTop_PosNum(uint16_t *PixelValueNum,uint16_t Length,uint8_t *TopPosition,uint16_t *TopNumber);
void CalTop2_PosNum(uint16_t *PixelValueNum,uint16_t Length,uint8_t *Top2Position,uint16_t *Top2Number,uint16_t *TopNumber);
void CalValley_PosNum(uint16_t *PixelValueNum,uint16_t Length,uint8_t *ValleyPos,uint16_t *ValleyNum,
						uint8_t *TopPosition,uint8_t *Top2Position);
void ImageBinaryzation(uint8_t *GrayImage,uint16_t Length,uint8_t *ValleyPos);
void ImageProcess(uint16_t *RGB565Image,uint8_t *GrayImage,uint16_t Length,uint16_t *PixelValueNum,uint16_t Length1);

#endif

