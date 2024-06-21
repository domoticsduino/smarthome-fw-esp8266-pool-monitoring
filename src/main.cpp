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
DDDS18B20Val tAir;
DDDS18B20Val tWater;

//ADS1115
DDADS1115 analog(0);

//PH4205c
DDPH4205C ph(ADC_VOLTAGE);
DDPH4205CVal phVal;

//TDS
DDTDS tds;
DDTDSVal tdsValue;

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

void createJsonInfo()
{
  JsonDocument wifiJson;
  wifiJson["ssid"] = USER_SETTINGS_WIFI_SSID;
  wifiJson["hostname"] = USER_SETTINGS_WIFI_HOSTNAME;
  JsonDocument mqttJson;
  mqttJson["host"] = USER_SETTINGS_MQTT_HOST;
  mqttJson["port"] = USER_SETTINGS_MQTT_PORT;
  mqttJson["device"] = USER_SETTINGS_MQTT_DEVICE;
  mqttJson["topic"] = USER_SETTINGS_MQTT_TOPIC;
  mqttJson["username"] = USER_SETTINGS_MQTT_USER;
  mqttJson["qos"] = USER_SETTINGS_MQTT_QOS;  
	jsonInfo["name"] = USER_SETTINGS_WIFI_HOSTNAME;
	jsonInfo["version"] = AUTO_VERSION;
  jsonInfo["wifi"] = wifiJson;
  jsonInfo["mqtt"] = mqttJson;
}

// String generateJsonMessageConfig()
// {
// 	String json;
// 	serializeJson(jsonConfig, json);
// 	return json;
// }

String generateJsonMessage()
{
  JsonDocument airTempJson;
  airTempJson["tempC"] = tAir.tempC;
  airTempJson["tempF"] = tAir.tempF;
  airTempJson["success"] = tAir.success;
  airTempJson["errorMsg"] = tAir.errorMsg;
  JsonDocument waterTempJson;
  waterTempJson["tempC"] = tWater.tempC;
  waterTempJson["tempF"] = tWater.tempF;
  waterTempJson["success"] = tWater.success;
  waterTempJson["errorMsg"] = tWater.errorMsg;
  JsonDocument phJson;
  phJson["ph"] = phVal.ph;
  phJson["voltage"] = phVal.voltage;
  JsonDocument tdsJson;
  tdsJson["value"] = tdsValue.value;
  tdsJson["voltage"] = tdsValue.voltage;
  JsonDocument json;
	json["airTemp"] = airTempJson;
	json["waterTemp"] = waterTempJson;
  json["ph"] = phJson;
  json["tds"] = tdsJson;
  json["config"] = jsonConfig;
  json["info"] = jsonInfo;
	String jsonString;
	serializeJson(json, jsonString);
	return jsonString;
}

void setup() {
  createJsonConfig();
  createJsonInfo();
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
	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(200, "application/json", generateJsonMessage());
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
  
  tAir = tempAir.getValue();
  tWater = tempWater.getValue();
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
  phVal = ph.convertValue(analogs.volt1);
  tdsValue = tds.convertValue(analogs.volt4, tWater.tempC);
  writeToSerial("PH: ", false); writeToSerial(phVal.ph, false); writeToSerial(" ", false); writeToSerial(phVal.voltage, false); writeToSerial(" V", true);
  writeToSerial("TDS: ", false); writeToSerial(tdsValue.value, false); writeToSerial(" ", false); writeToSerial(tdsValue.voltage, true);

  clientMqtt.sendMessage(MQTT_TOPIC, generateJsonMessage());

}
