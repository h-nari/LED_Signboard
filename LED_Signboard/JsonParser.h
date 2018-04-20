#ifndef _json_parser_h_
#define _json_parser_h_

#include <Arduino.h>
#include <SD.h>

enum json_value_type_t {
  JVT_NULL, JVT_IGNORE, JVT_BOOLEAN, JVT_INT, JVT_FLOAT, JVT_STRING
};

struct json_value_t {
  json_value_type_t type;
  union {
    bool bVal;
    int  iVal;
    float fVal;
  };
  const char *sVal;
};

#define UNGET_BUF_MAX 4

class JsonParser
{
 protected:
  String       m_str;
  json_value_t m_value;
  File         m_file;
  int          m_line;
  int          m_col;
  int          m_unget_buf[UNGET_BUF_MAX];
  int          m_unget_sp;
  char         m_errorBuf[80];

  String       m_filename;
  char         m_lineBuf[80];
  
 public:
  JsonParser();
  void reset();
  bool virtual parse_file(const char *path);
  bool error(const char *fmt, ...);
  bool warning(const char *fmt, ...);
  bool getBoolean(json_value_t *vp, bool *bp);
  bool getInt(json_value_t *vp, int *ip);
  bool getFloat(json_value_t *vp, float *fp);

 protected:

  virtual bool handleBeginHash(){};
  virtual bool handleEndHash(){};
  virtual bool handleBeginArray(){};
  virtual bool handleEndArray(){};
  virtual bool handleKey(const char *key){};
  virtual bool handleValue(json_value_t *value){};

  int  get_char();
  void ungetc(int c);
  int  blank_skip();
  bool parse_string(bool bKey = false);
  bool parse_number();
  bool parse_name();
  bool parse_object();
  bool parse_array();
  bool parse_value();
  void error0(const char *fmt, va_list ap);
};

#endif /* _json_parser_h_ */
