#include <time.h>
#include "my_strftime.h"

String my_strftime(const char *str)
{
  time_t t = time(NULL);
  struct tm *tm = localtime(&t);
  static const char *wd[7]={"日","月","火","水","木","金","土"};
  static const char *wd2[7]={"Sun","Mon","Tue","Wed","Thr","Fri","Sat"};
  String s;

  const char *p = str;
  while(*p){
    char c = *p++;
    if(c != '%')
      s += c;
    else {
      c = *p++;
      switch(c){
      case 'Y': s += tm->tm_year+1900; break;
      case 'm': s += tm->tm_mon+1;     break;
      case 'd': s += tm->tm_mday;      break;
      case 'H': s += tm->tm_hour;      break;
      case 'M': s += tm->tm_min;       break;
      case 'S': s += tm->tm_sec;       break;
      case 'a': s += wd[tm->tm_wday];  break;
      case 'A': s += wd2[tm->tm_wday]; break;
      case 0:
	s += '%';
	p--;
	break;
      default:
	s += '%';
	s += c;
	break;
      }
    }
  }
  return s;
}
