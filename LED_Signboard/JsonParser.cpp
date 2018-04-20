#include "JsonParser.h"
#include <stdlib.h>

JsonParser::JsonParser()  
{
  reset();
}

void JsonParser::reset()
{
  m_str = "";
  m_line = 1;
  m_col  = 0;
  m_unget_sp = 0;
}

int JsonParser::get_char()
{
  if(m_unget_sp > 0){
    return m_unget_buf[--m_unget_sp];
  }
  int c = m_file.read();
  if(c == '\n'){
    m_col = 0;
    m_line++;
  } else {
    if(m_col < sizeof m_lineBuf-1)
      m_lineBuf[m_col] = c;
    m_col++;
  }
  return c;
}

void JsonParser::ungetc(int c)
{
  if(m_unget_sp < UNGET_BUF_MAX - 1)
    m_unget_buf[m_unget_sp++] = c;
}

int JsonParser::blank_skip()
{
 loop:
  
  int c = get_char();
  while(isspace(c))
    c = get_char();

  if(c == '/'){
    c = get_char();
    if(c == '*'){
      while(1){
	c = get_char();
	if(c == '*'){
	  c = get_char();
	  if(c == '/') break;
	}
	else if(c < 0)
	  return c;
      }
      goto loop;
    }
    else if(c == '/'){
      while(c != '\n'){
	c = get_char();
	if(c < 0) return c;
      }
    }
    else ungetc(c);
  }
  return c;
}

bool JsonParser::error(const char *fmt, ...)
{
  va_list ap;
  va_start(ap,fmt);
  error0(fmt,ap);
  va_end(ap);
  Serial.printf("Error:%s\n",m_errorBuf);
  return false;
}

bool JsonParser::warning(const char *fmt, ...)
{
  va_list ap;
  va_start(ap,fmt);
  vsnprintf(m_errorBuf, sizeof(m_errorBuf), fmt, ap);
  va_end(ap);
  Serial.printf("Warning:%s\n",m_errorBuf);
  return true;
}

void JsonParser::error0(const char *fmt, va_list ap)
{
  int i = m_col;
  int line = m_line;
  int col  = m_col;
  while(i < sizeof(m_lineBuf)-1){
    int c = get_char();
    if(c == '\n' || c < 0) break;
    m_lineBuf[i++] = c;
  }
  m_lineBuf[i] = 0;
  Serial.println(m_lineBuf);
  for(int i=0; i<col; i++)
    Serial.print(' ');
  Serial.println('^');
  Serial.printf("%s:%d:%d\n", m_filename.c_str(), line, col);
  
  vsnprintf(m_errorBuf, sizeof(m_errorBuf), fmt, ap);
}


bool JsonParser::parse_string(bool bKey)
{
  int c = blank_skip();
  
  if(c != '"') return error("'\"' expected. %c found",c);

  m_str = "";
  c = get_char();
  while(c != '"'){
    if(c == '\\'){
      c = get_char();
      if(c < 0)
	return error("unexpected EOF in string");
      else if(!isprint(c))
	return error("unprintable char %02x found in string",c & 0xff);
      else if(c == 'n')      m_str += '\n';
      else if(c == 'r')	m_str += '\r';
      else if(c == 'b') m_str += '\b';
      else if(c == 'f') m_str += '\f';
      else if(c == 't') m_str += '\t';
      else if(c == '"' || c == '\\')
	m_str += (char)c;
      else return error("\\%c not defined.",c);
    } else {
      m_str += (char)c;
    }
    c = get_char();
  }
  m_value.type = JVT_STRING;
  m_value.sVal = m_str.c_str();

  if(bKey)
    return handleKey(m_value.sVal);
  else
    return handleValue(&m_value);
}

bool JsonParser::parse_number()
{
  int c = blank_skip();
  bool bFloat = false;
  
  if(c == '-'){
    m_str = (char)c;
    c = get_char();
  }
  else m_str = "";
    
  while(isdigit(c)){
    m_str += (char)c;
    c = get_char();
  }
  if(c == '.'){
    m_str += (char)c;
    bFloat = true;
    c = get_char();
    while(isdigit(c)){
      m_str += (char)c;
      c = get_char();
    }
  }
  if(c == 'e' || c == 'E'){
    m_str += (char)c;
    c = get_char();
    if(c == '+' || c == '-'){
      m_str += (char)c;
      c = get_char();
    }
    if(!isdigit(c))
      return error("digit expected");
    while(isdigit(c)){
      m_str += (char)c;
      c = get_char();
    }
  }
  if(isalpha(c))
    return error("bad number");
  ungetc(c);

  if(bFloat){
    m_value.type = JVT_FLOAT;
    m_value.fVal = m_str.toFloat();
  } else {
    m_value.type = JVT_INT;
    m_value.iVal = m_str.toInt();
  }
  return handleValue(&m_value);
}

