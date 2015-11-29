#ifndef INCLUDE_CONFIGURATION_H_
#define INCLUDE_CONFIGURATION_H_

#include <user_config.h>
#include <SmingCore/SmingCore.h>

const char HEAT_CONFIG_FILE[] = ".heat.conf"; // leading point for security reasons :)

struct HeatConfig
{
	HeatConfig()
	{
		mode_switch_temp = 60; //C degree
		mode_switch_temp_delta = 1; //C dgree
		pump_on_delay = 10; //seconds
		pump_off_delay = 10; //seconds
		caldron_on_delay = 10; //seconds
		room_off_delay = 15; //seconds
		start_minutes = 480; // minutes since 0:00 to start WHPump cycles
		stop_minutes = 1380; // minutes since 0:00 to stop WHPump cycles
		cycle_duration = 5; // Duration of pump stay turning on in minutes
		cycle_interval = 60; // Interval of turning HWPump On for a while in minutes
		time_zone = 2; // Time zone offset from GMT

	}

	String NetworkSSID;
	String NetworkPassword;

// HeatControl settings
	float mode_switch_temp;
	float mode_switch_temp_delta;
	uint16_t pump_on_delay;
	uint16_t pump_off_delay;
	uint16_t caldron_on_delay;
	uint16_t room_off_delay;
	uint16_t start_minutes;
	uint16_t stop_minutes;
	uint8_t cycle_duration;
	uint8_t cycle_interval;
	uint8_t time_zone;

};

HeatConfig loadConfig();
void saveConfig(HeatConfig& cfg);

extern HeatConfig ActiveConfig;

#endif /* INCLUDE_CONFIGURATION_H_ */
