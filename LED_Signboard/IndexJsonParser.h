#ifndef _index_json_parser_h_
#define _index_json_parser_h_

#include "JsonParser.h"

class SignboardPlayer;
class IndexJsonParser : public JsonParser {
 protected:
  SignboardPlayer *m_pTarget;
  bool m_bRenum;
  File m_fRenum;
  int  m_cMsg;
  int  m_cKey;
  enum State {
    S_BEGIN,
    S_END,
    S_TOP_HASH,
    S_MSG,
    S_MSG_ARRAY,
    S_MSG_HASH,
    S_ID,
    S_ENABLE,
    S_TITLE,
    S_PLAYER,
  };
  State m_state;

 public:
  IndexJsonParser(SignboardPlayer *pTarget = NULL);
  void reset();
  bool renum(const char *path);
  
 protected:
  bool handleBeginHash()                override;
  bool handleEndHash()                  override;
  bool handleBeginArray()               override;
  bool handleEndArray()                 override;
  bool handleKey(const char *key)       override;
  bool handleValue(json_value_t *value) override;
};


#endif /* _index_json_parser_h_ */
