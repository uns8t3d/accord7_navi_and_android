#ifndef navi_hvac_h
#define navi_hvac_h

#include "Arduino.h"

#define CLIMAT_MSG_LEN 16
#define CLIMAT_CMD_LEN 13
#define FRAME_PIN 3
#define CONTROL_A 5

class NaviHVAC {
	public:
		NaviHVAC();
		void begin();
    boolean read();
    void getState(byte *state);
    uint16_t getDTemp();
    uint16_t getPTemp();
    boolean isAcOn();
    void sendCommand(uint16_t command);
    void printMessage(byte* message);
  private:
    static void frameChange();
};

#endif
