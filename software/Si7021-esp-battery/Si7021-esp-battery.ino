/*
 * ESP-12F with Si7021 (battery-powered)
 *
 * MQTT Temperature and Humidity Sensor
 *
 * Designed for integration with HomeAssistant
 *
 */

#define debug false // serial output at 74880 baud
#define dhcp false // whether to obtain an IP address (slower) via dhcp
 
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Si7021.h>

#include "config.h" // WiFi and device settings

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_Si7021 sensor = Adafruit_Si7021();

// CRC function used to ensure data validity
uint32_t calculateCRC32(const uint8_t *data, size_t length);

// helper function to dump memory contents as hex
void printMemory();

// helper function to save memory
void saveMemory();

// Structure which will be stored in RTC memory.
// First field is CRC32, which is calculated based on the
// rest of structure contents.
// Must be 4-byte aligned
struct {
  uint32_t crc32; // 4 bytes
  struct {
    float temperature;    // 4 bytes
    float humidity;       // 4 bytes
    float vbatt;            // 4 bytes
    uint8_t bssid[6];       // 6 bytes
    uint8_t channel;        // 1 byte
    bool publish;           // 1 byte
    bool publishHumidity; // 1 byte
    bool publishVbatt;      // 1 byte
    uint8_t padding[2];   // 2 bytes
  } data;
} rtcData;

bool quickConnect = false; // setting to connect quickly from RTC data
float temperature, humidity;

