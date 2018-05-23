#ifndef _signboard_player_h_
#define _signboard_player_h_

#include <Humblesoft_LedMat.h>
#include <ScrollText.h>
#include "MsgPlayer.h"

#define CONFIG_FILE     "CONFIG.JSN"
#define INDEX_FILE      "INDEX.JSN"
#define TMP_INDEX_FILE  "INDEX.TMP"

#define MSG_FILE_HEADER "MSG"
#define MSG_FILE        "MSG%02d"
#define MSG_JSN		"MSG%02d.JSN"
#define MSG_TXT		"MSG%02d.TXT"
#define MSG_BMP		"MSG%02d.BMP"
#define MSG_PATH_PAT    "%s/MSG%02d.%s"
#define TMP_FILE_HEADER "TMP"
#define TMP_PATH_PAT    "%s/TMP%02d.%s"

#define MsgMax  50
#define FontMax 10

class MsgPlayer;

class SignboardPlayer {
  friend class ConfigJsonParser;
  friend class IndexJsonParser;
  
  struct Msg {
    uint   m_id;
    bool   m_enable;
  };

  struct FontEntry {
    String m_name;
    Fontx   *m_fontx;
    const GFXfont *m_gfxFont;
  };
  
 protected:
  bool         m_bStop; 
  bool         m_bOneShot;      // メッセージを１つ再生して停止する
  LMModuleType m_moduleType;
  uint8_t      m_moduleColumn;
  uint8_t      m_moduleRow;
  uint8_t      m_bright;
  uint8_t      m_plane;
  uint8_t      m_iMsg;		// 再生中のメッセージ
  uint8_t      m_cMsg;		// 書き込まれたメッセージの数
  uint8_t      m_ledMode;
  char         m_script[10];
  Msg          m_aMsg[MsgMax];
  
  MsgPlayer    *m_player;

  uint8_t      m_cFont;	// 登録済のfont数
  FontEntry    m_aFont[FontMax];
  
 public:
  SignboardPlayer ();
  void update();
  bool load();
  bool load_config();
  bool load_index();
  bool play(const char *path = NULL);
  bool getStop(){return m_bStop;}
  uint8_t getCurrentMsg(){return m_iMsg;}
  uint8_t getTotalMsg(){return m_cMsg;}
  uint8_t getCurrentMsgId(){ return m_aMsg[m_iMsg].m_id;}
  const char *getScript(){return m_script;}
  void setStop(bool b){m_bStop = b;}
  void setOneShot(bool b){m_bOneShot = b;}
  bool renumber_msg();

  const char *getScriptDir() {return m_script;}
  void makeMsgFile(char *buf, size_t bufLen, uint16_t id, const char *ext);
  void setFont(int idx);
  void setFont(ScrollText *st, int idx);
  void addGlcdFont( const char *name);
  void addFont(const GFXfont *font, const char *name);
  void addFont(Fontx *font, const char *name);
  String getFontListJson();
  bool dispMsg(uint id);

 protected:
  bool loadMsg(uint16_t id);

};

#endif /* _signboard_player_h_ */
