#include "IndexJsonParser.h"
#include "SignboardPlayer.h"
#include "utils.h"

IndexJsonParser::IndexJsonParser(SignboardPlayer *pTarget) : JsonParser()
{
  m_pTarget = pTarget;
  m_bRenum = false;
  m_cMsg = 0;
  reset();
}

void IndexJsonParser::reset()
{
  m_state = S_BEGIN;
}

bool IndexJsonParser::handleBeginHash()
{
  if(m_state == S_BEGIN){
    m_state = S_TOP_HASH;
    m_cMsg = 0;
    if(m_bRenum)
      m_fRenum.print('{');
  } else if(m_state == S_MSG_ARRAY){

    if(m_bRenum){
      if(m_cMsg > 0)
	m_fRenum.print(',');
      m_fRenum.print('{');
    }
    
    m_state = S_MSG_HASH;
    if(m_pTarget){
      if(m_pTarget->m_cMsg >= MsgMax)
	return error("msg overflow");
    } else
      m_cKey = 0;
  } else
    return error("syntax error %s:%d",__FUNCTION__,__LINE__);
  return true;
}

bool IndexJsonParser::handleEndHash()
{
  if(m_bRenum) m_fRenum.println("}");
  
  if(m_state == S_TOP_HASH) {
    m_state = S_END;
  } else if(m_state == S_MSG_HASH) {
    m_state = S_MSG_ARRAY;
    if(m_pTarget)
      m_pTarget->m_cMsg++;
    else
      m_cMsg++;
  }
  else
    return error("syntax error %s:%d",__FUNCTION__,__LINE__);
  return true;
}

bool IndexJsonParser::handleBeginArray()
{
  if(m_bRenum) m_fRenum.println("[");
  
  if(m_state == S_MSG) {
    m_state = S_MSG_ARRAY;
    if(m_pTarget)
      m_pTarget->m_cMsg = 0;
    else
      m_cMsg = 0;
  }
  else
    return error("syntax error %s:%d",__FUNCTION__,__LINE__);
  return true;
}

bool IndexJsonParser::handleEndArray()
{
  if(m_bRenum) m_fRenum.println("]");
  
  if(m_state == S_MSG_ARRAY) m_state = S_TOP_HASH;
  else
    return error("syntax error %s:%d",__FUNCTION__,__LINE__);
  return true;
}

bool IndexJsonParser::handleKey(const char *key)
{
  if(m_state == S_TOP_HASH){

    if(m_bRenum) {
      m_fRenum.print(' ');
      m_fRenum.print('"');
      m_fRenum.print(key);
      m_fRenum.print('"');
      m_fRenum.print(':');
    }
    
    if(strcmp(key,"msg")==0) m_state = S_MSG;
    else return error("bad key in top hash");
  } else if(m_state == S_MSG_HASH){

    if(m_bRenum) {
      if(m_cKey++ > 0)
	m_fRenum.print(',');
      m_fRenum.print(' ');
      m_fRenum.print('"');
      m_fRenum.print(key);
      m_fRenum.print('"');
      m_fRenum.print(':');
    }
    
    if(strcmp(key,"id")==0)          m_state = S_ID;
    else if(strcmp(key,"enable")==0) m_state = S_ENABLE;
    else if(strcmp(key,"title")==0)  m_state = S_TITLE;
    else if(strcmp(key,"player")==0) m_state = S_PLAYER;
    else return error("bad key:%s in msg hash",key);
  } else
    return error("syntax error %s:%d",__FUNCTION__,__LINE__);
  return true;
}

bool IndexJsonParser::handleValue(json_value_t *value)
{
  if(m_bRenum){
    m_fRenum.print(' ');
    if(m_state == S_ID)
      m_fRenum.print(m_cMsg + 1);
    else if(value->type != JVT_STRING) {
      m_fRenum.print(value->sVal);
    } else {
      m_fRenum.print('"');
      m_fRenum.print(value->sVal);
      m_fRenum.print('"');
    }
  }
  
  int iVal;
  switch(m_state){
  case S_ID:
    if(!getInt(value, &iVal)) return false;
    if(m_pTarget)
      m_pTarget->m_aMsg[m_pTarget->m_cMsg].m_id = iVal;
    break;
  case S_ENABLE:
    if(value->type != JVT_BOOLEAN) return error("boolean expected");
    if(m_pTarget)
      m_pTarget->m_aMsg[m_pTarget->m_cMsg].m_enable = value->bVal;
    break;
  case S_TITLE:
  case S_PLAYER:
    if(value->type != JVT_STRING) return error("string expected");
    break;
  default:
    return error("Bad state:%d",m_state);
  }
  m_state = S_MSG_HASH;
  return true;
}


bool IndexJsonParser::renum(const char *path)
{
  m_bRenum = true;
  m_fRenum = SD.open( TMP_INDEX_FILE, FILE_WRITE);
  if(!m_fRenum) return error("open %s failed.", TMP_INDEX_FILE);
  if(!parse_file(path))
    return error("renum: parse_file failed");
  m_fRenum.close();
  return fileCopy(TMP_INDEX_FILE, path) && SD.remove(TMP_INDEX_FILE);
}
