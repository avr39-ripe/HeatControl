#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <heatcontrol.h>
#include <thermostat.h>


void HeatingSystem::check_room(uint8_t room_id)
{
	int thermostat_state = pinState(_rooms[room_id]->thermostat_pin);

	if ( thermostat_state & JUSTPRESSED)
	{
		if (_mode == WARMY)
		{
			setState(out_reg,_rooms[room_id]->low_t_control_pin, true);
		}
		else if (_mode == COLDY)
		{
			setState(out_reg, _rooms[room_id]->low_t_control_pin, true);
			setState(out_reg, _rooms[room_id]->hi_t_control_pin, true);
		}
	}

	if ( thermostat_state & JUSTRELEASED)
	{
		;
	}
}

void HeatingSystem::room_turn_on(uint8_t room_id)
{
	if (_mode == WARMY && _rooms[room_id]->low_t_control_pin != NO_PIN) //room with lo_temp hot floor and MAY BE hi_temp heaters
	{
		setState(out_reg,_rooms[room_id]->low_t_control_pin, true);
		_pumps[LOW_T_PUMP]->turn_on();
		caldron_turn_on();
	}
	else if (_mode == WARMY && _rooms[room_id]->hi_t_control_pin != NO_PIN) //room with JUST hi_temp heater
	{
		setState(out_reg,_rooms[room_id]->hi_t_control_pin, true);
		_pumps[HI_T_PUMP]->turn_on();
		caldron_turn_on();
	}

	else if (_mode == COLDY)
	{
		if ( _rooms[room_id]->low_t_control_pin != NO_PIN)
		{
			setState(out_reg, _rooms[room_id]->low_t_control_pin, true);
			_pumps[LOW_T_PUMP]->turn_on();
			caldron_turn_on();
		}
		if (_rooms[room_id]->hi_t_control_pin != NO_PIN)
		{
			setState(out_reg, _rooms[room_id]->hi_t_control_pin, true);
			_pumps[HI_T_PUMP]->turn_on();
			caldron_turn_on();
		}
	}
}

void HeatingSystem::room_turn_off(uint8_t room_id)
{
	if (_mode == WARMY && _rooms[room_id]->low_t_control_pin != NO_PIN) //room with lo_temp hot floor and MAY BE hi_temp heaters
	{
		setState(out_reg,_rooms[room_id]->low_t_control_pin, false);
		_pumps[LOW_T_PUMP]->turn_off();
		caldron_turn_off();
	}
	else if (_mode == WARMY && _rooms[room_id]->hi_t_control_pin != NO_PIN) //room with JUST hi_temp heater
	{
		setState(out_reg,_rooms[room_id]->low_t_control_pin, false);
		_pumps[HI_T_PUMP]->turn_off();
		caldron_turn_off();
	}

	else if (_mode == COLDY)
	{
		if ( _rooms[room_id]->low_t_control_pin != NO_PIN)
		{
			this->_roomTimer.initializeMs(_room_coldy_lo_t_off_delay * 1000, myTimerDelegate(&HeatingSystem::_room_coldy_lo_t_off_delayed, this, room_id)).start(false);
		}
		if (_rooms[room_id]->hi_t_control_pin != NO_PIN)
		{
			setState(out_reg, _rooms[room_id]->hi_t_control_pin, false);
			_pumps[HI_T_PUMP]->turn_off();
			caldron_turn_off();
		}
	}
}

//Pump implementation

Pump::Pump(uint8_t pump_pin, uint8_t pump_on_delay, uint8_t pump_off_delay)
{
	this->_pump_pin = pump_pin;
	this->_pump_on_delay = pump_on_delay;
	this->_pump_on_delay = pump_off_delay;
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

HeatingSystem::HeatingSystem(uint8_t mode_pin, uint8_t caldron_pin, uint8_t caldron_on_delay, uint8_t _room_coldy_off_delay)
{
	this->_mode_pin = mode_pin;
	this->_caldron_pin = caldron_pin;
	this->_caldron_on_delay = caldron_on_delay;
	this->_mode = WARMY;
}

void HeatingSystem::check_mode()
{
	int mode_state = pinState(_mode_pin);

		if ( mode_state & JUSTPRESSED)
		{
			_mode == COLDY; //Thermostat turns ON when there is too cold outside
		}

		if ( mode_state & JUSTRELEASED)
		{
			_mode == WARMY; //Thermostat turns OFF when there is too hot outside
		}
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

void HeatingSystem::_room_coldy_lo_t_off_delayed(uint8_t room_id)
{
	setState(out_reg, _rooms[room_id]->low_t_control_pin, false);
	_pumps[LOW_T_PUMP]->turn_off();
	caldron_turn_off();
}
