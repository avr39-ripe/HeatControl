#include <user_config.h>
#include <SmingCore/SmingCore.h>

#include <configuration.h>
#include <heatcontrol.h>
#include <thermostat.h>


bool serverStarted = false;
HttpServer server;

void onIndex(HttpRequest &request, HttpResponse &response)
{
	TemplateFileStream *tmpl = new TemplateFileStream("index.html");
	auto &vars = tmpl->variables();
	vars["Counter"] = String(counter);
	vars["mode_temp"] = String(HSystem._mode_curr_temp);
	response.sendTemplate(tmpl);
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
		}
		if (request.getPostParameter("mode_switch_temp").length() > 0) // Settings
		{
			ActiveConfig.mode_switch_temp = request.getPostParameter("mode_switch_temp").toFloat();
			ActiveConfig.mode_switch_temp_delta = request.getPostParameter("mode_switch_temp_delta").toFloat();
			ActiveConfig.pump_on_delay = request.getPostParameter("pump_on_delay").toInt();
			ActiveConfig.pump_off_delay = request.getPostParameter("pump_off_delay").toInt();
			ActiveConfig.caldron_on_delay = request.getPostParameter("caldron_on_delay").toInt();

//			DynamicJsonBuffer jsonBuffer;
//			JsonObject& root = jsonBuffer.createObject();
//
//			JsonObject& settings = jsonBuffer.createObject();
//			root["settings"] = settings;
//			settings["set_temp"] = ActiveConfig.set_temp;
//			settings["temp_delta"] = ActiveConfig.temp_delta;
//			settings["temp_interval"] = ActiveConfig.temp_interval;
//			settings["switch_interval"] = ActiveConfig.switch_interval;
//
//			root.printTo(Serial);
//			Serial.println();
		}

		saveConfig(ActiveConfig);
		response.redirect();
	}

	debugf("Send template");
	TemplateFileStream *tmpl = new TemplateFileStream("config.html");
	auto &vars = tmpl->variables();
	vars["SSID"] = ActiveConfig.NetworkSSID;
	vars["m_s_t"] = String(ActiveConfig.mode_switch_temp, 2);
	vars["m_s_t_d"] = String(ActiveConfig.mode_switch_temp_delta, 2);
	vars["p_on_d"] = String(ActiveConfig.pump_on_delay);
	vars["p_off_d"] = String(ActiveConfig.pump_off_delay);
	vars["c_on_d"] = String(ActiveConfig.caldron_on_delay);

	response.sendTemplate(tmpl);
}

void onConfiguration_json(HttpRequest &request, HttpResponse &response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();

	json["SSID"] = ActiveConfig.NetworkSSID;
	json["Password"] = ActiveConfig.NetworkPassword;

	json["mode_switch_temp"] = ActiveConfig.mode_switch_temp;
	json["mode_switch_temp_delta"] = ActiveConfig.mode_switch_temp_delta;
	json["pump_on_delay"] = ActiveConfig.pump_on_delay;
	json["pump_off_delay"] = ActiveConfig.pump_off_delay;
	json["caldron_on_delay"] = ActiveConfig.caldron_on_delay;

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
//	JsonArray& sensors = json.createNestedArray("sensors");

//TODO: add temrerature on caldron exit here to display in status
	json["counter"] = counter;
	json["mode_curr_temp"] = HSystem._mode_curr_temp;
	_date_time_str = SystemClock.getSystemTimeString();
	json["date_time"] = _date_time_str.c_str();
//	for (byte n = 0; n < NUM_SENSORS; n++)
//	{
//		JsonObject& sensor = sensors.createNestedObject();
//		sensor["name"] = temp_sensors[n].addr_str;
//		sensor["temp"] = temp_sensors[n].value;
//		sensor["state"] = relay_pins[n].state;
//	}
//
	response.sendJsonObject(stream);
}


void startWebServer()
{
	if (serverStarted) return;

	server.listen(80);
	server.addPath("/", onIndex);
	server.addPath("/config", onConfiguration);
	server.addPath("/config.json", onConfiguration_json);
	server.addPath("/state", onAJAXGetState);
	server.setDefaultHandler(onFile);
	serverStarted = true;

	if (WifiStation.isEnabled())
		debugf("STA: %s", WifiStation.getIP().toString().c_str());
	if (WifiAccessPoint.isEnabled())
		debugf("AP: %s", WifiAccessPoint.getIP().toString().c_str());
}
