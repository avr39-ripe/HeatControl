#ifndef INCLUDE_HEATCONTROL_H_
#define INCLUDE_HEATCONTROL_H_

//SPI_loop stuff
const int miso_pin = 12; // Connect Pin 11 to SER_OUT (serial data out)

const byte reg_in_latch = 4;
const byte reg_out_latch = 5;

const byte num_ch = 16;
const byte num_reg = num_ch / 8;

typedef union {
  uint32_t word;
  uint8_t bytes[4];
} TYPE_32BIT;

extern TYPE_32BIT out_reg;

extern TYPE_32BIT in_reg;

struct inPin {
  bool _mode;  //HIGH == pressed (1) or LOW == pressed (0)

  bool _lastState;
  bool _currentState;

  bool _debounced;
  bool _lastDebouncedState;
  bool _currentDebouncedState;
  unsigned long int _debounceTimerStartTime;
  unsigned int _debounceDelay;

  bool _pressed;
  bool _released;

  bool _changed;
  bool _justPressed;
  bool _justReleased;

  unsigned long int _currentTime;
};

extern inPin inPins[num_ch];
//SPI_loop stuff

extern unsigned long counter;

#endif /* INCLUDE_HEATCONTROL_H_ */

void debouncePin(byte pin);
void setupPin(byte pin, unsigned int debounceDelay, bool mode);
void print_byte(byte val);
void SPI_loop();
