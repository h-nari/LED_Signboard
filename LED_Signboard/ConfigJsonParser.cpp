#include "ConfigJsonParser.h"
#include "SignboardPlayer.h"

ConfigJsonParser::ConfigJsonParser(SignboardPlayer *pTarget) : JsonParser()
{
  m_pTarget = pTarget;
  reset();
}

void ConfigJsonParser::reset()
{
  JsonParser::reset();
  m_state = S_BEGIN;
}

bool ConfigJsonParser::handleBeginHash()
{
  if(m_state == S_BEGIN){
    m_state = S_TOP_HASH;
    return true;
  } else {
    return error("syntax error %s:%d",__FUNCTION__,__LINE__);
  }
}

bool ConfigJsonParser::handleEndHash()
{
  if(m_state == S_TOP_HASH){
    m_state = S_END;
    return true;
  } else {
    return error("syntax error %s:%d",__FUNCTION__,__LINE__);
  }
}

bool ConfigJsonParser::handleBeginArray()
{
  return error("syntax error %s:%d",__FUNCTION__,__LINE__);
}

bool ConfigJsonParser::handleEndArray()
{
  return error("syntax error %s:%d",__FUNCTION__,__LINE__);
}

bool ConfigJsonParser::handleKey(const char *key)
{
  if(m_state != S_TOP_HASH)
    return error("syntax error %s:%d",__FUNCTION__,__LINE__);
  else if(strcmp(key,"moduleType")==0)
    m_state = S_MODULE_TYPE;
  else if(strcmp(key,"moduleRow")==0)
    m_state = S_MODULE_ROW;
  else if(strcmp(key,"moduleColumn")==0)
    m_state = S_MODULE_COLUMN;
  else if(strcmp(key,"bright")==0)
    m_state = S_BRIGHT;
  else if(strcmp(key,"script")==0)
    m_state = S_SCRIPT;
  else if(strcmp(key,"plane")==0)
    m_state = S_PLANE;
  else if(strcmp(key,"ledMode")==0)
    m_state = S_LEDMODE;
  else 
    return error("param:%s not defined.",key);
  return true;
}

bool ConfigJsonParser::handleValue(json_value_t *vp)
{
  const char *t,*s;
  int iVal;

  switch(m_state){
  case S_MODULE_TYPE:
    if(vp->type != JVT_STRING) return error("string expected");
    t = vp->sVal;
    if(     strcmp(t,"64x32s16")==0) m_pTarget->m_moduleType = LMMT64x32s16;
    else if(strcmp(t,"32x32s16")==0) m_pTarget->m_moduleType = LMMT32x32s16;
    else if(strcmp(t, "32x16s8")==0) m_pTarget->m_moduleType = LMMT32x16s8;
    else return error("Bad module type:%s\n",t);
    break;
  case S_MODULE_ROW:
    if(!getInt(vp, &iVal)) return false;
    if(iVal > 0) 
      m_pTarget->m_moduleRow = iVal;
    else
      Serial.printf("Bad row value:%d\n", iVal);
    break;
  case S_MODULE_COLUMN:
    if(!getInt(vp, &iVal)) return false;
    if(iVal > 0)
      m_pTarget->m_moduleColumn = iVal;
    else
      Serial.printf("Bad column value:%d\n", iVal);
    break;
  case S_BRIGHT:
    if(!getInt(vp, &iVal)) return false;
    if(iVal >= 0 && iVal <= 100)
      m_pTarget->m_bright = iVal;
    else
      Serial.printf("Bad bright value:%d\n", iVal);
    break;
  case S_PLANE:
    if(!getInt(vp, &iVal)) return false;
    if(iVal > 0 && iVal <= 8)
      m_pTarget->m_plane = iVal;
    else
      Serial.printf("Bad plane value:%d\n", iVal);
    break;
  case S_SCRIPT:
    if(vp->type != JVT_STRING) return error("string expected");
    s = vp->sVal;
    if(strlen(s) > 8) return error("Too long script name, max 8 chars");
    strcpy(m_pTarget->m_script,s);
    break;
  case S_LEDMODE:
    if(!getInt(vp, &iVal)) return false;
    if(iVal >= 0)
      m_pTarget->m_ledMode = iVal;
    else
      Serial.printf("Bad LEDMODE value:%d\n", iVal);
    break;
  default:
    return error("Bad state:%d",m_state);
  }
  m_state = S_TOP_HASH;
  return true;
}

