#ifndef __sendData__h__
#define __sendData__h__
// h23a20c13
#include "timers.h"
#include "soilHumidity.h"
#include "tempSensor.h"
#include "irrig.h"
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <math.h>

bool sendData(int ) {
  // next timer
  time_t rawtime;
  time (&rawtime);
  timer.at((rawtime/(5*60)+1)*5*60,sendData);
  // send data
  StaticJsonDocument<192> payload;
  struct tm * timeinfo;
  timeinfo = localtime (&rawtime);
  char buffer[20];
  strftime(buffer,20,"%F %T",timeinfo);
  //String s=String("\"")+buffer+String("\"");
  String s=buffer;
  payload["time"] = s;
  getTemperature();
  payload["AirTemp"] = tempSensor.temperature;
  payload["AirHumidity"] = tempSensor.humidity;
  getSoilResistance();
  payload["SoilResistance"] = SoilResitance;
  payload["Irrig"] = getIrrigStatus();
  payload["AbsHumidity"] = tempSensor.abshumidity;
  payload["dewPoint"] = tempSensor.dewPoint;
  payload["heatIndex"] = tempSensor.heatIndex;
  getGas();
  payload["gas"] = mqgas;
  payload["pressure"] = tempSensor.pressure;
  String out;
  serializeJson(payload, out);
  WiFiClient client;
  HTTPClient http;
  if (http.begin(client, "http://35.206.71.241:8000/api/Webhook")) {
    //Serial.println("http.begin ok");
    //Log.print(String("http.begin ok\n"));
  } else {
    //Serial.println("http.begin fail");
    Log.print(String("http.begin fail\n"));
  }
  http.addHeader("Content-Type", "application/json");
  //Serial.println(String("Posting:") + out + String("\n"));
  Log.print(String("Posting:") + out + String("\n"));
  int httpResponseCode = http.POST(out);
  if (httpResponseCode!=201) {
    //Serial.print("HTTP Response code: ");
    //Serial.println(httpResponseCode);
    Log.print(String("HTTP Response code: ")+String(httpResponseCode)+String("\n"));
  }
  http.end();
  // Grava os recerdes diários
  // Se virar o dia seta o recorde com dados da primeira hora, senão ajusta 
  // ((rawtime-3*60*60)/(60*60*24)+1)*60*60*24+3*60*60 - amanhã 0h
  // ((rawtime-3*60*60)/(60*60*24)+0)*60*60*24+3*60*60 - hoje 0h
  if (((rawtime-3*60*60)/(24*60*60))==((recorde.tmin.dt-3*60*60)/(24*60*60))) {
    if (recorde.tmin.val>tempSensor.temperature) {
      recorde.tmin.val= tempSensor.temperature;
      recorde.tmin.dt = rawtime;
    }
    if (recorde.tmax.val<tempSensor.temperature) {
      recorde.tmax.val= tempSensor.temperature;
      recorde.tmax.dt = rawtime;
    }
    if (recorde.hmin.val>tempSensor.humidity) {
      recorde.hmin.val= tempSensor.humidity;
      recorde.hmin.dt = rawtime;
    }
    if (recorde.hmax.val<tempSensor.humidity) {
      recorde.hmax.val= tempSensor.humidity;
      recorde.hmax.dt = rawtime;
    }
    if (recorde.pmin.val>tempSensor.pressure) {
      recorde.pmin.val= tempSensor.pressure;
      recorde.pmin.dt = rawtime;
    }
    if (recorde.pmax.val<tempSensor.pressure) {
      recorde.pmax.val= tempSensor.pressure;
      recorde.pmax.dt = rawtime;
    }
    if (recorde.dmin.val>tempSensor.dewPoint) {
      recorde.dmin.val= tempSensor.dewPoint;
      recorde.dmin.dt = rawtime;
    }
    if (recorde.dmax.val<tempSensor.dewPoint) {
      recorde.dmax.val= tempSensor.dewPoint;
      recorde.dmax.dt = rawtime;
    }
    if (recorde.amin.val>tempSensor.abshumidity) {
      recorde.amin.val= tempSensor.abshumidity;
      recorde.amin.dt = rawtime;
    }
    if (recorde.amax.val<tempSensor.abshumidity) {
      recorde.amax.val= tempSensor.abshumidity;
      recorde.amax.dt = rawtime;
    }
    if (recorde.imin.val>tempSensor.heatIndex) {
      recorde.imin.val= tempSensor.heatIndex;
      recorde.imin.dt = rawtime;
    }
    if (recorde.imax.val<tempSensor.heatIndex) {
      recorde.imax.val= tempSensor.heatIndex;
      recorde.imax.dt = rawtime;
    }
    if (recorde.rmin.val>SoilResitance) {
      recorde.rmin.val= SoilResitance;
      recorde.rmin.dt = rawtime;
    }
    if (recorde.rmax.val<SoilResitance) {
      recorde.rmax.val= SoilResitance;
      recorde.rmax.dt = rawtime;
    }
    if (recorde.gmin.val>float(mqgas)) {
      recorde.gmin.val= float(mqgas);
      recorde.gmin.dt = rawtime;
    }
    if (recorde.gmax.val<float(mqgas)) {
      recorde.gmax.val= float(mqgas);
      recorde.gmax.dt = rawtime;
    }
  } else {
    recorde.tmin.val= tempSensor.temperature;
    recorde.tmin.dt = rawtime;
    recorde.tmax.val= tempSensor.temperature;
    recorde.tmax.dt = rawtime;
    recorde.hmin.val= tempSensor.humidity;
    recorde.hmin.dt = rawtime;
    recorde.hmax.val= tempSensor.humidity;
    recorde.hmax.dt = rawtime;
    recorde.pmin.val= tempSensor.pressure;
    recorde.pmin.dt = rawtime;
    recorde.pmax.val= tempSensor.pressure;
    recorde.pmax.dt = rawtime;
    recorde.dmin.val= tempSensor.dewPoint;
    recorde.dmin.dt = rawtime;
    recorde.dmax.val= tempSensor.dewPoint;
    recorde.dmax.dt = rawtime;
    recorde.amin.val= tempSensor.abshumidity;
    recorde.amin.dt = rawtime;
    recorde.amax.val= tempSensor.abshumidity;
    recorde.amax.dt = rawtime;
    recorde.imin.val= tempSensor.heatIndex;
    recorde.imin.dt = rawtime;
    recorde.imax.val= tempSensor.heatIndex;
    recorde.imax.dt = rawtime;
    recorde.rmin.val= SoilResitance;
    recorde.rmin.dt = rawtime;
    recorde.rmax.val= SoilResitance;
    recorde.rmax.dt = rawtime;
    recorde.gmin.val= float(mqgas);
    recorde.gmin.dt = rawtime;
    recorde.gmax.val= float(mqgas);
    recorde.gmax.dt = rawtime;
    recorde.dur=0;
  }
  return true;
}

bool syncrSend(int) {
  timer.every(60*5, sendData);
  timer.in(0, sendData);
  return false;
}

void setupSendData() {
  time_t rawtime;
  time (&rawtime);
  struct tm * timeinfo;
  timeinfo = localtime (&rawtime);
  //
  char buffer[20];
  Serial.print("Agora: ");
  strftime(buffer,20,"%F %T ",timeinfo);
  Serial.print(buffer);
  //
  timeinfo->tm_min=(timeinfo->tm_min/5+1)*5;
  timeinfo->tm_sec=0;
  rawtime=mktime(timeinfo);
  timer.at(rawtime, sendData);
  Serial.print("  Next sycr: ");
  strftime(buffer,20,"%F %T ",timeinfo);
  Serial.println(buffer);
}

#endif
