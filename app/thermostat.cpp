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
		_consumers++;
		this->pumpTimer.initializeMs(_pump_on_delay * 1000, TimerDelegate(&Pump::turn_on_delayed, this)).start(false);
	}
	else
		_consumers++;


}

void Pump::turn_off()
{
	_consumers--;
	if (_consumers == 0)
	{
		this->pumpTimer.initializeMs(_pump_off_delay * 1000, TimerDelegate(&Pump::turn_off_delayed, this)).start(false);
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

HeatingSystem::HeatingSystem(uint8_t mode_pin, uint8_t caldron_pin, uint8_t caldron_on_delay)
{
	this->_mode_pin = mode_pin;
	this->_caldron_pin = caldron_pin;
	this->_caldron_on_delay = caldron_on_delay;
	this->_mode = WARMY;
}
