#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <heatcontrol.h>
#include <thermostat.h>

//Pump implementation

Pump::Pump(uint8_t pump_pin, uint16_t pump_on_delay, uint16_t pump_off_delay)
{
	this->_pump_pin = pump_pin;
	this->_pump_on_delay = pump_on_delay;
	this->_pump_off_delay = pump_off_delay;
}

void Pump::turn_on()
{
	if (_consumers == 0)
	{
		this->_pumpTimer.initializeMs(_pump_on_delay * 1000, TimerDelegate(&Pump::turn_on_delayed, this)).start(false);
	}
	_consumers++;


}

void Pump::turn_off()
{
	_consumers--;
	if (_consumers == 0)
	{
		this->_pumpTimer.initializeMs(_pump_off_delay * 1000, TimerDelegate(&Pump::turn_off_delayed, this)).start(false);
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

HeatingSystem::HeatingSystem(uint8_t mode_pin, uint8_t caldron_pin, uint16_t caldron_on_delay)
{
	this->_mode_pin = mode_pin;
	this->_caldron_pin = caldron_pin;
	this->_caldron_on_delay = caldron_on_delay;
	this->_mode = GAS;
	this->_mode_switch_temp = 60;
	this->_mode_switch_temp_delta = 1;
}

void HeatingSystem::check_mode()
{
//TODO: implement mode chek by 1-wire thermometer
}

void HeatingSystem::caldron_turn_on()
{
	if (_caldron_consumers == 0)
		{
			this->_caldronTimer.initializeMs(_caldron_on_delay * 1000, TimerDelegate(&HeatingSystem::_caldron_turn_on_delayed, this)).start(false);
		}
		_caldron_consumers++;
}

void HeatingSystem::caldron_turn_off()
{
	_caldron_consumers--;
	if (_caldron_consumers == 0)
	{
		setState(out_reg, _caldron_pin, false);
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
