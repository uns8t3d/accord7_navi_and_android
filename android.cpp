#include "Arduino.h"
#include "android.h"
#include <SoftwareSerial.h>
#include "EEPROM.h"

SoftwareSerial mySerial(8, 9);
Time currentTime;

//  optimization of software serial, not read while important processes runs
bool allowSoftwareSerialRead = true;

// buffer for serial message
static uint8_t rx_buffer[RX_BUFFER_SIZE];
static uint8_t rx_idx = 0;
static uint8_t rx_state = RX_WAIT_START;

// manual AC state variables
uint8_t selectedMode = 0x00;
uint8_t selectedFanSpeed = 0x00;
uint8_t acOn = 0x00;

// music info variables
char title[72];
bool receivingTitle = false;
bool musicOn = false;
unsigned long int musicOnTimer = 0;
int titleIndex = 0;
unsigned long previousSubDisplayUpdate = 0;
const unsigned long updateSubdisplayMusicInterval = 300;
int titlePosition = 0;
char displayBuffer[SUBDISPLAY_WIDTH+1];

// base variables
bool timeInitialized = false;
bool climateStateRestored = false;

Android::Android() {
  
}

void Android::begin() {
    acOn = EEPROM.read(EEPROM_AC_STATE);
    selectedMode = EEPROM.read(EEPROM_MODE_STATE);
    selectedFanSpeed = EEPROM.read(EEPROM_FAN_STATE);
    mySerial.begin(38400);  
}

int Android::read() {
  if (timeInitialized && !climateStateRestored) {
    createMessage();        
    climateStateRestored = true;
  }
  if (allowSoftwareSerialRead && mySerial.available()) {
    uint8_t byteRead = mySerial.read();
    int result = readMessage(byteRead);
    return result;
  } 
  return NONE;
}

int Android::readMessage(uint8_t ch)
{
	switch (rx_state) {
		case RX_WAIT_START:
      memset(rx_buffer, 0, sizeof(rx_buffer));
			if (ch != START_BYTE)
				break;
			rx_idx = 0;
			rx_buffer[rx_idx++] = ch;
			rx_state = RX_CMD;
			break;
		case RX_CMD:
      if (ch == MUSIC_STATE_MESSAGE || ch == GENERAL_MESSAGE || ch == MUSIC_INFO_MESSAGE) {
        rx_buffer[rx_idx++] = ch;
			rx_state = RX_LEN;
			break;
      } else {
        rx_state = RX_WAIT_START;
        rx_idx = 0;        
        break;
      }      
		case RX_LEN:
			rx_buffer[rx_idx++] = ch;
			rx_state = ch ? RX_DATA : RX_CRC;
			break;
		case RX_DATA:
			rx_buffer[rx_idx++] = ch;
			{
				uint8_t len = rx_buffer[2];
				rx_state = ((rx_idx - 2) > len) ? RX_CRC : RX_DATA;
			}
			break;
		case RX_CRC:
			rx_buffer[rx_idx++] = ch;
      rx_buffer[rx_idx++] = 0xFF;
			rx_state = RX_WAIT_START;
      int result = processMessage(rx_buffer, sizeof(rx_buffer));
      return result;
	}
	if (rx_idx > RX_BUFFER_SIZE)
		rx_state = RX_WAIT_START;
  return 0;
}

