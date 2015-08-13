#ifndef INCLUDE_CONFIGURATION_H_
#define INCLUDE_CONFIGURATION_H_

#include <user_config.h>
#include <SmingCore/SmingCore.h>

// Put you SSID and Password here
#define WIFI_SSID "infjust"
#define WIFI_PWD "jujust12"

#define ONEWIRE_PIN 4

#define VALVE_CONFIG_FILE ".valve.conf" // leading point for security reasons :)

struct ValveConfig
{
	ValveConfig()
	{
		set_temp = 0;
		temp_delta = 0;
		temp_interval = 0;
		switch_interval = 0;
	}

	String NetworkSSID;
	String NetworkPassword;
// ValveControl settings
	float set_temp;
	float temp_delta;
	int temp_interval;
	int switch_interval;
};

ValveConfig loadConfig();
void saveConfig(ValveConfig& cfg);

extern ValveConfig ActiveConfig;

#endif /* INCLUDE_CONFIGURATION_H_ */
