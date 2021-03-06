# MaxLedController(with Android App)
PlatformiO project for controlling adressable leds (WS2812b, WS2813 and others) from Android. It was created for Esp8266 and after uploading it to Esp8266, you can manage your leds from you smartphone.
It uses NeoPixelBus library (https://github.com/Makuna/NeoPixelBus) to drive leds. 
It gives you possibility to set any color to your leds, create some effects, store them to palettes, create playlist from palettes. Also you can have some Esp8266 on wifi network and you can control them from app separately or all together.

Features:
 - up to 500 leds, connected to microchip ESP8266
 - up to 20 microchips ESP8266, connected to application via WiFI
 - change strip color
 - create simple effects
 - store color combinations and effects in application for further reuse
 - recording playlist of stored scenes into microchip ESP8266 for playing back without application
   
After first start, it creates WiFi access point(also if it cannot connect ot your existing Wifi access point). After connecting to it you can change some settings:
 - name
 - led count
 - network mode (Esp8266 creates own Access Point or connects to your existing access point)
 - your SSID
 - your PASSWORD
 
 Default access point settings:
  - ssid: MaxLedNet
  - password: 11223344
  
  Use GPIO3 (RX) pin for connecting leds.
  
  Video, which demonstrates look and possibilities of mobileApp: https://youtu.be/RkItq3M3n-4
  
  Android App for controlling: https://play.google.com/store/apps/details?id=alfarom.apps.ledcontroller
  
  Application is created with Flutter, so it can be installed on iOs devices also. I can share project and some instructions, so you can install application on your iOs device manually(nedded real macOS device or with the help of virtual machine)
  
  1) Get project
     - download project, import it to VSCode with installed PlatformiO plugin, connect your Esp8266 with usb, flash
     - download allready compiled binary file(firmware.bin in the root directory of project) and upload it to your Esp8266 with help of:
       - esptool.py (https://github.com/espressif/esptool). Some info: https://nodemcu.readthedocs.io/en/latest/flash/
       
         my example command for Nodemcu: esptool.py --port COM6 write_flash -fm qio 0x00000 c:/firmware.bin (change COM6 to your port, with connected NodeMcu)
     
     During reboot first led of strip should blink three times in blue color. 
  2) Get Android app from PlayMarket: https://play.google.com/store/apps/details?id=alfarom.apps.ledcontroller
  3) Connect to network with name, starting with "MaxLedNet" with password: "11223344"
  4) Start Application, press SCAN button. In bottom part of screen should be a list of detected Esp8266s with uploaded current project.
  
  NOTES:
  - 500 leds is the maximum available count at current version
  - Effect section is in experimental mode, so its behaviour can be not very clear
  - connecting via router/access point is more stable than direct connection to Esp8266
  - closing and reopening application and power reset of Esp8266 can help when something is going wrong)
  
  
