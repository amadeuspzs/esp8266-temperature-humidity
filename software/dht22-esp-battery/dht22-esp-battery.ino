/*
 * ESP-12F with DHT22 (battery-powered)
 *
 * MQTT Temperature and Humidity Sensor
 *
 * Designed for integration with HomeAssistant
 *
 * Only publish a temperature/humidity/vcc change if
 * temperature change outside the repeatability range
 * of the sensor
 *
 */

bool debug = false; // serial output
bool dhcp = false; // whether to obtain an IP address (slower)

#include <DHTesp.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "config.h" // WiFi and device settings

WiFiClient espClient;

PubSubClient client(espClient);

DHTesp dht;

ADC_MODE(ADC_VCC); // for low power detection

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
    float vcc;            // 4 bytes
    uint8_t bssid[6];       // 6 bytes
    uint8_t channel;        // 1 byte
    bool publish;           // 1 byte
    bool publishHumidity; // 1 byte
    bool publishVcc;      // 1 byte
    uint8_t padding[2];   // 2 bytes
  } data;
} rtcData;

bool quickConnect = false; // setting to connect quickly from RTC data
float temperature, humidity;

void setup() {

  // switch off WiFi until we need it
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  delay(1);

  if (debug) {
    Serial.begin(74880);
    while (!Serial) {}
    Serial.setDebugOutput(true);
  }

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
      rtcData.data.publishVcc = true;
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
        Serial.print("RTC Vcc: ");
        Serial.println(rtcData.data.vcc);
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
        Serial.print("RTC publish vcc: ");
        Serial.println(rtcData.data.publishVcc);
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
    rtcData.data.publishVcc = true;
  }

  // we should have RF, and have been asked to publish
  if (rtcData.data.publish) {

    if (debug) {
      Serial.println("Publishing:");
      Serial.print("Temperature: ");
      Serial.println(rtcData.data.publish); // should always be true
      Serial.print("Humidity: ");
      Serial.println(rtcData.data.publishHumidity);
      Serial.print("Vcc: ");
      Serial.println(rtcData.data.publishVcc);
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
    if (debug) Serial.print("Publishing "); Serial.print(String(rtcData.data.temperature).c_str()); Serial.print(" to "); Serial.println(temperature_topic);
    
    client.publish(temperature_topic, String(rtcData.data.temperature).c_str(), true);
    if (rtcData.data.publishHumidity) client.publish(humidity_topic, String(rtcData.data.humidity).c_str(), true);
    if (rtcData.data.publishVcc) client.publish(vcc_topic, String(rtcData.data.vcc).c_str(), true);

    client.disconnect();

    // always store BSSID and channel in case device hops APs
    if (debug) Serial.println("Storing BSSID: ");
    rtcData.data.channel = WiFi.channel();
    memcpy( rtcData.data.bssid, WiFi.BSSID(), 6 ); // Copy 6 bytes of BSSID (AP's MAC address)
    if (debug) {
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
    rtcData.data.publishVcc = false;

    saveMemory();

    if (debug) Serial.println("Publishing complete, going to RF deep sleep");

    ESP.deepSleepInstant(polling_us, WAKE_RF_DISABLED ); // in uS

  } else { // check to see if we need to publish due to temp change

    readDHT22();

    float tempDifference = fabs(temperature - rtcData.data.temperature);
    if (debug) {
      Serial.print("Temperature difference: ");
      Serial.println(tempDifference);
    }

    if (tempDifference > dhtTemperatureChangeThreshold) {
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

      if (humidityDifference > dhtHumidityChangeThreshold) {
        if (debug) Serial.println("Humidity change detected, flagging to publish");
        rtcData.data.publishHumidity = true;
        rtcData.data.humidity = humidity;
      } else {
        if (debug) Serial.println("Skipping humidity publication");
      }

      float vcc = readVcc();
      float vccDifference = fabs(rtcData.data.vcc-vcc);

      if (debug) {
        Serial.print("Vcc difference: ");
        Serial.println(vccDifference);
      }

      // prepare to publish vcc
      if (vccDifference >= 0.01) {
        if (debug) Serial.println("Vcc change detected; will publish");
        rtcData.data.publishVcc = true;
        rtcData.data.vcc = vcc;
      } else {
        if (debug) Serial.println("Skipping Vcc publication");
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
      Serial.print("Vcc: ");
      Serial.println(rtcData.data.vcc);
      Serial.print("Publish: ");
      Serial.println(rtcData.data.publish);
      Serial.print("Publish humidity: ");
      Serial.println(rtcData.data.publishHumidity);
      Serial.print("Publish vcc: ");
      Serial.println(rtcData.data.publishVcc);
    }
  } else {
    if (debug) Serial.println("Failed to write to memory");
  }

} // end saveMemory

// function to read DHT22
void readDHT22() {
  // switch on the DHT22 sensor
  pinMode(dhtPowerPin,OUTPUT);
  digitalWrite(dhtPowerPin, HIGH);

  dht.setup(dhtDataPin, DHTesp::DHT22); // Connect DHT sensor

  // allow DHT22 1s as per data sheet to make reading
  // with debug output true, this can be reduced to 800ms
  // with debug output false, this can be reduced to 800ms
  delay(800);

  // read current temperature, humidity
  temperature = dht.getTemperature();
  humidity = dht.getHumidity();

  // turn off DHT22 sensor
  digitalWrite(dhtPowerPin, LOW);

  if (debug) {
    Serial.print("Reading temperature: ");
    Serial.println(temperature);
    Serial.print("Reading humidity: ");
    Serial.println(humidity);
  }
} // end readDHT22

// read Vcc
float readVcc() {
    float vcc = ESP.getVcc() / 1000.0;
    if (debug) {
      Serial.print("Reading Vcc: ");
      Serial.println(vcc);
    }
    return vcc;
} // end readVcc

// read all sensors and store in rtcData
void readAllSensors() {
    readDHT22();
    // store in RTC variables
    rtcData.data.temperature = temperature;
    rtcData.data.humidity = humidity;
    rtcData.data.vcc = readVcc();
}
