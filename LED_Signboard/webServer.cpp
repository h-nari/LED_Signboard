#include "conf.h"
#include "webServer.h"
#include <Humblesoft_LedMat.h>
#include <MoviePlayer.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <SD.h>
#define FS_NO_GLOBALS
#include <FS.h>
// #include "FsHandler.h"
// #include "SpiffsHandler.h"
#include "SdHandler.h"
#include <time.h>
#include "SignboardPlayer.h"

static void list_script();
static void new_script();
static void delete_script();
static void delete_msgs();
static void copy_script();
static void movie_list();

extern ESP8266WebServer server;
extern SignboardPlayer player;

#if USE_SDFS
SdHandler sdHandler("sd", "", true, true);
#endif

static bool auth_check()
{
#if USE_AUTH
  if(!server.authenticate(WWW_USER, WWW_PASSWD)){
    server.requestAuthentication();
    Serial.printf("%s:%d auth NG\n",__FUNCTION__,__LINE__);
    return false;
  }
#endif
  return true;
}

static const char *browser_name()
{
  String ua = server.header("User-Agent");
  const char *name = "unknown";
  
  ua.toLowerCase();
  const char *u = ua.c_str();
  if(strstr(u,"msie"))         name = "ie";
  else if(strstr(u,"trident")) name = "ie";
  else if(strstr(u,"edge"))    name = "edge";
  else if(strstr(u,"chrome"))  name = "chrome";
  else if(strstr(u,"safari"))  name = "safari";
  else if(strstr(u,"firefox")) name = "firefox";
  else if(strstr(u,"opera"))   name = "opera";

  Serial.printf("%s(%d) %s : %s\n",__FUNCTION__,__LINE__,name, u);

  return name;
}

static bool browser_check()
{
  const char *b = browser_name();

  if(strcmp(b,"chrome")==0 || strcmp(b,"firefox")==0)
    return true;

  server.send(200,"text/plain;charset=UTF-8",
              F("ブラウザはchrome, firefoxのみサポートされています。"));
  return false;
}

static void sendOK()
{
  server.send(200,F("text/plain"),F(""));
}

static void send_file(const char *header,const char *name,
		      const char *content_type, bool ignoreNotFound = false)
{
  if(!auth_check()) return;
  if(!browser_check()) return;
  
  char path[40];
  String mes;
  snprintf(path,sizeof path,"%s/%s",header,name);
  
  File f = SD.open(path);
  if(!f) {
    if(ignoreNotFound)
      sendOK();
    else {
      mes = path;
      mes += " not found.";
      server.send(406,"text/plain", mes);
    }
  } else {
    server.streamFile(f, content_type);
    f.close();
  }
}

static void handle_file_upload()
{
  if(!auth_check()) return;
  
  static File f;
  static unsigned long tStart,tUpdated;
  unsigned long now = micros();
  HTTPUpload& upload = server.upload();
  char name[16];

  if(upload.status == UPLOAD_FILE_START){
    String path;

    if(server.uri() == "/upload_file"){
      path = upload.filename;
    } else if(server.uri() == "/upload_config"){
      path = "config.jsn";
    } else if(server.uri() == "/upload_index"){
      path = player.getScriptDir();
      path += "/";
      path += INDEX_FILE;
    } else if(server.uri() == "/upload_msg"){
      snprintf(name,sizeof name, MSG_JSN, upload.filename.toInt());
      path = player.getScriptDir();
      path += "/";
      path += name;
    } else if(server.uri() == "/upload_txt"){
      snprintf(name,sizeof name, MSG_TXT, upload.filename.toInt());
      path = player.getScriptDir();
      path += "/";
      path += name;
    } else if(server.uri() == "/upload_bmp"){
      snprintf(name,sizeof name, MSG_BMP, upload.filename.toInt());
      path = player.getScriptDir();
      path += "/";
      path += name;
    } else if(server.uri() == "/upload_file"){
      path += player.getScriptDir();
      path += "/";
      path += upload.filename;
    }
    else return;
    
    Serial.printf("%s path:%s\n",server.uri().c_str(),path.c_str());
    f = SD.open(path, FILE_WRITE);
    if(!f) Serial.printf("open %s failed.\n", path.c_str());
    tStart = tUpdated = now;
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(f) f.write(upload.buf, upload.currentSize);
    if(now - tUpdated > 1000){
      tUpdated = now;
      uint32_t t = upload.totalSize;
      Serial.printf("upload: write %dbyte %lumsec\n",t ,(now - tStart)/1000);
    }
  } else if(upload.status == UPLOAD_FILE_END){
    if(f) f.close();
    uint32_t t = upload.totalSize;
    Serial.printf("Upload: END, Size: %ubyte %lumsec\n", t,(now-tStart)/1000);
  }
}


