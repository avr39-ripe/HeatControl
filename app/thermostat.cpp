#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/OneWire/OneWire.h>

#include <configuration.h>
#include <heatcontrol.h>
#include <thermostat.h>

//Room implementation
Room::Room(uint8_t thermostat_pin, HeatingSystem* heating_system)
{
	this->_thermostat_pin = thermostat_pin;
	this->_heating_system = heating_system;
	this->_room_off_delay = 0;
//	_terminal_units = Vector<TerminalUnit> (1,1);
}


void Room::turn_on()
{
	if (_terminal_units[HIGH_TEMP].circuit_pin != NO_PIN )
	{
		if (getState(out_reg, _terminal_units[HIGH_TEMP].circuit_pin) == false) //ensure room is really turned OFF
		{
			setState(out_reg, _terminal_units[HIGH_TEMP].circuit_pin, true);
			_heating_system->_pumps[_terminal_units[HIGH_TEMP].pump_id]->turn_on();
			if (_heating_system->_mode == GAS)
				_heating_system->caldron_turn_on();
		}
	}
}

void Room::turn_off()
{
	if (_terminal_units[HIGH_TEMP].circuit_pin != NO_PIN )
	{
		if (getState(out_reg, _terminal_units[HIGH_TEMP].circuit_pin) == true) //ensure room is really turned OFF
		{
			setState(out_reg, _terminal_units[HIGH_TEMP].circuit_pin, false);
			_heating_system->_pumps[_terminal_units[HIGH_TEMP].pump_id]->turn_off();
			if (_heating_system->_mode == GAS)
				_heating_system->caldron_turn_off();
		}
	}
}
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
	if (_consumers > 0)
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
	for(uint8_t room_id = 0; room_id < numRooms; room_id++)
	{
		this->_rooms[room_id] = new Room(room_id, this);
	}
	//assign each room corresponding TUs
	this->_rooms[0]->_terminal_units[HIGH_TEMP] ={0, PUMP_1};
	this->_rooms[1]->_terminal_units[HIGH_TEMP] ={1, PUMP_1};
	this->_rooms[2]->_terminal_units[HIGH_TEMP] ={2, PUMP_1};
	this->_rooms[3]->_terminal_units[HIGH_TEMP] ={3, PUMP_2};
	this->_rooms[4]->_terminal_units[HIGH_TEMP] ={4, PUMP_2};

	//Turn everything OFF
	for (auto room: _rooms)
		room->turn_off();
	for (auto pump: _pumps)
		pump->turn_off();
	this->caldron_turn_off();

	//Arm temperature start timer
	_temp_startTimer.initializeMs(4000, TimerDelegate(&HeatingSystem::_temp_start, this)).start(true);
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
	if ((_mode_curr_temp >= ActiveConfig.mode_switch_temp + ActiveConfig.mode_switch_temp_delta) && (_mode != WOOD))
	{
		_mode = WOOD;
		caldron_turn_off();
		for(uint8_t room_id = 0; room_id < numRooms; room_id++)
		{
			_rooms[room_id]->turn_on();
		}
	}
	if ((_mode_curr_temp <= ActiveConfig.mode_switch_temp - ActiveConfig.mode_switch_temp_delta) && (_mode != GAS))
	{
		_mode = GAS;
		for(uint8_t room_id = 0; room_id < numRooms; room_id++)
		{
			_rooms[room_id]->turn_off();
		}
	}
}

void HeatingSystem::caldron_turn_on()
{
	if (_mode == GAS)
	{
		if (_caldron_consumers == 0)
		{
			this->_caldronTimer.initializeMs(ActiveConfig.caldron_on_delay * 1000, TimerDelegate(&HeatingSystem::_caldron_turn_on_delayed, this)).start(false);
		}
		_caldron_consumers++;
	}
}

void HeatingSystem::caldron_turn_off()
{
	if (_mode == GAS)
	{
		if (_caldron_consumers > 0) //after switch back from WOOD mode we have all rooms ON but gas caldron is already turned OFF and without this we overturn off caldron
			_caldron_consumers--;
		if (_caldron_consumers == 0)
		{
			setState(out_reg, _caldron_pin, false);
			this->_caldronTimer.stop();
		}
		return;
	}
	if (_mode == WOOD)
	{
		_caldron_consumers = 0;
		setState(out_reg, _caldron_pin, false);
		this->_caldronTimer.stop();
	}
}

void HeatingSystem::_caldron_turn_on_delayed()
{
	setState(out_reg, _caldron_pin, true);
}

void HeatingSystem::check_room(uint8_t room_id)
{
	if (_mode == GAS)
	{
		int thermostat_state = pinState(_rooms[room_id]->_thermostat_pin);

		if (thermostat_state & PRESSED)
		{
			_rooms[room_id]->turn_on();
		}

		if (thermostat_state & RELEASED)
		{
			_rooms[room_id]->turn_off();
		}
	}
	else
	{
		//To enshure that even after power fail, or reset or whatever - all rooms in WOOD mode are turned on
		_rooms[room_id]->turn_on();
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

void HeatingSystem::_temp_start()
{
	if (!_temp_readTimer.isStarted())
	{
		ds.reset();
		ds.skip();
		ds.write(0x44); // start conversion

		_temp_readTimer.initializeMs(750, TimerDelegate(&HeatingSystem::_temp_read, this)).start(false);
	}
}

void HeatingSystem::_temp_read()
{


	ds.reset();
//	ds.select(temp_sensors[n].addr);
	ds.skip();
	ds.write(0xBE); // Read Scratchpad

	for (uint8_t i = 0; i < 9; i++)
	{
		_temp_data[i] = ds.read();
	}

	float tempRead = ((_temp_data[1] << 8) | _temp_data[0]); //using two's compliment
	_mode_curr_temp = tempRead / 16;

//	Serial.print("_mode_curr_temp = "); Serial.println(_mode_curr_temp);
	_temp_readTimer.stop();
}
//OneWire system initialisation
OneWire ds(ONEWIRE_PIN);

//HeatingSystem initialisation
HeatingSystem HSystem(0, 5);
