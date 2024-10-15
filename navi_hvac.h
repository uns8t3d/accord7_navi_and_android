#ifndef navi_hvac_h
#define navi_hvac_h

#include "Arduino.h"

#define CLIMAT_MSG_LEN 16
#define CLIMAT_CMD_LEN 13
#define FRAME_PIN 3
#define CONTROL_A 5

// #define DEFROST_ON_ 0x0801
// #define DEFROST_OFF 0x0800
// #define WIND_SHIELD_CENTER 0x0301
// #define WIND_SHIELD_CENTER_FLOOR 0x0302
// #define WIND_SHIELD_FLOOR 0x0303
// #define WIND_SHIELD_DEFROST_FLOOR 0x0304
// #define FAN_SPEED_1 0x0501
// #define FAN_SPEED_2 0x0502
// #define FAN_SPEED_3 0x0503
// #define FAN_SPEED_4 0x0504
// #define FAN_SPEED_5 0x0505
// #define AC_ON 0x0201
// #define AC_OFF 0x0200
// #define REC_ON 0x0B01
// #define REC_OFF 0x0B00

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
