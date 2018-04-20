#include "JsonParser.h"

class SignboardPlayer;
class ConfigJsonParser : public JsonParser {
 protected:
  SignboardPlayer *m_pTarget;
  enum State {
    S_BEGIN,
    S_END,
    S_TOP_HASH,
    S_MODULE_TYPE,
    S_MODULE_ROW,
    S_MODULE_COLUMN,
    S_BRIGHT,
    S_SCRIPT,
    S_PLANE,
    S_LEDMODE,
  };
  State m_state;

 public:
  ConfigJsonParser(SignboardPlayer *m_pSignboardPlayer);
  void reset();
  
 protected:
  bool handleBeginHash()                override;
  bool handleEndHash()                  override;
  bool handleBeginArray()               override;
  bool handleEndArray()                 override;
  bool handleKey(const char *key)       override;
  bool handleValue(json_value_t *value) override;
};
