#ifndef _bmp_h
#define _bmp_h

typedef struct {
  char     bfType[2];
  uint16_t bfSize0;
  uint16_t bfSize1;
  uint16_t bfReserved1;
  uint16_t bfReserved2;
  uint16_t bfOffBits0;
  uint16_t bfOffBits1;
} bitmapFileHeader_t ;

typedef struct {
  uint32_t  biSize;
  int32_t   biWidth;
  int32_t   biHeight;
  uint16_t  biPlanes;
  uint16_t  biBitCount;
  uint32_t  biCompression;
  uint32_t  biSizeImage;
  int32_t   biXPixPerMeter;
  int32_t   biYPixPerMeter;
  uint32_t  biClrUsed;
  uint32_t  biClrImporant;
} bitmapInfoHeader_t;


#endif /* _bmp_h */