void setup() {

  if (debug) {
    Serial.begin(74880);
    while (!Serial) {}
    Serial.setDebugOutput(true);
    Serial.println("Starting up");
    Serial.flush();
  }
  
  // switch off WiFi until we need it
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  delay(1);

  // power down the Si7021 sensor until we need it
  pinMode(siPowerPin,OUTPUT);
  digitalWrite(siPowerPin, HIGH); // inverted

  // power down the ADC
  pinMode(adcPowerPin, OUTPUT);
  digitalWrite(adcPowerPin, HIGH); // inverted

  // read RTC memory
  if (ESP.rtcUserMemoryRead(0, (uint32_t*) &rtcData, sizeof(rtcData))) {
    uint32_t crcOfData = calculateCRC32((uint8_t*) &rtcData.data, sizeof(rtcData.data));
    if (crcOfData != rtcData.crc32) {
      if (debug) Serial.println("CRC32 in RTC memory doesn't match CRC32 of data. Data is probably invalid!");
      // take fresh readings
      readAllSensors();
      // flag to publish
      rtcData.data.publish = true;
      rtcData.data.publishHumidity = true;
      rtcData.data.publishVbatt = true;
    } else {
      if (debug) {
        Serial.println("CRC32 check ok, data is probably valid.");
        Serial.print("RTC memory size: ");
        Serial.println(sizeof(rtcData));
        Serial.println();
        Serial.println("Read: ");
        printMemory();
        Serial.println();
        Serial.print("CRC32 of data: ");
        Serial.println(crcOfData, HEX);
        Serial.print("CRC32 read from RTC: ");
        Serial.println(rtcData.crc32, HEX);
        Serial.print("RTC Temperature: ");
        Serial.println(rtcData.data.temperature);
        Serial.print("RTC Humidity: ");
        Serial.println(rtcData.data.humidity);
        Serial.print("RTC Vbatt: ");
        Serial.println(rtcData.data.vbatt);
        Serial.print("RTC WiFi channel: ");
        Serial.println(rtcData.data.channel);
        Serial.print("RTC BSSID: ");
        for (int i=0; i<6; i++) {
          Serial.print(rtcData.data.bssid[i],HEX);
        }
        Serial.println();
        Serial.print("RTC publish: ");
        Serial.println(rtcData.data.publish);
        Serial.print("RTC publish humidity: ");
        Serial.println(rtcData.data.publishHumidity);
        Serial.print("RTC publish vbatt: ");
        Serial.println(rtcData.data.publishVbatt);
      }
      quickConnect = true;
    }
  } else {
    if (debug) Serial.println("Failed to read from memory");
    // take fresh readings
    readAllSensors();
    // flag to publish
    rtcData.data.publish = true;
    rtcData.data.publishHumidity = true;
    rtcData.data.publishVbatt = true;
  }

  // we should have RF, and have been asked to publish
  if (rtcData.data.publish) {

    if (debug) {
      Serial.println("Publishing:");
      Serial.print("Temperature: ");
      Serial.println(rtcData.data.publish); // should always be true
      Serial.print("Humidity: ");
      Serial.println(rtcData.data.publishHumidity);
      Serial.print("Vbatt: ");
      Serial.println(rtcData.data.publishVbatt);
      Serial.println();
      Serial.print("Connecting to ");
      Serial.println(wifi_ssid);
    }

    // minimise WiFi cycle as much as possible

    WiFi.forceSleepWake();
    delay(1);

    WiFi.persistent(false);

    // Bring up the WiFi connection
    WiFi.mode(WIFI_STA);

    // Static IP assignment
    if (!dhcp) WiFi.config(ip,gateway,subnet);

    WiFi.hostname(wifi_hostname);

    if (quickConnect) {
      if (debug) Serial.println("Quick connect");
      WiFi.begin( wifi_ssid, wifi_password, rtcData.data.channel, rtcData.data.bssid, true );
    } else {
      if (debug) Serial.println("Standard connect");
      WiFi.begin(wifi_ssid, wifi_password);
    }

    int count = 0;
    while (WiFi.status() != WL_CONNECTED) {
      if (count > 150) { // 15 seconds
          if (debug) {
            Serial.println("WiFi connection timeout, dropping back into deep sleep");
            Serial.print("WiFi status code: ");
            Serial.println(WiFi.status());
            int n = WiFi.scanNetworks(false, true);
            Serial.print(n);
            Serial.println(" WiFi network(s) detected");
          }
          // go back to sleep and try again when woken up
          ESP.deepSleepInstant(polling_us, WAKE_RF_DEFAULT ); // in uS
      }
      delay(100);
      if (debug) Serial.print(".");
      count++;
    }

    if (debug) {
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      Serial.println();
      Serial.println("MAC address: ");
      Serial.println(WiFi.macAddress());
      Serial.println();
    }

    client.setServer(mqtt_server, 1883);

    if (!client.connected()) {
      if (debug) Serial.print("Attempting MQTT connection...");
      if (client.connect("ESP8266Client")) {
        if (debug) Serial.println("connected");
      } else {
          if (debug) {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" going back to sleep to try again later");
          }
          // go back to sleep and try again when woken up
          ESP.deepSleepInstant(polling_us, WAKE_RF_DEFAULT ); // in uS
      }
    }

    // publish requested data
    client.publish(temperature_topic, String(rtcData.data.temperature).c_str(), true);
    if (rtcData.data.publishHumidity) client.publish(humidity_topic, String(rtcData.data.humidity).c_str(), true);
    if (rtcData.data.publishVbatt) client.publish(vbatt_topic, String(rtcData.data.vbatt).c_str(), true);

    client.disconnect();

    // always store BSSID and channel in case device hops APs
    rtcData.data.channel = WiFi.channel();
    memcpy( rtcData.data.bssid, WiFi.BSSID(), 6 ); // Copy 6 bytes of BSSID (AP's MAC address)
    if (debug) {
      Serial.print("Storing BSSID: ");
      for (int i=0; i<6; i++) {
        Serial.print(rtcData.data.bssid[i],HEX);
      }
      Serial.println();
      Serial.print("Storing channel: ");
      Serial.println(rtcData.data.channel);
    }

    // update values
    rtcData.data.publish = false;
    rtcData.data.publishHumidity = false;
    rtcData.data.publishVbatt = false;

    saveMemory();

    if (debug) Serial.println("Publishing complete, going to RF deep sleep");

    ESP.deepSleepInstant(polling_us, WAKE_RF_DISABLED ); // in uS

  } else { // check to see if we need to publish due to temp change

    readSi7021();

    float tempDifference = fabs(temperature - rtcData.data.temperature);
    if (debug) {
      Serial.print("Temperature difference: ");
      Serial.println(tempDifference);
    }

    if (tempDifference > temperatureChangeThreshold) {
      if (debug) Serial.println("Temperature change detected, flagging to publish");
      rtcData.data.publish = true;
      rtcData.data.temperature = temperature;
    } else {
      if (debug) Serial.println("No significant temperature change detected, skipping");
    }

    // only check other readings if temperature is going to be published
    if (rtcData.data.publish) {
      float humidityDifference = fabs(humidity - rtcData.data.humidity);

      if (debug) {
        Serial.print("Humidity difference: ");
        Serial.println(humidityDifference);
      }

      if (humidityDifference > humidityChangeThreshold) {
        if (debug) Serial.println("Humidity change detected, flagging to publish");
        rtcData.data.publishHumidity = true;
        rtcData.data.humidity = humidity;
      } else {
        if (debug) Serial.println("Skipping humidity publication");
      }

      float vbatt = readVbatt();
      float vbattDifference = fabs(rtcData.data.vbatt-vbatt);

      if (debug) {
        Serial.print("Vbatt difference: ");
        Serial.println(vbattDifference);
      }

      // prepare to publish vcc
      if (vbattDifference >= 0.01) {
        if (debug) Serial.println("Vbatt change detected; will publish");
        rtcData.data.publishVbatt = true;
        rtcData.data.vbatt = vbatt;
      } else {
        if (debug) Serial.println("Skipping Vbatt publication");
      }

      saveMemory();

      // reset with rf enabled
      ESP.deepSleepInstant(1000, WAKE_RF_DEFAULT ); // in uS

    }  else {// end if temp will be published

      if (debug) Serial.println("No change detected, going back to sleep");

      ESP.deepSleepInstant(polling_us, WAKE_RF_DISABLED ); // in uS

    } // end no publication

  } // end check if temp will be published
  
} // end setup

void loop() {
} // end loop

