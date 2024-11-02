#include "android.h"
#include "navi_subdisplay.h"
#include "navi_hvac.h"
#include "can.h"

NaviSubDisplay subDisplay;
NaviHVAC hvac;
Android android;
BCAN can;
Time time;

uint16_t dTemp;
uint16_t pTemp;

// optimizations for subdisplay render
unsigned long int timer = 0;
unsigned long updateSubdisplayInterval = 0;

// variables for restoration of climate default state
bool wasAcOn = false;
bool actionPerformed = false;

// variables to prevent frequent command sending
unsigned long int command_send_timer = 0;
const int COMMAND_SEND_TIMEOUT = 1000;

void setup() {
  android.begin();
  subDisplay.begin();
  hvac.begin();
  can.begin();
  // Serial.begin(38400);  // initialize Serial. This is the only baud rate that works with. Only for debug purpose, should be disabled in prod
}

void loop() {
  bool isRead = can.read();
  if (isRead) {
    android.createDoorsMessage(can.getDoorsState());    
  }
  hvac.read();
  int command = android.read();
  if (command != 0) {
      hvac.sendCommand(COMMANDS[command]);
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
    wasAcOn = true;
    actionPerformed = false;
    dTemp = hvac.getDTemp();
    pTemp = hvac.getPTemp();
    subDisplay.setClimatTemp(dTemp, pTemp);
  } else if (wasAcOn && !actionPerformed) {
    android.defaultState();
    actionPerformed = true;
    wasAcOn = false;
  }
  if (android.clockAvailable()) {
      time = android.getClock();
      subDisplay.clock(time.hours, time.minutes, time.seconds % 2 == 0);
  }
  if (android.musicAvailable()) {
    char* displayBuffer = android.getTrackDisplayNamePartial();
    subDisplay.text(displayBuffer);
  }
  subDisplay.render();
}