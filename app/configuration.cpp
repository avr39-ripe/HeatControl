#include "../include/configuration.h"

#include <SmingCore/SmingCore.h>

HeatConfig ActiveConfig;

HeatConfig loadConfig()
{
	DynamicJsonBuffer jsonBuffer;
	HeatConfig cfg;
	if (fileExist(HEAT_CONFIG_FILE))
	{
		int size = fileGetSize(HEAT_CONFIG_FILE);
		char* jsonString = new char[size + 1];
		fileGetContent(HEAT_CONFIG_FILE, jsonString, size + 1);
		JsonObject& root = jsonBuffer.parseObject(jsonString);

		JsonObject& network = root["network"];
		cfg.NetworkSSID = String((const char*)network["ssid"]);
		cfg.NetworkPassword = String((const char*)network["password"]);
		cfg.sta_enable = network["sta_enable"];

		JsonObject& settings = root["settings"];
		cfg.mode_switch_temp = settings["mode_switch_temp"];
		cfg.mode_switch_temp_delta = settings["mode_switch_temp_delta"];
		cfg.pump_on_delay = settings["pump_on_delay"];
		cfg.pump_off_delay = settings["pump_off_delay"];
		cfg.caldron_on_delay = settings["caldron_on_delay"];
		cfg.room_off_delay = settings["room_off_delay"];
		cfg.start_minutes = settings["start_minutes"];
		cfg.stop_minutes = settings["stop_minutes"];
		cfg.cycle_duration = settings["cycle_duration"];
		cfg.cycle_interval = settings["cycle_interval"];
		cfg.time_zone = settings["time_zone"];

		delete[] jsonString;
	}
	else
	{
		//Factory defaults if no config file present
		cfg.NetworkSSID = WIFI_SSID;
		cfg.NetworkPassword = WIFI_PWD;
	}
	return cfg;
}

void saveConfig(HeatConfig& cfg)
{
//	ActiveConfig = cfg;

	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	JsonObject& network = jsonBuffer.createObject();
	root["network"] = network;
	network["ssid"] = cfg.NetworkSSID.c_str();
	network["password"] = cfg.NetworkPassword.c_str();
	network["sta_enable"] = cfg.sta_enable;

	JsonObject& settings = jsonBuffer.createObject();
	root["settings"] = settings;
	settings["mode_switch_temp"] = cfg.mode_switch_temp;
	settings["mode_switch_temp_delta"] = cfg.mode_switch_temp_delta;
	settings["pump_on_delay"] = cfg.pump_on_delay;
	settings["pump_off_delay"] = cfg.pump_off_delay;
	settings["caldron_on_delay"] = cfg.caldron_on_delay;
	settings["room_off_delay"] = cfg.room_off_delay;
	settings["start_minutes"] = cfg.start_minutes;
	settings["stop_minutes"] = cfg.stop_minutes;
	settings["cycle_duration"] = cfg.cycle_duration;
	settings["cycle_interval"] = cfg.cycle_interval;
	settings["time_zone"] = cfg.time_zone;

	char buf[4048];
	root.prettyPrintTo(buf, sizeof(buf));
	fileSetContent(HEAT_CONFIG_FILE, buf);
}


