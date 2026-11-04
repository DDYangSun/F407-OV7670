#ifndef __OV7670_H
#define __OV7670_H

void OV7670_DCMI_Init(void);
void OV7670_RegConfig(void);
void OV7670_Rest(void);
void OV7670_MCLK_start(void);
void OV7670_Window_Set(uint16_t sx,uint16_t sy,uint16_t width,uint16_t height);
void OV7670_GetPicture(void);
void OV7670_StopGetPicture(void);
void OV7670DMA_DCMItoSRAM(void);
#endif
