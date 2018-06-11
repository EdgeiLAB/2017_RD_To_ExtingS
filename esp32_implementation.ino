#include <Arduino.h>
#include <MAX17043GU.h>
#include <Wire.h>
#include <WifiLocation.h>
#include <WiFi.h>
#include <WiFiClient.h>

#include "thingplug.h"
#include "SPI.h"
#include "SPIFFS.h"

#include "WROVER_KIT_LCD.h"
#include "AudioFileSourceSPIFFS.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

#define LOW_BATTERY  3.70     //3.80
#define TIME        100000    // 0.1초

// 핀 선언 및 정의
const int pinLed = 2;
const int pinSwitch = 4;

// 공유기, 구글 API, ThingPlug 엑세스 요구사항
const char *googleApiKey = "AIzaSyBjBymHPAl222oj_9JpM54bUZPmgxQr0Q4";
const char *ssid = "edgeiLAB";
const char *passwd = "iotiotiot";
const char *addr = "mqtt.sktiot.com";
const char *id = "edgeilab";
const char *pw = "ZEwxMW9DZmNQK3dudWdRcTV4bVhEK1ByK3U2amtxU3NCWjE0OERNREI3QkUwdCtsSmhZWDQ4eGRURkd0NVFIUw==";

///////////////////////////////////////////////////////
/// 변경하기!!!!!!!!!!!!!!!!!!!!!!!!!
const char *deviceId = "edgeilab_20180611_To_ExtingS_08";
const char *targetDeviceId = "edgeilab_20180611_To_FireD_01";
///////////////////////////////////////
///////////////////////////////////////////////////////
const char *devicePw = "123456";
const char *containerSmoke = "Smoke";
const char *containerGeolocation_latitude = "Geolocation_latitude";
const char *containerGeolocation_longtitude = "Geolocation_longtitude";



// 객체 생성
WROVER_KIT_LCD tft;
MAX17043GU battery;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

WifiLocation *location = NULL;
AudioGeneratorMP3 *mp3 = NULL;
AudioFileSourceSPIFFS *file = NULL;
AudioOutputI2S *out = NULL;
AudioFileSourceID3 *id3 = NULL;

// 측정 값 저장 변수
float voltageBattery = 0.0;
int voltagePercentage = 0;
int pre_voltagePercentage = 0;

// 위 경도 값 저장 변수
float latitude = 0.0;
float longtitude = 0.0;
float accuracy = 0.0;

// 상태 확인 변수
bool flagSwitch = false;
bool flagLed = false;
bool flagGenerator = false;
bool flagWarning = false;
bool flagCharge = false;

// 시간 측정 변수
int countSecond = 1;

// 디스플레이 문자열 변수
String strDisplay= String("0");

// 오디오 재생 파일명
const char *soundStart = "/start.mp3";
const char *soundWarning = "/warning.mp3";


// 타이머 인터럽트
hw_timer_t *timer = NULL;

void IRAM_ATTR onTimer() {
  if (flagLed) {
    digitalWrite(pinLed, LOW);
    flagLed = false;
  } else {
    digitalWrite(pinLed, HIGH);
    flagLed = true;
  }
}

void IRAM_ATTR onCounter() {
  countSecond++;              // TIME마다 업
}
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

void setup() {

  Serial.begin(115200);  

  SPIFFS.begin();   

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(WROVER_BLACK);   

  // 타이머 인터럽트 초기화
  timer = timerBegin(0, 80, true);            // timer 0, div 80MHz
  timerAttachInterrupt(timer, &onTimer, true);// attach callback
  timerAlarmWrite(timer, TIME*5, true);
  timerAlarmEnable(timer);
  
  // I2C 초기화
  Wire.begin();
  battery.restart();

  // Pin Setup
  setupPin_ledSwitch();
  
  tft.setTextColor(WROVER_YELLOW);
  Serial.println("SETUP START"); 
  displayString(30, 100, 4, "SETUP START");
  delay(2000);



  
  // 배터리 측정 --------------------------------------------
  tft.fillScreen(WROVER_BLACK);  
  tft.setTextColor(WROVER_WHITE);
  displayString(10, 80, 4, "BATTERY");
  displayString(10, 120, 4, "MEASURING...");
  while(!getBattery()) {
    // 실패
    Serial.println("Please charge the the battery");    
    delay(1000);
  }
  // 성공
  Serial.println("Succeed to measure the battery");
  delay(2000);  
  // --------------------------------------------------------

 
  // WiFi 연결 ----------------------------------------------  
  tft.fillScreen(WROVER_BLACK);  
  displayString(10, 80, 4, "WiFi");
  displayString(10, 120, 4, "CONNECTING..");  
  
  connectingWifi();     
  while(!isConnectingWifi()) {
    Serial.println("Fail to connect WiFi");
    Serial.println("Try Again");    
    delay(1000);
    connectingWifi();    
  }
  // 성공
  Serial.println("Succeed to connect WiFI");
  delay(2000);  
  // --------------------------------------------------------



  // ThingPlug 연결 -----------------------------------------
  tft.fillScreen(WROVER_BLACK);   
  displayString(10, 80, 4, "THINGPLUG");
  displayString(10, 120, 4, "CONNECTING..");    
  while(!connectingThingplug()) {
    Serial.println("Fail to connect ThingPlug");
    delay(1000);    
  }
  // 성공
  Serial.println("Succeed to connect ThingPlug");  
  delay(2000);  
  // --------------------------------------------------------



  // 위경도 측정 --------------------------------------------
  tft.fillScreen(WROVER_BLACK);    
  displayString(10, 80, 4, "GEOLOCATION");
  displayString(10, 120, 4, "CONNECTING..");    
  while(!getGeolocation()) {
    // 실패
    Serial.println("Fail to measure Geolocation");
    delay(1000);
  }
  // 성공
  Serial.println("Succeed to measure Geolocation");  
  delay(2000);
  // -------------------------------------------------------


  // 초기화 성공시 소리 출력, LED Blink Off ----------
  Serial.println("Setup Success");
  tft.fillScreen(WROVER_BLACK);   
  displayString(10, 80, 4, "SETUP");
  displayString(10, 120, 4, "SUCCESS..");  

  // 음악 재생
  audioGenerateMp3(soundStart);
  while(mp3->isRunning()) {
    if(!mp3->loop())  {    
      stop();    
      delay(10);
    }
  }
 
  // LED Blink 정지  
  timerEnd(timer);
  timer = NULL;
  digitalWrite(pinLed, LOW);


  // ---------------------------------------------------------
    
  // 외부 인터럽트 선언
  attachInterrupt(digitalPinToInterrupt(pinSwitch), handleInterrupt, RISING);  
  delay(2000);
  tft.fillScreen(WROVER_BLACK);     
  displayBattery();

  timer = timerBegin(0, 80, true);            // timer 0, div 80MHz
  timerAttachInterrupt(timer, &onCounter, true);// attach callback
  timerAlarmWrite(timer, TIME, true);       // 0.1 second
  timerAlarmEnable(timer);
  
}


