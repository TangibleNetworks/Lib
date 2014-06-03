/* 
  TN.cpp - Library for TN-04
  Tangible Networks
  Espen Knoop, 19th May 2014
*/


#include "Arduino.h"
#include "TN.h"


// Constructor
TN::TN(double minVal, double maxVal) {
  _minVal = minVal;
  _maxVal = maxVal;
  pinMode(IN1_A,INPUT);
  pinMode(IN1_HS,INPUT);
  pinMode(IN2_A,INPUT);
  pinMode(IN2_HS,INPUT);
  pinMode(IN3_A,INPUT);
  pinMode(IN3_HS,INPUT);
  pinMode(LED_R,OUTPUT);
  pinMode(LED_G,OUTPUT);
  pinMode(LED_B,OUTPUT);
  pinMode(SW,INPUT);
  ::digitalWrite(SW,HIGH);
  _dacSetup();
  // Go through all the functions, to initialise state vars
  for (int i=1; i<4; i++) {
    analogWrite(i,_minVal);
    analogRead(i);
  }
  colour(0,0,0);
  dip1();
  dip2();
  dip3();
  masterRead();
  masterSw();
  pot();
  sw();
}


// For DAC
void TN::_dacSetup() {
  // Set pins to output
  sbi(I2C_DDR,I2C_CLK);
  sbi(I2C_DDR,I2C_DAT);
  sbi(I2C_DDR,I2C_CS1);
  sbi(I2C_DDR,I2C_CS2);
}

inline void TN::_startcond() {
  I2C_DATA_LO();
  delayMicroseconds(5);
  I2C_CLOCK_LO();
  ;
}

inline void TN::_stopcond() {
  I2C_CLOCK_HI();
  delayMicroseconds(5);
  I2C_DATA_HI();
  delayMicroseconds(5);
}

// Write value to output (will be clipped to range [minVal, maxVal]
void TN::analogWrite(int output, double doubleVal) {
  doubleVal = MINMAX(doubleVal,_minVal,_maxVal);
  int value = floor(4095.0* (doubleVal - _minVal)/(_maxVal - _minVal));
  int address = 0;
  switch (output) {
    case 3:
      address = 0;
      I2C_CS1_HI();
      I2C_CS2_HI();
      _outs[2] = doubleVal;
      break;
    case 2:
      address = 1;
      I2C_CS1_LO();
      I2C_CS2_HI();
      _outs[1] = doubleVal;
      break;
    default: // pick 1 as default
      address = 1;
      I2C_CS1_HI();
      I2C_CS2_LO();
      _outs[0] = doubleVal;
  }
  _startcond();
  I2C_WRITEBIT(1);
  I2C_WRITEBIT(1);
  I2C_WRITEBIT(0);
  I2C_WRITEBIT(0);
  I2C_WRITEBIT(0);
  I2C_WRITEBIT(0);
  I2C_WRITEBIT(address);
  I2C_WRITEBIT(0);
  
  I2C_WRITEBIT(1);
  
  I2C_WRITEBIT(0);
  I2C_WRITEBIT(0);
  I2C_WRITEBIT(0);
  I2C_WRITEBIT(0);

  for (int i=11; i > 7; i--) {
    I2C_WRITEBIT(1 & (value >> i) );
  }
  
  I2C_WRITEBIT(1);
  
  for (int i=7; i >= 0; i--) {
    I2C_WRITEBIT(1 & (value >> i) );
  }
  
  I2C_WRITEBIT(1);
  _stopcond();
}


// Set LED to RGB colour (ints, [0, 255])
void TN::colour(int r, int g, int b) {
  r = MINMAX(r,0,255);
  g = MINMAX(g,0,255);
  b = MINMAX(b,0,255);
  _colour[0] = r;
  _colour[1] = g;
  _colour[2] = b;
  ::analogWrite(LED_R,r);
  ::analogWrite(LED_G,g);
  ::analogWrite(LED_B,b);
}


// Set LED to RGB colour (doubles, [0.0, 1.0])
void TN::colour(double r, double g, double b) {
  colour((int)(r*255.0),(int)(g*255.0),(int)(b*255.0));
}
 

// Check if an input is connected
boolean TN::isConnected(int input) {
  switch (input) {
    case 3:
      return ::digitalRead(IN3_HS);
      break;
    case 2:
      return ::digitalRead(IN2_HS);
      break;
    default:
      return ::digitalRead(IN1_HS);  
  }  
}


