## ESP8266 MQTT Sharp-fu-y30 air purifier controller

### Disclaimer
Note: if you decide to do those changes to your air purifier, you take all the responsibility for doing so with all it's consequences such as loosing warranty for your device or even destroying it! 

### What you need
* ESP8266 WiFi controller (for example Wemos D1 mini)
* 4 x PC817-B Sharp optocouplers 
* 4 x 270 ohms resistors
* 1 x 10 uF electrolytic capacitor
* some dupont jumper wiring cables
* some soldering skills

To integrate the ESP8266 with Sharp FU-Y30 air purifier you need to access both of it's PCBs. One is the mainboard and it is at the bottom of the device closed in the black plastic box. 

![Mainboard](./doc/sharp-fu-y30-mainboard-front.png)

Second board (on the upper side of the device) contains only some buttons and LEDs, but you will need to solder some wires there as well. 

![Control panel](./doc/sharp-fu-y30-control-panel-front.png)

Both boards are labeled as QPWBFA085KKZZ with additional CPWB-A274KKKZ number.

### The wiring
You need to connect your ESP to the points marked as circles on the Sharp's PCBs.

![The wiring](./doc/sharp-fu-y30-esp8266-wiring.png)

Note the places you need to connect on the Sharp's PCBs:

![Mainboard](./doc/sharp-fu-y30-mainboard.png)

When you connect to the Q45, Q46 and Q47, you need to connect to base (B) pins of those transistors.

![Control panel](./doc/sharp-fu-y30-control-panel.png)

Be sure to properly connect the optocouplers with switch buttons on the control panels (note the polarity).

## Software
You need to create the `config.h` file by copying the `config.example.h`. Enter your WiFi data there, also include the MQTT server addres. You can use some of public MQTT servers with no problem.

After you deploy your code to the ESP8266, you will be able to perform OTA updates. Just uncomment the `espota` upload protocol and change the ESP8266 IP address in the `platformio.ini` file.

## Demo
See the demo of how you can control you purifier using `mosquito` command line client:

[![Demo](https://img.youtube.com/vi/q9jJ4eS9DLY/0.jpg)](https://www.youtube.com/watch?v=q9jJ4eS9DLY)

## Home Assistant integrations
This firmware is able to integrate with Home Assistant out of the box if you have enabled the MQTT extension.