bool JsonParser::parse_name()
{
  int c = blank_skip();

  if(!isalpha(c) && c != '_') return error("Bad char in name");
  m_str = "";

  while(isalnum(c) || c == '_'){
    m_str += (char)c;
    c = get_char();
  }
  ungetc(c);

  if(m_str == "null") {
    m_value.type = JVT_NULL;
  } else if(m_str == "true"){
    m_value.type = JVT_BOOLEAN;
    m_value.bVal = true;
  } else if(m_str == "false"){
    m_value.type = JVT_BOOLEAN;
    m_value.bVal = false;
  } else {
    return error("undefined name:%s",m_str.c_str());
  }
  
  return handleValue(&m_value);
}

bool JsonParser::parse_object()
{
  int c = blank_skip();

  if(c != '{') return error("'{' expected");

  if(!handleBeginHash()) return false;

  while(1){
    c = blank_skip();
    if(c == '}') break;
    ungetc(c);
    
    if(!parse_string(true)) return false;

    c = blank_skip();
    if(c != ':') return error("':' expected,`%c(0x%02x)` found",c,c&0xff);
    if(!parse_value()) return false;
    c = blank_skip();
    if(c == '}') break;
    if(c != ',') return error("',' or '}' expected. %c found",c);
  }
  
  return handleEndHash();
}
  
bool JsonParser::parse_array()
{
  int c = blank_skip();
  
  if(c != '[') return error("'[' expected");

  if(!handleBeginArray()) return false;

  c = blank_skip();
  if(c != ']'){
    ungetc(c);
    while(1){
      if(!parse_value()) return false;
      c = blank_skip();
      if(c == ']') break;
      if(c != ',') return error("',' or ']' expected.");
    }
  }
  return handleEndArray();
}
  

bool JsonParser::parse_value()
{
  int c = blank_skip();

  if(c < 0) return error("Unexpected EOF");

  ungetc(c);
  
  if(c == '\"')
    return parse_string(false);
  else if(c == '-' || isdigit(c))
    return parse_number();
  else if(isalpha(c))
    return parse_name();
  else if(c == '{')
    return parse_object();
  else if(c == '[')
    return parse_array();
  else
    return error("Unexpected charactor:'%c' found before value",c);
}

bool JsonParser::parse_file(const char *path)
{
  bool b = false;

  m_filename = path;
  m_file = SD.open(path);

  if(m_file){
    b = parse_value();
    m_file.close();
  } else {
    Serial.printf("Error: %s not found.\n", path);
  }
  return b;
}


bool JsonParser::getBoolean(json_value_t *vp, bool *bp)
{
  bool b = false;
  if(vp->type == JVT_BOOLEAN)
    b = vp->bVal;
  else if(vp->type == JVT_STRING){
    if(strchr("to1",*vp->sVal))
      b = true;
  }
  else return error("boolean or string expected.");
  *bp = b;
  return true;
}

bool JsonParser::getInt(json_value_t *vp, int *ip)
{
  if(vp->type == JVT_INT)
    *ip = vp->iVal;
  else if(vp->type == JVT_STRING){
    char *ep;
    long v = strtol(vp->sVal, &ep, 10);
    if(ep == vp->sVal || *ep)
      return error("Bad integer string:%s", vp->sVal);
    *ip = v;
  }
  else return error("integer or string expected.");
  return true;
}

bool JsonParser::getFloat(json_value_t *vp, float *fp)
{
  if(vp->type == JVT_FLOAT)
    *fp = vp->fVal;
  else if(vp->type == JVT_INT)
    *fp = (float)vp->iVal;
  else if(vp->type == JVT_STRING){
    char *ep;
    float f = strtod(vp->sVal, &ep);
    if(ep == vp->sVal || *ep)
      return error("Bad float string:%s", vp->sVal);
    *fp = f;
  }
  else return error("float or string expected.");
  return true;
}




