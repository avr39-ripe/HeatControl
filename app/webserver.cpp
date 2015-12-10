#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/DS3232RTC/DS3232RTC.h>

#include <configuration.h>
#include <heatcontrol.h>
#include <thermostat.h>


bool serverStarted = false;
HttpServer server;

void onIndex(HttpRequest &request, HttpResponse &response)
{
	response.setCache(86400, true); // It's important to use cache for better performance.
	response.sendFile("index.html");
}

void onConfiguration(HttpRequest &request, HttpResponse &response)
{
//	ValveConfig cfg = loadConfig();
	if (request.getRequestMethod() == RequestMethod::POST)
	{
		debugf("Update config");
		// Update config
		if (request.getPostParameter("SSID").length() > 0) // Network
		{
			ActiveConfig.NetworkSSID = request.getPostParameter("SSID");
			ActiveConfig.NetworkPassword = request.getPostParameter("Password");
			ActiveConfig.sta_enable = request.getPostParameter("sta_enable").toInt();

			if (ActiveConfig.sta_enable == 1)
			{
				WifiStation.config(ActiveConfig.NetworkSSID, ActiveConfig.NetworkPassword);
				WifiStation.enable(true);
			}
			else
			{
				WifiStation.enable(false);
			}

		}
		if (request.getPostParameter("mode_switch_temp").length() > 0) // Settings
		{
			ActiveConfig.mode_switch_temp = request.getPostParameter("mode_switch_temp").toFloat();
			ActiveConfig.mode_switch_temp_delta = request.getPostParameter("mode_switch_temp_delta").toFloat();
			ActiveConfig.pump_on_delay = request.getPostParameter("pump_on_delay").toInt();
			ActiveConfig.pump_off_delay = request.getPostParameter("pump_off_delay").toInt();
			ActiveConfig.caldron_on_delay = request.getPostParameter("caldron_on_delay").toInt();
			ActiveConfig.room_off_delay = request.getPostParameter("room_off_delay").toInt();
		}
		if (request.getPostParameter("start_minutes").length() > 0) // Settings
		{
			ActiveConfig.start_minutes = request.getPostParameter("start_minutes").toInt();
			ActiveConfig.stop_minutes = request.getPostParameter("stop_minutes").toInt();
			ActiveConfig.cycle_duration = request.getPostParameter("cycle_duration").toInt();
			ActiveConfig.cycle_interval = request.getPostParameter("cycle_interval").toInt();
			HSystem._hwpump->cycle();
		}

		if (request.bodyBuf == NULL)
			Serial.println("NULL bodyBuf");
		else
		{
			Serial.print("HERE IS bodyBuf ! ");
			Serial.println(request.bodyBuf);
		}
		saveConfig(ActiveConfig);
	//	response.redirect();
	}
	else
	{
		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile("config.html");
	}
}

void onConfiguration_json(HttpRequest &request, HttpResponse &response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();

	json["SSID"] = ActiveConfig.NetworkSSID;
	json["Password"] = ActiveConfig.NetworkPassword;
	json["sta_enable"] = ActiveConfig.sta_enable;

	json["mode_switch_temp"] = ActiveConfig.mode_switch_temp;
	json["mode_switch_temp_delta"] = ActiveConfig.mode_switch_temp_delta;
	json["pump_on_delay"] = ActiveConfig.pump_on_delay;
	json["pump_off_delay"] = ActiveConfig.pump_off_delay;
	json["caldron_on_delay"] = ActiveConfig.caldron_on_delay;
	json["room_off_delay"] = ActiveConfig.room_off_delay;
	json["start_minutes"] = ActiveConfig.start_minutes;
	json["stop_minutes"] = ActiveConfig.stop_minutes;
	json["cycle_duration"] = ActiveConfig.cycle_duration;
	json["cycle_interval"] = ActiveConfig.cycle_interval;

	response.sendJsonObject(stream);
}
void onFile(HttpRequest &request, HttpResponse &response)
{
	String file = request.getPath();
	if (file[0] == '/')
		file = file.substring(1);

	if (file[0] == '.')
		response.forbidden();
	else
	{
		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile(file);
	}
}

void onAJAXGetState(HttpRequest &request, HttpResponse &response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();

	json["counter"] = counter;
	json["mode_curr_temp"] = HSystem._mode_curr_temp;
	_date_time_str = SystemClock.getSystemTimeString();
	json["date_time"] = _date_time_str.c_str();
	json["mode"] = HSystem._mode;

	response.sendJsonObject(stream);
}

void onAJAXDateTime(HttpRequest &request, HttpResponse &response)
{
TimeElements tm;
	if (request.getRequestMethod() == RequestMethod::POST)
	{
		if (request.getPostParameter("time_zone").length() > 0)
		{
			ActiveConfig.time_zone = request.getPostParameter("time_zone").toInt();
			saveConfig(ActiveConfig);

			SystemClock.setTimeZone(ActiveConfig.time_zone);

			tm.Second = request.getPostParameter("Second").toInt();
			tm.Minute = request.getPostParameter("Minute").toInt();
			tm.Hour = request.getPostParameter("Hour").toInt();
			tm.Wday = request.getPostParameter("Wday").toInt();
			tm.Day = request.getPostParameter("Day").toInt();
			tm.Month = request.getPostParameter("Month").toInt() + 1; // JS Counts month from 0 and tm.elements from 1
			tm.Year = CalendarYrToTm(request.getPostParameter("Year").toInt());

			DSRTC.set(makeTime(tm));
			SystemClock.setTime(DSRTC.get(), eTZ_UTC);
		}
	}
}

void startWebServer()
{
	if (serverStarted) return;

	server.listen(80);
	server.addPath("/", onIndex);
	server.addPath("/config", onConfiguration);
	server.addPath("/config.json", onConfiguration_json);
	server.addPath("/state", onAJAXGetState);
	server.addPath("/datetime", onAJAXDateTime);
	server.setDefaultHandler(onFile);
	serverStarted = true;

	if (WifiStation.isEnabled())
		debugf("STA: %s", WifiStation.getIP().toString().c_str());
	if (WifiAccessPoint.isEnabled())
		debugf("AP: %s", WifiAccessPoint.getIP().toString().c_str());
}
