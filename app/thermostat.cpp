#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <heatcontrol.h>
#include <thermostat.h>

//Room implementation

Room::Room(uint8_t thermostat_pin, uint8_t hi_t_control_pin, uint8_t low_t_control_pin)
{
	this->_thermostat_pin = thermostat_pin;
	this->_hi_t_control_pin = hi_t_control_pin;
	this->_low_t_control_pin = low_t_control_pin;

}

void Room::run()
{
	int thermostat_state = pinState(_thermostat_pin);

	if ( thermostat_state & JUSTPRESSED)
	{
		if (HeatingSystem::mode == WARMY)
		{
			setState(out_reg, _low_t_control_pin, true);
		}
		else if (HeatingSystem::mode == COLDY)
		{
			setState(out_reg, _low_t_control_pin, true);
			setState(out_reg, _hi_t_control_pin, true);
		}
	}

	if ( thermostat_state & JUSTRELEASED)
	{
		;
	}
}
//Pump implementation

Pump::Pump(uint8_t pump_pin, uint8_t pump_on_delay)
{
	this->_pump_pin = pump_pin;
	this->_pump_on_delay = pump_on_delay;
	this->pumpTimer.initializeMs(_pump_on_delay * 1000, TimerDelegate(&Pump::turn_on_delayed, this));
}

void Pump::turn_on()
{
	if (_consumers == 0)
	{
		_consumers++;
		pumpTimer.start(false);
	}
	else
		_consumers++;


}

void Pump::turn_off()
{
	_consumers--;
	if (_consumers == 0)
	{
		setState(out_reg, _pump_pin, false);
	}

}

void Pump::turn_on_delayed()
{
	setState(out_reg, _pump_pin, true);
}

//HeatingSystem implementation

HeatingSystem::HeatingSystem(uint8_t mode_pin, uint8_t caldron_pin, uint8_t caldron_on_delay)
{
	this->_mode_pin = mode_pin;
	this->_caldron_pin = caldron_pin;
	this->_caldron_on_delay = caldron_on_delay;
	this->mode = WARMY;
}
