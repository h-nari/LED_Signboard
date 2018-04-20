#ifndef _parser_base_h_
#define _parser_base_h_

#define ERROR_MESSAGE_MAX_LENGTH	80

// #include <stdint.h>
#include <Arduino.h>

class ParserBase {
 protected:
  uint16_t m_nLine;
  uint16_t m_nColumn;
  bool     m_bError;
  char     m_sErrMes[ERROR_MESSAGE_MAX_LENGTH];
  
 public:
  
  ParserBase();	// Constructor
  
  virtual void reset();
  bool feed(uint8_t c);
  bool feedEnd();
  bool isError() { return m_bError; }
  const char *getErrorMessage(){ return m_sErrMes; }

 protected:
  bool error(const char *fmt, ...);
  virtual bool parse(char c) = 0;
  virtual bool parseEnd(){return !m_bError;};
};

#endif /* _parser_base_h_ */
