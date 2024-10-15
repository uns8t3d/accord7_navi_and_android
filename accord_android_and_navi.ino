#include "android.h"
#include "navi_subdisplay.h"
#include "navi_hvac.h"
#include <iarduino_RTC.h>
#include <EEPROM.h>

#include <NecDecoder.h>

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

// define infrared buttons code
#define IR_UP 0x18
#define IR_DOWN 0x4A
#define IR_LEFT 0x10
#define IR_RIGHT 0x5A
#define IR_OK 0x38
#define IR_1 0xA2
#define IR_2 0x62
#define IR_3 0xE2
#define IR_0 0x98

uint16_t commands[16] = {0x0000, 0x0301, 0x0302, 0x0303, 0x0304, 0x0201, 0x0200, 0x0000, 0x0000, 0x0000, 0x0000, 0x0501, 0x0502, 0x0503, 0x0504, 0x0505};

NaviSubDisplay subDisplay;
NaviHVAC hvac;
Android android;
NecDecoder ir;

unsigned int CURRENT_TEXT = 0;

uint16_t dTemp;
uint16_t pTemp;
unsigned long int timer = 0;

boolean clockPointsState = false;
iarduino_RTC watch(RTC_DS1307);
bool show_semicolon = false;
int lastHour;
int lastMinute;

void setup() {
  android.begin();  
  subDisplay.begin();
  hvac.begin();
  EEPROM.get(0, CURRENT_TEXT);   
  attachInterrupt(0, irIsr, FALLING);
  watch.begin();
  // Serial.begin(9600);  // initialize Serial. This is the only baud rate that works with
}

void irIsr() {
  // callback for IR interrupt
  ir.tick();
}

void loop() {
  hvac.read();
  int command = android.read();  
  if (command != 0) {
    hvac.sendCommand(commands[command]);
    android.createMessage();
    android.sendMessage();
  }
  if (ir.available()) {
    ir_callback(ir.readCommand());
  }
  renderSubdisplay();
}

void renderSubdisplay() {
  subDisplay.clear();
  if (hvac.isAcOn()) {
    dTemp = hvac.getDTemp();
    pTemp = hvac.getPTemp();

    subDisplay.setClimatTemp(dTemp, pTemp);  
  }
  if (millis() - timer >= 1000) {
    watch.gettime();
    if (watch.seconds % 2 == 0) {
      show_semicolon = true;
    } else {
      show_semicolon = false;
    }
    lastHour = watch.Hours;
    lastMinute = watch.minutes;
    timer = millis();
  }  
  subDisplay.clock(lastHour, lastMinute, show_semicolon);
  switch (CURRENT_TEXT) {
    case 0:
      break;
    case 1:
      subDisplay.text("UNS8T3D");
      break;
    case 2:
      subDisplay.text("HONDA");
      break;
    case 3:
      subDisplay.text("ACCORD");
      break;
    default:
      break;
  }  
  subDisplay.render();
}

void ir_callback(byte command) {
  watch.gettime();
  volatile int minutes = watch.minutes;
  volatile int hours = watch.Hours;
  volatile bool setTime = false;
  switch (command) {
    case IR_UP:
      setTime = true;
      if (hours == 23) {
        hours = 0;
      } else {
        hours++;
      }
      break;
    case IR_DOWN:
      setTime = true;
      if (hours == 0) {
        hours = 23;
      } else {
        hours--;
      }
      break;
    case IR_LEFT:
      setTime = true;
      if (minutes == 0) {
        minutes = 59;
      } else {
        minutes--;
      }
      break;
    case IR_RIGHT: 
      setTime = true;
      if (minutes == 59) {
        minutes = 0;
      } else {
        minutes++;
      }
      break;
    case IR_OK:
      break;
    case IR_1:
      EEPROM.put(0, 1);
      CURRENT_TEXT = 1;
      break;
    case IR_2:
      EEPROM.put(0, 2);
      CURRENT_TEXT = 2;
      break;
    case IR_3:
      EEPROM.put(0, 3);
      CURRENT_TEXT = 3;
      break;
    case IR_0:
      EEPROM.put(0, 0);
      CURRENT_TEXT = 0;
      break;
  }
  if (setTime) {
    watch.settime(0, minutes, hours);
    setTime = false;
  }
}