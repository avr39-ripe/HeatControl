#include "../include/configuration.h"

#include <SmingCore/SmingCore.h>

ValveConfig ActiveConfig;

ValveConfig loadConfig()
{
	DynamicJsonBuffer jsonBuffer;
	ValveConfig cfg;
	if (fileExist(VALVE_CONFIG_FILE))
	{
		int size = fileGetSize(VALVE_CONFIG_FILE);
		char* jsonString = new char[size + 1];
		fileGetContent(VALVE_CONFIG_FILE, jsonString, size + 1);
		JsonObject& root = jsonBuffer.parseObject(jsonString);

		JsonObject& network = root["network"];
		cfg.NetworkSSID = String((const char*)network["ssid"]);
		cfg.NetworkPassword = String((const char*)network["password"]);

		JsonObject& settings = root["settings"];
		cfg.set_temp = settings["set_temp"];
		cfg.temp_delta = settings["temp_delta"];
		cfg.temp_interval = settings["temp_interval"];
		cfg.switch_interval = settings["switch_interval"];

		delete[] jsonString;
	}
	else
	{
		//Factory defaults if no config file present
		cfg.NetworkSSID = WIFI_SSID;
		cfg.NetworkPassword = WIFI_PWD;
		cfg.set_temp = 20;
		cfg.temp_delta = 0.5;
		cfg.temp_interval = 20;
		cfg.switch_interval = 3;
	}
	return cfg;
}

void saveConfig(ValveConfig& cfg)
{
	ActiveConfig = cfg;

	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	JsonObject& network = jsonBuffer.createObject();
	root["network"] = network;
	network["ssid"] = cfg.NetworkSSID.c_str();
	network["password"] = cfg.NetworkPassword.c_str();

	JsonObject& settings = jsonBuffer.createObject();
	root["settings"] = settings;
	settings["set_temp"] = cfg.set_temp;
	settings["temp_delta"] = cfg.temp_delta;
	settings["temp_interval"] = cfg.temp_interval;
	settings["switch_interval"] = cfg.switch_interval;

	char buf[3048];
	root.prettyPrintTo(buf, sizeof(buf));
	fileSetContent(VALVE_CONFIG_FILE, buf);
}


