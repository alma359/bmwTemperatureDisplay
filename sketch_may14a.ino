#include <SPI.h>
#include "mcp_can.h"
#include <TimedAction.h>

const int SPI_CS_PIN = 10;//pin used for can shield
MCP_CAN CAN(SPI_CS_PIN);// Set CS pin
//pins used for display
int pwmled1 = 9;
int pwmled2 = 6;
int pwmled3 = 5;
int dataPin = 3;
int latchPin = 7;
int clockPin = 4;
//these variables will store the current state
int firstdigit = -1;
int seconddigit = -1;
int thirddigit = -1;
int value = 0;

int lowbrightness = 10;//lower to make dimmer, used when lights are on
int highbrightness = 1180;//increase to make brighter, used when lights are off
int pwm = highbrightness;//stores the current brightness value. (low or high)


void displayvalue() { // displays the value in the 7segment-display

  if (value >= 100) { // example 312
     firstdigit = value/100; // firstdigit = 3
     int temp = value - (firstdigit*100); // 312 - 300 = 12
     seconddigit = temp/10; // seconddigit = 1
     thirddigit = temp - (seconddigit*10); // 12 - 10 = 2
     updatedisplay();
  } else if (value >= 10) { // example 31
    firstdigit = -1; // will shift out empty
    seconddigit = value/10; // 3
    thirddigit = value - (seconddigit*10); // 31 - 30 = 1
    updatedisplay();
  } else if (value >= 0 ) { //value < 10 // example 3
    firstdigit = -1;
    seconddigit = -1;
    thirddigit = value;
    updatedisplay();
  } else if (value > -10) {
    firstdigit = -1;
    seconddigit = -2;
    thirddigit = -value;
    updatedisplay();
  } else { // <= -10
    int t = -value;
    firstdigit = -2;
    seconddigit = t/10; // 3
    thirddigit = t - (seconddigit*10); // 31 - 30 = 1
    updatedisplay();
  }
}


TimedAction timedAction = TimedAction(1,displayvalue);

void setup() {
  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  while (CAN_OK != CAN.begin(CAN_100KBPS)) {
        delay(100);
    }
}

void loop() {
  unsigned char len = 0;
  unsigned char buf[8];
  if(CAN_MSGAVAIL == CAN.checkReceive()) {
    CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf
    
    if (CAN.getCanId() == 464) {
      value = buf[0] - 48;//engine temp is calculated.
    } else if (CAN.getCanId() == 538) {
      if ((buf[0] & 4) == 4) {//adjusting the brightness of display
        pwm = lowbrightness;//lights are on
      } else {
        pwm = highbrightness;//lights are off
      }
    }
  } 
  timedAction.check();
}

void shiftnext(int value) { // will shift a digit into the 595 Chip
  if (value == 0) {
    shiftOut(dataPin, clockPin, MSBFIRST, 192);// 64 + 128
  } else if (value == 1) {
    shiftOut(dataPin, clockPin, MSBFIRST, 249);// 121 + 128
  } else if (value == 2) {
    shiftOut(dataPin, clockPin, MSBFIRST, 164);// 36 + 128
  } else if (value == 3) {
    shiftOut(dataPin, clockPin, MSBFIRST, 176);// 48 + 128
  } else if (value == 4) {
    shiftOut(dataPin, clockPin, MSBFIRST, 153);// 25 + 128
  } else if (value == 5) {
    shiftOut(dataPin, clockPin, MSBFIRST, 146);// 18 + 128
  } else if (value == 6) {
    shiftOut(dataPin, clockPin, MSBFIRST, 130);// 2 + 128
  } else if (value == 7) {
    shiftOut(dataPin, clockPin, MSBFIRST, 248);// 120 + 128
  } else if (value == 8) {
    shiftOut(dataPin, clockPin, MSBFIRST, 128);// 0 + 128
  } else if (value == 9) {
    shiftOut(dataPin, clockPin, MSBFIRST, 144);// 24 + 128
  }
}

void updatefirstdigit() {
  if (firstdigit == -1) { //reset it
    digitalWrite(latchPin,LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, 255);// EMPTY
    digitalWrite(latchPin,HIGH);
  } else if (firstdigit == -2) { //minus it
    digitalWrite(latchPin,LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, 191);// MINUS
    digitalWrite(latchPin,HIGH);
  } else {
    digitalWrite(latchPin,LOW);
    shiftnext(firstdigit);
    digitalWrite(latchPin,HIGH);
  }
  analogWrite(pwmled1,255);
  delayMicroseconds(pwm);
  analogWrite(pwmled1,0);
}
void updateseconddigit() {
  if (seconddigit == -1) { //reset it
    digitalWrite(latchPin,LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, 255);// EMPTY
    digitalWrite(latchPin,HIGH);
  } else if (seconddigit == -2) { //minus it
    digitalWrite(latchPin,LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, 191);// MINUS
    digitalWrite(latchPin,HIGH);
  } else {
    digitalWrite(latchPin,LOW);
    shiftnext(seconddigit);
    digitalWrite(latchPin,HIGH);
  }
  analogWrite(pwmled2,255);
  delayMicroseconds(pwm);
  analogWrite(pwmled2,0);
}
void updatethirddigit() {
  if (thirddigit == -1) { //reset it
    digitalWrite(latchPin,LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, 255);// EMPTY
    digitalWrite(latchPin,HIGH);
  } else if (thirddigit == -2) { //minus it
    digitalWrite(latchPin,LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, 191);// MINUS
    digitalWrite(latchPin,HIGH);
  } else {
    digitalWrite(latchPin,LOW);
    shiftnext(thirddigit);
    digitalWrite(latchPin,HIGH);
  }
  analogWrite(pwmled3,255);
  delayMicroseconds(pwm);
  analogWrite(pwmled3,0);
}

void updatedisplay() {
  updatefirstdigit();
  updateseconddigit();
  updatethirddigit();
}


