#include <AsyncTelegram2.h>
// Timezone definition
#include <time.h>
#define USE_CLIENTSSL false

#ifdef ESP8266
  #include <ESP8266WiFi.h>
  WiFiClientSecure client;
  Session   session;
  X509List  certificate(telegram_cert);
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WiFiClient.h>
  #if USE_CLIENTSSL
    #include <SSLClient.h>      // https://github.com/OPEnSLab-OSU/SSLClient/
    #include "tg_certificate.h"
    WiFiClient base_client;
    SSLClient client(base_client, TAs, (size_t)TAs_NUM, A0, 1, SSLClient::SSL_ERROR);
  #else
    #include <WiFiClientSecure.h>
    WiFiClientSecure client;
  #endif
#endif
#include <WiFiClient.h>
#include <HTTPClient.h>
#include "basicOTA.h"
#include "FS.h"
#include <LittleFS.h>

/*
String listDir(fs::FS &fs, const char * dirname) {
  String str;
  str+=String("Listing directory: ")+dirname+"\n";
  File root = fs.open(dirname);
  if(!root){
      str+=String("- failed to open directory\n");
      return str;
  }
  if(!root.isDirectory()){
      str+=String(" - not a directory\n");
      return str;
  }

  File file = root.openNextFile();
  while(file){
      if(file.isDirectory()){
          str+=String("  DIR : ")+file.name()+"\n";
          str+=listDir(fs, file.path());
      } else {
          str+=String("  FILE: ")+file.name()+"\tSIZE: "+String(file.size());
      }
      file = root.openNextFile();
  }
  return str;
}
*/
AsyncTelegram2 myBot(client);

const char* token =  "5366228963:AAHtBihnDkKOzBZ5v9bn4bNHuzUJWD1vInc";  // Telegram token

// Check the userid with the help of bot @JsonDumpBot or @getidsbot (work also with groups)
// https://t.me/JsonDumpBot  or  https://t.me/getidsbot
const int64_t userid = 1701208339;

ReplyKeyboard myReplyKbd;   // reply keyboard object helper
InlineKeyboard myInlineKbd; // inline keyboard object helper
bool isKeyboardActive;      // store if the reply keyboard is shown

#define LIGHT_ON_CALLBACK  "lightON"  // callback data sent when "LIGHT ON" button is pressed
#define LIGHT_OFF_CALLBACK "lightOFF" // callback data sent when "LIGHT OFF" button is pressed
const uint8_t LED = 2;
uint8_t retryBot=0;

