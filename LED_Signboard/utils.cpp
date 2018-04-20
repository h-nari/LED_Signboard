#include <Arduino.h>
#include <SD.h>
#include "utils.h"

bool error(const char *fmt, ...)
{
  va_list ap;
  char buf[80];

  va_start(ap, fmt);
  vsnprintf(buf, sizeof buf, fmt, ap);
  va_end(ap);
  Serial.println(buf);
  return false;
}


bool fileCopy(const char *src, const char *dst)
{
  Serial.printf("%s:%d %s -> %s\n",__FUNCTION__,__LINE__,src,dst);

  File ifp = SD.open(src, FILE_READ);
  if(!ifp) return error("%s not found", src);

  File ofp = SD.open(dst, O_WRITE|O_CREAT|O_TRUNC);
  if(!ofp) return error("open %s failed", dst);


  int n;
  do {
    uint8_t buf[80];
    n = ifp.read(buf, sizeof buf);
    if(n < 0)
      return error("read error:%d", n);
    uint8_t *p = buf;
    size_t left = n;
    while(left > 0){
      n = ofp.write(p, left);
      if(n < 0)
	return error("write error:%d", n);
      left -= n;
      p += n;
    }
  }
  while(n > 0);
  ofp.close();
  ifp.close();

  return true;
}

