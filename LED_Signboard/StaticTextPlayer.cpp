#include <Fontx.h>
#include "StaticTextPlayer.h"
#include "JsonParser.h"
#include "SignboardPlayer.h"
#include "my_strftime.h"

extern SignboardPlayer player;
extern ScrollText scrollText;

static MsgPlayer::ParaTbl paraTbl[] = {
  { "halign",       JVT_INT,     offsetof(StaticTextPlayer,m_hAlign)},
  { "valign",       JVT_INT,     offsetof(StaticTextPlayer,m_vAlign)},
  { "ox",           JVT_INT,     offsetof(StaticTextPlayer,m_ox)},
  { "oy",           JVT_INT,     offsetof(StaticTextPlayer,m_oy)},
  { "color",        JVT_STRING,  offsetof(StaticTextPlayer,m_color)},
  { "text",         JVT_STRING,  offsetof(StaticTextPlayer,m_text)},
  { "time",         JVT_FLOAT,   offsetof(StaticTextPlayer,m_time)},
  { "font",         JVT_INT,     offsetof(StaticTextPlayer,m_font)},
  { "fontsize",     JVT_INT,     offsetof(StaticTextPlayer,m_fontsize)},
  { "replace",      JVT_BOOLEAN, offsetof(StaticTextPlayer,m_bReplace)},
  { NULL },
};
 
StaticTextPlayer::StaticTextPlayer() : MsgPlayer()
{
  reset();
  m_paraTbl = paraTbl;
}

void StaticTextPlayer::reset()
{
}

void StaticTextPlayer::initValues()
{
  MsgPlayer::initValues();
  m_color = "#ffffff";
  m_text  = "ABC";
  m_time  = 0.5f;
  m_hAlign = 1;
  m_vAlign = 1;
  m_ox = 0;
  m_oy = 0;
  m_font = 0;
  m_fontsize = 0;
  m_bReplace = true;
}

bool StaticTextPlayer::init(uint8_t id)
{
  int16_t x1, y1, ox, oy;
  uint16_t w,h;
  String str;

  if(m_bReplace)
    str = my_strftime(m_text.c_str());
  else
    str = m_text;;
  
  MsgPlayer::init(id);
  LedMat.setClippingArea(m_rx, m_ry, m_rw, m_rh);

  player.setFont(m_font);
  LedMat.setTextSize(m_fontsize);
  
  LedMat.getTextBounds(str.c_str(), 0, 0, &x1, &y1, &w, &h);
  
  ox = (m_rw - w) * m_hAlign / 2 - x1 + m_ox;
  oy = (m_rh - h) * m_vAlign / 2 - y1 + m_oy;

  m_tStart = millis();
  m_msec = m_time * 1000;

  LedMat.setTextColor(m_color.c_str());
  LedMat.setCursor(m_rx + ox,m_ry + oy);
  LedMat.print(str);

  LedMat.display();
  return true;
}

bool StaticTextPlayer::update()
{
  if(millis() - m_tStart < m_msec)
    return true;
  LedMat.unsetClippingArea();
  return MsgPlayer::update();
}