bool checkBot(int) {
  ArduinoOTA.handle();
  yield();
  if (!myBot.checkConnection()) {
    retryBot++;
    if ((!myBot.reset()) && (retryBot>360)) {
      //Serial.println("Reiniciando a placa...");
      // Salvar o estado atual (minimax, tempSensor, timers)
      Log.print("Reiniciando a placa...\n");
      //delay(10000);
      ESP.restart();
    }
    //Serial.println("Bot sem conexão");
    Log.print("Bot sem conexão\n");
    return true;
  } else {
    retryBot=0;
  }
  // if there is an incoming message...
  // local variable to store telegram message data
  TBMessage msg;
  if (myBot.getNewMessage(msg)) {
    // check what kind of message I received
    MessageType msgType = msg.messageType;
    String msgText = msg.text;

    switch (msgType) {
      case MessageText :
        // received a text message
        //Serial.print("\nText message received: ");
        //Serial.println(msgText);
        Log.print("\nText message received: "+String(msgText));
        // check if is show keyboard command
        if (msgText.equalsIgnoreCase("/reply_keyboard")) {
          // the user is asking to show the reply keyboard --> show it
          myBot.sendMessage(msg, "This is reply keyboard:", myReplyKbd);
          isKeyboardActive = true;
        }
        else if (msgText.equalsIgnoreCase("/inline_keyboard")) {
          myBot.sendMessage(msg, "This is inline keyboard:", myInlineKbd);
        }
        else if (msgText.equalsIgnoreCase("/help")) {
          String str;
          str+="/help , this help";str+="\n";
          str+="/status , get status device sensors";str+="\n";
          str+="/irrigOn , turn on irrigation system";str+="\n";
          str+="/irrigOff , turn off irrigation system";str+="\n";
          str+="/getip , get local IP";str+="\n";
          str+="/getmems , get all mem hourly";str+="\n";
          str+="/setmem MX HH:MM , set mem X to HH:MM";str+="\n";
          str+="/reply_keyboard , replace keyboard to menu";str+="\n";
          str+="/tasks , return task timers";str+="\n";
          str+="/uptime , get system uptime \n";
          str+="/restart , restart system\n";
          str+="/logstatus , get logger status \n";
          str+="/logweb , enable log to webserver\n";
          str+="/logserial , enable log to serial\n";
          str+="/minmax , get min and max values\n";
          str+="/fscheck , check file system\n";
          str+="/about";
          myBot.sendMessage(msg, str);
        } //1039469,1039577,1039605,1039745,1041289,1042093,1042469,1042757,1046009,1046433,1046621,1046773,1082117,1083341,1083705,1085177,1103201,1103469,1103541,1103737,1103853,1104125,1104413*,1104629
        else if (msgText.equalsIgnoreCase("/status")) {
          String str;
          str+="Temp: "+String(tempSensor.temperature,1)+" ºC\n";
          str+="Humid: "+String(tempSensor.humidity,1)+" %\n";
          str+="Press: "+String(tempSensor.pressure,1)+" hPa\n";
          str+="AbsHumid: "+String(tempSensor.abshumidity,1)+" g/m³\n";
          str+="Index: "+String(tempSensor.heatIndex,1)+" ºC\n";
          str+=tempSensor.comfortStatus+"\n";
          str+="Dew: "+String(tempSensor.dewPoint,1)+" ºC\n";
          str+="Solo: "+String(SoilResitance/1000.0,1)+" kΩ\n";
          str+="Irrig: "+String(getIrrigStatus())+"\n";
          str+="Gas: "+String(mqgas)+" mV\n";
          myBot.sendMessage(msg, str);
        }
        else if (msgText.equalsIgnoreCase("/getip")) {
          String str;
          str+=String("IP: ");str+="\n";
          str+=WiFi.localIP().toString();
          myBot.sendMessage(msg, str);
        }
        else if (msgText.equalsIgnoreCase("/irrigOn")) {
          timer.in(0, irrigOn, -1);
          myBot.sendMessage(msg, "Irrigation enable");
        }
        else if (msgText.equalsIgnoreCase("/irrigOff")) {
          irrigRetry=0;
          timer.in(0, irrigOff, -1);
          myBot.sendMessage(msg, "Irrigation disable");
        }
        else if (msgText.startsWith("/setmem")) {
          int k=msgText.substring(msgText.indexOf(" M")+2,msgText.indexOf(" M")+3).toInt();
          // Elimina o timer antigo e coloca o timer atual
          mem[k].h=msgText.substring(msgText.indexOf(":")-2,msgText.indexOf(":")-0).toInt();
          mem[k].m=msgText.substring(msgText.indexOf(":")+1,msgText.indexOf(":")+3).toInt();
          time_t raw=secs();
          struct tm *info = localtime( &raw );
          if ((mem[k].h*60+mem[k].m)<(info->tm_hour*60+info->tm_min)) {
            info->tm_mday++;
          }
          info->tm_hour=mem[k].h;
          info->tm_min=mem[k].m;
          info->tm_sec=0;
          timer.cancel(mem[k].T);
          mem[k].T=timer.at(mktime(info), syncIrrig, k);
          char str[40];
          sprintf(str,"Memory %d has been set to %02d:%02d",k,mem[k].h,mem[k].m);
          myBot.sendMessage(msg, str);
        }
        else if (msgText.startsWith("/getmems")) {
          String str;
          for(int k=0;k<=2;k++) {
            char a[20];
            sprintf(a,"Mem %d: %02d:%02d id:%d\n",k,mem[k].h,mem[k].m,mem[k].T);
            str+=a;
          }
          myBot.sendMessage(msg, str);
        }
        else if (msgText.equalsIgnoreCase("/logweb")) {
          if (Log.getWeb()==true) {
            Log.setWeb(false);
            myBot.sendMessage(msg, "logWeb is off");
          } else {
            Log.setWeb(true);
            myBot.sendMessage(msg, "logWeb is on");
          }
        }
        else if (msgText.equalsIgnoreCase("/logserial")) {
          if (Log.getSerial()==true) {
            Log.setSerial(false);
            myBot.sendMessage(msg, "logSerial is off");
          } else {
            Log.setSerial(true);
            myBot.sendMessage(msg, "logSerial is on");
          }
        }
        else if (msgText.equalsIgnoreCase("/tasks")) {
          String str;
          char buffer[22];
          time_t raw=secs();
          struct tm *info = localtime( &raw );
          strftime (buffer,22,"%d/%m/%y %X",info);
          str+=String("Agora: ")+buffer+"\n";
          str+="Timers: "+String(timer.size())+"\n";
          for(size_t i; i<Timer_Tasks; i++) {
            if (timer.tasks[i].handler) {
              raw=timer.tasks[i].start+timer.tasks[i].expires;
              info = localtime( &raw );
              strftime (buffer,22,": %d/%m %X",info);
              char buff[8];
              sprintf(buff,"%3d",timer.tasks[i].id);
              str+=String("id: ")+buff+buffer+"\n";
            }
          }
          myBot.sendMessage(msg, str);
        }
        else if (msgText.equalsIgnoreCase("/uptime")) {
          time_t leaptime = secs()-uptime;
          struct tm *info = gmtime( &leaptime );
          char buff[30];
          sprintf(buff, "%d days %02d:%02d:%02d", leaptime/(24*60*60), info->tm_hour, info->tm_min, info->tm_sec);
          String str;
          str+=String("O sistema está em funcionamento ininterrupto por:\n")+buff;
		  info = localtime( &uptime );
		  strftime (buff,22,": %D %X",info);
		  str+=String("\nDesde: ")+buff;
          myBot.sendMessage(msg, str);
        } 
        else if (msgText.equalsIgnoreCase("/restart")) {
          String str;
          str+=String("O sistema será reiniciado agora\n");
          myBot.sendMessage(msg, str);
          ESP.restart();
        }
        else if (msgText.equalsIgnoreCase("/logstatus")) {
          String str;
          if (Log.getSerial()) {
            str+="Serial on\n";
          } else {
            str+="Serial off\n";
          }
          if (Log.getWeb()) {
            str+="Web on\n";
          } else {
            str+="Web off\n";
          }
          if (Log.getTelnet()) {
            str+="Telnet on\n";
          } else {
            str+="Telnet off\n";
          }
          if (Log.getBot()) {
            str+="Bot on\n";
          } else {
            str+="Bot off\n";
          }
          myBot.sendMessage(msg, str);
        }
        else if (msgText.equalsIgnoreCase("/minmax")) {
          String str;
          char buffer[22];
          struct tm *info;
          info = localtime( &recorde.tmin.dt );
          strftime (buffer,22,"%d/%m",info);
          str+=String("Os recordes em ")+ buffer +" são:\n";
          strftime (buffer,22,"%H:%M",info);
          str+="Tmin: "+String(recorde.tmin.val,1)+" ºC em " +buffer+"\n";
          info = localtime( &recorde.tmax.dt );
          strftime (buffer,22,"%H:%M",info);
          str+="Tmax: "+String(recorde.tmax.val,1)+" ºC em " +buffer+"\n";
          info = localtime( &recorde.hmin.dt );
          strftime (buffer,22,"%H:%M",info);
          str+="Hmin: "+String(recorde.hmin.val,1)+" % em " +buffer+"\n";
          info = localtime( &recorde.hmax.dt );
          strftime (buffer,22,"%H:%M",info);
          str+="Hmax: "+String(recorde.hmax.val,1)+" % em " +buffer+"\n";
          info = localtime( &recorde.pmin.dt );
          strftime (buffer,22,"%H:%M",info);
          str+="Pmin: "+String(recorde.pmin.val,1)+" hPa em " +buffer+"\n";
          info = localtime( &recorde.pmax.dt );
          strftime (buffer,22,"%H:%M",info);
          str+="Pmax: "+String(recorde.pmax.val,1)+" hPa em " +buffer+"\n";
          info = localtime( &recorde.dmin.dt );
          strftime (buffer,22,"%H:%M",info);
          str+="Dmin: "+String(recorde.dmin.val,1)+" ºC em " +buffer+"\n";
          info = localtime( &recorde.dmax.dt );
          strftime (buffer,22,"%H:%M",info);
          str+="Dmax: "+String(recorde.dmax.val,1)+" ºC em " +buffer+"\n";
          info = localtime( &recorde.amin.dt );
          strftime (buffer,22,"%H:%M",info);
          str+="Amin: "+String(recorde.amin.val,1)+" g/m³ em " +buffer+"\n";
          info = localtime( &recorde.amax.dt );
          strftime (buffer,22,"%H:%M",info);
          str+="Amax: "+String(recorde.amax.val,1)+" g/m³ em " +buffer+"\n";
          info = localtime( &recorde.imin.dt );
          strftime (buffer,22,"%H:%M",info);
          str+="Imin: "+String(recorde.imin.val,1)+" ºC em " +buffer+"\n";
          info = localtime( &recorde.imax.dt );
          strftime (buffer,22,"%H:%M",info);
          str+="Imax: "+String(recorde.imax.val,1)+" ºC em " +buffer+"\n";
          info = localtime( &recorde.rmin.dt );
          strftime (buffer,22,"%H:%M",info);
          str+="Rmin: "+String(recorde.rmax.val,1)+" kΩ em " +buffer+"\n";
          info = localtime( &recorde.rmax.dt );
          strftime (buffer,22,"%H:%M",info);
          str+="Rmax: "+String(recorde.rmax.val,1)+" kΩ em " +buffer+"\n";
          info = gmtime( &recorde.dur );
          strftime (buffer,22,"%H:%M:%S",info);
          str+=String("Duração da irrigação: ") +buffer+"\n";
          info = localtime( &recorde.gmin.dt );
          strftime (buffer,22,"%H:%M",info);
          str+="Gmin: "+String(recorde.gmin.val,0)+" mV em " +buffer+"\n";
          info = localtime( &recorde.gmax.dt );
          strftime (buffer,22,"%H:%M",info);
          str+="Gmax: "+String(recorde.gmax.val,0)+" mV em " +buffer+"\n";
          myBot.sendMessage(msg, str);
        }
        else if (msgText.equalsIgnoreCase("/fscheck")) {
          String str;
          if (LittleFS.begin()) {
            str=listDir(LittleFS,"/");
            str+="Size=" + String(LittleFS.totalBytes())+" bytes\n";
            str+="Used=" + String(LittleFS.usedBytes())+" bytes\n";
            LittleFS.end();
          } else{
            str="FS mount failed\n";
          }
          str+="Internal Heap Free: "+String(ESP.getFreeHeap())+" bytes\n";
          str+="Internal Heap frag: "+String(100 - ESP.getMaxAllocHeap() * 100.0 / ESP.getFreeHeap())+" %\n";
          str+="SPI Heap Free: "+String(ESP.getFreeHeap())+" bytes\n";
          str+="SPI Heap frag: "+String(100 - ESP.getMaxAllocPsram() * 100.0 / (ESP.getFreePsram() + 0.0001) )+" %\n";
          str+="Free Sketch: "+String(ESP.getFreeSketchSpace())+" bytes\n";
          myBot.sendMessage(msg, str);
        }
        else if (msgText.equalsIgnoreCase("/about")) {
          String str;
    		  str+=String("Irriga Horta e sensoriamento\n");
    		  str+=String("Autor: Henrique Nunes Braga\n");
    		  str+=String("Criado em: 08/05/2022\n");
          str+=String("Compilado em: ")+ __DATE__ +" "+ __TIME__ +"\n";
          myBot.sendMessage(msg, str);
        }
        // check if the reply keyboard is active
        else if (isKeyboardActive) {
          // is active -> manage the text messages sent by pressing the reply keyboard buttons
          if (msgText.equalsIgnoreCase("/hide_keyboard")||msgText.equalsIgnoreCase("/hide_kb")) {
            // sent the "hide keyboard" message --> hide the reply keyboard
            myBot.removeReplyKeyboard(msg, "Reply keyboard removed");
            isKeyboardActive = false;
          } else {
            // print every others messages received
            //myBot.sendMessage(msg, msg.text);
          }
        }

        // the user write anything else and the reply keyboard is not active --> show a hint message
        else {
          myBot.sendMessage(msg, "Try /reply_keyboard or /inline_keyboard");
        }
        break;

      case MessageQuery:
        // received a callback query message
        msgText = msg.callbackQueryData;
        //Serial.print("\nCallback query message received: ");
        //Serial.println(msg.callbackQueryData);
        Log.print("\nCallback query message received: "+msg.callbackQueryData+"\n");

        if (msgText.equalsIgnoreCase(LIGHT_ON_CALLBACK)) {
          // pushed "LIGHT ON" button...
          //Serial.println("\nSet light ON");
          Log.print("\nSet light ON");
          digitalWrite(LED, HIGH);
          // terminate the callback with an alert message
          myBot.endQuery(msg, "Light on", true);
        }
        else if (msgText.equalsIgnoreCase(LIGHT_OFF_CALLBACK)) {
          // pushed "LIGHT OFF" button...
          //Serial.println("\nSet light OFF");
          Log.print("\nSet light OFF");
          digitalWrite(LED, LOW);
          // terminate the callback with a popup message
          myBot.endQuery(msg, "Light off");
        }
        break;

      case MessageLocation: {
          // received a location message
          String reply = "Longitude: ";
          reply += msg.location.longitude;
          reply += "; Latitude: ";
          reply += msg.location.latitude;
          //Serial.println(reply);
          Log.print(reply+"\n");
          myBot.sendMessage(msg, reply);
          break;
        }

      case MessageContact: {
          // received a contact message
          String reply = "Contact information received:";
          reply += msg.contact.firstName;
          reply += " ";
          reply += msg.contact.lastName;
          reply += ", mobile ";
          reply += msg.contact.phoneNumber;
          //Serial.println(reply);
          Log.print(reply+"\n");
          myBot.sendMessage(msg, reply);
          break;
        }

      default:
        break;
    } //switch (msgType)
  } //if (myBot.getNewMessage(msg))

  return true;
}