void webServer_init()
{
  // server.addHandler(&fsHandler);
#if USE_SDFS
  server.addHandler(&sdHandler);
#if USE_AUTH
  sdHandler.setAuth(WWW_USER, WWW_PASSWD);
#endif
  if(LedMat.sdEnable())
    sdHandler.enable();
#endif
  
  const char *headers[] = { "User-Agent" };
  server.collectHeaders( headers, 1);
  
  server.on("/",[]{
      send_file("template","index.htm","text/html;charset=UTF-8");});
  server.on("/list", []{
      send_file("template","list.htm","text/html;charset=UTF-8");});
  server.on("/edit", []{
      send_file("template","edit.htm","text/html;charset=UTF-8");});
  server.on("/config", []{
      send_file("template","config.htm","text/html;charset=UTF-8");});
  server.on("/upload_config", HTTP_POST,
	    [](){sendOK();}, handle_file_upload);
  server.on("/upload_index", HTTP_POST,
	    [](){sendOK();}, handle_file_upload);
  server.on("/upload_list", HTTP_POST,
	    [](){sendOK();}, handle_file_upload);
  server.on("/upload_msg", HTTP_POST,
	    [](){sendOK();}, handle_file_upload);
  server.on("/upload_txt", HTTP_POST,
	    [](){sendOK();}, handle_file_upload);
  server.on("/upload_bmp", HTTP_POST,
	    [](){sendOK();}, handle_file_upload);
  server.on("/upload_file", HTTP_POST,
	    [](){sendOK();}, handle_file_upload);

  server.on("/api", []{
      if(!auth_check()) return;
      
      char name[16];
      String cmd = server.arg("cmd");
      String script_str;
      const char *script;

      if(server.hasArg("script")){
        script_str = server.arg("script");
        script = script_str.c_str();
      } else{
        script = player.getScriptDir();
      }
      
      Serial.printf("%s:%d cmd:%s\n",__FUNCTION__,__LINE__,cmd.c_str());

      if(cmd.length() < 1)
	server.send(406,"text/plain", "paramter cmd not found");
      else if(cmd == "read_config")
	send_file("","config.jsn","text/json");
      else if(cmd == "read_index")
	send_file( script, INDEX_FILE, "text/json;charset=UTF-8");
      else if(cmd == "read_msg"){
	snprintf(name,sizeof name, MSG_JSN,server.arg("id").toInt());
	send_file( script, name, "text/json;charset=UTF-8");
      }
      else if(cmd == "read_txt"){
	snprintf(name,sizeof name, MSG_TXT,server.arg("id").toInt());
	send_file( script, name, "text/plain");
      }
      else if(cmd == "read_bmp"){
	snprintf(name,sizeof name, MSG_BMP,server.arg("id").toInt());
	send_file( script, name, "image/bitmap");
      }
      else if(cmd == "font_list"){
	String s = player.getFontListJson();
	server.send(200,"text/json;charset=UTF-8", s);
      } else if(cmd == "info"){
	String s = "{";
	s += F("\"freeHeap\":");
	s += ESP.getFreeHeap();
	s += F(",\"freeSketchSpace\":");
	s += ESP.getFreeSketchSpace();
	s += F(",\"sketchSize\":");
	s += ESP.getSketchSize();
	s += F(",\"chipId\":");
	s += ESP.getChipId();
	s += F(",\"currentMsg\":");
	s += player.getCurrentMsg();
	s += F(",\"totalMsg\":");
	s += player.getTotalMsg();
	s += F(",\"stop\":");
	s += player.getStop();
	s += F("}");
	server.send(200, "text/json", s);
      }
      else if(cmd == "reload_config"){
	if(player.load())
	  sendOK();
	else
	  server.send(406,"text/plain","reload_config failed.");
      }
      else if(cmd == "reload_index"){
	if(player.load_index())
	  sendOK();
	else
	  server.send(406,"text/plain","reload_index failed");
      }
      else if(cmd == "disp_msg"){
	if(!server.hasArg(F("id")))
	  server.send(406,"text/plain","id parameter needed");
	else {
	  int id = server.arg(F("id")).toInt();

	  if(server.arg(F("clear")).toInt()){
	    LedMat.clear();
	    LedMat.display();
	  }
	  
	  if(player.dispMsg((uint)id))
	    sendOK();
	  else
	    server.send(406,"text/plain","dispMsg failed.");
	}
      }
      else if(cmd == "stop"){
	if(server.hasArg(F("clear"))){
	  int clear = server.arg(F("clear")).toInt();
	  if(clear){
	    LedMat.clear();
	    LedMat.display();
	  }
	}
	player.setStop(true);
	sendOK();
      }
      else if(cmd == "start"){
	if(server.arg(F("clear")).toInt())
	  LedMat.clear();
	if(server.arg(F("reload")).toInt())
	  player.load_index();
	player.setStop(false);
	sendOK();
      }
      else if(cmd == "restart"){
	sendOK();
	delay(500);
	ESP.restart();
      }
      else if(cmd == "delete_msg")	delete_msgs();
      else if(cmd == "list_script")	list_script();
      else if(cmd == "new_script")	new_script();
      else if(cmd == "delete_script")   delete_script();
      else if(cmd == "copy_script")     copy_script();
      else if(cmd == "renum_msg"){
	if(!player.renumber_msg())
	  server.send(406,"text/plain", "renum_msg failed");
	else
	  send_file(player.getScriptDir(), INDEX_FILE,
                    "text/json;charset=UTF-8");
      }
      else if(cmd == "movie_list")      movie_list();
      else server.send(406,"text/plain", "cmd not defined");
    });

  server.onNotFound([]{
      const char *ct = "text/plain";

      if(!auth_check()) return;

      if(server.uri().endsWith(".js"))
	ct = "text/json";
      else if(server.uri().endsWith(".css"))
	ct = "text/css";
      else if(server.uri().endsWith(".css"))
	ct = "image/vnd.microsoft.icon";
      send_file("static",server.uri().substring(1).c_str(),ct);
    });
}

