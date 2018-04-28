#include "SignboardPlayer.h"
#define FS_NO_GLOBALS
#include "FS.h"
#include "ConfigJsonParser.h"
#include "IndexJsonParser.h"
#include "MsgJsonParser.h"
#include "utils.h"

ScrollText scrollBuf(&LedMat);

SignboardPlayer::SignboardPlayer()
{
  m_bStop        = false;
  m_moduleType   = LMMT64x32s16;
  m_moduleColumn = 1;
  m_moduleRow    = 1;
  m_bright       = 10;
  m_plane        = 4;
  m_ledMode      = 0;
  m_script[0]    = 0;
  m_cMsg = m_iMsg = 0;

  m_player = NULL;
  m_bOneShot = false;
}

bool SignboardPlayer::load()
{
  LedMat.clear();
  scrollBuf.clear();
  return load_config() && load_index();
}

bool SignboardPlayer::load_config()
{
  ConfigJsonParser parser(this);

  if(!parser.parse_file(CONFIG_FILE)) return false;
  LedMat.clear();
  LedMat.display();
  LedMat.setConf(m_moduleType, m_moduleColumn, m_moduleRow);
  LedMat.setBright(m_bright);
  LedMat.setPlane(m_plane);
  LedMat.setLedMode(m_ledMode);
  LedMat.clear();
  LedMat.display();
  return true;
}

bool SignboardPlayer::load_index()
{
  IndexJsonParser parser(this);
  char path[32];

  snprintf(path,sizeof path,"%s/%s",m_script, INDEX_FILE);
  if(!parser.parse_file(path)) return false;

  LedMat.clear();
  LedMat.display();
  m_iMsg = 0;
  m_player = NULL;
  return true;
}


void SignboardPlayer::makeMsgFile(char *buf, size_t bufLen, uint16_t id,
				  const char *ext)
{
  snprintf(buf, bufLen, MSG_PATH_PAT, m_script, (int)id, ext);
}

// m_iMsg番目のMsgを読み込み、playerをセット

bool SignboardPlayer::loadMsg(uint16_t id)
{
  char filename[32]; 
  MsgJsonParser parser;
#if 0
  Msg *p = &m_aMsg[m_iMsg]; 
  uint id = p->m_id;
#endif
  
  makeMsgFile(filename,sizeof filename, id, "jsn");
  
  if(!parser.parse_file(filename)) {
    return false;  
  }
  m_player = parser.getPlayer();
  
  return true;
}

void SignboardPlayer::update()
{
  if(m_bStop) return;
  
  if(!m_player && m_cMsg){
    if(m_iMsg >= m_cMsg) m_iMsg = 0;

    if(m_aMsg[m_iMsg].m_enable){
      uint id = m_aMsg[m_iMsg].m_id;
      if(!loadMsg(id)|| !m_player->init(id)){
	delay(500);
	m_iMsg++;
      }
    } else {
      m_iMsg++;
    }
  }

  if(m_player){
    if(!m_player->update()){
      if(m_bOneShot){
	m_bOneShot = false;
	m_bStop    = true;
      }
      m_player = NULL;
      m_iMsg++;
    }
  }
}

bool SignboardPlayer::dispMsg(uint id)
{
  if(!loadMsg(id)) return false;
  if(!m_player->init(id)) return false;
  m_bStop = false;
  m_bOneShot = true;
}

void SignboardPlayer::setFont(int idx)
{
  if(idx >= 0 && idx < m_cFont){
    FontEntry *p = m_aFont + idx;
    if(p->m_fontx)
      LedMat.setFont(p->m_fontx);
    else
      LedMat.setFont(p->m_gfxFont);
  }
}

void SignboardPlayer::setFont(ScrollText *st, int idx)
{
  if(idx >= 0 && idx < m_cFont){
    FontEntry *p = m_aFont + idx;
    if(p->m_fontx)
      st->setFont(p->m_fontx);
    else
      st->setFont(p->m_gfxFont);
  }
}

