#ifndef _msg_player_h_
#define _msg_player_h_

#include "Humblesoft_LedMat.h"
#include "MsgJsonParser.h"

class MsgPlayer  {
 public:
  struct ParaTbl {
    const char *name;
    json_value_type_t type;
    size_t   offset;
  };
  ParaTbl *m_paraTbl;
  
 protected:
  enum State {
    S_START,
    S_END,
    S_TOP_OBJECT,
    S_TOP_VALUE,
  };
  uint8_t m_id;
  State m_state;
  MsgJsonParser *m_parser;
  ParaTbl *m_selectedParam;

 public:   // Common Parameters;
  bool m_bPreClear;		// 表示前クリア	
  bool m_bPostClear;		// 表示後クリア
  int  m_rx, m_ry, m_rw, m_rh;	// 描画領域
  String m_bg;			// 背景色
  
 public:
  MsgPlayer();

  virtual bool init(uint8_t id);
  virtual bool update();	// true:継続 flase: 終了
  virtual void resetParser(MsgJsonParser *parser);
  virtual void initValues();
  virtual bool handleBeginHash();
  virtual bool handleEndHash();
  virtual bool handleBeginArray();
  virtual bool handleEndArray();
  virtual bool handleKey(const char *key);
  virtual bool handleValue(json_value_t *value);
  
  
 protected:
  bool findKey(const char *key, ParaTbl **pp);
};



#endif /* _msg_player_h_ */
