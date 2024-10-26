#include <stdint.h>
#ifndef android_h
#define android_h
#include <Arduino.h>

#define RX_BUFFER_SIZE 64
#define NONE 0
#define MODE1 1
#define MODE2 2
#define MODE3 3
#define MODE4 4
#define AC_ON 5
#define AC_OFF 6
#define FANSPEED1 11
#define FANSPEED2 12
#define FANSPEED3 13
#define FANSPEED4 14
#define FANSPEED5 15

#define EEPROM_AC_STATE 0
#define EEPROM_MODE_STATE 1
#define EEPROM_FAN_STATE 2

#define SUBDISPLAY_WIDTH 8

// message sync byte
#define START_BYTE 0x2E

// message types
#define GENERAL_MESSAGE 0xC6
#define MUSIC_INFO_MESSAGE 0xCB
#define MUSIC_STATE_MESSAGE 0xC3

// message subtypes
#define MUSIC_TITLE 0x02
#define MUSIC_ARTIST 0x04

enum rx_state
{
	RX_WAIT_START,
	RX_LEN,
	RX_CMD,
	RX_DATA,
	RX_CRC
};

struct Time {
  int hours;
  int minutes;
  int seconds;
};

class Android {
  public:
    Android();
    void begin();
    
    int read();
    int readMessage(uint8_t ch);
    int processMessage(const uint8_t* message, int length);
    
    uint8_t calculateChecksum(uint8_t * buf, uint8_t len);
    void sendMessage(uint8_t type, uint8_t * msg, uint8_t size);
    void defaultState();
    void createMessage();
    
    void setTime(Time &time, int h, int m, int s);
    bool clockAvailable();
    Time getClock();
    
    bool musicAvailable();
    char* getTrackName();
    char* getTrackDisplayNamePartial();

    void saveToEEPROM(uint8_t value, int index);
};
#endif