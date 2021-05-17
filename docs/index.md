## Features

* 9 months+ runtime on a 1200mAh Li-Ion battery
* Highly accurate Si7021 temperature and humidity sensor
* Physical case with low impact on temperature (<0.05&deg;C)
* MQTT integration with e.g. Home Assistant directly over WiFi
* PH2, PH3 and GH2 connectors for range of Li-Ion batteries
* Programming header and jumper for quick flashing
* Battery voltage measurement for power management
* [Technical details available on Github](https://github.com/amadeuspzs/esp8266-temperature-humidity/)

## PCB

* No through-hole components for easy stacking on top of Li-Ion battery
* Two-layer, 50 mm x 34 mm
* [KiCad files](https://github.com/amadeuspzs/esp8266-temperature-humidity/tree/master/hardware/esp12f-si7021/)
* [Gerber files](https://github.com/amadeuspzs/esp8266-temperature-humidity/tree/master/hardware/esp12f-si7021/gerber)
* [Partial BoM](https://github.com/amadeuspzs/esp8266-temperature-humidity/tree/master/hardware/esp12f-si7021/esp12f-si7021_bom.csv)

## Software

* [Arduino files](https://github.com/amadeuspzs/esp8266-temperature-humidity/tree/master/software/Si7021-esp-battery)
* Update `config.h` with your settings
* Fine-tune `vref` for accurate battery voltage measurements

## Case

* 3D printed case: [STL](https://github.com/amadeuspzs/esp8266-temperature-humidity/tree/master/hardware/case_with_lid.stl), [SCAD](https://github.com/amadeuspzs/esp8266-temperature-humidity/tree/master/hardware/case_with_lid.scad)

![Render of 3D printed case2](https://user-images.githubusercontent.com/534681/118548141-dc3ab100-b751-11eb-9fb6-8693e208d46b.png)

![Photo of 3D printed case1](https://user-images.githubusercontent.com/534681/118548034-b8776b00-b751-11eb-959d-c519ca402267.png)

* Analysis shows latest case design (`case2`) only impacts temperature readings by `0.05`&degC;, less than the sensor repeatability:

![Analysis of case impact](https://user-images.githubusercontent.com/534681/118548343-199f3e80-b752-11eb-8731-24e7569f7ab5.png)
