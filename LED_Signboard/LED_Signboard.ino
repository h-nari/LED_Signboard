#define DEBUG_ESP_WIFI  1

#include "conf.h"

#include <Arduino.h>
#if USE_NETWORK
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "ota_util.h"

#define FS_NO_GLOBALS
#include <FS.h>
#include <FsHandler.h>
#include <SpiffsHandler.h>
#include <SdHandler.h> 

#include "webServer.h"
#endif

#include <stdlib.h>
#include <time.h>

#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Fontx.h>
#include <Humblesoft_GFX.h>
#include <Humblesoft_LedMat.h>

#include "SignboardPlayer.h"

#include <fontx/ILGH16XB.h>
#include <fontx/ILGZ16XB.h>
RomFontx fontx(ILGH16XB,ILGZ16XB);

uint8_t imgBuf[16*1024];

#if USE_NETWORK
ESP8266WebServer server(80);
#endif
SignboardPlayer player;
#if USE_NETWORK
bool bNetwork = true;
#endif

void setup(void){
  Serial.begin(115200);
	delay(100);
  Serial.println("\n\nReset:");

  LedMat.begin();
  LedMat.setImgBuf(imgBuf, sizeof(imgBuf));
	LedMat.setFont(&fontx);

	player.addFont(&fontx, "IPAゴシック16ドット");
	player.addGlcdFont("glcdfont 5x7dot 英字のみ");
	
	if(player.load())
		Serial.println("load succeeded.");
	else
		Serial.println("load failed");

	LedMat.println("SIGNBOARD");
	LedMat.display();
	
#if USE_NETWORK
	WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

	unsigned long tStart = millis();
  while (WiFi.status() != WL_CONNECTED) {
		unsigned long now = millis();
		if(now - tStart > 10000){
			Serial.printf("\nStatus:%d\n", WiFi.status());
			tStart = now;
			WiFi.disconnect();
      bNetwork = false;
      break;
		}
		else {
			delay(500);
			Serial.print(".");
		}
  }
  Serial.println();

  if(bNetwork){
#if USE_FIX_IP
    WiFi.config(IP_ADDR, GATEWAY, NETMASK, DNS_SERVER);
#endif
    Serial.print("IP ADDRESS:  ");
    Serial.println(WiFi.localIP());
    uint8_t mac[6];
    WiFi.macAddress(mac);
    Serial.print("MAC ADDRESS:");
    for(int i=0; i<6; i++)
		Serial.printf("%c%02x",i>0 ?'-':' ',mac[i]);
    Serial.println();

    configTzTime( "JST-9", "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");
    webServer_init();
    server.begin();
    Serial.println("HTTP server started");

#if USE_OTA
    ota_init();
#endif
  } else {
    Serial.println("network is not available");
  }
#else  // USE_NETWORK
  Serial.println("no network is used");
#endif // USE_NETWORK
  
	LedMat.clear();
	LedMat.display();
}

void loop(void){
	LedMat.checkSubcon();
  player.update();

#if USE_NETWORK
  if(bNetwork){
#if USE_OTA
    ArduinoOTA.handle();
#endif
    server.handleClient();
  }
#endif
	// heap_check();
}

/*** Local variables: ***/
/*** tab-width:2 ***/
/*** End: ***/
