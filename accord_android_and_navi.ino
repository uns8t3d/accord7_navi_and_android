#include "android.h"
#include "navi_subdisplay.h"
#include "navi_hvac.h"

#define DEFROST_ON_ 0x0801
#define DEFROST_OFF 0x0800
#define WIND_SHIELD_CENTER 0x0301
#define WIND_SHIELD_CENTER_FLOOR 0x0302
#define WIND_SHIELD_FLOOR 0x0303
#define WIND_SHIELD_DEFROST_FLOOR 0x0304
#define FAN_SPEED_1 0x0501
#define FAN_SPEED_2 0x0502
#define FAN_SPEED_3 0x0503
#define FAN_SPEED_4 0x0504
#define FAN_SPEED_5 0x0505
#define AC_ON 0x0201
#define AC_OFF 0x0200
#define REC_ON 0x0B01
#define REC_OFF 0x0B00

uint16_t commands[16] = {0x0000, 0x0301, 0x0302, 0x0303, 0x0304, 0x0201, 0x0200, 0x0000, 0x0000, 0x0000, 0x0000, 0x0501, 0x0502, 0x0503, 0x0504, 0x0505};

NaviSubDisplay subDisplay;
NaviHVAC hvac;
Android android;
Time time;

uint16_t dTemp;
uint16_t pTemp;
unsigned long int timer = 0;
unsigned long previousMillis = 0;
const unsigned long interval = 300; 
unsigned long updateSubdisplayInterval = 0;
const int displayWidth = 8;
int position = 0;
bool show_semicolon = false;
int lastHour;
int lastMinute;

void setup() {
  android.begin();  
  subDisplay.begin();
  hvac.begin();
  // Serial.begin(38400);  // initialize Serial. This is the only baud rate that works with
}

void loop() {
  hvac.read();
  int command = android.read();  
  if (command != 0) {
    hvac.sendCommand(commands[command]);
    android.createMessage();
  }
  if (millis() - updateSubdisplayInterval >= 300) {
    renderSubdisplay();
    updateSubdisplayInterval = millis();
  }
}

void renderSubdisplay() {  
  subDisplay.clear();
  if (hvac.isAcOn()) {
    dTemp = hvac.getDTemp();
    pTemp = hvac.getPTemp();

    subDisplay.setClimatTemp(dTemp, pTemp);  
  }
  if (millis() - timer >= 1000) {
    if (android.clockAvailable()) {
       time = android.getClock();
       subDisplay.clock(time.hours, time.minutes, time.seconds % 2 == 0);
    }
  }
  if (android.musicAvailable()) {
    char* text = android.getTrackName();
    int textLength = strlen(text);
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    char displayBuffer[displayWidth];
    for (int i = 0; i < displayWidth; i++) {
      int charPosition = (position + i) % textLength;
      displayBuffer[i] = text[charPosition];
    }
    displayBuffer[displayWidth] = '\0';
    subDisplay.text(displayBuffer);
    position = (position + 1) % textLength;
  }  
  }      
  subDisplay.render();
}