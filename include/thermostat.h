#ifndef INCLUDE_THERMOSTAT_H_
#define INCLUDE_THERMOSTAT_H_

const uint8_t numRooms = 9;

//HeatingSystem modes
enum HeatingSystemModes { GAS = 0u, WOOD = 1u };

//Pump indexes
enum PumpIndexes {PUMP_1 = 0u, PUMP_2 = 1u };


struct Room
{
	uint8_t thermostat_pin;
	uint8_t hi_t_control_pin;
	uint8_t pump_id;
};

class Pump
{
public:
	Pump(uint8_t pump_pin, uint16_t pump_on_delay, uint16_t pump_off_delay);
	void turn_on();
	void turn_off();
	uint16_t _pump_on_delay;
	uint16_t _pump_off_delay;
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
	HeatingSystem(uint8_t mode_pin, uint8_t caldron_pin, uint16_t caldron_on_delay);
	void caldron_turn_on();
	void caldron_turn_off();
	void check_room(uint8_t room_id);
	void room_turn_on(uint8_t room_id);
	void room_turn_off(uint8_t room_id);
	void check_mode();
	void check(); //run periodically by timer to check mode and rooms
	uint16_t _caldron_on_delay;
	float	_mode_switch_temp;
	float	_mode_switch_temp_delta;
private:
	void _caldron_turn_on_delayed();
	uint8_t _mode;
	uint8_t _mode_pin;
	uint8_t _caldron_pin;
	uint8_t _caldron_consumers = 0;
	Timer _caldronTimer;
	Room* _rooms[numRooms];
	Pump* _pumps[2];
};

#endif /* INCLUDE_THERMOSTAT_H_ */
