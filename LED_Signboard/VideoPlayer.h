#ifndef _video_player_h_
#define _video_player_h_

#include <MoviePlayer.h>
#include "MsgPlayer.h"

class VideoPlayer: public MsgPlayer {
  /* parameter */
 public:
  String m_sFile;
  bool   m_bPartial;
  float  m_fStart;
  float  m_fEnd;
  float  m_fSpeed;
  
 protected:
  MoviePlayer m_movie;
  
 public:
  VideoPlayer();
  void initValues() override;
  bool init(uint8_t id) override;
  bool update() override;

 protected:
};

#endif /* _video_player_h_ */
