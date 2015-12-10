#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/OneWire/OneWire.h>

#include <SmingCore/Wire.h>
#include <Libraries/DS3232RTC/DS3232RTC.h>

#include <configuration.h> // application configuration
#include <heatcontrol.h>
#include <thermostat.h>

#ifdef MCP23S17 //use MCP23S17 SPI_loop
#include <Libraries/MCP23S17/MCP23S17.h>
#endif

#include "webserver.h"

Timer SPITimer;
Timer HSystemTimer;

String _date_time_str = "";
//double sys_TZ = 3;

bool web_ap_started = false;

unsigned long counter = 0;

void connectOk();
void connectFail();
void HSystem_loop();

#ifdef MCP23S17 //use MCP23S17 SPI_loop
MCP inputchip(1, mcp23s17_cs);             // Instantiate an object called "inputchip" on an MCP23S17 device at  address 1 = 0b00000001 and CS pin = GPIO16
MCP outputchip(0, mcp23s17_cs);            // Instantiate an object called "outputchip" on an MCP23S17 device at address 0 = 0b00000010 and CS pin = GPIO16
#endif

void init()
{
	spiffs_mount(); // Mount file system, in order to work with files
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(false); // Debug output to serial
	Serial.commandProcessing(false);
	Serial.setTimeout(2000);

	//SET higher CPU freq & disable wifi sleep
	system_update_cpu_freq(SYS_CPU_160MHZ);
	wifi_set_sleep_type(NONE_SLEEP_T);

	ActiveConfig = loadConfig();

	if (ActiveConfig.sta_enable == 1)
	{
		WifiStation.config(ActiveConfig.NetworkSSID, ActiveConfig.NetworkPassword);
		WifiStation.enable(true);
		WifiStation.waitConnection(connectOk, 20, connectFail); // We recommend 20+ seconds for connection timeout at start
	}
	else
	{
		WifiStation.enable(false);
	}
//	wifi_set_phy_mode(PHY_MODE_11G);
	WifiAccessPoint.config("HeatConfig", "20040229", AUTH_WPA2_PSK);
	WifiAccessPoint.enable(true);
	startWebServer();

//SPI_loop init
#ifdef MCP23S17 //use MCP23S17 SPI_loop
	inputchip.begin();
	outputchip.begin();
	inputchip.pinMode(0xFFFF);     // Use word-write mode to set all of the pins on inputchip to be inputs
	inputchip.pullupMode(0xFFFF);  // Use word-write mode to Turn on the internal pull-up resistors.
//	inputchip.inputInvert(0x0000); // Use word-write mode to invert the inputs so that logic 0 is read as HIGH
	outputchip.pinMode(0x0000);    // Use word-write mode to Set all of the pins on outputchip to be outputs

	Serial.println("---===MCP23S17 VERSION===---");
#endif

#ifndef MCP23S17
	pinMode(reg_in_latch, OUTPUT);
	pinMode(reg_out_latch, OUTPUT);
#endif

	SPITimer.initializeMs(200, SPI_loop).start();

//HSystem.check() timer
//	HSystem.check();
	HSystemTimer.initializeMs(2000, HSystem_loop).start();

	// I2C bus config and init
//    Wire.pins(scl_pin, sda_pin);
//    Wire.begin();

    //Initial setup & sync from DSRTC system clock
    SystemClock.setTimeZone(ActiveConfig.time_zone);
//    SystemClock.setTime(DSRTC.get(), eTZ_UTC);

    // Start initial HWPump cycle
    HSystem._hwpump->cycle();
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
		WifiAccessPoint.config("HeatConfig", "20040229", AUTH_WPA2_PSK);
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
		WifiAccessPoint.config("HeatConfig", "20040229", AUTH_WPA2_PSK);
		WifiAccessPoint.enable(true);
		startWebServer();
	}

	WifiStation.waitConnection(connectOk); // Wait connection
}
