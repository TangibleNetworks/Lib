/* 
  TN.h - Library for TN-04
  Tangible Networks
  Espen Knoop, 19th May 2014
*/

#ifndef TN_h
#define TN_h

#include "Arduino.h"

// Clear bit, set bit
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif 


// Fast min, max
#define MIN(x,y) x < y ? x : y
#define MAX(x,y) x > y ? x : y
#define MINMAX(x,lower,upper) MIN(MAX(x,lower),upper)


// Pins
#define LED_R 11
#define LED_G 6
#define LED_B 5
#define POT A7
#define SW 2
#define DIP A4
#define AUX_IN A3
#define AUX_OUT 3
#define IN1_A A0
#define IN1_HS 7
#define IN2_A A1
#define IN2_HS 4
#define IN3_A A2
#define IN3_HS A5
#define MASTER_A A6
#define MASTER_HS 8


// For DAC
#define I2C_DDR DDRB
#define I2C_PIN PINB
#define I2C_PORT PORTB
#define I2C_CLK 1
#define I2C_DAT 2
#define I2C_CS2 4
#define I2C_CS1 5
#define I2C_CLOCK_LO()    cbi(I2C_PORT,I2C_CLK)
#define I2C_CLOCK_HI()    sbi(I2C_PORT,I2C_CLK) 
#define I2C_CLOCK_PULSE() {delayMicroseconds(5); I2C_CLOCK_HI(); delayMicroseconds(5); I2C_CLOCK_LO();}
#define I2C_DATA_LO()     cbi(I2C_PORT,I2C_DAT)
#define I2C_DATA_HI()     sbi(I2C_PORT,I2C_DAT)
#define I2C_DATA_SET(v)   if(v){I2C_DATA_HI();}else{I2C_DATA_LO();}
#define I2C_CS1_LO()      cbi(I2C_PORT,I2C_CS1)
#define I2C_CS1_HI()      sbi(I2C_PORT,I2C_CS1)
#define I2C_CS2_LO()      cbi(I2C_PORT,I2C_CS2)
#define I2C_CS2_HI()      sbi(I2C_PORT,I2C_CS2)
#define I2C_WRITEBIT(v)   {I2C_DATA_SET(v); I2C_CLOCK_PULSE();}



class TN {
  public:
    // Constructor
    TN(double minVal=0.0, double maxVal=1.0);
    
    // Set LED to RGB colour (ints, [0, 255])
    void colour(int r, int g, int b);
    
    // Set LED to RGB colour (doubles, [0.0, 1.0])
    void colour(double r, double g, double b);
    
    // Check if an input is connected
    boolean isConnected(int input);
    
    // Read the analog value from input (returns double in [minVal, maxVal])
    // Returns minVal if input is not connected
    double analogRead(int input);
    
    // Write value to output (will be clipped to range [minVal, maxVal]
    void analogWrite(int output, double value);
    
    // Read the digital value (0 or 1) from input (returns 0 if not connected)
    int digitalRead(int input);

    // Write digital value (0 or 1) to output
    void digitalWrite(int output, int value);
    
    // Get state of DIP switches (1 for on)
    boolean dip1();
    boolean dip2();
    boolean dip3();
    
    // Get state of switch (1 is pressed)
    boolean sw();
    
    // Get position of pot (double, [0.0, 1.0])
    double pot();
    
    // 1 if master controller is connected
    boolean masterConnected();
    
    // Value of master controller (double, [0.0, 1.0])
    double masterRead();  

    // Read state of master sw (1 is pressed)
    boolean masterSw();

    // Print current state to serial
    void printState();
  private:
    // Process dip input
    int _dip();
    
    // For dac
    void _dacSetup();
    void _startcond();
    void _stopcond();
  
    // Defines range of inputs/outputs (set in constructor)
    double _minVal;
    double _maxVal;

    // Stores current state (for printing)
    int _colour[3];
    double _outs[3];
    double _ins[3];
    double _pot;
    double _master;
    boolean _masterSw;
    boolean _dips[3];
    boolean _sw;
};


#endif


