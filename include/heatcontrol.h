#ifndef INCLUDE_HEATCONTROL_H_
#define INCLUDE_HEATCONTROL_H_
#include <Libraries/OneWire/OneWire.h>

//OneWire stuff
const uint8_t onewire_pin = 2;
extern OneWire ds;

//I2C stuff
const uint8_t scl_pin = 0;
const uint8_t sda_pin = 2;

//#define MCP23S17
//SPI_loop stuff
#ifdef MCP23S17 //use MCP23S17 SPI_loop
#include <Libraries/MCP23S17/MCP23S17.h>
const byte mcp23s17_cs = 16; // MCP23S17 ChipSelect pin
extern MCP inputchip;
extern MCP outputchip;
#endif
const int miso_pin = 12; // 74hc165 data out

const byte reg_in_latch = 15;
const byte reg_out_latch = 16;

const byte num_ch = 16;
const byte num_reg = num_ch / 8;

extern uint8_t out_reg[num_reg];
extern uint8_t in_reg[num_reg];
extern uint8_t in_reg_prev[num_reg]; //stores previous pin state for catching JUST* events
extern uint8_t out_reg_am[num_reg]; //stores configuration of active_mode for the pin either HIGH or LOW

enum pinStates {
	RELEASED		= (1u << 0),
	PRESSED			= (1u << 1),
	JUSTRELEASED	= (1u << 2),
	JUSTPRESSED		= (1u << 3),
	CHANGED			= (1u << 4)
};

extern unsigned long counter;
extern String _date_time_str;
extern double sys_TZ;

void print_byte(byte val);
void SPI_loop();

void setState(uint8_t * reg, int ch, uint8_t state, uint8_t active_mode = false);
bool getState(uint8_t * reg, int ch, uint8_t active_mode = false);
int pinState(int ch);
void setOutState(int ch, uint8_t state);
bool getOutState(int ch);

#endif /* INCLUDE_HEATCONTROL_H_ */