int Android::processMessage(const uint8_t* message, int length) {
  switch (message[1]) {
    case GENERAL_MESSAGE:
      if (message[3] == 0x50) {
        setTime(currentTime, int(message[4]), int(message[5]), int(message[6]));
        timeInitialized = true;  
        break;
      }
      if (message[2] == 0x02) {
        switch (message[3]) {
          case 0xAC:
            switch (message[4]) {
              case 0x01:
                allowSoftwareSerialRead = false;
                acOn = 0x40;
                saveToEEPROM(acOn, EEPROM_AC_STATE);
                return AC_ON;
              case 0x02:
                allowSoftwareSerialRead = false;
                acOn = 0x00;
                saveToEEPROM(acOn, EEPROM_AC_STATE);
                return AC_OFF;
              case 0x03:
                allowSoftwareSerialRead = false;
                selectedMode = 0x40;
                saveToEEPROM(selectedMode, EEPROM_MODE_STATE);
                return MODE1;
              case 0x04:
                allowSoftwareSerialRead = false;
                selectedMode = 0x60;
                saveToEEPROM(selectedMode, EEPROM_MODE_STATE);
                return MODE2;
              case 0x05:
                allowSoftwareSerialRead = false;
                selectedMode = 0x20;
                saveToEEPROM(selectedMode, EEPROM_MODE_STATE);
                return MODE3;
              case 0x06:
                allowSoftwareSerialRead = false;
                selectedMode = 0xA0;
                saveToEEPROM(selectedMode, EEPROM_MODE_STATE);
                return MODE4;
            }
            break;
          case 0xAD:
            switch (message[4]) {
              case 0x01:
                allowSoftwareSerialRead = false;
                selectedFanSpeed = 0x02;
                saveToEEPROM(selectedFanSpeed, EEPROM_FAN_STATE);
                return FANSPEED1;
              case 0x02:
                allowSoftwareSerialRead = false;
                selectedFanSpeed = 0x02;
                saveToEEPROM(selectedFanSpeed, EEPROM_FAN_STATE);
                return FANSPEED1;
              case 0x03:
                allowSoftwareSerialRead = false;
                selectedFanSpeed = 0x03;
                saveToEEPROM(selectedFanSpeed, EEPROM_FAN_STATE);
                return FANSPEED2;
              case 0x04:
                allowSoftwareSerialRead = false;
                selectedFanSpeed = 0x04;
                saveToEEPROM(selectedFanSpeed, EEPROM_FAN_STATE);
                return FANSPEED3;
              case 0x05:
                allowSoftwareSerialRead = false;
                selectedFanSpeed = 0x05;
                saveToEEPROM(selectedFanSpeed, EEPROM_FAN_STATE);
                return FANSPEED4;
              case 0x06:
                allowSoftwareSerialRead = false;
                selectedFanSpeed = 0x06;
                saveToEEPROM(selectedFanSpeed, EEPROM_FAN_STATE);
                return FANSPEED5;
              case 0x07:
                allowSoftwareSerialRead = false;
                selectedFanSpeed = 0x06;
                saveToEEPROM(selectedFanSpeed, EEPROM_FAN_STATE);
                return FANSPEED5;
            }
            break;
        }
      }
      break;
    case MUSIC_INFO_MESSAGE:
      // check only for necessary messages  
      if (message[4] != 0xA7 && message[3] != 0x03) {         
        if (titleIndex == 0) {
          memset(title, 0, sizeof(title));
        }
        if (message[3] == MUSIC_ARTIST) {
          receivingTitle = true;
        }
        for (int i = 4; i <= 35; i++) {
          // add to array only if char bytes
          if (message[i] >= 0x20 && message[i] <= 0x7E) {       
              title[titleIndex] = char(message[i]);
              titleIndex += 1;
            }
          }
        if (message[3] == MUSIC_TITLE) {
          title[titleIndex] = ' ';
          titleIndex += 1;
          title[titleIndex] = ' ';
          titleIndex += 1;
          title[titleIndex] = ' ';
          titleIndex += 1;
        }  
        if (message[3] == MUSIC_ARTIST) {
          title[titleIndex] = ' ';  
          titleIndex += 1;
          title[titleIndex] = '-';  
          titleIndex += 1;
          title[titleIndex] = ' ';  
          titleIndex += 1;
        }
        if (receivingTitle) {
          receivingTitle = false;
          titleIndex = 0;
          titlePosition = 0;
        }        
        break;
      }
      break;
    case MUSIC_STATE_MESSAGE:
      musicOn = true;
      musicOnTimer = millis();
      break;      
  }
  if (musicOn && millis() - musicOnTimer >= 1100) {
    musicOn = false;
  }
  return NONE;
}

uint8_t Android::calculateChecksum(uint8_t * buf, uint8_t len)
{
	uint8_t sum = 0;
	for (uint8_t i = 0; i < len; i++)
		sum += buf[i];
	sum = sum ^ 0xff;
	return sum;
}

void Android::sendMessage(uint8_t type, uint8_t * msg, uint8_t size)
{
	uint8_t buf[4 + size];
	buf[0] = 0x2E;
	buf[1] = type;
	buf[2] = size;
	memcpy(buf + 3, msg, size);
	buf[3 + size] = calculateChecksum(buf + 1, size + 2);
	mySerial.write(buf, sizeof(buf));
  allowSoftwareSerialRead = true;
}

void Android::defaultState() {
  acOn = 0x00;
  saveToEEPROM(acOn, EEPROM_AC_STATE);
  selectedMode = 0x00;
  saveToEEPROM(selectedMode, EEPROM_MODE_STATE);
  selectedFanSpeed = 0x00;
  saveToEEPROM(selectedFanSpeed, EEPROM_FAN_STATE);
  uint8_t buffer[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
  sendMessage(0x21, buffer, sizeof(buffer));
}

void Android::createMessage() {
    allowSoftwareSerialRead = false;
    uint8_t buffer[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
    buffer[0] |= acOn;
    buffer[1] |= selectedMode;
    buffer[1] += selectedFanSpeed;
    sendMessage(0x21, buffer, sizeof(buffer));       
}

void Android::setTime(Time &time, int h, int m, int s) {
  time.hours = h;
  time.minutes = m;
  time.seconds = s;
}

bool Android::clockAvailable() {
  return timeInitialized;
}

Time Android::getClock() {
  return currentTime;
}

bool Android::musicAvailable() {
  return musicOn;
}

char* Android::getTrackName() {
  return title;
}

char* Android::getTrackDisplayNamePartial() {
  int textLength = strlen(title);
  unsigned long currentMillis = millis();
  if (currentMillis - previousSubDisplayUpdate >= updateSubdisplayMusicInterval) {       
    previousSubDisplayUpdate = currentMillis;
    for (int i = 0; i < SUBDISPLAY_WIDTH && i < textLength; i++) {
      int charPosition = (titlePosition + i) % textLength;
      displayBuffer[i] = title[charPosition];
    }
    displayBuffer[SUBDISPLAY_WIDTH] = '\0';
    titlePosition = (titlePosition + 1) % textLength;
    return displayBuffer;
  }  
  return displayBuffer;
}

void Android::saveToEEPROM(uint8_t value, int index) {
  // optimize EEPROM usage
  if (EEPROM.read(index) != value) {
    EEPROM.update(index, value);  
  }
}
