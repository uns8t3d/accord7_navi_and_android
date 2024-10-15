#include "HardwareSerial.h"
#include "Arduino.h"
#include "navi_hvac.h"
#include <SPI.h>

volatile bool frameIsFree = false;
byte climatState[CLIMAT_MSG_LEN];
byte climatLastState[CLIMAT_MSG_LEN];
bool startReadState = false;

byte climatCmd[CLIMAT_CMD_LEN] = { 0x66, 0x0A, 0x98, 0xFF, 0x40, 0x41, 0x59, 0x26, 0x00, 0x00, 0x00, 0x00, 0x00 };

NaviHVAC::NaviHVAC() {

}

void NaviHVAC::begin() {
  pinMode(FRAME_PIN, INPUT_PULLUP);
  pinMode(CONTROL_A, OUTPUT);    
  digitalWrite(CONTROL_A, LOW);
  Serial.begin(19200, SERIAL_8E1);
  attachInterrupt(1, frameChange, CHANGE);
}

boolean NaviHVAC::read() {
  static byte index = 0;

  if (Serial.available()) {
    byte inByte = Serial.read();

    if (startReadState) {
      climatState[index++] = inByte;

      if (index == 3 && inByte != 0x1E) {
        startReadState = false;
        index = 0;
      } else if (index == CLIMAT_MSG_LEN) {
        index = 0;
        startReadState = false;
        memcpy(climatLastState, climatState, CLIMAT_MSG_LEN);
        // printMessage(climatState);
        return true;
      }
    }

    if (inByte == 0x98 && !startReadState) {
      climatState[index++] = inByte;
      startReadState = true;
    }
  }

  return false;
}

void NaviHVAC::printMessage(byte* message) {
  Serial.print("Message: ");
  for (int i = 0; i < CLIMAT_MSG_LEN; i++) {
      Serial.print("0x");
      Serial.print(message[i], HEX); // Выводим байты в шестнадцатеричном формате
      Serial.print(" ");
  }
  Serial.println();
  Serial.print("pin is free: ");
  Serial.print(digitalRead(FRAME_PIN));
  Serial.println();
}

void NaviHVAC::getState(byte *state) {
  memcpy(state, climatLastState, CLIMAT_MSG_LEN);
}

uint16_t NaviHVAC::getDTemp() {
  return (climatLastState[8] << 8) | climatLastState[9];
}

uint16_t NaviHVAC::getPTemp() {
  return (climatLastState[10] << 8) | climatLastState[11];
}

boolean NaviHVAC::isAcOn() {
  return climatLastState[8] != 0;
}

void NaviHVAC::sendCommand(uint16_t command) {
  byte group = command >> 8;
  byte action = command & 0xFF;
  byte crc = 0x00;

  climatCmd[10] = group;
  climatCmd[11] = action;

  for (byte i = 1; i < CLIMAT_CMD_LEN - 1; i++) {
    crc ^= climatCmd[i];
  }

  climatCmd[12] = crc;

  while(!frameIsFree);
  
  pinMode(FRAME_PIN, OUTPUT);
  digitalWrite(CONTROL_A, HIGH);  
  digitalWrite(FRAME_PIN, LOW);
  delay(300);
  while(frameIsFree);
  Serial.write(climatCmd, CLIMAT_CMD_LEN);
  Serial.flush();
  digitalWrite(CONTROL_A, LOW);
  digitalWrite(FRAME_PIN, 1);
  pinMode(FRAME_PIN, INPUT_PULLUP);
  attachInterrupt(1, frameChange, CHANGE);
}

void NaviHVAC::frameChange() {
  frameIsFree = digitalRead(FRAME_PIN); 
}