# ADC calibration

The ESP12F (esp8266) ADC has a built-in voltage reference of ~1.0V.

The circuit contains a voltage divider to convert a maximum 4.2V input (LiPo max voltage) to 1.0V.

The resulting measurement is used to report battery voltage via MQTT.

Both the ADC voltage reference and voltage divider values will vary from component to component.

The stock code assumes an ideal value for both:

```arduino
// Ideal ADC and voltage divider behaviour
// is 4.2/1024.0
// For accurate battery voltage measurements
// use an accurate voltage input, measure ADC
// and replace values below (assumes linear response)
#define vbatt_calibration 4.2/1024.0
```

ie. for a 4.2V input, an ADC reading of 1024.

In reality, you will want to use a known voltage, measure the ADC value and update `vbatt_calibration` to match.
