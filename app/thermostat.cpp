#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <configuration.h>
#include <heatcontrol.h>
#include <thermostat.h>

//Pump implementation

Pump::Pump(uint8_t pump_pin)
{
	this->_pump_pin = pump_pin;
	this->_consumers = 0;
	this->_pump_on_delay = defaultDelay;
	this->_pump_off_delay = defaultDelay;
}

void Pump::turn_on()
{
	if (_consumers == 0)
	{
		this->_pumpTimer.initializeMs(ActiveConfig.pump_on_delay * 1000, TimerDelegate(&Pump::turn_on_delayed, this)).start(false);
	}
	_consumers++;


}

void Pump::turn_off()
{
	_consumers--;
	if (_consumers == 0)
	{
		this->_pumpTimer.initializeMs(ActiveConfig.pump_off_delay * 1000, TimerDelegate(&Pump::turn_off_delayed, this)).start(false);
	}

}

void Pump::turn_on_delayed()
{
	setState(out_reg, _pump_pin, true);
}

void Pump::turn_off_delayed()
{
	setState(out_reg, _pump_pin, false);
}

//HeatingSystem implementation

HeatingSystem::HeatingSystem(uint8_t mode_pin, uint8_t caldron_pin)
{
	this->_mode_pin = mode_pin;
	this->_caldron_consumers = 0;
	this->_caldron_pin = caldron_pin;
	this->_caldron_on_delay = defaultDelay;
	this->_mode = GAS;
	this->_mode_switch_temp = 60;
	this->_mode_switch_temp_delta = 1;
	//pumps init
	this->_pumps[0] = new Pump(6);
	this->_pumps[1] = new Pump(7);
	//rooms init
	for(uint8_t id = 0; id < numRooms; id++)
	{
		this->_rooms[id] = new Room;
		this->_rooms[id]->hi_t_control_pin = id;
		this->_rooms[id]->thermostat_pin = id;
	}
	//assign each room corresponding pump_id
	this->_rooms[0]->pump_id = PUMP_1;
	this->_rooms[1]->pump_id = PUMP_1;
	this->_rooms[2]->pump_id = PUMP_1;
	this->_rooms[3]->pump_id = PUMP_2;
	this->_rooms[4]->pump_id = PUMP_2;
//	this->_rooms[5]->pump_id = PUMP_2;
//	this->_rooms[6]->pump_id = PUMP_2;
//	this->_rooms[7]->pump_id = PUMP_2;
//	this->_rooms[8]->pump_id = PUMP_2;
}

HeatingSystem::~HeatingSystem()
{
	delete this->_pumps[0];
	delete this->_pumps[1];
	for(uint8_t id = 0; id < numRooms; id++)
	{
		delete this->_rooms[id];
	}
}

void HeatingSystem::check_mode()
{
//TODO: implement mode chek by 1-wire thermometer
}

void HeatingSystem::caldron_turn_on()
{
	if (_caldron_consumers == 0)
		{
			this->_caldronTimer.initializeMs(ActiveConfig.caldron_on_delay * 1000, TimerDelegate(&HeatingSystem::_caldron_turn_on_delayed, this)).start(false);
		}
		_caldron_consumers++;
}

void HeatingSystem::caldron_turn_off()
{
	_caldron_consumers--;
	if (_caldron_consumers == 0)
	{
		setState(out_reg, _caldron_pin, false);
		this->_caldronTimer.stop();
	}

}

void HeatingSystem::_caldron_turn_on_delayed()
{
	setState(out_reg, _caldron_pin, true);
}

void HeatingSystem::room_turn_on(uint8_t room_id)
{
	setState(out_reg, _rooms[room_id]->hi_t_control_pin, true);
	_pumps[_rooms[room_id]->pump_id]->turn_on();
	caldron_turn_on();
}

void HeatingSystem::room_turn_off(uint8_t room_id)
{
	if (_mode == GAS)
	{
		setState(out_reg,_rooms[room_id]->hi_t_control_pin, false);
		_pumps[_rooms[room_id]->pump_id]->turn_off();
		caldron_turn_off();
	}
}

void HeatingSystem::check_room(uint8_t room_id)
{
	if (_mode == GAS)
	{
		int thermostat_state = pinState(_rooms[room_id]->thermostat_pin);

		if (thermostat_state & JUSTPRESSED)
		{
			room_turn_on(room_id);
		}

		if (thermostat_state & JUSTRELEASED)
		{
			room_turn_off(room_id);
		}
	}
	else
	{
		//To enshure that even after power fail, or reset or whatever - all rooms in WOOD mode are turned on
		room_turn_on(room_id);
	}
}

void HeatingSystem::check()
{
	check_mode();
	for(uint8_t room_id = 0; room_id < numRooms; room_id ++)
	{
		check_room(room_id);
	}
}

//HeatingSystem initialisation
HeatingSystem HSystem(0, 5);
