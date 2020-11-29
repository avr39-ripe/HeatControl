#ifndef INCLUDE_HEATCONTROL_H_
#define INCLUDE_HEATCONTROL_H_
#include <SmingCore.h>
#include <Libraries/OneWire/OneWire.h>
#include <JsonObjectStream.h>

#define GPIO_MCP23017
#ifdef GPIO_MCP23017 //use MCP23017
#include <Libraries/MCP23017/MCP23017.h>
extern MCP23017* mcp000;
extern MCP23017* mcp001;
#elif
//SPI_loop stuff
const int miso_pin = 12; // 74hc165 data out
const byte reg_in_latch = 4;
const byte reg_out_latch = 5;
#endif

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

void print_byte(byte val);
void SPI_loop();

void setState(uint8_t * reg, int ch, uint8_t state, uint8_t active_mode = false);
bool getState(uint8_t * reg, int ch, uint8_t active_mode = false);
int pinState(int ch);
void setOutState(int ch, uint8_t state);
void getOutState(int ch);

//OneWire stuff
const uint8_t ONEWIRE_PIN = 0;
extern OneWire ds;

#endif /* INCLUDE_HEATCONTROL_H_ */
