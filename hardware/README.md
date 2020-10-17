# Hardware

## Battery powered

Considerations:

1. SMD design
2. UART access for flashing
3. Download jumper for flashing
4. Battery powered
5. DHT22 power controlled over GPIO

### Schematic

![schematic](https://user-images.githubusercontent.com/534681/81606445-059c6080-93cb-11ea-8b18-0f8d1f223274.png)

### Batteries

Device|Min voltage (V)|Stated voltage (V)|Max voltage (V)
---|---|---|---
[ESP-12F](http://wiki.ai-thinker.com/_media/esp8266/a014ps01.pdf)|3.0|3.3|3.6
Li-Ion Cell|2.8|3.7|4.2
2 X AA Alkaline|2.8|3|3.3

Two options:

1. Regulate the Li-Ion voltage to 3.3V using e.g. [MCP1700](https://www.microchip.com/wwwproducts/en/MCP1700)
2. Run off alkaline batteries without regulator

### KiCad

- [X] Schematic
- [ ] PCB

## Mains powered

Features:

* USB powered via NodeMCU board

![diagram](nodemcu-mains.png)