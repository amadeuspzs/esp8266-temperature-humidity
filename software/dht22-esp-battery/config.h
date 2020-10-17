// Device name
#define device "device" // will prefix later parameters

// WiFi credentials.
#define wifi_hostname device "-sensor"
#define wifi_ssid "SSID"
#define wifi_password "PASSWORD"
IPAddress ip(0,0,0,0);
IPAddress gateway(0,0,0,0);
IPAddress subnet(0,0,0,0);

// Sensor configuration
#define dhtDataPin 14 // GPIO14 / D5
#define dhtPowerPin 12 // GPIO12 / D6
#define dhtTemperatureChangeThreshold 0.2 // repeatability from the data sheet
#define dhtHumidityChangeThreshold 1 // repeatability from the data sheet

// Reporting
#define polling_m 10 // every X minutes
#define polling_s polling_m * 60
#define polling_ms polling_s * 1000
#define polling_us polling_ms * 1000
#define mqtt_server "0.0.0.0"
#define humidity_topic device "/humidity"
#define temperature_topic device "/temperature"
#define vcc_topic device "/vcc"
