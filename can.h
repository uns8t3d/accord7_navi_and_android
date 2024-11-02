#ifndef can_h
#define can_h
#define CAN_SPI_PIN 10
#include <mcp2515.h>

struct CarDoorsState {
  bool bonnet;
  bool trunk;
  bool fl_door;
  bool fr_door;
  bool rl_door;
  bool rr_door;
};

class BCAN
{
  public:
    struct can_frame canMsg;
    int aunstb = 6;
    int aumode = 7;

    void begin();
    bool read();
    void updateStructure();
    CarDoorsState getDoorsState();
};

#endif