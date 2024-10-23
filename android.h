#include <stdint.h>
#ifndef android_h
#define android_h
#include <Arduino.h>

const byte MESSAGE_LENGTH = 6;
const byte START_BYTE = 0x2E;

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
    int canbox_process(uint8_t ch);
    int processMessage(const uint8_t* message, int length);
    uint8_t canbox_checksum(uint8_t * buf, uint8_t len);
    void send_canbox_msg(uint8_t type, uint8_t * msg, uint8_t size);
    void createMessage();
    void printMessage();
    void setTime(Time &time, int h, int m, int s);
    bool isStartSequence(const uint8_t* sequence, int length);
    bool clockAvailable();
    void saveToEEPROM(uint8_t value, int index);
    Time getClock();
    bool musicAvailable();
    char* getTrackName();
};
#endif