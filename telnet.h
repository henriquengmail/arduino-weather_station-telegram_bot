#ifndef __logger__h__
#define __logger__h__

#include <TelnetStream.h>
#include <AsyncTelegram2.h>
#include <HTTPClient.h>

#define Serial_print(arg) (Serial.print(arq); TelnetStream.print(arg))
#define Serial_println(arg) (Serial.println(arq); TelnetStream.println(arg))

#define SECS_2020 1577836800

class Logger {
  bool toserial;
  bool totelnet;
  bool tobot;
  bool toweb;
  AsyncTelegram2 *bot;
  TBMessage msg;
public:
  Logger(uint32_t baud) {
    Serial.begin(baud);
    toserial=true;
    totelnet=false;
    tobot=false;
    toweb=false;
  }
  bool setSerial(bool f) {
    if (f) {
      toserial=true;
    } else {
      toserial=false;
    }
    return toserial;
  }
  bool setTelnet(bool f) {
    if (f) {
      TelnetStream.begin();
      totelnet=true;
    } else {
      totelnet=false;
    }
    return totelnet;
  }
  bool setWeb(bool f) {
    if (f) {
      toweb=true;
    } else {
      toweb=false;
    }
    return toweb;
  }
  bool setBot(bool f, AsyncTelegram2 * _bot) {
    if (f) {
      tobot=true;
      bot=_bot;
    } else {
      tobot=false;
    }
    return tobot;
  }
  bool getSerial() {
    return toserial;
  }
  bool getTelnet() {
    return totelnet;
  }
  bool getBot() {
    return tobot;
  }
  bool getWeb() {
    return toweb;
  }
  void sendWeb(String str) {
    StaticJsonDocument<120> payload;
    time_t rawtime;
    time (&rawtime);
    struct tm * timeinfo;
    timeinfo = localtime (&rawtime);
    char buffer[20];
    strftime(buffer,20,"%F %T",timeinfo);
    //String s=String("\"")+buffer+String("\"");
    String s=buffer;
    payload["time"] = s;
    payload["log"] = str;
    String out;
    serializeJson(payload, out);
    WiFiClient client;
    HTTPClient http;
    if (http.begin(client, "http://35.206.71.241:8000/api/Webhook")) {
    } else {
      Serial.println("http.begin fail");
    }
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(out);
    if (httpResponseCode!=201) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
  void print(String str) {
    if (toserial) {
      Serial.print(str); 
    } 
    if (totelnet) {
      TelnetStream.print(str);
    }
    if (tobot) {
      //TBMessage msg;
      //msg.messageType=MessageText;
      //bot->sendMessage(msg, str);
    }
    if (toweb) {
      sendWeb(str);
    }
  }
};

Logger Log(115200);

void setupTelnetStream() {
  configTime(-3*60*60,0, "time.google.com", "time.windows.com", "pool.ntp.org");
  yield();
  time_t now = time(nullptr);
  while (now < SECS_2020) {
    delay(100);
    now = time(nullptr);
  }
  uptime=now-millis()/1000;
  //Serial.print("IP: ");
  //Serial.println(WiFi.localIP());
  Log.print(String("IP: ")+WiFi.localIP().toString()+"\n");
  TelnetStream.begin();
}
#endif
