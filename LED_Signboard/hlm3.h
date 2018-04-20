#ifndef _hlm_h
#define _hlm_h

#include <stdint.h>
/*
 * Humblesoft Led Movie format
 */

typedef struct {
  char signature[4];            /* HLM */
  uint16_t size;                /* この構造体のサイズ: 40 */
  uint16_t version;		/* 3 */
  uint16_t width;
  uint16_t height;              /* heightは scan*2の倍数 */
  uint16_t scan;
  uint16_t depth;
  uint16_t fps_numerator;
  uint16_t fps_denominator;
  uint32_t frames;              /* フレーム数 */
  uint32_t flags;
  uint32_t frame_size;          /* 1フレームデータのバイト数        */   
  uint32_t frame_start;         /* 最初のフレームデータのオフセット */
  uint32_t frame_offset;        /* フレームごとのオフセット         */
} hlm3_header_t;

/* flags の値 */
#define HLM_REPEAT  (1 << 0)     

#endif
