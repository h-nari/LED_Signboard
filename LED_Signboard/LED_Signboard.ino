#define DEBUG_ESP_WIFI  1

#include "conf.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <stdlib.h>
#include <time.h>
#include "ota_util.h"

#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Fontx.h>
#include <Humblesoft_GFX.h>
#include <Humblesoft_LedMat.h>

#define FS_NO_GLOBALS
#include <FS.h>
#include "FsHandler.h"
#include "SpiffsHandler.h"
#include "SdHandler.h"

#include "SignboardPlayer.h"
#include "webServer.h"

#include <fontx/ILGH16XB.h>
#include <fontx/ILGZ16XB.h>
RomFontx fontx(ILGH16XB,ILGZ16XB);

#define JST 3600*9

uint8_t imgBuf[16*1024];

ESP8266WebServer server(80);
SignboardPlayer player;

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
	
	WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

	unsigned long tStart = millis();
  while (WiFi.status() != WL_CONNECTED) {
		unsigned long now = millis();
		if(now - tStart > 10000){
			Serial.printf("\nStatus:%d\n", WiFi.status());
			tStart = now;
			WiFi.disconnect();
			// ESP.restart();
			delay(1000);
			WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
		}
		else {
			delay(500);
			Serial.print(".");
		}
  }
  Serial.println();

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

	configTime( JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");
	webServer_init();
  server.begin();
  Serial.println("HTTP server started");

#if USE_OTA
	ota_init();
#endif

	LedMat.clear();
	LedMat.display();
}

void loop(void){
#if USE_OTA
  ArduinoOTA.handle();
#endif

	LedMat.checkSubcon();
  server.handleClient();
  player.update();

	// heap_check();
}

/*** Local variables: ***/
/*** tab-width:2 ***/
/*** End: ***/
