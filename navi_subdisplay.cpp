#include "navi_subdisplay.h"
#include <SPI.h>

SPISettings settings(38000, MSBFIRST, SPI_MODE0);
byte latchesData[4][LATCH_SIZE];

NaviSubDisplay::NaviSubDisplay() {

}

void NaviSubDisplay::begin() {
  pinMode(SLAVE_PIN, OUTPUT);
  digitalWrite(SLAVE_PIN, LOW);

  SPI.begin();
}

void NaviSubDisplay::render() {
  for (byte l = 0; l < 4; l++) {
    SPI.beginTransaction(settings);

    for (byte i = 0; i < DATA_SIZE; i++) {
      SPI.transfer(latchesData[l][i]);
    }

    SPI.transfer(latchAddress[l] | latchesData[l][DATA_SIZE]);

    SPI.endTransaction();

    digitalWrite(SLAVE_PIN, HIGH);
    digitalWrite(SLAVE_PIN, LOW);

    //delay(5);
  }
}

void NaviSubDisplay::text(char *pString) {
  byte pos = 1;

  while(*pString) {
    byte asciiIndex = (int) *pString++ - 32;

    byte charInd = round(pos / 2.0);

    for (byte i = 0; i < 4; i++) {
      latchesData[i][charInd] |= (pos % 2 == 0 ? asciiMap[asciiIndex][i] >> 4 : asciiMap[asciiIndex][i]);
    }

    pos++;

    if (pos > 8) {
      break;
    }
  }
}

void NaviSubDisplay::clear() {
  memset(latchesData, 0, sizeof(latchesData));
}

void NaviSubDisplay::setClimatTemp(uint16_t dTemp, uint16_t pTemp) {
  static int8_t dTempIndex = -1;
  static int8_t pTempIndex = -1;

  for (byte i = 0; i < 15; i++) {
    if (tempIndexes[i] == dTemp) {
      dTempIndex = i;
    }

    if (tempIndexes[i] == pTemp) {
      pTempIndex = i;
    }
  }

  latchesData[3][6] |= 0b10000000; // right temp
  latchesData[3][5] |= 0b00000001; // right frame
  latchesData[3][0] |= 0b00000010; // left temp
  latchesData[2][0] |= 0b00001000; // left frame

  for (byte b = 0; b < 3; b++) {
    if (dTempIndex >= 0) {
      byte latchByte = tempMap[dTempIndex][0][b][0];

      for (byte l = 0; l < 4; l++) {
        byte latchData = tempMap[dTempIndex][0][b][l + 1];

        if (latchData != 0) {
          latchesData[l][latchByte] |= latchData;
        }
      }
    }

    if (pTempIndex >= 0) {
      byte latchByte = tempMap[pTempIndex][1][b][0];

      for (byte l = 0; l < 4; l++) {
        byte latchData = tempMap[pTempIndex][1][b][l + 1];

        if (latchData != 0) {
          latchesData[l][latchByte] |= latchData;
        }
      }
    }
  }
}

void NaviSubDisplay::clock(byte hours, byte minutes, boolean showPoints) {
  latchesData[3][5] |= 0b00000100; // CLOCK

  if (showPoints) {
    latchesData[3][5] |= 0b00010000; // points
  }

  byte hourDigit1 = hours / 10;
  byte hourDigit2 = hours % 10;
  byte minuteDigit1 = minutes / 10;
  byte minuteDigit2 = minutes % 10;

  byte digits[4] = { hourDigit1, hourDigit2, minuteDigit1, minuteDigit2 };

  for (byte d = 0; d < 4; d++) {
    byte digitsOffset = d == 0 ? 0 : 1;

    for (byte l = 0; l < 4; l++) {
      latchesData[l][5] |= (clockDigits[digits[d]][digitsOffset][l] << (6 - (d * 2)));
    }
  }

  /*byte shift = 6;

  latchesData[0][5] |= 0b00000110 << shift;
  latchesData[1][5] |= 0b00000010 << shift;
  latchesData[2][5] |= 0b00000110 << shift;
  latchesData[3][5] |= 0b00000010 << shift;*/
}
