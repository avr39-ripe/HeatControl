#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/OneWire/OneWire.h>

#include <configuration.h>
#include <heatcontrol.h>
#include <thermostat.h>

//OneWire system initialisation
OneWire ds(onewire_pin);

//HWPump implementation

HWPump::HWPump(uint8_t pump_pin)
{
	this->_pump_pin = pump_pin;
}

void HWPump::cycle()
{
	DateTime _now = SystemClock.now(eTZ_Local);
	uint16_t _now_minutes = _now.Hour * 60 + _now.Minute;

	if ((_now_minutes >= ActiveConfig.start_minutes) && (_now_minutes <= ActiveConfig.stop_minutes))
	{
		Serial.print(_now.toFullDateTimeString()); Serial.println(" Turn HWPump ON! ");
		turn_on();
		//Here we have delayed turn off, not immediately!!!
		turn_off();
	}
	else
	{
		Serial.print(_now.toFullDateTimeString()); Serial.println(" Sleep time for HWPump! ");
	}
	this->_intervalTimer.initializeMs((ActiveConfig.cycle_interval) * 60000, TimerDelegate(&HWPump::cycle, this)).start(false); //cycle_duration in minute so multiple by 60 * 1000 to be in ms.
}

void HWPump::turn_on()
{
	setState(out_reg, _pump_pin, true);
}

void HWPump::turn_off()
{
		this->_durationTimer.initializeMs(ActiveConfig.cycle_duration * 60000, TimerDelegate(&HWPump::turn_off_delayed, this)).start(false); //cycle_interval in minute so multiple by 60 * 1000 to be in ms.
}

void HWPump::turn_off_delayed()
{
	setState(out_reg, _pump_pin, false);
}

//TerminalUnit implementation
TerminalUnit::TerminalUnit(uint8_t circuit_pin, uint8_t pump_id, HeatingSystem* heating_system)
{
	this->_circuit_pin = circuit_pin;
	this->_pump_id = pump_id;
	this->_heating_system = heating_system;
}

void TerminalUnit::turn_on()
{
//XXX DO NOT CHECK FOR tu == nullptr here! DO IT OUTSIDE BEFORE CALL THIS!!!

	if (getState(out_reg, _circuit_pin) == false) //ensure room is really turned OFF
	{
		setState(out_reg, _circuit_pin, true);
		_heating_system->_pumps[_pump_id]->turn_on();
		if (_heating_system->_mode & GAS)
			_heating_system->caldron_turn_on();
	}
}

void TerminalUnit::turn_off()
{
//XXX DO NOT CHECK FOR tu == nullptr here! DO IT OUTSIDE BEFORE CALL THIS!!!

	if (getState(out_reg, _circuit_pin) == true) //ensure terminal unit is really turned ON
	{
		setState(out_reg, _circuit_pin, false);
		_heating_system->_pumps[_pump_id]->turn_off();
		if (_heating_system->_mode & GAS)
			_heating_system->caldron_turn_off();
	}
}

//Room implementation
Room::Room(uint8_t thermostat_pin, HeatingSystem* heating_system)
{
	this->_thermostat_pin = thermostat_pin;
	this->_heating_system = heating_system;
	this->_room_off_delay = defaultDelay;
	this->_terminal_units[HIGH_TEMP] = nullptr;
	this->_terminal_units[LOW_TEMP] = nullptr;
}


void Room::turn_on()
{
	if (_heating_system->_mode & WARMY)
	{
		if (_terminal_units[LOW_TEMP] != nullptr )
		{
			_roomTimer.stop();
			_terminal_units[LOW_TEMP]->turn_on();
		}
		else if (_terminal_units[HIGH_TEMP] != nullptr )
			_terminal_units[HIGH_TEMP]->turn_on();
	}
	if ((_heating_system->_mode & WOOD) || (_heating_system->_mode & COLDY))
	{
		if (_terminal_units[HIGH_TEMP] != nullptr )
			_terminal_units[HIGH_TEMP]->turn_on();

		if (_terminal_units[LOW_TEMP] != nullptr )
		{
			_roomTimer.stop();
			_terminal_units[LOW_TEMP]->turn_on();
		}
	}
}

