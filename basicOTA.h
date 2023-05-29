#ifndef __basicOTA__h__
#define __basicOTA__h__

#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "timers.h"

bool otaHandle(int) {
  ArduinoOTA.handle();
  return true;
}

void setupOTA() {
  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  // ArduinoOTA.setHostname("myesp32");
  ArduinoOTA.setHostname("HortaHNB");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      //Serial.println("Start updating " + type);
      Log.print("Start updating " + type + "\n");
    })
    .onEnd([]() {
      //Serial.println("\nEnd");
      Log.print("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      //Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      Log.print("Progress: "+String((progress / (total / 100)))+String("\r"));
    })
    .onError([](ota_error_t error) {
      //Serial.printf("Error[%u]: ", error);
      Log.print("Error[%u]: "+String(error));
      if (error == OTA_AUTH_ERROR) Log.print("Auth Failed");//Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Log.print("Begin Failed");//Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Log.print("Connect Failed");//Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Log.print("Receive Failed");//Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Log.print("End Failed");//Serial.println("End Failed");
    });

  ArduinoOTA.begin();
  //timer.every(60, otaHandle);
}

#endif