// calculate checksum of struct
uint32_t calculateCRC32(const uint8_t *data, size_t length) {
  uint32_t crc = 0xffffffff;
  while (length--) {
    uint8_t c = *data++;
    for (uint32_t i = 0x80; i > 0; i >>= 1) {
      bool bit = crc & 0x80000000;
      if (c & i) {
        bit = !bit;
      }
      crc <<= 1;
      if (bit) {
        crc ^= 0x04c11db7;
      }
    }
  }
  return crc;
}

//prints all rtcData, including the leading crc32
void printMemory() {
  char buf[3];
  uint8_t *ptr = (uint8_t *)&rtcData;
  for (size_t i = 0; i < sizeof(rtcData); i++) {
    sprintf(buf, "%02X", ptr[i]);
    Serial.print(buf);
    if ((i + 1) % 32 == 0) {
      Serial.println();
    } else {
      Serial.print(" ");
    }
  }
  Serial.println();
} // end printMemory

// save data to rtc
void saveMemory() {
  // Update CRC32 of data
  rtcData.crc32 = calculateCRC32((uint8_t*) &rtcData.data, sizeof(rtcData.data));
  // Write struct to RTC memory
  if (ESP.rtcUserMemoryWrite(0, (uint32_t*) &rtcData, sizeof(rtcData))) {
    if (debug) {
      Serial.println("Wrote: ");
      printMemory();
      Serial.println();
      Serial.print("Temperature: ");
      Serial.println(rtcData.data.temperature);
      Serial.print("Humidity: ");
      Serial.println(rtcData.data.humidity);
      Serial.print("Vbatt: ");
      Serial.println(rtcData.data.vbatt);
      Serial.print("Publish: ");
      Serial.println(rtcData.data.publish);
      Serial.print("Publish humidity: ");
      Serial.println(rtcData.data.publishHumidity);
      Serial.print("Publish vbatt: ");
      Serial.println(rtcData.data.publishVbatt);
    }
  } else {
    if (debug) Serial.println("Failed to write to memory");
  }

} // end saveMemory

// read voltage of battery
float readVbatt() {

    if (debug) {
      Serial.println("Measuring battery voltage");
    }

    // switch on ADC circuit
    digitalWrite(adcPowerPin, LOW); // inverted

    if (debug) {
      Serial.println("ADC voltage enabled for 3s");
      delay(3000);
    }

    // first adc reading is garbage, so take another
    int adc = analogRead(A0);
    adc = analogRead(A0);
    if (debug) {
      Serial.print("ADC reading: ");
      Serial.println(adc);
    }
    
    // switch off ADC circuit
    digitalWrite(adcPowerPin, HIGH); // inverted

    if (debug) {
      Serial.println("ADC voltage disabled for 3s");
      delay(3000);
    }

    float vadc = mapfloat(adc,0,1023,0,vref);
    float vbatt = ((R1 + R2)/R2) * vadc;

    if (debug) {
      Serial.print("Vadc: ");
      Serial.println(vadc);
      Serial.print("Vbatt: ");
      Serial.println(vbatt);
    }

    return vbatt;
} // end readVbatt

// read all sensors and store in rtcData
void readAllSensors() {
    readSi7021();
    // store in RTC variables
    rtcData.data.temperature = temperature;
    rtcData.data.humidity = humidity;
    rtcData.data.vbatt = readVbatt();
} // end readAllSensors

// read Si7021 chip using Adafruit library
void readSi7021() {

  if (debug) {
    Serial.println("Powering on the Si7021 sensor in 3s..."); 
    delay(3000);
  }
  
  // switch on the Si7021 sensor
  digitalWrite(siPowerPin, LOW); // inverted

  if (debug) {
    Serial.println("Si7021 sensor is now powered on for 3s...");
    delay(3000);
  }

  // allow the Si7021 sensor time to wake up
  delay(25); 
  if (!sensor.begin()) {
    if (debug) Serial.println("Did not find Si7021 sensor! Crashing now...");
    while (true)
      ; // causes a crash/reboot
  }

  if (debug) {
    Serial.print("Found model ");
    switch(sensor.getModel()) {
      case SI_Engineering_Samples:
        Serial.print("SI engineering samples"); break;
      case SI_7013:
        Serial.print("Si7013"); break;
      case SI_7020:
        Serial.print("Si7020"); break;
      case SI_7021:
        Serial.print("Si7021"); break;
      case SI_UNKNOWN:
      default:
        Serial.print("Unknown");
    }
    Serial.print(" Rev(");
    Serial.print(sensor.getRevision());
    Serial.print(")");
    Serial.print(" Serial #"); Serial.print(sensor.sernum_a, HEX); Serial.println(sensor.sernum_b, HEX);
  }
  
  // read latest sensors
  humidity = sensor.readHumidity();
  temperature = sensor.readTemperature();

  if (debug) {
    Serial.print("Reading temperature: ");
    Serial.println(temperature);
    Serial.print("Reading humidity: ");
    Serial.println(humidity);
  }

  // turn off Si7021 sensor
  digitalWrite(siPowerPin, HIGH); // inverted
} // end readSi7021

// map two float ranges
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
 return float ( (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
} // end mapfloat
