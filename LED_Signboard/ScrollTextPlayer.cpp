#include <Fontx.h>
#include "ScrollTextPlayer.h"
#include "JsonParser.h"
#include "SignboardPlayer.h"
#include "my_strftime.h"

extern SignboardPlayer player;
extern ScrollText scrollBuf;

static MsgPlayer::ParaTbl paraTbl[] = {
  { "valign",       JVT_INT,     offsetof(ScrollTextPlayer,m_vAlign)},
  { "oy",           JVT_INT,     offsetof(ScrollTextPlayer,m_oy)},
  { "out",          JVT_BOOLEAN, offsetof(ScrollTextPlayer,m_bScrollOut)},
  { "x",            JVT_INT,     offsetof(ScrollTextPlayer,m_scrollX)},
  { "color",        JVT_STRING,  offsetof(ScrollTextPlayer,m_color)},
  //  { "text",         JVT_STRING,  offsetof(ScrollTextPlayer,m_text)},
  { "speed",        JVT_FLOAT,   offsetof(ScrollTextPlayer,m_speed)},
  { "font",         JVT_INT,     offsetof(ScrollTextPlayer,m_font)},
  { "fontsize",     JVT_INT,     offsetof(ScrollTextPlayer,m_fontsize)},
  { "replace",      JVT_BOOLEAN, offsetof(ScrollTextPlayer,m_bReplace)},
  { NULL },
};
 
ScrollTextPlayer::ScrollTextPlayer() : MsgPlayer()
{
  reset();
  m_paraTbl = paraTbl;
}

void ScrollTextPlayer::reset()
{
  m_speed = 32.0;
}

void ScrollTextPlayer::initValues()
{
  MsgPlayer::initValues();
  m_vAlign = 1;
  m_oy = 0;
  m_bScrollOut = true;
  m_scrollX = LedMat.width();
  m_color = "#ffffff";
  // m_text  = "scroll";
  m_font = 0;
  m_fontsize = 1;
  m_bReplace = true;
}

bool ScrollTextPlayer::init(uint8_t id)
{
  int16_t x1, y1, oy;
  uint16_t w,h;
  char textFile[32];
  
  MsgPlayer::init(id);

  m_bScrollDone = false;
  m_bSkipBlankLine = true;

  scrollBuf.clear();
  player.setFont(m_font);
  LedMat.setTextSize(m_fontsize);
  player.setFont(&scrollBuf, m_font);
  scrollBuf.setTextSize(m_fontsize);
  
  LedMat.getTextBounds("A", 0, 0, &x1, &y1, &w, &h);

  oy = (m_rh - h) * m_vAlign / 2 - y1 + m_oy;

  scrollBuf.setScrollArea(m_rx, m_ry, m_rw, m_rh);
  scrollBuf.setXPos(m_scrollX);
  scrollBuf.setYPos(oy);
  scrollBuf.setSpeed(m_speed);
  scrollBuf.setTextColor(m_color.c_str());
  scrollBuf.setBgColor(m_bg.c_str());
  // scrollBuf.setFont(&fontx);
  scrollBuf.update(true);

  player.makeMsgFile(textFile,sizeof textFile, m_id, "txt");
  m_file = SD.open(textFile);
  if(m_file){
    LedMat.display();
    return true;
  } else {
    Serial.printf("Error: %s not found.\n",textFile);
    return false;
  }
}

bool ScrollTextPlayer::update()
{
  if(scrollBuf.update())
    return true;

  if(m_file){
    if(m_file.available() > 0){
      String line = m_file.readStringUntil('\n');
      if(line == ""){
	if(!m_bSkipBlankLine){
	  scrollBuf.scrollOut();
	  m_bSkipBlankLine = true;
	}
      } else {
	scrollBuf.print( m_bReplace ? my_strftime(line.c_str()) : line);
	m_bSkipBlankLine = false;
      }
      scrollBuf.update();
      return true;
    }
    m_file.close();
  }
    
  if(m_bScrollOut){
    if(!m_bScrollDone){
      scrollBuf.scrollOut();
      scrollBuf.update();
      m_bScrollDone = true;
      return true;
    }
  }
  return MsgPlayer::update();
}




