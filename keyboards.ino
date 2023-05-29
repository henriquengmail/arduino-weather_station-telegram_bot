/*
  Name:        hortaHNB
  Created:     05/03/2022
  Description: Timer irrigation system and enviroment station logger
*/
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <esp_task_wdt.h>
time_t uptime;
#include "memop.h"
#include "telnet.h"
#include "sendData.h"
#include "irrig.h"
#include "bot.h"
#include "basicOTA.h"

void setupWifiManager() {
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  // it is a good practice to make sure your code sets wifi mode how you want it.

  //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wm;

  // reset settings - wipe stored credentials for testing
  // these are stored by the esp library
  wm.resetSettings();
  
  const char* ssid  =  "Tenda_6AFE30";     // SSID WiFi network
  const char* pass  =  "catarina";     // Password  WiFi network
  //Lê o FS em busca de senha armazenada
  ///FSgetSSID(ssid,pass);
  
  // Carrega a senha no wifimanager
  wm.preloadWiFi(ssid,pass);
  
  // Se a senha for atualizada
  //vm.setSaveConfigCallback(FSsetSSID);
  
  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
  // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
  // then goes into a blocking loop awaiting configuration and will return success result

  // Se o usuário não não digitar uma rede em 5min, retorna falso
  wm.setConfigPortalTimeout(5*60);

  bool res;
  // res = wm.autoConnect(); // auto generated AP name from chipid
  res = wm.autoConnect("HortaAutoConnect"); // anonymous ap
  // res = wm.autoConnect("AutoConnectAP","password"); // password protected ap

  if(!res) {
      Serial.println("Failed to connect");
      ESP.restart();
  } 
  else {
      //if you get here you have connected to the WiFi    
      Serial.println("connected...yeey :)");
  }
}

void setup() {
  // initialize the Serial
  Serial.begin(115200);
  
  setupWifiManager();
  /*
  const char* ssid  =  "Tenda_6AFE30";     // SSID WiFi network
  const char* pass  =  "catarina";     // Password  WiFi network
  // connects to the access point
  WiFi.begin(ssid, pass);
  delay(500);
  unsigned int wifiTime=millis();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
    if ((millis()-wifiTime)>15000) {
      setupWifiManager();
    }
  }
  */
  setupTelnetStream();
  yield();
  setupBot();
  yield();
  setupTempSensor();
  setupSoilHumidity();
  yield();
  setupOTA();
  yield();
  setupSendData();
  setupIrrig();
  yield();
  //Habilita o watchdog configurando o timeout para 10 segundos
  esp_task_wdt_init(60, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL);  //add current thread to WDT watch
  // Habilita alguns perifericos durante do deep sleep para evitar bug de temperatura baixa
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH,   ESP_PD_OPTION_ON);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_ON);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_ON);
  esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL,         ESP_PD_OPTION_ON);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC8M,        ESP_PD_OPTION_ON);
  esp_sleep_pd_config(ESP_PD_DOMAIN_VDDSDIO,      ESP_PD_OPTION_ON);
  esp_sleep_pd_config(ESP_PD_DOMAIN_MAX,          ESP_PD_OPTION_ON);
  //
  SetupFS();
}

void loop() {
  delay(min(timer.tick()*1000,(long unsigned int) 59000));
  esp_task_wdt_reset();
  if (timer.size()==0) {
    Log.print("Sem timers, reinicia");
    ESP.restart();
  }
}
/* TODO:
 *  Avisar quando o equipamento entra em operação? ou O webhook deve avisar quando para de receber?
 *  Testar OTA no outro comp
 *  Colocar telnet stream pra funcionar
 *  Colocar o detector de fumaça pra funcionar
 *  Gravar os timiers na memória
 *  Ao reiniciar por perda de conexão, grava na SRAM o mimmax, timers e um bloco de dados de medição
 *  Se ão conseguir enviar o dado grava na memória, e seta um flag que tem conteudo ainda não enviado e grava o conteudo na memória
 *  Ao conseguir conexão esvazia o bloco de dados da memória antes de enviar o próximo
 *  A qualquer alteração nos timers sempre altera a memória SRAM
 *  Detectar o fuso horário pela localização do IP
 *  Salvar hora do reboot e motivo, acumulando dados, quando reestabelecer a conexão envia para o bot a hora reinicialização e o motivo
 */
/* Atualizações:
 *  03/08/22: Usa o wifimanager para tentar conectar na rede de casa
 *  03/08/22: Inibe alguns logs
 *  03/08/22: Mostra o heap free e acomapanha a fragmentação do heap space
 *  15/08/22: Aumenta a amostragem das leituras analógicas
 *  16/09/22: Mostra além do tempo total de atividade, mostra a hora que foi iniciado o equipamento, considerando o atraso entre ligar o equipamento, conectar na rede e sincronizar a hora
 */
