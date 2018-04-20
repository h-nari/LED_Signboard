#ifndef _scroll_text_player_h_
#define _scroll_text_player_h_

#include "MsgPlayer.h"
#include <ScrollText.h>

class ScrollTextPlayer : public MsgPlayer {

  /* parameter */
 public:
  String m_color;
  // String m_text;
  float  m_speed;
  int m_time;		// msec
  int m_hAlign;		// 0:left 1:center 2:right
  int m_vAlign;		// 0:top  1:center 2:bottom
  int m_ox;		// x position offset 
  int m_oy;		// y position offset
  bool m_bScroll;
  bool m_bScrollOut;
  int m_scrollX;	// scroll start position
  int m_font;
  int m_fontsize;
  bool m_bReplace;
  
 protected:
  unsigned long m_tStart;
  File m_file;
  bool m_bScrollDone;
  bool m_bSkipBlankLine;
  
 public:
  ScrollTextPlayer();
  void initValues() override;
  bool init(uint8_t id) override;
  bool update() override;

 protected:
  void reset();
};


#endif /* _scroll_text_player_h_ */
