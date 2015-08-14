#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include "../include/heatcontrol.h"
#include <SPI.h>

//const int miso_pin = 12; // Connect Pin 11 to SER_OUT (serial data out)

//const byte reg_in_latch = 4;
//const byte reg_out_latch = 5;

//const byte num_ch = 16;
//const byte num_reg = num_ch / 8;
//
//typedef union {
//  uint32_t word;
//  uint8_t bytes[4];
//} TYPE_32BIT;

TYPE_32BIT out_reg;

TYPE_32BIT in_reg;

//struct inPin {
//  bool _mode;  //HIGH == pressed (1) or LOW == pressed (0)
//
//  bool _lastState;
//  bool _currentState;
//
//  bool _debounced;
//  bool _lastDebouncedState;
//  bool _currentDebouncedState;
//  unsigned long int _debounceTimerStartTime;
//  unsigned int _debounceDelay;
//
//  bool _pressed;
//  bool _released;
//
//  bool _changed;
//  bool _justPressed;
//  bool _justReleased;
//
//  unsigned long int _currentTime;
//};
//

inPin inPins[num_ch];

//void debouncePin(byte pin);
//void setupPin(byte pin, unsigned int debounceDelay, bool mode);
//
//void print_byte(byte val);


//void init()
//{
//	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
//	Serial.systemDebugOutput(false); // Allow debug output to serial
//
//
//	Serial.println("Sming start");
//
////initial out_reg state
////  out_reg.word = 4294967295L;
////	out_reg.word = 43690;
////	out_reg.word = 21845L;
//
////  for(int i = 0; i < num_ch; i++)
////  {
////    setupPin(i, 100, LOW);
////  }
////
////  pinMode(reg_in_latch, OUTPUT);
////  pinMode(reg_out_latch, OUTPUT);
//
//
//}


void SPI_loop()
{
	counter++;
for(int cnt = 0; cnt < 4; cnt++)
{
  uint8_t first_bit;
  int byteIndex;
  int shiftIndex;
  
  digitalWrite(reg_in_latch, LOW);
  
  delayMicroseconds(5);
  
  digitalWrite(reg_in_latch, HIGH);
  
  digitalWrite(reg_out_latch, LOW);

  pinMode(miso_pin, INPUT); //workaround for 74hc165 "left shifted bit" bug
  first_bit = digitalRead(miso_pin); //read and store very first bit of 74hc165 chain outside regular SPI.transfer

  SPI.begin();

  for(int i = 0; i < num_reg; i++)
  {
    in_reg.bytes[i] = SPI.transfer(out_reg.bytes[num_reg - 1 - i]);
  }

  digitalWrite(reg_out_latch, HIGH);

  in_reg.bytes[1] = (in_reg.bytes[1] >> 1) | ((in_reg.bytes[0] & 1) << 7); //re-arrange bits in place
  in_reg.bytes[0] = (in_reg.bytes[0] >> 1) | (first_bit << 7); //here we use our stored first bit

//  for(int i = 0; i < num_reg; i++)
//  {
//    Serial.print("REG-IN"); Serial.print(i);Serial.print(" ");
//    print_byte(in_reg.bytes[i]);
//    Serial.print("REG-OUT"); Serial.print(i);Serial.print(" ");
//    print_byte(out_reg.bytes[i]);
//  }

for(int i = 0; i < num_ch; i++)
  {
    debouncePin(i);
     switch (i) {

     default:  
    if ( inPins[i]._changed && inPins[i]._pressed) {
           byteIndex = i / 8;
           shiftIndex = i % 8;
           out_reg.bytes[byteIndex] ^= (1 << shiftIndex);
         }
     }
}

} //cnt loop
}

void setupPin(byte pin, unsigned int debounceDelay, bool mode)
{
  inPins[pin]._mode = mode;
  inPins[pin]._lastState = 0;
    inPins[pin]._currentState = 0;
    
    inPins[pin]._debounced = 1;
    inPins[pin]._lastDebouncedState = 0;
    inPins[pin]._currentDebouncedState = 0;
    inPins[pin]._debounceTimerStartTime = 0;
    inPins[pin]._debounceDelay = debounceDelay;
    
    inPins[pin]._pressed = 0;
    inPins[pin]._released = 1;
  
    inPins[pin]._changed = 0;
    inPins[pin]._justPressed = 0;
    inPins[pin]._justReleased = 0;
     
}

void debouncePin(byte pin)
{
  byte byteId = pin / 8; 
  byte bitId = pin % 8;
  unsigned long int _currentTime = millis();
  
  inPins[pin]._currentState = in_reg.bytes[byteId] & (1 << bitId);
  
  if (inPins[pin]._currentState != inPins[pin]._lastState) {
    inPins[pin]._debounced = false;
    inPins[pin]._debounceTimerStartTime = _currentTime;
  } else if ((_currentTime - inPins[pin]._debounceTimerStartTime) > inPins[pin]._debounceDelay) {
    inPins[pin]._debounced = true;
  }
  
  if (inPins[pin]._debounced) {
    inPins[pin]._lastDebouncedState = inPins[pin]._currentDebouncedState;
    inPins[pin]._currentDebouncedState = inPins[pin]._currentState;
  }
  
  
  if (inPins[pin]._currentDebouncedState == inPins[pin]._mode) {
    inPins[pin]._pressed = true;
    inPins[pin]._released = false;
    inPins[pin]._justReleased = false;
  } else {
    inPins[pin]._pressed = false;
    inPins[pin]._released = true;
    inPins[pin]._justPressed = false;
  }
  
  
  if (inPins[pin]._lastDebouncedState != inPins[pin]._currentDebouncedState) {
    inPins[pin]._changed = true;
  } else {
    inPins[pin]._changed = false;
    inPins[pin]._justPressed = false;
    inPins[pin]._justReleased = false;
  }
  
  if (inPins[pin]._changed && inPins[pin]._pressed) {
    inPins[pin]._justPressed = true;
    inPins[pin]._justReleased = false;
  } else if(inPins[pin]._changed && inPins[pin]._released){
    inPins[pin]._justPressed = false;
    inPins[pin]._justReleased = true;
    
  } 
  inPins[pin]._lastState = inPins[pin]._currentState;
}


// A function that prints all the 1's and 0's of a byte, so 8 bits +or- 2
void print_byte(byte val)
{
    byte i;
    for(byte i=0; i<=7; i++)
    {
      Serial.print(val >> i & 1, BIN); // Magic bit shift, if you care look up the <<, >>, and & operators
    }
    Serial.print("\n"); // Go to the next line, do not collect $200
}
