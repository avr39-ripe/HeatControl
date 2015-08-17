#ifndef INCLUDE_HEATCONTROL_H_
#define INCLUDE_HEATCONTROL_H_

//SPI_loop stuff
const int miso_pin = 12; // 74hc165 data out

const byte reg_in_latch = 4;
const byte reg_out_latch = 5;

const byte num_ch = 16;
const byte num_reg = num_ch / 8;

extern uint8_t out_reg[num_reg];
extern uint8_t in_reg[num_reg];
extern uint8_t in_reg_prev[num_reg];

extern unsigned long counter;

void print_byte(byte val);
void SPI_loop();

void setState(uint8_t * reg, int ch, uint8_t state);
bool getState(uint8_t * reg, int ch);

#endif /* INCLUDE_HEATCONTROL_H_ */