void heap_check()
{
  static unsigned long tChecked;
  static uint32_t heap0;
  unsigned long now = millis();
  uint32_t heap = ESP.getFreeHeap();
  
  if(now - tChecked > 60000 || heap0 != heap){
    time_t t;
    struct tm *tm;

    t = time(NULL);
    tm = localtime(&t);
    Serial.printf("%02d:%02d:%02d(%10u) heap:%u\n",
		  tm->tm_hour, tm->tm_min, tm->tm_sec,
		  ESP.getCycleCount(),heap);
    tChecked = now;
    heap0 = heap;
  }
}

static void list_script()
{
  String s = "[";
  File dir = SD.open("/");
  dir.rewindDirectory();
  File f = dir.openNextFile();
  int cnt = 0;
  while(f){
    if(f.isDirectory()){
      char path[40];
      snprintf(path,sizeof path,"%s/%s", f.name(), INDEX_FILE);
      if(SD.exists(path)){
	if(cnt++ > 0)
	  s += ',';
	s += '"';
	s += f.name();
	s += '"';
      }
    }
    f.close();
    f = dir.openNextFile();
  }
  dir.close();
  s += "]";
  server.send(200, "text/json", s);
}

static void new_script()
{
  String name = server.arg("script");

  if(name.length() < 1){
    server.send(406,"text/plain", "name parameter not found.");
  } else if(SD.exists((char *)name.c_str())) {
    server.send(406,"text/plain", "name already exists");
  } else if(!SD.mkdir((char*)name.c_str())){
    server.send(406,"text/plain", "mkdir failed.");
  } else {
    char path[40];
    snprintf(path, sizeof path,"/%s/%s" , name.c_str(), INDEX_FILE);
    File f = SD.open(path, FILE_WRITE);
    if(!f)
      server.send(404,"text/plain", "open index file failed");
    else {
      f.print("{ \"msg\": [] }\n");
      f.close();
    }
    list_script();
  }
}

