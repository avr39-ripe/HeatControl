#ifndef INCLUDE_CONFIGURATION_H_
#define INCLUDE_CONFIGURATION_H_

#include <SmingCore.h>
#include <JsonObjectStream.h>

//Wifi Station mode SSID and PASSWORD
#ifndef WIFI_SSID
        #define WIFI_SSID "PleaseEnterSSID" // Put you Station mode SSID and Password here
        #define WIFI_PWD "PleaseEnterPass"
#endif

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
