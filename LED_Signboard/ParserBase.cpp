#include "ParserBase.h"

ParserBase::ParserBase()
{
  reset();
}

void ParserBase::reset()
{
  m_nLine      = 1;
  m_nColumn    = 0;
  m_bError     = false;
  m_sErrMes[0] = 0;
}

bool ParserBase::feed(uint8_t c)
{
  if(m_bError)
    return false;
  
  if(c == '\n') {
    m_nLine++;
    m_nColumn = 0;
  } else {
    m_nColumn++;
  }
  return parse(c);
}

bool ParserBase::feedEnd()
{
  if(m_bError)
    return false;

  return parseEnd();
}


bool ParserBase::error(const char *fmt, ...)
{
  va_list ap;
  char *p = m_sErrMes;
  uint16_t left = sizeof(m_sErrMes) - 1;
  int len;

  snprintf(p,left,"%d:%d ", m_nLine, m_nColumn);
  len = strlen(p);
  
  va_start(ap,fmt);
  vsnprintf(p+len, left-len, fmt, ap);
  va_end(ap);
  m_bError = true;
  return false;
}
