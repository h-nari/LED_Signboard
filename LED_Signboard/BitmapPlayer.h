#ifndef _bitmap_player_h_
#define _bitmap_player_h_

#include "MsgPlayer.h"

class BitmapPlayer: public MsgPlayer {
  /* parameter */
 public:
  int   m_ox;			// 水平位置オフセット
  int   m_oy;			// 垂直位置オフセット
  int   m_hAlign;		// 0:left 1:center 2:right
 				// 3:画像の左端を領域の右端に
  				// 4:画像の右端を端を領域の左端に
  int   m_vAlign;		// 0:top  1:center 2:bottom
 				// 3:画像の上端を領域の下端に 
 				// 4:画像の下端を領域の上端に 
  int   m_rot;			// 回転
  float m_fTime;		// 静止時間 [秒]
  int   m_nScrollDir;  		// 0: 左 1: 上 2: 右 3:下
  int   m_nScrollAmount;	// スクロール指定量
  int   m_nScrollType; 		// 0:無し 1:指定量 2:画像サイズ＋指定量
 				// 3:画像サイズ+領域サイズ+指定量
  float m_fScrollSpeed;		// スクロール速度 [dot/秒]

 protected:
  int16_t   m_oxx;		// 描画オフセット位置、m_ox + m_hAlign
  int16_t   m_oyy;		// 描画オフセット位置、m_oy + m_vAlign
  uint16_t  m_w0;		// 回転後画像幅
  uint16_t  m_h0;		// 回転後画像高
  uint16_t  m_w;		// 回転後画像幅
  uint16_t  m_h;		// 回転後画像高
  unsigned long m_tStart;
  int   m_msec;			// 静止時間 [msec], m_fTime * 1000
  File  m_file;
  bool  m_bScrollDone;
  int16_t  m_sc;		// x方向スクロール量
  uint16_t m_offBits;   // BMPファイルの画像データ位置

 public:
  BitmapPlayer();
  void initValues() override;
  bool init(uint8_t id) override;
  bool update() override;

 protected:
  bool bmp_read(const char *path);
  void bmp_disp(int scx=0, int scy=0);
  bool bmp_get_pixel(int x,int y,uint8_t *rp, uint8_t *gp,uint8_t  *bp);
};

#endif /* _bitmap_player_h_ */
