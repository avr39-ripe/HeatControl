#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/OneWire/OneWire.h>

#include "../include/configuration.h" // application configuration

#include "valvecontrol.h"
#include "webserver.h"

Timer tempTimer;
Timer read_tempTimer;
Timer relayTimer;

OneWire ds(ONEWIRE_PIN);

bool web_ap_started = false;

unsigned long counter = 0;
temp_sensor temp_sensors[] = {{{0x28, 0x9D, 0x14, 0x3E, 0x00, 0x00, 0x00, 0xDB},"Sensor1\0",0},
				{{0x28, 0xE3, 0x1D, 0x3E, 0x00, 0x00, 0x00, 0xA3},"Sensor2\0",0},
				{{0x28, 0x97, 0xDD, 0x3D, 0x00, 0x00, 0x00, 0x4D},"Sensor3\0",0}};

relay_pin relay_pins[NUM_SENSORS] = {{14,0},{15,0},{5,0}};

//void process();
void connectOk();
void connectFail();

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(false); // Debug output to serial
	Serial.commandProcessing(false);
	Serial.setTimeout(2000);

	ds.begin();
	for (byte n = 0; n < NUM_SENSORS; n++ )
	{
		pinMode(relay_pins[n].pin, OUTPUT);
	}

	ActiveConfig = loadConfig();
//	ActiveConfig.NetworkSSID = WIFI_SSID;
//	ActiveConfig.NetworkPassword = WIFI_PWD;

	WifiStation.config(ActiveConfig.NetworkSSID, ActiveConfig.NetworkPassword);
	WifiStation.enable(true);
//	WifiAccessPoint.enable(false);

	WifiStation.waitConnection(connectOk, 20, connectFail); // We recommend 20+ seconds for connection timeout at start
//	wifi_set_phy_mode(PHY_MODE_11G);
	WifiAccessPoint.config("ValveConfig", "", AUTH_OPEN);
	WifiAccessPoint.enable(true);
	startWebServer();


	tempTimer.initializeMs(2000, startTemp).start();
	relayTimer.initializeMs(4000, thermostat).start();
//	process();
}


void startTemp() //Start temperature conversion on all sensors
{
	if (! read_tempTimer.isStarted())
	{
		ds.reset();
		ds.skip();
		ds.write(0x44); // start conversion

		read_tempTimer.initializeMs(750, readTemp).start(false);
	}
}
void readTemp()
{
	counter++;

//	ds.reset();
//	ds.skip();
//	ds.write(0x44); // start conversion

//  system_soft_wdt_stop();
//	delay(750);
//  system_soft_wdt_restart();

	for (byte n = 0; n < NUM_SENSORS; n++)
	{

		if (OneWire::crc8(temp_sensors[n].addr, 7) != temp_sensors[n].addr[7])
		{
			Serial.println("CRC is not valid!");
			temp_sensors[n].value = -1000;
		}

		ds.reset();
		ds.select(temp_sensors[n].addr);
		ds.write(0xBE); // Read Scratchpad

		for (byte i = 0; i < 9; i++)
		{
			temp_sensors[n].data[i] = ds.read();
		}

		float tempRead = ((temp_sensors[n].data[1] << 8)
				| temp_sensors[n].data[0]); //using two's compliment
		temp_sensors[n].value = tempRead / 16;


	}
	read_tempTimer.stop();
//	Serial.println();
//	root.printTo(Serial);

}

void thermostat()
{
	for (byte n = 0; n < NUM_SENSORS; n++ )
	{

		if (temp_sensors[n].value >= ActiveConfig.set_temp + ActiveConfig.temp_delta && relay_pins[n].state == 0)
		{
			relay_pins[n].state = 1;
			digitalWrite(relay_pins[n].pin, HIGH);
		}

		if (temp_sensors[n].value <= ActiveConfig.set_temp - ActiveConfig.temp_delta && relay_pins[n].state == 1)
		{
			relay_pins[n].state = 0;
			digitalWrite(relay_pins[n].pin, LOW);
		}
	}
}

void connectOk()
{
	debugf("connected");
//	WifiAccessPoint.enable(false);
	if(! web_ap_started)
	{
		web_ap_started = true;
		WifiAccessPoint.config("ValveConfig", "", AUTH_OPEN);
		WifiAccessPoint.enable(false);
//		startWebServer();
//		procTimer.restart();
	}

//	// At first run we will download web server content
//	if (!fileExist("index.html") || !fileExist("config.html") || !fileExist("bootstrap.css.gz") || !fileExist("jquery.js.gz"))
//	{
//	//If we miss some file in spiff_rom.bin image list FS content for debug
//		auto dir = fileList();
//		for (int i = 0; i < dir.size(); i++)
//			Serial.println(dir[i]);
//		downloadContentFiles();
//	}
//	else
//		startWebServer();
}

void connectFail()
{
	debugf("connection FAILED");

	if(! web_ap_started)
	{
		web_ap_started = true;
		WifiAccessPoint.config("ValveConfig", "", AUTH_OPEN);
		WifiAccessPoint.enable(true);
		startWebServer();
	}

	WifiStation.waitConnection(connectOk); // Wait connection
}