static bool delete_msg(int id)
{
  const char *script = player.getScriptDir();
  File dir = SD.open(script);
  if(!dir){
    Serial.printf("SD.open(%s) failed.\n",script);
    return false;
  }
  
  char msg[16];
  snprintf(msg, sizeof msg, MSG_FILE ".", id);
  dir.rewindDirectory();
  File f = dir.openNextFile();
  int err = 0;
  while(f){
    char path[40];

    int match = !strncmp(msg, f.name(), 6);
    snprintf(path,sizeof path,"%s/%s", script, f.name());
    Serial.printf("%s:%d match %d msg:%s file:%s\n",__FUNCTION__,__LINE__,
		  match, msg,f.name());
    f.close();
    f = dir.openNextFile();
    if(match){
      Serial.printf("%s:%d remove %s\n",__FUNCTION__,__LINE__, path);
      if(!SD.remove(path)){
	Serial.printf("remove %s failed.\n", path);
	err++;
      }
    }
  }
  dir.close();
  return err == 0;
}

static void delete_msgs()
{
  String id_list = server.arg("id");
  const char *p = id_list.c_str();

  Serial.printf("%s:%d id:%s\n",__FUNCTION__,__LINE__,id_list.c_str());
  
  if(id_list.length() == 0){
    server.send(406,"text/plain", "id parameter not found.");
    return;
  }
  
  while(true){
    char *next;
    Serial.printf("%s:%d p:%s\n",__FUNCTION__,__LINE__,p);
    int id = strtol(p, &next, 10);
    if(next == p) {
      server.send(404,"text/plain", "Bad id data"); 
      return;
    }
    
    if(!delete_msg(id)){
      server.send(404,"text/plain", "delete_msg failed"); 
      return;
    }
    else if(*next == 0) break;
    else if(*next != ','){
      server.send(404,"text/plain", "Bad id list");
      return;
    }
    else p = next+1;
  }
  sendOK();
}

static void delete_script()
{
  String script = server.arg("script");
  if(script.length() == 0)
    server.send(406,"text/plain", "script parameter not found");
  else if(!SD.exists(script))
    server.send(406,"text/plain", "script not exist");
  else {
    int ec = 0;
    File dir = SD.open(script);
    dir.rewindDirectory();
    File f = dir.openNextFile();
    while(f){
      char path[40];
      snprintf(path, sizeof path, "%s/%s", script.c_str(), f.name());
      f.close();
      f = dir.openNextFile();
      Serial.printf("%s(%d) remove %s\n",__FUNCTION__,__LINE__,path);
      if(!SD.remove(path)){
        ec++;
        Serial.printf("remove %s failed\n",path);
      }
    }
    Serial.printf("%s(%d) rmdir %s\n",__FUNCTION__,__LINE__,script.c_str());
    if(!ec && !SD.rmdir(script)){
      ec++;
      Serial.printf("rmdir %s failed\n",script.c_str());
    }
    dir.close();
    if(ec)
      server.send(500,"text/plain","delete_script failed");
    else
      list_script();
  }
}

