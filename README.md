## ESP8266 MQTT Sharp-fu-y30 air purifier controller

### What you need
* ESP8266 WiFi controller (for example Wemos D1 mini)
* 4 x PC817-B Sharp optocouplers 
* 4 x 270 ohms resistors
* 1 x 10 uF electrolytic capacitor
* some dupont jumper wiring cables
* some soldering skills

To integrate the ESP8266 with Sharp FU-Y30 air purifier you need to both of it's PCBs. One is the mainboard and it is ad the bottom of the device. 

![Mainboard](./doc/sharp-fu-y30-mainboard-front.png)

Second board (on the upper side of the device) contains only some buttons and LEDs, but we will need to solder some wires there as well. 

![Control panel](./doc/sharp-fu-y30-control-panel-front.png)

Both boards are labeled as QPWBFA085KKZZ with additional CPWB-A274KKKZ number.

### The wiring
You need to connect your ESP to the points marked as circles on the Sharp's PCBs.

![The wiring](./doc/sharp-fu-y30-esp8266-wiring.png)

Note the places you need to connect on the Sharp's PCBs:

![Mainboard](./doc/sharp-fu-y30-mainboard.png)

When you connect to the Q45, Q46 and Q47, you need to connect to base (B) pins of those transistors.

![Control panel](./doc/sharp-fu-y30-control-panel.png)

