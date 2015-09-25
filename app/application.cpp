#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/OneWire/OneWire.h>

#include <configuration.h> // application configuration
#include <heatcontrol.h>
#include <thermostat.h>

#include "webserver.h"

Timer SPITimer;
Timer HSystemTimer;

bool web_ap_started = false;

unsigned long counter = 0;

void connectOk();
void connectFail();
void HSystem_loop();

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(false); // Debug output to serial
	Serial.commandProcessing(false);
	Serial.setTimeout(2000);

	ActiveConfig = loadConfig();
//	ActiveConfig.NetworkSSID = WIFI_SSID;
//	ActiveConfig.NetworkPassword = WIFI_PWD;

//	WifiStation.config(ActiveConfig.NetworkSSID, ActiveConfig.NetworkPassword);
//	WifiStation.enable(true);
//	WifiAccessPoint.enable(true);

//	WifiStation.waitConnection(connectOk, 20, connectFail); // We recommend 20+ seconds for connection timeout at start
//	wifi_set_phy_mode(PHY_MODE_11G);
	web_ap_started = true;
	WifiAccessPoint.config("HeatConfig", "", AUTH_OPEN);
	WifiAccessPoint.enable(true);
	startWebServer();

//SPI_loop init
	pinMode(reg_in_latch, OUTPUT);
	pinMode(reg_out_latch, OUTPUT);

	SPITimer.initializeMs(200, SPI_loop).start();

//HSystem.check() timer
//	HSystem.check();
	HSystemTimer.initializeMs(2000, HSystem_loop).start();
}

void HSystem_loop()
{
	HSystem.check();
}

void connectOk()
{
	debugf("connected");
//	WifiAccessPoint.enable(false);
	if(! web_ap_started)
	{
		web_ap_started = true;
		WifiAccessPoint.config("HeatConfig", "", AUTH_OPEN);
		WifiAccessPoint.enable(false);
//		startWebServer();
//		procTimer.restart();
	}
}

void connectFail()
{
	debugf("connection FAILED");

	if(! web_ap_started)
	{
		web_ap_started = true;
		WifiAccessPoint.config("HeatConfig", "", AUTH_OPEN);
		WifiAccessPoint.enable(true);
		startWebServer();
	}

	WifiStation.waitConnection(connectOk); // Wait connection
}
