#ifndef INCLUDE_HEATCONTROL_H_
#define INCLUDE_HEATCONTROL_H_
#include <Libraries/OneWire/OneWire.h>

//SPI_loop stuff
const int miso_pin = 12; // 74hc165 data out

const byte reg_in_latch = 4;
const byte reg_out_latch = 5;

const byte num_ch = 16;
const byte num_reg = num_ch / 8;

extern uint8_t out_reg[num_reg];
extern uint8_t in_reg[num_reg];
extern uint8_t in_reg_prev[num_reg];

enum pinStates {
	RELEASED		= (1u << 0),
	PRESSED			= (1u << 1),
	JUSTRELEASED	= (1u << 2),
	JUSTPRESSED		= (1u << 3),
	CHANGED			= (1u << 4)
};

extern unsigned long counter;

void print_byte(byte val);
void SPI_loop();

void setState(uint8_t * reg, int ch, uint8_t state);
bool getState(uint8_t * reg, int ch);
int pinState(int ch);

//OneWire stuff
const uint8_t ONEWIRE_PIN = 0;
extern OneWire ds;

#endif /* INCLUDE_HEATCONTROL_H_ */
