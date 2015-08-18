#ifndef INCLUDE_THERMOSTAT_H_
#define INCLUDE_THERMOSTAT_H_

//HeatingSystem modes
#define	WARMY 0
#define	COLDY 1

//Pump indexes
#define HI_T_PUMP 0
#define LOW_T_PUMP 1
//When there is no pin use this
#define NO_PIN 255

class Room
{
public:
	Room(uint8_t thermostat_pin, uint8_t hi_t_control_pin, uint8_t low_t_control_pin);
	void run();

private:
	uint8_t _thermostat_pin;
	uint8_t _hi_t_control_pin;
	uint8_t _low_t_control_pin;
};

class Pump
{
public:
	Pump(uint8_t pump_pin, uint8_t pump_on_delay);

private:
	uint8_t _pump_pin;
	uint8_t _pump_on_delay;
	uint8_t _consumers = 0;

};

class HeatingSystem
{
public:
	HeatingSystem(uint8_t mode_pin, uint8_t caldron_pin, uint8_t caldron_on_delay);
	static uint8_t mode;
private:
	uint8_t _caldron_pin;
	uint8_t _caldron_on_delay;
	uint8_t _caldron_consumers = 0;
	uint8_t _mode_pin;

	Room* _rooms[9];
	Pump* _pumps[2];
};

#endif /* INCLUDE_THERMOSTAT_H_ */
