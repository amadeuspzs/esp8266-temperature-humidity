// Device name
#define device "device" // will prefix later parameters

// WiFi credentials.
#define wifi_hostname device "-sensor"
#define wifi_ssid "SSID"
#define wifi_password "PASSWORD"

// Sensor configuration
#define temperatureChangeThreshold 0.08 // repeatability from the data sheet
#define humidityChangeThreshold 0.2 // repeatability from the data sheet

// Reporting
#define polling_m 1 // every X minutes
#define polling_s polling_m * 60
#define polling_ms polling_s * 1000
#define mqtt_server "0.0.0.0"
#define humidity_topic device "/humidity"
#define temperature_topic device "/temperature"
