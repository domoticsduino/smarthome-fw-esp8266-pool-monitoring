# Smarthome firmware for ESP8266 - POOL Monitoring (PH, TEMP, TDS)
Firmware for my board named **ESP8266MCU14**, based on **ESP8266 microcontroller**.

This device use two **DS18B20 probe** to get water and air temperature, a **TDS Meter** to monitor *water quality* and a **PH-4502C board** to monitor PH value. Use **MQTT protocol** to send data to **OpenHAB Smart Home Automation System**.

TDS Meter and PH-4502C are connected to ESP8266 using an external ADC (ADS1115).

**ADS1115** has a maximum analog value of *17625* if powered with **3.3V** with standard gain:

*6,144 (gain) /	32767 (15bit max val) =	0,000187506 (conversion factor between analog value and volt)*

**PH-4502C board** PH formula with calibration:

*PH = 7 - (2.5 - InputVolt) * M*

*M = (pH7 - pHCal) / (Vph7 - VphCal)*

Built with **platformio** (https://platformio.org) and **visual studio code** (https://code.visualstudio.com)

Depends on the following *dd libraries*:

 - ddcommon
 - ddwifi
 - ddmqtt
 - ddtds
 - ddds18b20
 - ddads1115
 
## Compile & Build

To build your firmware bin file:
 - clone this repository with the *--recursive* flag to checkout **dd-libraries**
 - rename file *include/user-config-template.h* in *include/user-config.h*
 - set your **WIFI** and **MQTT** settings in file *include/user-config.h*
 
Firmware file will be in ***.pio/build/esp12e/firmware.bin***

## OTA / WebServer support

To upload using OTA, open *http://ip-device/update* on a browser and point to the firmware \*.bin file

Open *http://ip-device* to get sensor state and device info in your browser

## Branches
 - ***main*** contains work in progress commits - **NOT STABLE branch**
 - ***master*** contains **STABLE** commits and releases

## Credits
 - *For autoversion management script* => **PlatformIO forum** ["How to build got revision into binary for version output?"](https://community.platformio.org/t/how-to-build-got-revision-into-binary-for-version-output/15380/5)
  - *For OTA/WebServer support* => **randomnerdtutorials.com** [ESP8266 NodeMCU OTA (Over-the-Air) Updates - AsyncElegantOTA (VS Code + PlatformIO)](https://randomnerdtutorials.com/esp8266-nodemcu-ota-over-the-air-vs-code), **[AsyncElegantOTA Library](https://github.com/ayushsharma82/AsyncElegantOTA)**