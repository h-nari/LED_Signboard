#include "BitmapPlayer.h"
#include "JsonParser.h"
#include "SignboardPlayer.h"
#include "bmp.h"
#include "utils.h"
#include "dump.h"

extern SignboardPlayer player;

static MsgPlayer::ParaTbl paraTbl[] = {
  { "ox",           JVT_INT,     offsetof(BitmapPlayer,m_ox)},
  { "oy",           JVT_INT,     offsetof(BitmapPlayer,m_oy)},
  { "halign",       JVT_INT,     offsetof(BitmapPlayer,m_hAlign)},
  { "valign",       JVT_INT,     offsetof(BitmapPlayer,m_vAlign)},
  { "rot",          JVT_INT,     offsetof(BitmapPlayer,m_rot)},
  { "time",         JVT_FLOAT,   offsetof(BitmapPlayer,m_fTime)},
  { "scrollDir",    JVT_INT,     offsetof(BitmapPlayer,m_nScrollDir)},
  { "scrollAmount", JVT_INT,     offsetof(BitmapPlayer,m_nScrollAmount)},
  { "scrollType",   JVT_INT,     offsetof(BitmapPlayer,m_nScrollType)},
  { "speed",        JVT_FLOAT,   offsetof(BitmapPlayer,m_fScrollSpeed)},
  { NULL },
};

BitmapPlayer::BitmapPlayer() : MsgPlayer()
{
  m_paraTbl = paraTbl;
  initValues();
}

void BitmapPlayer::initValues()
{
  m_ox = 0;
  m_oy = 0;
  m_rot = 0;
  m_nScrollType = 0;
  m_nScrollAmount  = 0;
  m_fTime = 1.0f;
  m_fScrollSpeed = 32.0f;
  m_hAlign = 1;
  m_vAlign = 1;
}


bool BitmapPlayer::init(uint8_t id)
{
  MsgPlayer::init(id);
  m_bScrollDone = false;
  m_msec = m_fTime * 1000;
  m_sc = 0;
  
  // bitmapを表示

  char path[40];
  snprintf(path,sizeof path,MSG_PATH_PAT,player.getScript(), id, "BMP");
  if(bmp_read(path)){
    LedMat.setClippingArea(m_rx, m_ry, m_rw, m_rh);
    bmp_disp();
  }

  m_tStart = millis();
  return true;
}

#ifndef MAX
#define MAX(a,b)  ((a)>(b)?(a):(b))
#endif

bool BitmapPlayer::update()
{
  unsigned long tNow = millis();
  if(m_nScrollType == 0){
    if(tNow - m_tStart < m_msec)
      return true;
  } else {
    float sec = (tNow - m_tStart) * 0.001;
    int sc = sec * m_fScrollSpeed;

    if(sc == m_sc)
      return true;
    if(m_fScrollSpeed < 0.1)
      goto end;
    
    int16_t scx,scy;
    switch(m_nScrollDir){
    case 0: scx = -sc; scy =   0; break;
    case 1: scx =   0; scy = -sc; break;
    case 2: scx =  sc; scy =   0; break;
    case 3: scx =   0; scy =  sc; break;
    default:scx =   0; scy =   0; break;
    }

    if(m_nScrollDir == 0){
      int shift = sc - m_sc;
      LedMat.shiftLeft(m_rx, m_ry, m_rw, m_rh, shift);

      // 新しい部分を描画
      uint16_t bg = LedMat.rgb(m_bg.c_str());
      if(m_rot & 1){
        for(int xi=0; xi < shift; xi++){
          for(int yi=0; yi < m_rh; yi++){
            uint8_t r,g,b;
            if(bmp_get_pixel(m_rw-shift+xi-scx-m_oxx,yi-m_oyy, &r, &g, &b))
              LedMat.drawPixelRGB(m_rx+m_rw-shift+xi, m_ry + yi, r, g, b);
            else
              LedMat.drawPixel(m_rx+m_rw-shift+xi, m_ry + yi, bg);
          }
        }
      } else {
        for(int yi=0; yi < m_rh; yi++){
          for(int xi=0; xi < shift; xi++){
            uint8_t r,g,b;
            if(bmp_get_pixel(m_rw-shift+xi-scx-m_oxx,yi-m_oyy, &r, &g, &b))
              LedMat.drawPixelRGB(m_rx+m_rw-shift+xi, m_ry + yi, r, g, b);
            else
              LedMat.drawPixel(m_rx+m_rw-shift+xi, m_ry + yi, bg);
          }
        }
      }
      LedMat.display();
    }
    else {
      bmp_disp(scx, scy);
    }
    m_sc = sc;
    
    int sm, so = m_nScrollAmount;
    if(m_nScrollType == 2){
      so += (m_nScrollDir % 2) ? m_h : m_w;
    } else if(m_nScrollType == 3){
      so += (m_nScrollDir % 2) ? (m_h + m_rh) : (m_w + m_rw);
    }
    if(sc < so)
      return true;
  }
 end:
  if(m_file)
    m_file.close();
  LedMat.unsetClippingArea();
  return MsgPlayer::update();
}