void Room::turn_off()
{
//	if (_heating_system->_mode & WARMY)
//	{
//		if (_terminal_units[LOW_TEMP] != nullptr )
//			_terminal_units[LOW_TEMP]->turn_off();
//		else if (_terminal_units[HIGH_TEMP] != nullptr )
//			_terminal_units[HIGH_TEMP]->turn_off();
//	}
	if (_heating_system->_mode & COLDY)
	{
		if (_terminal_units[LOW_TEMP] != nullptr)
		{
			if ((_terminal_units[LOW_TEMP]->is_on()) && (! this->_roomTimer.isStarted()))
			{
				this->_roomTimer.initializeMs(ActiveConfig.room_off_delay * 1000, TimerDelegate(&Room::_coldy_lo_t_off_delayed, this)).start(false);
			}
		}
		if (_terminal_units[HIGH_TEMP] != nullptr )
			_terminal_units[HIGH_TEMP]->turn_off();
	}
	if ((_heating_system->_mode & WOOD) || (_heating_system->_mode & WARMY))
	{
		if (_terminal_units[HIGH_TEMP] != nullptr )
			_terminal_units[HIGH_TEMP]->turn_off();

		if (_terminal_units[LOW_TEMP] != nullptr )
			_terminal_units[LOW_TEMP]->turn_off();
	}
}

void Room::_coldy_lo_t_off_delayed()
{
	_terminal_units[LOW_TEMP]->turn_off();
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
	this->_mode_curr_temp = 26.07;
	this->_temp_accum = 0;
	this->_temp_counter =0;
	//hwpump init
	this->_hwpump = new HWPump(12);
	this->_hwpump->_cycle_interval = 60; // in minutes
	this->_hwpump->_cycle_duration = 5; // in minutes
	this->_hwpump->_start_minutes = 420; // 7 * 60 = 7:00
	this->_hwpump->_stop_minutes = 1380; // 23 * 60 = 23:00

//	this->_hwpump->cycle();

	//pumps init
	this->_pumps[0] = new Pump(14); //HIGH_TEMP
	this->_pumps[1] = new Pump(13); //LOW_TEMP
	//rooms init
	for(uint8_t room_id = 0; room_id < numRooms; room_id++)
	{
		this->_rooms[room_id] = new Room(room_id, this);
	}
	//assign each room corresponding TUs
	this->_rooms[0]->_terminal_units[HIGH_TEMP] = new TerminalUnit(0, HIGH_T_PUMP_1, this); // Badroom
	this->_rooms[0]->_terminal_units[LOW_TEMP] = new TerminalUnit(8, LOW_T_PUMP_1, this);

	this->_rooms[1]->_terminal_units[HIGH_TEMP] = new TerminalUnit(1, HIGH_T_PUMP_1, this); // Hallway
	this->_rooms[1]->_terminal_units[LOW_TEMP] = new TerminalUnit(9, LOW_T_PUMP_1, this);

	this->_rooms[2]->_terminal_units[HIGH_TEMP] = new TerminalUnit(2, HIGH_T_PUMP_1, this); // WC
	this->_rooms[2]->_terminal_units[LOW_TEMP] = nullptr;

	this->_rooms[3]->_terminal_units[HIGH_TEMP] = new TerminalUnit(3, HIGH_T_PUMP_1, this); // Hall
	this->_rooms[3]->_terminal_units[LOW_TEMP] = new TerminalUnit(10, LOW_T_PUMP_1, this);

	this->_rooms[4]->_terminal_units[HIGH_TEMP] = nullptr;
	this->_rooms[4]->_terminal_units[LOW_TEMP] = new TerminalUnit(11, LOW_T_PUMP_1, this); // Kitchen

	this->_rooms[5]->_terminal_units[HIGH_TEMP] = new TerminalUnit(4, HIGH_T_PUMP_1, this); // Room 1 with fireplace
	this->_rooms[5]->_terminal_units[LOW_TEMP] = nullptr;

	this->_rooms[6]->_terminal_units[HIGH_TEMP] = new TerminalUnit(5, HIGH_T_PUMP_1, this); // Room 2
	this->_rooms[6]->_terminal_units[LOW_TEMP] = nullptr;

	this->_rooms[7]->_terminal_units[HIGH_TEMP] = new TerminalUnit(6, HIGH_T_PUMP_1, this); // Room 3 Badroom
	this->_rooms[7]->_terminal_units[LOW_TEMP] = nullptr;

	this->_rooms[8]->_terminal_units[HIGH_TEMP] = new TerminalUnit(7, HIGH_T_PUMP_1, this); // Room 4 Hall
	this->_rooms[8]->_terminal_units[LOW_TEMP] = nullptr;

	//Turn everything OFF
	for (auto room: _rooms)
		room->turn_off();
	for (auto pump: _pumps)
		pump->turn_off();
	this->caldron_turn_off();

	//start OneWire bus
	ds.begin();
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
	if ((_mode_curr_temp >= ActiveConfig.mode_switch_temp + ActiveConfig.mode_switch_temp_delta) && (_mode & GAS))
	{
		_mode = WOOD;
		caldron_turn_off();
		for(uint8_t room_id = 0; room_id < numRooms; room_id++)
		{
			_rooms[room_id]->turn_on();
		}
	}
	if ((_mode_curr_temp <= ActiveConfig.mode_switch_temp - ActiveConfig.mode_switch_temp_delta) && (_mode & WOOD))
	{
		for(uint8_t room_id = 0; room_id < numRooms; room_id++)
		{
			_rooms[room_id]->turn_off();
		}
		_mode = GAS;
	}

	if ( _mode & GAS)
	{
		int mode_state = pinState(_mode_pin);

		if ( (mode_state & PRESSED) && !(_mode & COLDY))
		{
			_mode = (GAS | COLDY); //Thermostat turns ON when there is too cold outside
		}

		if ( (mode_state & RELEASED) && !(_mode & WARMY))
		{
			_mode = (GAS | WARMY); //Thermostat turns OFF when there is too hot outside
		}
	}

	//MODE output for debugging
//	if (_mode & GAS)
//		Serial.println("MODE: GAS");
//	if (_mode & WOOD)
//		Serial.println("MODE: WOOD");
//	if (_mode & COLDY)
//		Serial.println("MODE: COLDY");
//	if (_mode & WARMY)
//		Serial.println("MODE: WARMY");
}

