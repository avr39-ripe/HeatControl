#ifndef INCLUDE_THERMOSTAT_H_
#define INCLUDE_THERMOSTAT_H_

const uint8_t numRooms = 5;
const uint16_t defaultDelay = 15; //4 * 60 - 4 minutes

//HeatingSystem modes
enum HeatingSystemModes { GAS = 0u, WOOD = 1u };

//Pump indexes
enum PumpIndexes {PUMP_1 = 0u, PUMP_2 = 1u };

//Circuit types
enum CircuitTypes {high_temp = 0u, low_temp = 1u };

struct TerminalUnit
{
	uint8_t circuit_pin;
	CircuitTypes circuit_type;
	uint8_t pump_id;

};

class HeatingSystem;

class Room
{
public:
	Room(uint8_t thermostat_pin, HeatingSystem* heating_system);
	void addTU(uint8_t circuit_pin, CircuitTypes circuit_type, uint8_t pump_id);
//TODO:	void deleteTU(uint8_t tu_id);
	void turn_on();
	void turn_off();
	uint8_t _thermostat_pin;
	Vector<TerminalUnit> terminal_units;
	uint16_t _room_off_delay;
private:
	HeatingSystem* _heating_system;
	Timer _roomTimer;
	void turn_off_delayed();
};

class Pump
{
public:
	Pump(uint8_t pump_pin);
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
	HeatingSystem(uint8_t mode_pin, uint8_t caldron_pin);
	~HeatingSystem();
	void caldron_turn_on();
	void caldron_turn_off();
	void check_room(uint8_t room_id);
	void room_turn_on(uint8_t room_id);
	void room_turn_off(uint8_t room_id);
	void check_mode();
	void check(); //run periodically by timer to check mode and rooms
	uint16_t _caldron_on_delay;
	float _mode_switch_temp;
	float _mode_switch_temp_delta;
	float _mode_curr_temp;
private:
	void _caldron_turn_on_delayed();
	void _temp_start();
	void _temp_read();
	uint8_t _mode;
	uint8_t _mode_pin;
	uint8_t _caldron_pin;
	uint8_t _caldron_consumers = 0;
	Timer _caldronTimer;
	Timer _temp_startTimer;
	Timer _temp_readTimer;
	uint8_t _temp_data[12];
	Room* _rooms[numRooms];
	Pump* _pumps[2];
};

extern HeatingSystem HSystem;
#endif /* INCLUDE_THERMOSTAT_H_ */
