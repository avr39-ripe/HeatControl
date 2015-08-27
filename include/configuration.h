#ifndef INCLUDE_CONFIGURATION_H_
#define INCLUDE_CONFIGURATION_H_

#include <user_config.h>
#include <SmingCore/SmingCore.h>

// Put you SSID and Password here
const char WIFI_SSID[] = "infjust";
const char WIFI_PWD[] = "jujust12";

const char HEAT_CONFIG_FILE[] = ".heat.conf"; // leading point for security reasons :)

struct HeatConfig
{
	HeatConfig()
	{
		mode_switch_temp = 60; //C degree
		mode_switch_temp_delta = 1; //C dgree
		pump_on_delay = 4 * 60; //seconds
		pump_off_delay = 4 *60; //seconds
		caldron_on_delay = 4 * 60; //seconds
	}

	String NetworkSSID;
	String NetworkPassword;

// HeatControl settings
	float mode_switch_temp;
	float mode_switch_temp_delta;
	uint16_t pump_on_delay;
	uint16_t pump_off_delay;
	uint16_t caldron_on_delay;
};

HeatConfig loadConfig();
void saveConfig(HeatConfig& cfg);

extern HeatConfig ActiveConfig;

#endif /* INCLUDE_CONFIGURATION_H_ */
