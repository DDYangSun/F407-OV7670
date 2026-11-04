#ifndef __SCCB_H
#define __SCCB_H
#include "stm32f4xx.h"

void SCCB_Init(void);
void SCCB_WriteReg(uint8_t RegAddress, uint8_t Data);	//	指定地址写
uint8_t SCCB_ReadReg(uint8_t RegAddress);				//	指定地址读
uint8_t SCCB_TestLine(void);							//	SDA 连线自检

#endif
