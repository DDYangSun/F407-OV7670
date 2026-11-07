#ifndef __IMAGEBINARYZATION_H__
#define __IMAGEBINARYZATION_H__

#include "stm32f4xx.h"

extern uint8_t  TopPosition,Top2Position,ValleyPos,OtsuPos;
extern uint16_t TopNumber,Top2Number,ValleyNum;

void ChangeGray(uint16_t *RGB565Image,uint16_t Length,uint8_t *GrayImage);
void DrawPixelChart(uint8_t *GrayImage,uint16_t Length,uint16_t *PixelValueNum);		//????????????????б└ик
void CalTop_PosNum(uint16_t *PixelValueNum,uint16_t Length,uint8_t *TopPosition,uint16_t *TopNumber);
void CalTop2_PosNum(uint16_t *PixelValueNum,uint16_t Length,uint8_t *Top2Position,uint16_t *Top2Number);
uint8_t CalValley_PosNum(uint16_t *PixelValueNum,uint16_t Length,uint8_t *ValleyPos,uint16_t *ValleyNum,
						uint8_t *TopPosition,uint8_t *Top2Position);
uint8_t Otsu_Threshold(uint16_t *PixelValueNum,uint16_t Length);
void ImageBinaryzation(uint8_t *GrayImage,uint16_t Length,uint8_t *Threshold);
uint8_t Image_GetThereshold(uint16_t *RGB565Image,uint8_t *GrayImage,uint16_t Length,uint16_t *PixelValueNum,uint16_t Length1);

#endif