void setupBot() {
  #ifdef ESP8266
  // Sync time with NTP, to check properly Telegram certificate
  //configTime(MYTZ, "time.google.com", "time.windows.com", "pool.ntp.org");
  //Set certficate, session and some other base client properies
  client.setSession(&session);
  client.setTrustAnchors(&certificate);
  client.setBufferSizes(1024, 1024);
#elif defined(ESP32)
  // Sync time with NTP
  //configTzTime(MYTZ, "time.google.com", "time.windows.com", "pool.ntp.org");
  //configTime(-3*60*60,0, "time.google.com", "time.windows.com", "pool.ntp.org");
  //yield();
  #if USE_CLIENTSSL == false
    client.setCACert(telegram_cert);
  #endif
#endif

  // Set the Telegram bot properties
  myBot.setUpdateTime(1000);
  myBot.setTelegramToken(token);

  // Check if all things are ok
  Serial.print("\nTest Telegram connection... ");
  myBot.begin() ? Serial.println("OK") : Serial.println("NOK");
  Serial.print("Bot name: @");
  Serial.println(myBot.getBotName());
  //Log.setBot(true,&myBot);
  /*
  // Add reply keyboard
  isKeyboardActive = false;
  // add a button that send a message with "Simple button" text
  myReplyKbd.addButton("Button1");
  myReplyKbd.addButton("Button2");
  myReplyKbd.addButton("Button3");
  // add a new empty button row
  myReplyKbd.addRow();
  // add another button that send the user position (location)
  myReplyKbd.addButton("Send Location", KeyboardButtonLocation);
  // add another button that send the user contact
  myReplyKbd.addButton("Send contact", KeyboardButtonContact);
  // add a new empty button row
  myReplyKbd.addRow();
  // add a button that send a message with "Hide replyKeyboard" text
  // (it will be used to hide the reply keyboard)
  myReplyKbd.addButton("/hide_keyboard");
  // resize the keyboard to fit only the needed space
  myReplyKbd.enableResize();
  */
  
  isKeyboardActive = true;
  /*
  myReplyKbd.addButton("/help");
  myReplyKbd.addButton("/status");
  myReplyKbd.addRow();
  //myReplyKbd.addButton("/reply_keyboard")
  myReplyKbd.addButton("/hide_kb");
  myReplyKbd.addButton("/getmems");
  myReplyKbd.addRow();
  myReplyKbd.addButton("/tasks");
  myReplyKbd.addButton("/getip");//uptime
  myReplyKbd.addRow();
  myReplyKbd.addButton("/restart");
  myReplyKbd.addButton("/uptime");
  myReplyKbd.addRow();
  myReplyKbd.addButton("/irrigOn");
  myReplyKbd.addButton("/irrigOff");
  myReplyKbd.addRow();
  myReplyKbd.addButton("/logstatus");
  myReplyKbd.addButton("/logserial");
  */
  myReplyKbd.addButton("/help");
  myReplyKbd.addButton("/status");
  myReplyKbd.addButton("/uptime");
  myReplyKbd.addRow();
  myReplyKbd.addButton("/hide_kb");
  myReplyKbd.addButton("/getmems");
  myReplyKbd.addButton("/tasks");
  myReplyKbd.addRow();
  myReplyKbd.addButton("/irrigOn");
  myReplyKbd.addButton("/irrigOff");

/*
  // Add sample inline keyboard
  myInlineKbd.addButton("ON", LIGHT_ON_CALLBACK, KeyboardButtonQuery);
  myInlineKbd.addButton("OFF", LIGHT_OFF_CALLBACK, KeyboardButtonQuery);
  myInlineKbd.addRow();
  myInlineKbd.addButton("GitHub", "https://github.com/cotestatnt/AsyncTelegram2/", KeyboardButtonURL);
*/

  timer.every( 4, checkBot);
}
