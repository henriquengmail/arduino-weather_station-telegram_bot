#ifndef __adc__h__
#define __adc__h__
#include "ADS1115_WE.h"
#include "soilHumidity.h"

#define ADS1115_I2C_ADDRESS 0x48
#define ADS_Interrupt_Pin  22
volatile bool valueReady;

class adc_t {
  public:
  short counter;
  float value;
  float sumValue;
  float maxValue;
  float minValue;
  bool soil;
  bool gas;
  ADS1115_WE ADS1115 = ADS1115_WE(ADS1115_I2C_ADDRESS);

  adc_t() {
    counter=0;
    value=0;
    valueReady=false;
    sumValue=0;
    maxValue=0;
    minValue=1e6;
    soil=false;
    gas=false;
    pinMode(ADS_Interrupt_Pin, INPUT_PULLUP);
  }

  bool setup();

  void addValue(float newValue) {
    counter++;
    sumValue+=newValue;
    maxValue=newValue>maxValue ? newValue : maxValue;
    minValue=newValue<minValue ? newValue : minValue;
    if (counter>2) {
      value=(sumValue-maxValue-minValue)/float(counter-2);
    } else {
      value=sumValue/float(counter);
    }
    if (counter>=16) {
      endRead();
    }
  }

  void readValue() {
		if (valueReady) {
			// Lê o dado no registro do ADC
  		valueReady=false;
			float newValue = ADS1115.getResult_V();
			addValue(newValue);
		}
  }

  float getValue() {
    return value;
  }

	void beginReadSoil() {
		// Inicia leitura com as configurações do ADC
    soil=false;
    ADS1115.setCompareChannels(ADS1115_COMP_0_GND);
    ADS1115.setMeasureMode(ADS1115_CONTINUOUS);
	}
	void beginReadGas() {
		// Inicia leitura com as configurações do ADC
    gas=false;
    ADS1115.setCompareChannels(ADS1115_COMP_1_GND);
    ADS1115.setMeasureMode(ADS1115_CONTINUOUS);
	}

	void endRead() {
    if (!soil){
      soil=true;
      SoilResitance=getValue();
      beginReadGas();
    }
    if (!gas) {
      gas=true;
      mqgas=getValue();
    }
    ADS1115.setSingleChannel(0);
	}
};

void convReadyAlert();

bool adc_t::setup() {
    if(!ADS1115.init()) {
      Serial.println("ADS1115 not connected!");
      return false;
    }
    ADS1115.setVoltageRange_mV(ADS1115_RANGE_4096);
    ADS1115.setCompareChannels(ADS1115_COMP_0_GND);
    ADS1115.setAlertPinMode(ADS1115_ASSERT_AFTER_1);
    ADS1115.setConvRate(ADS1115_128_SPS);
    //ADS1115.setMeasureMode(ADS1115_CONTINUOUS);
    attachInterrupt(digitalPinToInterrupt(ADS_Interrupt_Pin), convReadyAlert, FALLING);
    return true;
  }

adc_t ads;

void convReadyAlert(); {
    valueReady=true;
    // Acorda o ESP32
}

#endif
