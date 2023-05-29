#ifndef __timers__h__
#define __timers__h__

#include <arduino-timer.h>
#include <time.h>
/*
unsigned long  secday() {
  time_t sec = time(NULL);
  struct tm *info = localtime( &sec );
  unsigned long h=h=info->tm_sec+info->tm_min*60+info->tm_hour*60*60;
  //unsigned long h=sec % (24*60*60);
  return h;
}
*/

long unsigned int  secs() {
  time_t sec = time(NULL);
  return sec;
}
#define Timer_Tasks 15
Timer<Timer_Tasks, secs, int> timer;
//  timer.at(, syncrSend);
//  timer.in(0,sendData);
//  timer.every(60*5, sendData);
//  timer.every( 5, checkBot);
//  timer.every(60, OTA)
//  timer.at( , irrigOn);
//  timer.every(24*60*60, irrigOn);
//  timer.at(, irrigOn);
//  timer.at(, irrigOn);
//  timer.in( 90, irrigOff);
//  timer.in(60,irrigOn);

//  timer.every(60*5, sendData);
//  timer.every( 5, checkBot);
//  timer.every(60, OTA)
//  timer.at(, irrigOn);
//  timer.at(, irrigOn);
//  timer.at(, irrigOn);

#endif
