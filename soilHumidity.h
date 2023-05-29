#ifndef __soilHumidity__h__
#define __soilHumidity__h__

#include <math.h>
#define SOILHUMIDITYPIN 34
#define TURNONHUMIDITYPIN 33
#define GASPIN 35

float SoilResitance=0;
int   mqgas=0;

void getSoilResistance() {
    digitalWrite(TURNONHUMIDITYPIN, HIGH);
    //int value = analogReadMilliVolts(SOILHUMIDITYPIN);
    float value=0;
    int k;
    for(k=0; k<50; k++) {
      delay(10);
      value+=analogReadMilliVolts(SOILHUMIDITYPIN);
    }
    value=value/float(k);
    SoilResitance=value*220000.0/float(3300-value);
    SoilResitance=roundf(SoilResitance/1000.0*10.0)/10.0;
    //#ifdef _DEBUG_Serial_
    //Log.print(String("Resist: ")+String(SoilResitance,1)+String(" Millivolt: ")+String(value)+String(" value ")+String(analogRead(SOILHUMIDITYPIN))+"\n");
    //Serial.println(str);
    //#endif
    digitalWrite(TURNONHUMIDITYPIN, LOW);
    //return SoilResitance;
}

void getGas() {
  float value=0;
  int k;
  for(k=0;k<50;k++) {
    delay(10);
    value+=analogReadMilliVolts(GASPIN);
  }
  mqgas = int(roundf(value/float(k)));
  //mqgas = analogReadMilliVolts(GASPIN);
  //Log.print(String("Gas milivolts: ")+String(mqgas)+"\n");
}

void setupSoilHumidity() {
  // Habilita o Pino para ativar o sensor
  pinMode(TURNONHUMIDITYPIN, OUTPUT);
  digitalWrite(TURNONHUMIDITYPIN, LOW);
  // Habilita o Pino para ler o sensor
  pinMode(SOILHUMIDITYPIN, ANALOG);
  // Calibra o sensor ou muda a atenuação
  analogSetWidth(12);
  analogSetPinAttenuation(SOILHUMIDITYPIN, ADC_0db);
  // Gas Sensor
  pinMode(GASPIN, ANALOG);
  analogSetPinAttenuation(GASPIN, ADC_0db);
  getSoilResistance();
  getGas();
}

#endif
