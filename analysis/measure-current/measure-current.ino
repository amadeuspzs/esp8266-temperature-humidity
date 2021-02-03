#include <Adafruit_INA260.h>

Adafruit_INA260 ina260 = Adafruit_INA260();

unsigned long timestamp;

void setup() {
  Serial.begin(115200);
  // Wait until serial port is opened
  while (!Serial) { }
  Serial.println("Looking for INA260 chip");
  if (!ina260.begin()) {
    Serial.println("Couldn't find INA260 chip");
    while (1);
  }
  Serial.println("Found INA260 chip");
  Serial.println("timestamp,current,voltage");
}

void loop() {
  Serial.print(millis());
  Serial.print(",");
  Serial.print(ina260.readCurrent());
  Serial.print(",");
  Serial.print(ina260.readBusVoltage());
  Serial.println();
}
