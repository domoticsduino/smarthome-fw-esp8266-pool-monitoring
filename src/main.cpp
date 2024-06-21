/* 1.0.0 VERSION */

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

#include "../include/config.h"

#include <ddcommon.h>
#include <ddwifi.h>
#include <ddmqtt.h>
#include <ddads1115.h>
#include <ddds18b20.h>
#include <ddtds.h>
#include <ddph4205c.h>

//Wifi
DDWifi wifi(ssid, password, wifihostname, LEDSTATUSPIN);

//MQTT
DDMqtt clientMqtt(MQTT_DEVICE, MQTT_HOST, MQTT_PORT, MQTT_USER, MQTT_PWD, MQTT_TOPIC, MQTT_QOS, LEDSTATUSPIN);

//DS18B20
DDDS18B20 tempWater(LEDSTATUSPIN, DS18B20WATERPIN);
DDDS18B20 tempAir(LEDSTATUSPIN, DS18B20AIRPIN);

//ADS1115
DDADS1115 analog(0);

//PH4205c
DDPH4205C ph(ADC_VOLTAGE);

//TDS
DDTDS tds;

//JSON
JsonDocument jsonConfig;
JsonDocument jsonInfo;

// WEB SERVER - OTA
AsyncWebServer server(80);

void createJsonConfig()
{
	jsonConfig["readInterval"] = READ_INTERVAL;
  jsonConfig["adcVoltage"] = ADC_VOLTAGE;
}

String generateJsonMessageConfig()
{
	String json;
	serializeJson(jsonConfig, json);
	return json;
}

String generateJsonMessage(DDDS18B20Val tAir, DDDS18B20Val tWater, float phValue, float tdsValue)
{
  JsonDocument json;
	json["airTemp"] = tAir.tempC;
	json["waterTemp"] = tWater.tempC;
  json["ph"] = phValue;
  json["tds"] = tdsValue;
	String jsonString;
	serializeJson(json, jsonString);
	return jsonString;
}

void setup() {
  createJsonConfig();
  pinMode(LEDSTATUSPIN, OUTPUT);
	digitalWrite(LEDSTATUSPIN, LOW);

	if (SERIAL_ENABLED)
		Serial.begin(SERIAL_BAUDRATE);
  writeToSerial(USER_SETTINGS_WIFI_HOSTNAME, false);
	writeToSerial(" Booting...", true);
	writeToSerial("FW Version: ", false);
	writeToSerial(AUTO_VERSION, true);

  // WIFI
	wifi.connect();

	//MQTT
	clientMqtt.reconnectMQTT();

  //WEB SERVER
	jsonInfo["name"] = USER_SETTINGS_WIFI_HOSTNAME;
	jsonInfo["version"] = AUTO_VERSION;
	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(200, "application/json", generateJsonMessageConfig());
	});
	AsyncElegantOTA.begin(&server);
	server.begin();
	writeToSerial("Http server started", true);

  //DS18B20
  pinMode(DS18B20AIRPIN, INPUT_PULLUP);
  pinMode(DS18B20WATERPIN, INPUT_PULLUP);
  tempAir.beginSensor();
  tempWater.beginSensor();

}

void loop() {

  AsyncElegantOTA.loop();

  delay(jsonConfig["readInterval"]);

  clientMqtt.loop();

  //TEMPERATURE
  
  DDDS18B20Val tAir = tempAir.getValue();
  DDDS18B20Val tWater = tempWater.getValue();
  writeToSerial("T-air: ", false);
  if(tAir.success)
    writeToSerial(tAir.tempC, true);
  else
    writeToSerial(tAir.errorMsg, true);
  writeToSerial("T-water: ", false);
  if(tWater.success)
    writeToSerial(tWater.tempC, true);
  else
    writeToSerial(tWater.errorMsg, true);
  //ANALOGS
  DDADS1115Val analogs = analog.getValues();
  DDPH4205CVal phVal = ph.convertValue(analogs.volt1);
  DDTDSVal tdsValue = tds.convertValue(analogs.volt4, tWater.tempC);
  writeToSerial("PH: ", false); writeToSerial(phVal.ph, false); writeToSerial(" ", false); writeToSerial(phVal.voltage, false); writeToSerial(" V", true);
  writeToSerial("TDS: ", false); writeToSerial(tdsValue.value, false); writeToSerial(" ", false); writeToSerial(tdsValue.voltage, true);

  clientMqtt.sendMessage(MQTT_TOPIC, generateJsonMessage(tAir, tWater, phVal.ph, tdsValue.value));

}
