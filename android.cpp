#include "Arduino.h"
#include "android.h"
#include <SoftwareSerial.h>
#include "EEPROM.h"

#define RX_BUFFER_SIZE 64

SoftwareSerial mySerial(8, 9);
Time currentTime;

bool allowSoftwareSerialRead = true;

static uint8_t rx_buffer[RX_BUFFER_SIZE];
static uint8_t rx_idx = 0;
static uint8_t rx_state = RX_WAIT_START;

uint8_t selectedMode = 0x00;
uint8_t selectedFanSpeed = 0x00;
uint8_t acOn = 0x00;

char title[72];
bool receivingTitle = false;
bool timeInitialized = false;
bool climateStateRestored = false;
bool musicOn = false;
unsigned long int musicOnTimer = 0;
int titleIndex = 0;

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
    acOn = EEPROM.read(0);
    selectedMode = EEPROM.read(1);
    selectedFanSpeed = EEPROM.read(2);
    mySerial.begin(38400);  
}

int Android::read() {
  if (timeInitialized && !climateStateRestored) {
    createMessage();        
    climateStateRestored = true;
  }
  if (allowSoftwareSerialRead && mySerial.available()) {
    uint8_t byteRead = mySerial.read();
    int result = canbox_process(byteRead);
    return result;
  } 
  return NONE;
}

int Android::canbox_process(uint8_t ch)
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
      if (ch == 0xC3 || ch == 0xC6 || ch == 0xCB) {
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

uint8_t Android::canbox_checksum(uint8_t * buf, uint8_t len)
{
	uint8_t sum = 0;
	for (uint8_t i = 0; i < len; i++)
		sum += buf[i];
	sum = sum ^ 0xff;
	return sum;
}

void Android::send_canbox_msg(uint8_t type, uint8_t * msg, uint8_t size)
{
	uint8_t buf[4 + size];
	buf[0] = 0x2E;
	buf[1] = type;
	buf[2] = size;
	memcpy(buf + 3, msg, size);
	buf[3 + size] = canbox_checksum(buf + 1, size + 2);
	mySerial.write(buf, sizeof(buf));
  allowSoftwareSerialRead = true;
}

int Android::processMessage(const uint8_t* message, int length) {
  switch (message[1]) {
    case 0xC6:
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
                saveToEEPROM(acOn, 0);
                return AC_ON;
              case 0x02:
                allowSoftwareSerialRead = false;
                acOn = 0x00;
                saveToEEPROM(acOn, 0);
                return AC_OFF;
              case 0x03:
                allowSoftwareSerialRead = false;
                selectedMode = 0x40;
                saveToEEPROM(selectedMode, 1);
                return MODE1;
              case 0x04:
                allowSoftwareSerialRead = false;
                selectedMode = 0x60;
                saveToEEPROM(selectedMode, 1);
                return MODE2;
              case 0x05:
                allowSoftwareSerialRead = false;
                selectedMode = 0x20;
                saveToEEPROM(selectedMode, 1);
                return MODE3;
              case 0x06:
                allowSoftwareSerialRead = false;
                selectedMode = 0xA0;
                saveToEEPROM(selectedMode, 1);
                return MODE4;
            }
            break;
          case 0xAD:
            switch (message[4]) {
              case 0x01:
                allowSoftwareSerialRead = false;
                selectedFanSpeed = 0x02;
                saveToEEPROM(selectedFanSpeed, 2);
                return FANSPEED1;
              case 0x02:
                allowSoftwareSerialRead = false;
                selectedFanSpeed = 0x02;
                saveToEEPROM(selectedFanSpeed, 2);
                return FANSPEED1;
              case 0x03:
                allowSoftwareSerialRead = false;
                selectedFanSpeed = 0x03;
                saveToEEPROM(selectedFanSpeed, 2);
                return FANSPEED2;
              case 0x04:
                allowSoftwareSerialRead = false;
                selectedFanSpeed = 0x04;
                saveToEEPROM(selectedFanSpeed, 2);
                return FANSPEED3;
              case 0x05:
                allowSoftwareSerialRead = false;
                selectedFanSpeed = 0x05;
                saveToEEPROM(selectedFanSpeed, 2);
                return FANSPEED4;
              case 0x06:
                allowSoftwareSerialRead = false;
                selectedFanSpeed = 0x06;
                saveToEEPROM(selectedFanSpeed, 2);
                return FANSPEED5;
              case 0x07:
                allowSoftwareSerialRead = false;
                selectedFanSpeed = 0x06;
                saveToEEPROM(selectedFanSpeed, 2);
                return FANSPEED5;
            }
            break;
        }
      }
      break;
    case 0xCB:     
      if (message[4] != 0xA7 && message[3] != 0x03) {         
        if (titleIndex == 0) {
          memset(title, 0, sizeof(title));
        }
        if (message[3] == 0x04) {
          receivingTitle = true;
        }
        for (int i = 4; i <= 35; i++) {
          if (message[i] >= 0x20 && message[i] <= 0x7E) {       
              title[titleIndex] = char(message[i]);
              titleIndex += 1;
            }
          }
        if (message[3] == 0x02) {
          title[titleIndex] = ' ';
          titleIndex += 1;
          title[titleIndex] = ' ';
          titleIndex += 1;
          title[titleIndex] = ' ';
          titleIndex += 1;
        }  
        if (message[3] == 0x04) {
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
        }        
        break;
      }
      break;
    case 0xC3:
      musicOn = true;
      musicOnTimer = millis();
      break;      
  }
  if (musicOn && millis() - musicOnTimer >= 1100) {
    musicOn = false;
  }
  return 0;
}

void Android::createMessage() {
    allowSoftwareSerialRead = false;
    uint8_t buffer[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
    buffer[0] |= acOn;
    buffer[1] |= selectedMode;
    buffer[1] += selectedFanSpeed;
    send_canbox_msg(0x21, buffer, sizeof(buffer));       
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

void Android::defaultState() {
  acOn = 0x00;
  saveToEEPROM(acOn, 0);
  selectedMode = 0x00;
  saveToEEPROM(selectedMode, 1);  
  selectedFanSpeed = 0x00;
  saveToEEPROM(selectedFanSpeed, 2);
  uint8_t buffer[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
  send_canbox_msg(0x21, buffer, sizeof(buffer));
}

void Android::saveToEEPROM(uint8_t value, int index) {
  EEPROM.update(index, value);  
}
