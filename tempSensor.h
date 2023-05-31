#ifndef __tempSensor__h__
#define __tempSensor__h__

#include <DHTesp.h>
#include <math.h>
#include "BME280I2C.h"
#include "EnvironmentCalculations.h"

DHTesp dht;
struct _tempSensor {
  float temperature;
  float humidity;
  float pressure;
  float heatIndex;
  float dewPoint;
  float abshumidity;
  float comfortRatio;
  ComfortState cf;
  String comfortStatus;
} tempSensor;
typedef struct {
  time_t dt;
  float val;
} minimax;
struct _recorde {
  minimax tmin;
  minimax tmax;
  minimax hmin;
  minimax hmax;
  minimax pmin;
  minimax pmax;
  minimax dmin;
  minimax dmax;
  minimax amin;
  minimax amax;
  minimax imin;
  minimax imax;
  minimax rmin;
  minimax rmax;
  minimax gmin;
  minimax gmax;
  time_t dur;
} recorde;

void AbsoluteHumidity() {
  // from https://github.com/finitespace/BME280/blob/master/src/EnvironmentCalculations.cpp
  // //taken from https://carnotcycle.wordpress.com/2012/08/04/how-to-convert-relative-humidity-to-absolute-humidity/
  const float mw = 18.01534;   // molar mass of water g/mol
  const float r = 8.31447215;   // Universal gas constant J/mol/K
  float temp = pow(2.718281828, (17.67 * tempSensor.temperature) / (tempSensor.temperature + 243.5));
  tempSensor.abshumidity = (6.112 * temp * tempSensor.humidity * mw) / ((273.15 + tempSensor.temperature) * r);   //long version
}

bool getTemperature() {
  // Reading temperature for humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
  TempAndHumidity newValues = dht.getTempAndHumidity();
  // Check if any reads failed and exit early (to try again).
  if (dht.getStatus() != 0) {
    //Serial.println("DHT11 error status: " + String(dht.getStatusString()));
    Log.print("DHT11 error status: " + String(dht.getStatusString())+String("\n"));
    return false;
  }
  tempSensor.temperature=newValues.temperature;
  tempSensor.humidity=newValues.humidity;
  tempSensor.heatIndex = dht.computeHeatIndex(tempSensor.temperature, tempSensor.humidity);
  tempSensor.dewPoint = dht.computeDewPoint(tempSensor.temperature, tempSensor.humidity);
  tempSensor.comfortRatio = dht.getComfortRatio(tempSensor.cf, tempSensor.temperature, tempSensor.humidity);

  switch(tempSensor.cf) {
    case Comfort_OK:
      tempSensor.comfortStatus = "Comfort_OK";
      break;
    case Comfort_TooHot:
      tempSensor.comfortStatus = "Comfort_TooHot";
      break;
    case Comfort_TooCold:
      tempSensor.comfortStatus = "Comfort_TooCold";
      break;
    case Comfort_TooDry:
      tempSensor.comfortStatus = "Comfort_TooDry";
      break;
    case Comfort_TooHumid:
      tempSensor.comfortStatus = "Comfort_TooHumid";
      break;
    case Comfort_HotAndHumid:
      tempSensor.comfortStatus = "Comfort_HotAndHumid";
      break;
    case Comfort_HotAndDry:
      tempSensor.comfortStatus = "Comfort_HotAndDry";
      break;
    case Comfort_ColdAndHumid:
      tempSensor.comfortStatus = "Comfort_ColdAndHumid";
      break;
    case Comfort_ColdAndDry:
      tempSensor.comfortStatus = "Comfort_ColdAndDry";
      break;
    default:
      tempSensor.comfortStatus = "Unknown:";
      break;
  };
  AbsoluteHumidity();
  //#ifdef _DEBUG_Serial_
  //Serial.println(" T:" + String(tempSensor.temperature,1) + " H:" + String(tempSensor.humidity,1) + " I:" + String(tempSensor.heatIndex) + " D:" + String(tempSensor.dewPoint) + " " + tempSensor.comfortStatus);
  //Log.print(" T:" + String(tempSensor.temperature,1) + " H:" + String(tempSensor.humidity,1) + " I:" + String(tempSensor.heatIndex) + " D:" + String(tempSensor.dewPoint) + " " + tempSensor.comfortStatus + String("\n"));
  //#endif
  return true;
}

BME280I2C::Settings settings(
   BME280::OSR_X1,
   BME280::OSR_X1,
   BME280::OSR_X1,
   BME280::Mode_Forced,
   BME280::StandbyTime_1000ms,
   BME280::Filter_Off,
   BME280::SpiEnable_False,
   BME280I2C::I2CAddr_0x76 // I2C address. I2C specific.
);

BME280I2C bme(settings);

bool getBMEsensors() {
  float temp(NAN), hum(NAN), pres(NAN);

  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_Pa);

  bme.read(pres, temp, hum, tempUnit, presUnit);
  tempSensor.temperature= temp;
  tempSensor.humidity   = hum;
  tempSensor.pressure   = pres;
  EnvironmentCalculations::TempUnit     envTempUnit =  EnvironmentCalculations::TempUnit_Celsius;
  tempSensor.heatIndex  = EnvironmentCalculations::HeatIndex(temp, hum, envTempUnit);;
  tempSensor.dewPoint   = EnvironmentCalculations::DewPoint(temp, hum, envTempUnit);
  tempSensor.abshumidity= EnvironmentCalculations::AbsoluteHumidity(temp, hum, envTempUnit);
  return true;
}

void setupTempSensor() {
  dht.setup(26, DHTesp::DHT22);
  Serial.println("DHT initiated");
  recorde.tmin.dt=0;
  recorde.tmax.dt=0;
  recorde.hmin.dt=0;
  recorde.hmax.dt=0;
  recorde.pmin.dt=0;
  recorde.pmax.dt=0;
  recorde.dmin.dt=0;
  recorde.dmax.dt=0;
  recorde.amin.dt=0;
  recorde.amax.dt=0;
  recorde.imin.dt=0;
  recorde.imax.dt=0;
  recorde.rmin.dt=0;
  recorde.rmax.dt=0;
  recorde.gmin.dt=0;
  recorde.gmax.dt=0;
  recorde.dur=0;
  getTemperature();
  // BME280
  if (!bme.begin()) {
    Serial.println("Not found BME280 sensor!");
    return;
  }
  switch(bme.chipModel())
  {
     case BME280::ChipModel_BME280:
       Serial.println("Found BME280 sensor! Success.");
       break;
     case BME280::ChipModel_BMP280:
       Serial.println("Found BMP280 sensor! No Humidity available.");
       break;
     default:
       Serial.println("Found UNKNOWN sensor! Error!");
  }  
}

#endif
