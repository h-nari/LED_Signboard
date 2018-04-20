#ifndef _msg_json_parser_h_
#define _msg_json_parser_h_

#include "JsonParser.h"

class MsgPlayer;

class MsgJsonParser : public JsonParser {
 protected:
  MsgPlayer *m_player;
  enum State {
    S_START,
    S_END,
    S_TOP_HASH,
    S_TITLE_VALUE,
    S_PLAYER_VALUE,
  };
  State m_state;
  String m_key;
  
 public:
  MsgJsonParser();
  void reset();
  MsgPlayer *getPlayer() { return m_player;};
  
 protected:
  bool handleBeginHash()                override;
  bool handleEndHash()                  override;
  bool handleBeginArray()               override;
  bool handleEndArray()                 override;
  bool handleKey(const char *key)       override;
  bool handleValue(json_value_t *value) override;
};


#endif /* _msg_json_parser_h_ */
