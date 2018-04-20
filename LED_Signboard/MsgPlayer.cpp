#include "MsgPlayer.h"
#include "SignboardPlayer.h"

static MsgPlayer::ParaTbl paraTbl[] = {
  { "title",  JVT_IGNORE}, 
  { "pre-clear",  JVT_BOOLEAN, offsetof(MsgPlayer,m_bPreClear)},
  { "post-clear", JVT_BOOLEAN, offsetof(MsgPlayer,m_bPostClear)},
  { "rx",    JVT_INT,     offsetof(MsgPlayer,m_rx)},
  { "ry",    JVT_INT,     offsetof(MsgPlayer,m_ry)},
  { "rw",    JVT_INT,     offsetof(MsgPlayer,m_rw)},
  { "rh",    JVT_INT,     offsetof(MsgPlayer,m_rh)},
  { "bg",    JVT_STRING,  offsetof(MsgPlayer,m_bg)},
  { NULL },
};

MsgPlayer::MsgPlayer()
{
  m_paraTbl = NULL;
  m_parser = NULL;
}

bool MsgPlayer::init(uint8_t id)
{
  m_id = id;
  if(m_bPreClear)
    LedMat.fillRect(m_rx, m_ry, m_rw, m_rh, m_bg.c_str());
  
  return true;
}

void MsgPlayer::initValues()
{
  m_bPreClear = m_bPostClear = true;
  m_rx = m_ry = 0;
  m_rw = LedMat.width();
  m_rh = LedMat.height();
  m_bg = "#000000";
}

bool MsgPlayer::update()
{
  if(m_bPostClear)
    LedMat.fillRect(m_rx, m_ry, m_rw, m_rh, m_bg.c_str());
  return false;
}
  

void MsgPlayer::resetParser(MsgJsonParser *parser)
{
  m_parser = parser;
  m_state = S_TOP_OBJECT;

  initValues();
}

bool MsgPlayer::findKey(const char *key, ParaTbl **pp)
{
  ParaTbl *p = paraTbl;

  while(p->name && strcmp(p->name,key)) p++;
  if(!p->name){
    p = m_paraTbl;
    if(p)
      while(p->name && strcmp(p->name,key))
	p++;
    if(!p || !p->name)
      return false;
  }
  if(pp)
    *pp = p;
  return true;
}

bool MsgPlayer::handleBeginHash()
{
  return m_parser->error("syntax error %s:%d",__FUNCTION__,__LINE__);
}

bool MsgPlayer::handleEndHash()
{
  if(m_state == S_TOP_OBJECT){
    m_state = S_END;
    return true;
  } else {
    return m_parser->error("syntax error %s:%d",__FUNCTION__,__LINE__);
  }
}

bool MsgPlayer::handleBeginArray()
{
  return m_parser->error("syntax error %s:%d",__FUNCTION__,__LINE__);
}

bool MsgPlayer::handleEndArray()
{
  return m_parser->error("syntax error %s:%d",__FUNCTION__,__LINE__);
}

bool MsgPlayer::handleKey(const char *key)
{
  if(m_state == S_TOP_OBJECT){
    m_state = S_TOP_VALUE;
    if(findKey(key, &m_selectedParam)){
      return true;
    } else {
      m_selectedParam = NULL;
      return m_parser->warning("param:%s not defined", key);
    }
  } else 
    return m_parser->error("syntax error %s:%d state:%d",__FUNCTION__,__LINE__,
		  m_state);
}

bool MsgPlayer::handleValue(json_value_t *vp)
{
  if(m_state == S_TOP_VALUE){
    if(!m_selectedParam){
      m_state = S_TOP_OBJECT;
      return true;
    }
#if 0
    if(m_selectedParam->type != vp->type && vp->type != JVT_STRING)
      return m_parser->error("data type for %s not match",
			     m_selectedParam->name);
#endif
    
    char *p = (char *)this + m_selectedParam->offset;
    switch(m_selectedParam->type){
    case JVT_IGNORE:
        /* do nothing */
      break;
      
    case JVT_BOOLEAN:
      if(!m_parser->getBoolean(vp, (bool *)p)) return false;
      break;
      
    case JVT_INT:
      if(!m_parser->getInt(vp, (int *)p)) return false;
      break;
      
    case JVT_FLOAT:
      if(!m_parser->getFloat(vp, (float *)p)) return false;
      break;
	
    case JVT_STRING:
      *(String *)p = vp->sVal;
      break;
	
    default:
      return m_parser->error("syntax error %s:%d type:%d",
                               __FUNCTION__,__LINE__, vp->type);
    }
    m_state = S_TOP_OBJECT;
    return true;
  } else {
    return m_parser->error("syntax error %s:%d",__FUNCTION__,__LINE__);
  }
}
