#ifndef android_h
#define android_h
#include <Arduino.h>

const byte MESSAGE_LENGTH = 6;
const byte START_BYTE = 0x2E;

class Android {
  public:
    Android();
    void begin();
    int read();
    int processMessage();
    void createMessage();
    void sendMessage();
    void printMessage();
};
#endif