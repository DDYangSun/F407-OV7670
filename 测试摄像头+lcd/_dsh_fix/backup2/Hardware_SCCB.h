#ifndef __SCCB_H
#define __SCCB_H

void SCCB_Init(void);
void SCCB_WaitEvent(I2C_TypeDef* I2Cx, uint32_t I2C_EVENT);
void SCCB_WriteReg(uint8_t RegAddress, uint8_t Data);//指定地址写
uint8_t SCCB_ReadReg(uint8_t RegAddress);
#endif
