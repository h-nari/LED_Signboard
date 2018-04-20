#ifndef _static_text_player_h_
#define _static_text_player_h_

#include "MsgPlayer.h"

class StaticTextPlayer : public MsgPlayer {

  /* parameter */
 public:
  String m_color;
  String m_text;
  float m_time;		// sec
  int m_hAlign;		// 0:left 1:center 2:right
  int m_vAlign;		// 0:top  1:center 2:bottom
  int m_ox;		// x position offset 
  int m_oy;		// y position offset
  int m_font;
  int m_fontsize;
  bool m_bReplace;
  
 protected:
  unsigned long m_tStart;
  int m_msec;		// m_time * 1000
  
 public:
  StaticTextPlayer();
  bool init(uint8_t id) override;
  void initValues() override;
  bool update() override;

 protected:
  void reset();
};


#endif /* _scroll_text_player_h_ */