void HeatingSystem::caldron_turn_on()
{
	if (_mode & GAS)
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
	if (_mode & GAS)
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
	if (_mode & WOOD)
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
	if (_mode & GAS)
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
		//set 10bit resolution
		ds.reset();
		ds.skip();
		ds.write(0x4e); // write scratchpad cmd
		ds.write(0xff); // write scratchpad 0
		ds.write(0xff); // write scratchpad 1
		ds.write(0b00111111); // write scratchpad config

		ds.reset();
		ds.skip();
		ds.write(0x44); // start conversion

		_temp_readTimer.initializeMs(190, TimerDelegate(&HeatingSystem::_temp_read, this)).start(false);
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

	if (OneWire::crc8(_temp_data, 8) != _temp_data[8])
	{
//		Serial.println("DS18B20 temp crc error!");
		_temp_counter = 0;
		_temp_accum = 0;
		_temp_readTimer.stop();
		_temp_start();
		return;
	}
	float tempRead = ((_temp_data[1] << 8) | _temp_data[0]); //using two's compliment
	if (_temp_counter < temp_reads)
	{
		_temp_counter++;
		_temp_accum += (tempRead / 16);

//		Serial.print("TA "); Serial.println(_temp_accum);
		_temp_readTimer.stop();
		_temp_start();
		return;
	}
	else
	{
		_mode_curr_temp = _temp_accum / temp_reads;
		_temp_counter = 0;
		_temp_accum = 0;
//		Serial.print("MT "); Serial.println(_mode_curr_temp);
	}

//	Serial.print("_mode_curr_temp = "); Serial.println(_mode_curr_temp);
	_temp_readTimer.stop();
}

//HeatingSystem initialisation
HeatingSystem HSystem(9, 15);
