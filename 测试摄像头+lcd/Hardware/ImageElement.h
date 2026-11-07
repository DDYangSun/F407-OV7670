#ifndef __IMAGE_ELEMENT_H__
#define __IMAGE_ELEMENT_H__

#define Image_Wide                        120
#define Image_Length                      160
#define Image_PixelNum                   Image_Wide * Image_Length

void Image_DimensionChange(uint8_t *GrayImage);
void FindLine(void);


#endif
