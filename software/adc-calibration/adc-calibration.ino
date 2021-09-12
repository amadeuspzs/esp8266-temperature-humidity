#define adcPowerPin 13 // GPIO13 for ADC
// Ideal ADC and voltage divider behaviour
// is 4.2/1024.0
// For accurate battery voltage measurements
// use an accurate voltage input, measure ADC
// and replace values below (assumes linear response)
#define vbatt_calibration 4.2/1024.0

void setup() {
  // put your setup code here, to run once:
  Serial.begin(74880);
  pinMode(adcPowerPin, OUTPUT);
  digitalWrite(adcPowerPin, LOW); // inverted
}

void loop() {
  // put your main code here, to run repeatedly:
  readVbatt();
  delay(1000);
}

float readVbatt() {

    // throw away first reading
    int adc = analogRead(A0);
    adc = analogRead(A0);
    
    float vbatt = vbatt_calibration * adc;
    
    Serial.print("adc: ");
    Serial.print(adc,3);
    Serial.print("\nvbatt: ");
    Serial.print(vbatt,3);
    Serial.print("\n");

    return vbatt;
} // end readVbatt
