# esp8266-temperature-humidity

Mains or "low-power" Battery powered IoT Temperature and Humidity sensor based on the ESP8266 chip and <strike>DHT22/</strike>Si7021 sensor.

![Example output via Home Assistant](https://user-images.githubusercontent.com/534681/108772794-601b3d80-7555-11eb-9b4d-4df79693293a.png)

![Example energy usage over startup and 4 measurements](https://user-images.githubusercontent.com/534681/108771132-1d586600-7553-11eb-81da-94f4f0a3b475.png)


## Prerequisites

1. An ESP8266 implementation like ESP-01, ESP-12F or NodeMCU
2. A <strike>DHT22 or</strike> Si7021 sensor
2. An MQTT server like [Mosquitto](https://mosquitto.org/)
3. 2.4GHz WiFi network with the ability to assign static IP addresses
4. Arduino IDE for flashing

## [Hardware](hardware/)

Tested on:

1. ESP-01 _lacks easy GPIO16 access for deep sleep_
2. ESP-12F **current preferred module**
3. NodeMCU _has power-hungry onboard components_

Tested with:

1. DHT22 _inefficient and low precision_
2. Si7021 **current preferred sensor**

Tested with:

1. 2xAA batteries _fine but unsustainable_
2. Generic JST Li-Ion battery **current preferred battery**

Tested with:

1. MCP1700 **current preferred regulator**
2. TPS62203 _switching so fiddly to get right_

End goal is to print the smallest PCB + battery package and 3D print a case for discrete deployment.

## [Software](software/)

Goal is to make the device last as long as possible on batteries.

## Energy performance

Current best energy usage is using the Si7021 sensor, measured using a [currentRanger](https://lowpowerlab.com/guide/currentranger/).

**Startup: `0.0789mAh`**

![Startup](https://user-images.githubusercontent.com/534681/108770101-d322b500-7551-11eb-8677-2e6cbe59e47d.png)

This includes initial connection to WiFi, sensor reading, publication, writing to RTC memory.

**Reading (no change): `0.0012mAh`**

![No change](https://user-images.githubusercontent.com/534681/108770106-d3bb4b80-7551-11eb-8f68-bae0528f92d5.png)

This shows the initial reading of RTC memory, sensor measurement, sleep and wake up to publish result.

**Reading (change): `0.0285mAh`**

![Change](https://user-images.githubusercontent.com/534681/108770099-d1f18800-7551-11eb-9519-3d9974935d29.png)

This shows the initial reading of RTC memory, sensor measurement, sleep and wake up to publish result.

**Energy estimates**

* Worst case (a reading every 10 minutes): `4.75mAh/day` this should last **~ 9 months** on a 1200mAh battery.
* Better case (heuristically [50 readings a day](analysis/temperature.ipynb)): `2.19mAh/day` this should last **over a year** on a 1200mAh battery.
