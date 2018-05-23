#include <Arduino.h>

void dump_buf(void *buf0, int len, int offset)
{
  uint8_t *buf = (uint8_t *)buf0;
  int i,j;

  for(i=0; i<len; i+=16){
    Serial.printf("%04x:", i + offset);
    for(j=0;j < 16 && i+j<len; j++)
      Serial.printf(" %02x", buf[i+j]);
    Serial.printf(" : ");
    for(j=0;j < 16 && i+j<len; j++){
      int c = buf[i+j];
      Serial.printf("%c", c > ' ' && c < 0x7f ? c : '.'); 
    }
    Serial.println();
  }
}
