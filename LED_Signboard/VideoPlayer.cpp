#include "VideoPlayer.h"
#include "JsonParser.h"
#include "SignboardPlayer.h"
#include "utils.h"
#include "dump.h"

extern SignboardPlayer player;

static MsgPlayer::ParaTbl paraTbl[] = {
  {"file",    JVT_STRING, offsetof(VideoPlayer, m_sFile)},
  {"partial", JVT_BOOLEAN,offsetof(VideoPlayer, m_bPartial)},
  {"start",   JVT_FLOAT,  offsetof(VideoPlayer, m_fStart)},
  {"end",     JVT_FLOAT,  offsetof(VideoPlayer, m_fEnd)},
  {"speed",   JVT_FLOAT,  offsetof(VideoPlayer, m_fSpeed)},
  { NULL },
};

VideoPlayer::VideoPlayer() : MsgPlayer()
{
  m_paraTbl = paraTbl;
  initValues();
}

void VideoPlayer::initValues()
{
  m_sFile = "";
  m_bPartial = false;
  m_fStart = 0.0f;
  m_fEnd   = 0.0f;
  m_fSpeed = 1.0f;
}


bool VideoPlayer::init(uint8_t id)
{
  MsgPlayer::init(id);
  
  // videoを表示

  char path[40];
  snprintf(path,sizeof path,"%s/%s", MOVIE_DIR, m_sFile.c_str());
  m_movie.begin(path);
  m_movie.setSpeed(m_fSpeed);
  if(m_bPartial){
    m_movie.seek(m_fStart);
    m_movie.setEnd(m_fEnd);
  }
  
  return true;
}

bool VideoPlayer::update()
{
  return m_movie.update();
}

