#include "Arduino.h"
#include "can.h"
#include <SPI.h>
MCP2515 car(CAN_SPI_PIN);

CarDoorsState carDoorsState;
bool doorsStateUpdated = false;
uint8_t doorMessage = 0x00;
uint8_t bonnetMessage = 0x00;
uint8_t trunkMessage = 0x00;

void BCAN::begin() {
  pinMode(CAN_SPI_PIN, OUTPUT);
  pinMode(aumode, OUTPUT);
  pinMode(aunstb, OUTPUT);
  digitalWrite(CAN_SPI_PIN, HIGH);
  digitalWrite(aunstb, HIGH);
  digitalWrite(aumode, HIGH);
  car.reset();
  car.setBitrate(CAN_33KBPS, MCP_16MHZ);

  // set filtering mask
  car.setFilterMask(MCP2515::MASK0, true, 0x1FFFFFFF);
  car.setFilterMask(MCP2515::MASK1, true, 0x1FFFFFFF);

  // set filtering ids
  car.setFilter(MCP2515::RXF0, true, 0x92F81010);
  car.setFilter(MCP2515::RXF1, true, 0x8AF81111);
  car.setFilter(MCP2515::RXF2, true, 0x92F83010);
  car.setNormalMode();
}

bool BCAN::read() {
  if (doorsStateUpdated) {
    doorsStateUpdated = false;
    return true;
  }
  digitalWrite(CAN_SPI_PIN, LOW);
  if (car.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    updateStructure();
  }
  digitalWrite(CAN_SPI_PIN, HIGH);
  return false;
}

void BCAN::updateStructure() {
  switch (canMsg.can_id) {
    case 0x92F81010:
      carDoorsState.trunk = canMsg.data[0] & 0x80;
      if (canMsg.data[0] != trunkMessage) {
        doorsStateUpdated = true;        
      }
      trunkMessage = canMsg.data[0];
      break;
    case 0x8AF81111:
      carDoorsState.bonnet = canMsg.data[0] & 0x80;
      if (canMsg.data[0] != bonnetMessage) {
        doorsStateUpdated = true;        
      }
      bonnetMessage = canMsg.data[0];
      break;
    case 0x92F83010:
      carDoorsState.fl_door = canMsg.data[0] & 0x80;
      carDoorsState.fr_door = canMsg.data[0] & 0x40;
      carDoorsState.rl_door = canMsg.data[0] & 0x20;
      carDoorsState.rr_door = canMsg.data[0] & 0x10;
      if (canMsg.data[0] != doorMessage) {
        doorsStateUpdated = true;        
      }
      doorMessage = canMsg.data[0];
      break;    
  }
}

CarDoorsState BCAN::getDoorsState() {
  return carDoorsState;
}