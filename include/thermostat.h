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
	void turn_on();
	void turn_off();
private:
	void turn_on_delayed();
	uint8_t _pump_pin;
	uint8_t _pump_on_delay;
	uint8_t _consumers = 0;
	Timer pumpTimer;

};

class HeatingSystem
{
public:
	HeatingSystem(uint8_t mode_pin, uint8_t caldron_pin, uint8_t caldron_on_delay);
	static uint8_t mode;
	static void turn_on_caldron();
	static void turn_off_caldron();
	static void turn_on_pump(uint8_t pump_id);
	static void turn_off_pump(uint8_t pump_id);
private:
	uint8_t _caldron_pin;
	uint8_t _caldron_on_delay;
	uint8_t _caldron_consumers = 0;
	uint8_t _mode_pin;

	Room* _rooms[9];
	Pump* _pumps[2];
};

#endif /* INCLUDE_THERMOSTAT_H_ */
