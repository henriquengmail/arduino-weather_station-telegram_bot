#ifndef __irrig__h__
#define __irrig__h__

#define IRRIGPIN 13
int irrigRetry=3;
struct mem_t {
  Timer<15, secs, int>::Task T;
  uint8_t h;
  uint8_t m;
} mem[3];

bool irrigOff(int k);

time_t sec=0;

bool irrigOn(int k) {
  digitalWrite(IRRIGPIN, HIGH);
  time_t now = time(nullptr);
  timer.in(180,irrigOff,k);
  sec=now;
  Log.print(String("irrigON:      ")+String(ctime(&now)));
  return true;
}

bool irrigOff(int k) {
  digitalWrite(IRRIGPIN, LOW);
  time_t now = time(nullptr);
  recorde.dur+=now-sec;
  if (--irrigRetry>0) {
    timer.in(60,irrigOn,k);
    Log.print(String("IrrigOff and scheduled\n"));
  } else {
    irrigRetry = 3;
    Log.print(String("IrrigOff and off\n"));
    if (k>=0) {
      time_t raw=secs();
      // ((raw-3*60*60)/(60*60*24)+1)*60*60*24+3*60*60 - amanhã 0h
      mem[k].T=timer.at(((raw-3*60*60)/(24*60*60)+1)*24*60*60+mem[k].h*60*60+mem[k].m*60+3*60*60,irrigOn,k);
    }
  }
  return true;
}

bool syncIrrig(int t) {
  mem[t].T=timer.every(24*60*60, irrigOn);
  //Serial.print("syncIrrig set M");
  //Serial.println(t);
  Log.print(String("syncIrrig set M")+String(t)+String("\n"));
  return false;
}

void setupIrrig() {
  pinMode(IRRIGPIN, OUTPUT);
  digitalWrite(IRRIGPIN, LOW);
  
  mem[0].h= 3;mem[0].m=3;
  mem[1].h=6;mem[1].m=6;
  mem[2].h=19;mem[2].m=19;
  
  for(int k=0; k<=2; k++) {
    time_t raw=secs();
    struct tm *info = localtime( &raw );
    if ((mem[k].h*60+mem[k].m)<(info->tm_hour*60+info->tm_min+1)) {
      info->tm_mday++;
    }
    info->tm_hour=mem[k].h;
    info->tm_min=mem[k].m;
    info->tm_sec=0;
    mem[k].T=timer.at(mktime(info), irrigOn, k);
    char str[20];
    sprintf(str,"M%d %02d:%02d set",k,mem[k].h,mem[k].m);
    Serial.println(str);
  }
}

int getIrrigStatus() {
  //gpio_num_t pin = (gpio_num_t)(your_pin_number_0to31 & 0x1F);
  gpio_num_t pin = (gpio_num_t)(IRRIGPIN & 0x1F);
  int state=0;
  if (GPIO_REG_READ(GPIO_ENABLE_REG) & BIT(pin)){
    //pin is output - read the GPIO_OUT_REG register
    state = (GPIO_REG_READ(GPIO_OUT_REG)  >> pin) & 1U;
  }
  else
  {
    //pin is input - read the GPIO_IN_REG register
    state = (GPIO_REG_READ(GPIO_IN_REG)  >> pin) & 1U;
  }
  Log.print("irrigState: "+String(state));
  return state;
}

#endif