void SignboardPlayer::addGlcdFont( const char *name)
{
  if(m_cFont < FontMax - 1){
    m_aFont[m_cFont].m_name    = name;
    m_aFont[m_cFont].m_fontx   = NULL;
    m_aFont[m_cFont].m_gfxFont = NULL;
    m_cFont++;
  }
  else Serial.printf("font table overflow\n");
}

void SignboardPlayer::addFont(const GFXfont *font, const char *name)
{
  if(m_cFont < FontMax - 1){
    m_aFont[m_cFont].m_name    = name;
    m_aFont[m_cFont].m_fontx   = NULL;
    m_aFont[m_cFont].m_gfxFont = font;
    m_cFont++;
  }
  else Serial.printf("font table overflow\n");
}

void SignboardPlayer::addFont(Fontx *font, const char *name)
{
  if(m_cFont < FontMax - 1){
    m_aFont[m_cFont].m_name    = name;
    m_aFont[m_cFont].m_fontx   = font;
    m_aFont[m_cFont].m_gfxFont = NULL;
    m_cFont++;
  }
  else Serial.printf("font table overflow\n");
}

String SignboardPlayer::getFontListJson()
{
  String s;
  int i;

  s += "[";
  for(int i=0; i<m_cFont; i++){
    if(i > 0)
      s += ",";
    s += '"';
    s += m_aFont[i].m_name;
    s += '"';
  }
  
  s += "]";
  return s;
}

static bool isMsgFile(const char *header, const char *name, int *pId,
		      char ext[4])
{
  int headerLen = strlen(header);
  const char *p;
  int iVal,i;

  if(strncmp(header,name, headerLen)!=0) return false;

  p = name + headerLen;
  if(!isdigit(*p)) return false;

  iVal = 0;
  while(isdigit(*p))
    iVal = iVal * 10 + *p++ - '0';
  if(*p != '.') return false;

  p++;
  for(i=0; i<3,*p; i++)
    ext[i] = *p++;
  ext[i] = 0;

  if(pId) *pId = iVal;

  return true;
}


bool SignboardPlayer::renumber_msg()
{
  // rename　MSG_org.ext => TMP_org.ext

  File dir = SD.open(m_script);
  if(!dir) return error("open %s failed", m_script);
  dir.rewindDirectory();
  File f = dir.openNextFile();
  while(f){
    int  id;
    char ext[4];
    bool bMsgFile = isMsgFile(MSG_FILE_HEADER, f.name(), &id, ext);
    f.close();
    if(bMsgFile){
      char srcPath[20];
      char dstPath[20];
      snprintf(srcPath,sizeof srcPath, MSG_PATH_PAT, m_script, id, ext);
      snprintf(dstPath,sizeof dstPath, TMP_PATH_PAT, m_script, id, ext);
      if(!fileCopy(srcPath, dstPath))
	return error("fileCopy error");
    }
    f = dir.openNextFile();
  }
  dir.close();


  // rename  TMP_org.ext => MSG_new.ext

  dir = SD.open(m_script);
  if(!dir) return error("open %s failed(2)", m_script);
  dir.rewindDirectory();
  f = dir.openNextFile();
  
  while(f){
    int  id;
    char ext[4];
    bool bMsgFile = isMsgFile(TMP_FILE_HEADER, f.name(), &id, ext);
    f.close();
    if(bMsgFile){
      char srcPath[20];
      char dstPath[20];
      int nid;
      for(nid=0; nid<m_cMsg; nid++)
	if(m_aMsg[nid].m_id == id) break;
      if(nid >= m_cMsg){
	dir.close();
	return error(MSG_FILE " not found",id);
      }
      nid++;
      snprintf(srcPath,sizeof srcPath, TMP_PATH_PAT, m_script, id,  ext);
      snprintf(dstPath,sizeof dstPath, MSG_PATH_PAT, m_script, nid, ext);
      if(!fileCopy(srcPath, dstPath))
	return error("fileCopy error");
      if(!SD.remove(srcPath))
	return error("remove failed");
    }
    f = dir.openNextFile();
  }
  dir.close();
  
  // write index.jsn

  IndexJsonParser parser;
  char path[40];
  snprintf(path, sizeof path, "%s/%s", m_script, INDEX_FILE);
  return parser.renum(path);
}

