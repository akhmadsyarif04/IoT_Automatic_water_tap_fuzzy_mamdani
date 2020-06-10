# IoT_Automatic_water_tap_fuzzy_mamdani

This is a system control and monitoring of temperature and humidity in the swallow nest using fuzzy Mamdani


DHT11 :
1. Vcc/+ -> 3,3V Arduino Wemos / -3dig Arduino
2. Data/out -> D5 Arduino Wemos / 2dig Arduino
3. GND/- -> GND Arduino Wemos / GND Arduino

LDR : 
1. GND -> GND Arduino Wemos / 3,3v Arduino
2. 5V -> 5V Arduino Wemos / GND Arduino
3. AO -> AO Arduino Wemos / AO analog Arduino

Servo :
1. yellow wire -> D4 Arduino Wemos / 9 Arduino
2. red wire -> 3V3 pada Arduino Wemos / 5v Arduino
3. brown wire -> GND Arduino Wemos / GND Arduino

LCD :
1. GND LCD -> GND Arduino Wemos / GND Arduino
2. VCC LCD -> 3V3 Arduino Wemos / 5v Arduino
3. SDA LCD -> SDA D14 Arduino Wemos / A4 input analog Arduino
4. SCL LCD -> SCL D15 Arduino Wemos / A5 input analog Arduino

This is prototipe Arduino Wemos :
![rancangan_arduino_wemos.jpg]({{site.baseurl}}/rancangan_arduino_wemos.jpg)

This is monitoring on lcd :
![monitoring_on_lcd.jpg]({{site.baseurl}}/monitoring_on_lcd.jpg)

This is microcontroller :
![alat_microcontroller.jpg]({{site.baseurl}}/alat_microcontroller.jpg)

This is monitoring on website :
![monitoring_on_web.png]({{site.baseurl}}/monitoring_on_web.png)
