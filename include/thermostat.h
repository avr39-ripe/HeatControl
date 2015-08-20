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

typedef Delegate<void(uint8_t)> myTimerDelegate;

struct Room
{
	uint8_t thermostat_pin;
	uint8_t hi_t_control_pin;
	uint8_t low_t_control_pin;
};

class Pump
{
public:
	Pump(uint8_t pump_pin, uint8_t pump_on_delay, uint8_t pump_off_delay);
	void turn_on();
	void turn_off();
	uint8_t _pump_on_delay;
	uint8_t _pump_off_delay;
private:
	void turn_on_delayed();
	void turn_off_delayed();
	uint8_t _pump_pin;
	uint8_t _consumers = 0;
	Timer _pumpTimer;

};

class HeatingSystem
{
public:
	HeatingSystem(uint8_t mode_pin, uint8_t caldron_pin, uint8_t caldron_on_delay, uint8_t _room_coldy_off_delay);
	void caldron_turn_on();
	void caldron_turn_off();
//	void turn_on_pump(uint8_t pump_id);
//	void turn_off_pump(uint8_t pump_id);
	void check_room(uint8_t room_id);
	void room_turn_on(uint8_t room_id);
	void room_turn_off(uint8_t room_id);
	void check_mode();
	void check(); //run periodicaly by timer to check mode and rooms
	uint8_t _caldron_on_delay;
	uint8_t _room_coldy_lo_t_off_delay;
private:
	void _caldron_turn_on_delayed();
	void _room_coldy_lo_t_off_delayed(uint8_t room_id);
	uint8_t _mode;
	uint8_t _caldron_pin;
	uint8_t _caldron_consumers = 0;
	uint8_t _mode_pin;
	Timer _caldronTimer;
	Timer _roomTimer;
	Room* _rooms[9];
	Pump* _pumps[2];
};

#endif /* INCLUDE_THERMOSTAT_H_ */