bool BitmapPlayer::bmp_read(const char *path)
{
  if(m_file)
    m_file.close();
  m_file = SD.open(path);
  if(!m_file) return error("%s not found", path);
  int n;
  bitmapFileHeader_t bfh;
  bitmapInfoHeader_t bih;

  n = m_file.read((uint8_t *)&bfh, sizeof bfh);
  if(n < 0){
    m_file.close();
    return error("read bfh failed");
  }

  n = m_file.read((uint8_t *)&bih, sizeof bih);
  if(n < 0) {
    m_file.close();
    return error("read bih failed");
  }

  m_offBits = bfh.bfOffBits0;
    
  m_w0 = bih.biWidth;
  m_h0 = -bih.biHeight;

  if(m_rot % 2) {
    m_w = m_h0;
    m_h = m_w0;
  } else {
    m_w = m_w0;
    m_h = m_h0;
  }
    
  return true;
}

bool BitmapPlayer::bmp_get_pixel(int x, int y,
                                 uint8_t *rp, uint8_t *gp,uint8_t  *bp)
{
#if 0
  if(y == 0){
    Serial.printf("%s(%d) x:%d y:%d w:%d h:%d\n",__FUNCTION__,__LINE__,x,y,
                  m_w,m_h);
  }
#endif

  if(x < 0 || x >= m_w || y < 0 || y >= m_h) return false;
  if(!m_file) return false;

  int xr,yr;
  switch(m_rot % 4){
  case 0: xr = x;           yr = y;           break;
  case 1: xr = m_h - y - 1; yr = x;           break;
  case 2: xr = m_w - x - 1; yr = m_h - y - 1; break;
  case 3: xr = y;           yr = m_w - x - 1; break;
  }

  uint32_t offset = m_offBits + (yr * m_w0  + xr) * 3;
  if(m_file.seek(offset) < 0){
    Serial.printf("%s(%d) seek(%u) failed\n",__FUNCTION__,__LINE__,offset);
    return false;
  }
  uint8_t bgr[3];
  int n = m_file.read(bgr, 3);
  if(n < 3) return false;
  *rp = bgr[2];
  *gp = bgr[1];
  *bp = bgr[0];

  return true;
}

void BitmapPlayer::bmp_disp(int scx, int scy)
{
  int ox, oy;

  m_file.seek(m_offBits);
  if(!m_file){
    Serial.printf("%s(%d) not Open\n",__FUNCTION__,__LINE__);
    return ;
  }

  if(m_hAlign == 3)
    ox = m_rw;
  else if(m_hAlign == 4)
    ox = -m_w;
  else
    ox = (m_rw - m_w) * m_hAlign / 2 + m_ox;

  if(m_vAlign == 3)
    oy = m_rh;
  else if(m_vAlign == 4)
    oy = -m_h;
  else
    oy = (m_rh - m_h) * m_vAlign / 2 + m_oy;

  m_oxx = ox;
  m_oyy = oy;
    
  ox += m_rx + scx;
  oy += m_ry + scy;

  LedMat.fillRect(m_rx, m_ry, m_rw, m_rh, m_bg.c_str());

  if(m_rot % 4 == 0){
    for(int y=0; y < m_h0; y++){
      if(y + oy + m_rh <= m_ry || y + oy >= m_ry + m_rh)
        continue;
      m_file.seek(m_offBits + y * m_w * 3);
      for(int x=0; x < m_w0; x++){
        uint8_t rgb[3];
        int n = m_file.read(rgb, 3);
        if(n != 3){
          Serial.printf("bmp_disp read error:%d\n",n);
          break;
        }
        LedMat.drawPixelRGB(ox + x, oy + y, rgb[2],rgb[1],rgb[0]);
      }
    }
  }
  else {
    for(int y=0; y < m_h0; y++){
      for(int x=0; x < m_w0; x++){
        uint8_t rgb[3];
        int n = m_file.read(rgb, 3);
        if(n != 3){
          Serial.printf("bmp_disp read error:%d\n",n);
          break;
        }
        int xd,yd;
        switch(m_rot % 4){
        case 0: xd = x; yd = y;                       break;
        case 1: xd = y; yd = m_w0 - x - 1;            break;
        case 2: xd = m_w0 - x - 1; yd = m_h0 - y - 1; break;
        case 3: xd = m_h0 - y - 1; yd = x;            break;
        }
        LedMat.drawPixelRGB(ox + xd, oy + yd, rgb[2],rgb[1],rgb[0]);
      }
    }
  }
  LedMat.display();
}
