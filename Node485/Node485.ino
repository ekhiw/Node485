#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include "NTPClient.h"

//needed for wifi manager
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "WiFiManager.h"         //https://github.com/tzapu/WiFiManager

#include "ESP8266HTTPClient.h"

#include <ThingSpeak.h>
#include <SoftwareSerial.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "id.pool.ntp.org", 3600*7, 60000);
WiFiClient  client;

unsigned long t0,tnow;
unsigned long interval;

#define RS485Transmit    HIGH
#define RS485Receive     LOW
#define RS485Control D3

#define RS485DI D0
#define RS485RO D1

SoftwareSerial mySerial(RS485RO, RS485DI);

#define CH_ID 213899           //Test channel for counting
#define READ_API "445L8T100BVZ2D1H" //API Key for Test channel

void initWifiManager() {
  WiFiManager wifiManager;
  //  wifiManager.resetSettings();
  wifiManager.setAPStaticIPConfig(IPAddress(192, 168, 11, 1), IPAddress(192, 168,11, 1), IPAddress(255, 255, 255, 0));
  wifiManager.autoConnect("Ekhiw Device");
}

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  
  pinMode(RS485Control,OUTPUT);

  initWifiManager();

  ThingSpeak.begin(client);

  timeClient.begin();
  timeClient.update();
  t0 = timeClient.getEpochTime();
}

void loop() {
  // put your main code here, to run repeatedly:

  if(cekInterval(15)){ //run every n second
    Serial.println("loop");
    int dec = getThingSpeakData();
    transmitMode();
    mySerial.print(String(dec)+"#");
  }else{
    recieveMode();
  }
}

int getThingSpeakData(){
  int statusCode = 0;

  int dataFromServer = ThingSpeak.readIntField(CH_ID, 1,READ_API);  
  
  statusCode = ThingSpeak.getLastReadStatus();

  if(statusCode ==200){
    Serial.println("dataFromServer "+ String(dataFromServer));
    return dataFromServer;
  }else{
    return -1;
  }
}

bool cekInterval(int interval){
  timeClient.update();
  tnow = timeClient.getEpochTime();
  unsigned long tdif = tnow-t0;
  if(tdif>=interval){
    t0=tnow;
    return true;
  }else{
    return false;
  }
}

void transmitMode(){
  digitalWrite(RS485Control,RS485Transmit);
  delay(10);
}

void recieveMode(){
  digitalWrite(RS485Control,RS485Receive);
  delay(10);
}
