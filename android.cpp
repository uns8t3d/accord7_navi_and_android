#include "Arduino.h"
#include "android.h"
#include <SoftwareSerial.h>

byte index = 0;
int selectedMode = 1;
int selectedFanSpeed = 1;
bool acOn = false;
byte message[12] = {0x2E, 0x01, 0x08, 0x16, 0x89, 0x11, 0x01, 0x00, 0x00, 0x00, 0x00, 0x45};
const unsigned int MAX_MESSAGE_LENGTH = 6;
SoftwareSerial mySerial(8, 9);
byte messageBuffer[MESSAGE_LENGTH];

extern int NONE = 0;
extern int MODE1 = 1;
extern int MODE2 = 2;
extern int MODE3 = 3;
extern int MODE4 = 4;
extern int AC_ON = 5;
extern int AC_OFF = 6;
extern int FANSPEED1 = 11;
extern int FANSPEED2 = 12;
extern int FANSPEED3 = 13;
extern int FANSPEED4 = 14;
extern int FANSPEED5 = 15;

Android::Android() {
  
}

void Android::begin() {
    mySerial.begin(9600);  
    sendMessage();
}

int Android::read() {
  if (mySerial.available()) {
    byte incomingByte = mySerial.read();
    // Serial.println(incomingByte, HEX);
    if (index == 0) {
      if (incomingByte == START_BYTE) {
        messageBuffer[index++] = incomingByte;  // Сохраняем стартовый байт и увеличиваем индекс
      }
    } else {
      messageBuffer[index++] = incomingByte;
      if (index == MESSAGE_LENGTH) {
        index = 0;
        int command = processMessage();
        return command;
      }
    }
  }
  return NONE;
}

int Android::processMessage() {
if (messageBuffer[3] == 0x08 && messageBuffer[5] == 0x83) {
    if (acOn) {
      acOn = false;
      return AC_OFF;
    } else {
      acOn = true;
      return AC_ON;
    }
  }
  if (messageBuffer[3] == 0xB && messageBuffer[5] == 0x80) {
    switch (selectedMode) {
      case 1:
        selectedMode = 2;
        return MODE2;
      case 2:
        selectedMode = 3;
        return MODE3;
      case 3:
        selectedMode = 4;
        return MODE4;
      case 4:
        selectedMode = 1;
        return MODE1;
    }
  }

  if (messageBuffer[3] == 0x6 && messageBuffer[5] == 0x85) {
    switch (selectedFanSpeed) {
      case 1:
        selectedFanSpeed = 2;
        return FANSPEED2;
      case 2:
        selectedFanSpeed = 3;
        return FANSPEED3;
      case 3:
        selectedFanSpeed = 4;
        return FANSPEED4;
      case 4:
        selectedFanSpeed = 5;
        return FANSPEED5;
      case 5:
        return NONE;
    }
  }

  if (messageBuffer[3] == 0x7 && messageBuffer[5] == 0x84) {
    switch (selectedFanSpeed) {
      case 1:
        return NONE;
      case 2:
        selectedFanSpeed = 1;
        return FANSPEED1;
      case 3:
        selectedFanSpeed = 2;
        return FANSPEED2;
      case 4:
        selectedFanSpeed = 3;
        return FANSPEED3;
      case 5:
        selectedFanSpeed = 4;
        return FANSPEED4;
    }
  }
  return NONE;  
}

void Android::createMessage() {
    int value;
  switch (selectedMode) {
    case 1:
      value = 136;
      break;
    case 2:
      value = 160;
      break;
    case 3:
      value = 152;
      break;
    case 4:
      value = 160;
      break;
  }
  value += selectedFanSpeed;
  int ac;
  int ac2;
  if (acOn) {
    ac = 21;
    ac2 = 3;
  } else {
    ac = 22;
    ac2 = 1;
  }
  int crc = 255 - value - ac - ac2 - 26;
  message[3] = ac;
  message[4] = value;
  message[6] = ac2;
  message[11] = crc;
}

void Android::sendMessage() {
  mySerial.write(message, 12);
  mySerial.flush();
}