static void copy_script()
{
  String strSrc = server.arg("src");
  String strDst = server.arg("dst");
  String strDel = server.arg("delete");
  const char *src = strSrc.c_str();
  const char *dst = strDst.c_str();
  int bDel = server.arg("delete").equals("true");

  Serial.printf("%s(%d) src:%s dst:%s bDel:%d strDel:%s\n",
                __FUNCTION__,__LINE__,src,dst,bDel, strDel.c_str());
  
  if(*src == 0)
    server.send(406,"text/plain", "src parameter not found");
  else if(*dst == 0)
    server.send(406,"text/plain", "dst parameter not found");
  else if(!SD.exists(src))
    server.send(406,"text/plain", "src not exist");
  else if(!SD.mkdir(dst))
    server.send(406,"text/plain", "dst already exist");
  else {
    int ec = 0;
    File dir = SD.open(src);
    dir.rewindDirectory();
    File f = dir.openNextFile();
    while(f){
      char src_path[40],dst_path[40];
      snprintf(src_path, sizeof dst_path, "%s/%s", src, f.name());
      snprintf(dst_path, sizeof dst_path, "%s/%s", dst, f.name());
      Serial.printf("%s(%d) copy %s -> %s\n",__FUNCTION__,__LINE__,
                    src_path, dst_path);

      File d = SD.open(dst_path, FILE_WRITE);
      if(d){
        int n,left;
        char *p,buf[256];

        while(1) {
          left = f.readBytes(buf, sizeof buf);
          if(left == 0) break;
          if(left < 0){
            Serial.printf("%s(%d) readBytes error:%d\n",
                          __FUNCTION__,__LINE__, left);
            ec++;
            break;
          }
          p = buf;
          while(left > 0){
            n = d.write(p, left);
            if(n < 0){
              ec++;
              Serial.printf("%s(%d) writeBytes error:%d\n",
                            __FUNCTION__,__LINE__,n);
              break;
            }
            left -= n;
            p += n;
          }
        }
        d.close();
      } else {
        Serial.printf("%s(%d) open %s failed\n",__FUNCTION__,__LINE__,
                      dst_path);
        ec++;
      }
      f.close();
      f = dir.openNextFile();
      if(bDel && !SD.remove(src_path)){
        ec++;
        Serial.printf("remove %s failed\n",src_path);
      }
    }

    if(bDel){
      Serial.printf("%s(%d) rmdir %s\n",__FUNCTION__,__LINE__, src);
      if(!ec && !SD.rmdir(src)){
        ec++;
        Serial.printf("rmdir %s failed\n",src);
      }
    }
    dir.close();
    if(ec)
      server.send(500,"text/plain","copy_script failed");
    else
      list_script();
  }
}

#define MovieDir "/movie"

void movie_list()
{
  File dir = SD.open(MovieDir);
  if(!dir){
    server.send(406,"movie dir not found");
    return;
  } 

  File f = dir.openNextFile();
  String s = "[\n";
  while(f){
    MoviePlayer mp;
    char path[40];
    snprintf(path, sizeof path, "%s/%s", MovieDir, f.name());

    if(mp.begin(path)){
      hlm3_header_t *h = mp.header();
      s += " { \"name\":\"";
      s += f.name();
      s += "\", \"frames\":";
      s += h->frames;
      s += ", \"num\":";
      s += h->fps_numerator;
      s += ", \"deno\":";
      s += h->fps_denominator;
      s += '}';
      f = dir.openNextFile();
      if(f) s += ",";
      s += '\n';
      mp.close();
    }
    else
      f = dir.openNextFile();
  }
  s += "]";

  server.send(200, "text/json", s);
}



  
