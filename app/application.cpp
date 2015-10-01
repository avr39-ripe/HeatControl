#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/OneWire/OneWire.h>

#include <SmingCore/Wire.h>
#include <Libraries/DS3232RTC/DS3232RTC.h>

#include <configuration.h> // application configuration
#include <heatcontrol.h>
#include <thermostat.h>

#include "webserver.h"

Timer SPITimer;
Timer HSystemTimer;

String _date_time_str = "";

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

	//SET higher CPU freq & disable wifi sleep
	system_update_cpu_freq(SYS_CPU_160MHZ);
	wifi_set_sleep_type(NONE_SLEEP_T);

	ActiveConfig = loadConfig();
//	ActiveConfig.NetworkSSID = WIFI_SSID;
//	ActiveConfig.NetworkPassword = WIFI_PWD;

	WifiStation.config(ActiveConfig.NetworkSSID, ActiveConfig.NetworkPassword);
	WifiStation.enable(true);
//	WifiAccessPoint.enable(false);

	WifiStation.waitConnection(connectOk, 20, connectFail); // We recommend 20+ seconds for connection timeout at start
//	wifi_set_phy_mode(PHY_MODE_11G);
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

    Wire.pins(0, 2);
    Wire.begin();
    SystemClock.setTime(DSRTC.get());
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
