/*
 * valvecontrol.h
 *
 *  Created on: 21 июля 2015 г.
 *      Author: shurik
 */

#ifndef INCLUDE_VALVECONTROL_H_
#define INCLUDE_VALVECONTROL_H_

#define NUM_SENSORS 3

extern unsigned long counter;

struct temp_sensor
{
  byte addr[8];
  char addr_str[17];
  float value;
  byte data[12];
};

extern temp_sensor temp_sensors[NUM_SENSORS];

struct relay_pin
{
	byte pin;
	byte state;
};

extern relay_pin relay_pins[NUM_SENSORS];

#endif /* INCLUDE_VALVECONTROL_H_ */

void thermostat(void);
void startTemp();
void readTemp(void);
