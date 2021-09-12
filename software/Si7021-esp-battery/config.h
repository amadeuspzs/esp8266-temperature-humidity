// Device name
#define device "device" // will prefix later parameters

// WiFi credentials.
#define wifi_hostname device "-sensor"
#define wifi_ssid "SSID"
#define wifi_password "PASSPHRASE"
// Once you have a static IP, enter it below
IPAddress ip(0,0,0,0);
IPAddress gateway(0,0,0,0);
IPAddress subnet(255,255,255,0);

// Sensor configuration
#define temperatureChangeThreshold 0.08 // repeatability from the data sheet
#define humidityChangeThreshold 0.2 // repeatability from the data sheet
#define adcPowerPin 13 // GPIO13 for ADC
// Ideal ADC and voltage divider behaviour
// is 4.2/1024.0
// For accurate battery voltage measurements
// use an accurate voltage input, measure ADC
// and replace values below (assumes linear response)
#define vbatt_calibration 4.2/1024.0

// Reporting
#define polling_m 10 // every X minutes
#define polling_s polling_m * 60
#define polling_ms polling_s * 1000
#define polling_us polling_ms * 1000
#define mqtt_server "0.0.0.0"
#define humidity_topic device "/humidity"
#define temperature_topic device "/temperature"
#define vbatt_topic device "/vbatt"
