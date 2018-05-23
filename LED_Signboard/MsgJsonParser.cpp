#include "MsgJsonParser.h"
#include "StaticTextPlayer.h"
#include "ScrollTextPlayer.h"
#include "BitmapPlayer.h"

StaticTextPlayer staticTextPlayer;
ScrollTextPlayer scrollTextPlayer;
BitmapPlayer     bitmapPlayer;

MsgJsonParser::MsgJsonParser() : JsonParser()
{
  reset();
}

void MsgJsonParser::reset()
{
  JsonParser::reset();
  m_state = S_START;
  m_player = NULL;
}

bool MsgJsonParser::handleBeginHash()
{
  if(m_player)
    return m_player->handleBeginHash();
  else if(m_state == S_START)
    m_state = S_TOP_HASH;
  else
    return error("syntax error at '{'");
  return true;
}

bool MsgJsonParser::handleEndHash(){
  if(m_player) return m_player->handleEndHash();
  else if(m_state == S_TOP_HASH)
    m_state = S_END;
  else return error("syntax error in state:%d",m_state);
  return true;
}

bool MsgJsonParser::handleBeginArray()
{
  return error("syntax error %s:%d",__FUNCTION__,__LINE__);
}

bool MsgJsonParser::handleEndArray()
{
  return error("syntax error %s:%d",__FUNCTION__,__LINE__);
}

bool MsgJsonParser::handleKey(const char *key)
{
  if(m_player) return m_player->handleKey(key);
  else if(m_state == S_TOP_HASH){
    if(strcmp(key,"title")==0)
      m_state = S_TITLE_VALUE;
    else if(strcmp(key,"player")==0)
      m_state = S_PLAYER_VALUE;
    else return error("set player or title first");
  }
  else
    return error("syntax error %s:%d",__FUNCTION__,__LINE__);
  return true;
}

bool MsgJsonParser::handleValue(json_value_t *value)
{
  if(m_player) return m_player->handleValue(value);
  else if(m_state == S_TITLE_VALUE) {
    m_state = S_TOP_HASH;
  }
  else if(m_state == S_PLAYER_VALUE){
    if(value->type != JVT_STRING) return error("player must be string");
    if(strcmp(value->sVal,"static-text")==0)
      m_player = &staticTextPlayer;
    else if(strcmp(value->sVal,"scroll-text")==0)
      m_player = &scrollTextPlayer;
    else if(strcmp(value->sVal,"bitmap")==0)
      m_player = &bitmapPlayer;
    else
      return error("player:%s not defined",value->sVal);
    m_player->resetParser(this);
  }
  else
    return error("syntax error %s:%d",__FUNCTION__,__LINE__);
  return true;
}
    