void loop() {

  mqttClient.loop();

  if(flagSwitch) {

    if(!isGenerateMp3()) {      
      Serial.println("Generate MP3");
      audioGenerateMp3(soundWarning);
      delay(100);
    }
    
    if(mp3->isRunning()) {
      if(!mp3->loop())  {
        Serial.println("MP3 Finish");
        stop();      
        delay(100);        
      }
    }     
  }
  else {
    if(isGenerateMp3()) {
      Serial.println("MP3 Stop");     
      stop();
      delay(10);    
    }
    if(!flagWarning)
      digitalWrite(pinLed, LOW);    
  }

/////////////////////////////////////////////////////
///////////////// TEST Code /////////////////////////
/*
  if(flagSwitch) {
    if(!isDisplayedMessage("TEST")) {      
      Serial.println("Test Display");       
      strDisplay = String("TEST");                  
    }
  } 
  else {
    if(!isDisplayedMessage("NORMAL")) {
      Serial.println("Normal Display");           
      strDisplay = String("NORMAL");                  
    }       
  }        
  
*/  
///////////////////////////////////////////////////

  // 0.5초마다 디스플레이
  if(countSecond%5 == 0) { 
    if(flagWarning) {
      if(!isDisplayedMessage("WARNING")) {
        Serial.println("Warning Display");        
        digitalWrite(pinLed, HIGH);      
        strDisplay = String("WARNING");        

        tft.fillScreen(WROVER_RED);  
        tft.setTextColor(WROVER_BLACK);
        displayString(70, 100, 5, "WARNING");                
        displayBattery(); 
        delay(100);    
      }   
    }
    else if(flagSwitch) {
      if(!isDisplayedMessage("TEST")) {      
        Serial.println("Test Display");
        digitalWrite(pinLed, HIGH);        
        strDisplay = String("TEST");      

        tft.fillScreen(WROVER_RED);  
        tft.setTextColor(WROVER_BLACK);
        displayString(100, 100, 5, "TEST");           
        displayBattery(); 
        delay(100);                  
      }
    } 
    else if(flagCharge) {
      if(!isDisplayedMessage("CHARGE")) {            
        digitalWrite(pinLed, HIGH);         
        Serial.println("Charge Display");  
        strDisplay = String("CHARGE");
               
        tft.fillScreen(WROVER_RED);
        tft.setTextColor(WROVER_BLACK);
        displayString(80, 80, 4, "NEED TO");
        displayString(90, 120, 4, "CHARGE");  
        displayBattery(); 
        delay(100);           
      }          
    }
    else {
      if(!isDisplayedMessage("NORMAL") || pre_voltagePercentage != voltagePercentage) {
        pre_voltagePercentage = voltagePercentage;
        Serial.println("Normal Display");  
        strDisplay = String("NORMAL");    
        tft.fillScreen(WROVER_BLACK); 
        tft.setTextColor(WROVER_YELLOW);               
        displayString(80, 100, 5, "NORMAL");  
        displayBattery(); 
        delay(100);                 
      }       
    }    
  }

/////////////////////////////////////////////////////////////////////////////
  // 1초마다 디스플레이
  if(countSecond%10 == 0) {
    Serial.print("Count Second : ");///  
    Serial.println(countSecond/10);
  }
/////////////////////////////////////////////////////////////////////////
  // 30초마다 배터리 측정
  if(countSecond%300 == 0) {
    if(getBattery()) {
      flagCharge = false;    
    }
    else {
      flagCharge = true;
      printBattery();  
    }
  }

////////////////////////////////////////////////////////////////////////////  

  // 1800초마다 위경도 측정. 단, 경고 및 테스트가 아닐 경우
  if((countSecond%18000 == 0) && (flagWarning == false) && (flagSwitch == false)) {
    if(getGeolocation()) {
      mqttPublish_Geolocation();      
      printLocation();
      yield();
    }
  }
  
}



