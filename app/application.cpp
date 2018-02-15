#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/OneWire/OneWire.h>

#include <configuration.h> // application configuration
#include <heatcontrol.h>
#include <thermostat.h>

#include "webserver.h"

#ifdef GPIO_MCP23017 //use MCP23S17
MCP23017* mcp000;
MCP23017* mcp001;
#endif

Timer SPITimer;
Timer HSystemTimer;

bool web_ap_started = false;

unsigned long counter = 0;

void connectOk();
void connectFail();
void HSystem_loop();

void init()
{
	spiffs_mount();
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(false); // Debug output to serial
	Serial.commandProcessing(false);
	Serial.setTimeout(2000);

	ActiveConfig = loadConfig();
//	ActiveConfig.NetworkSSID = WIFI_SSID;
//	ActiveConfig.NetworkPassword = WIFI_PWD;

//	WifiStation.config(ActiveConfig.NetworkSSID, ActiveConfig.NetworkPassword);
//	WifiStation.enable(false,true);
//	WifiAccessPoint.enable(true);

//	WifiStation.waitConnection(connectOk, 20, connectFail); // We recommend 20+ seconds for connection timeout at start
//	wifi_set_phy_mode(PHY_MODE_11G);
	web_ap_started = true;
	WifiAccessPoint.config("HeatConfig", "20040229", AUTH_WPA2_PSK);
	WifiAccessPoint.enable(true);
	startWebServer();

//SPI_loop init
#ifdef GPIO_MCP23017
	Wire.pins(14,13);
	mcp000 = new MCP23017;
	mcp000->begin(0x000);
	mcp001 = new MCP23017;
	mcp001->begin(0x001);

	mcp000->pinMode(0xFF00); // Set PORTA to OUTPUT 0x00, PORTB to INPUT 0xFF
	mcp000->pullupMode(0xFF00); // turn on internal pull-up for PORTB 0xFF
	mcp000->digitalWrite(0x0000); //Set all PORTA to 0xFF for simple relay which is active LOW

	mcp001->pinMode(0xFF00); // Set PORTA to OUTPUT 0x00, PORTB to INPUT 0xFF
	mcp001->pullupMode(0xFF00); // turn on internal pull-up for PORTB 0xFF
	mcp001->digitalWrite(0x0000); //Set all PORTA to 0xFF for simple relay which is active LOW
#elif
	pinMode(reg_in_latch, OUTPUT);
	pinMode(reg_out_latch, OUTPUT);
#endif
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