// Read the analog value from input (returns double in [minVal, maxVal])
// Returns minVal if input is not connected
double TN::analogRead(int input) {
  int idx;
  if (isConnected(input)) {
    int rawVal;
    switch (input) {
      case 3:
        rawVal = ::analogRead(IN3_A);
        idx = 2;
        break;
      case 2:
        rawVal = ::analogRead(IN2_A);
        idx = 1;
        break;
      default:
        rawVal = ::analogRead(IN1_A);
        idx = 0;
    }
    _ins[idx] = _minVal + (_maxVal - _minVal)*rawVal/1023.0;
    
  }
  else _ins[idx] = _minVal;
  return _ins[idx];
}


// Read the digital value (0 or 1) from input (returns 0 if not connected)
int TN::digitalRead(int input) {
  switch (input) {
    case 3:
      if (isConnected(3)) _ins[2] = ::digitalRead(IN3_A);
      else _ins[2] = 0;
      return ::digitalRead(IN3_A);
    case 2:
      if (isConnected(2)) _ins[1] = ::digitalRead(IN2_A);
      else _ins[1] = 0;
      return _ins[1];
    default: // assume 1
      if (isConnected(1)) _ins[0] = ::digitalRead(IN1_A);
      else _ins[0] = 0;
      return _ins[0];
  }
}


// Write digital value (0 or 1) to output
void TN::digitalWrite(int output, int value) {
  if (value) analogWrite(output, _maxVal);
  else analogWrite(output, _minVal);
}


// Get state of DIP switches (1 for on)
boolean TN::dip1() {
  if (0b100 & _dip()) _dips[0] = 1;
  else _dips[0] = 0;
  return _dips[0];
}

boolean TN::dip2() {
  if (0b010 & _dip()) _dips[1] = 1;
  else _dips[1] = 0;
  return _dips[1];
}

boolean TN::dip3() {
  if (0b001 & _dip()) _dips[2] = 1;
  else _dips[2] = 0;
  return _dips[2];
}


// Get state of switch (1 is pressed)
boolean TN::sw() {
  _sw = ! ::digitalRead(SW);
  return (_sw);
}


// Get position of pot (double, [0.0, 1.0])
double TN::pot() {
  _pot = ::analogRead(POT)/1023.0;
  return _pot;
}


// 1 if master controller is connected
boolean TN::masterConnected() {
  return (!::digitalRead(MASTER_HS));
}


// Value of master controller (double, [0.0, 1.0])
double TN::masterRead() {
  if (masterConnected()) {
    _master = ::analogRead(MASTER_A)/1023.0;
  }
  else _master = 0.0;
  return _master;
}


// Process dip input
int TN::_dip() {

  int val = ::analogRead(DIP);

  // 000 1023
  if (val > 895) return 0b000;
  // 001 768
  else if (val > 725) return 0b001;
  // 010 682
  else if (val > 646) return 0b010;
  // 100 610
  else if (val > 584) return 0b100;
  // 011 557
  else if (val > 533) return 0b011;
  // 101 509
  else if (val > 489) return 0b101;
  // 110 469
  else if (val > 438) return 0b110;
  // 111 407
  else return 0b111;
}

// Get state of master switch (1 is pressed)
boolean TN::masterSw() {
  _masterSw = ! ::digitalRead(MASTER_HS);
  return (_masterSw);
}


// Print current state to serial 
// (takes ~5 ms to execute.  Requires Serial.begin(115200) in setup(). )
void TN::printState() {
  Serial.print("RGB: ");
  for (int i=0; i<3; i++) {
    Serial.print(_colour[i]);
    Serial.print(", ");
  }
  Serial.print("Ins: ");
  for (int i=0; i<3; i++) {
    Serial.print(_ins[i]);
    Serial.print(", ");
  }
  Serial.print("Outs: ");
  for (int i=0; i<3; i++) {
    Serial.print(_outs[i]);
    Serial.print(", ");
  }
  Serial.print("Pot: ");
  Serial.print(_pot);
  Serial.print(", Master: ");
  Serial.print(_master);
  Serial.print(", Master Sw: ");
  Serial.print(_masterSw);
  Serial.print(", DIPs: ");
  for (int i=0; i<3; i++) {
    Serial.print(_dips[i]);
    Serial.print(", ");
  }
  Serial.print("Sw: ");
  Serial.println(_sw);
}



    
