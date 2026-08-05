# ESP32-S3-Super-Mini-Test

PlatformIO template for a random "ESP32-S3 Super Mini" product based on the ESP32-S3FH4R2 I found. (4MB flash, 2MB PSRAM) Prints all system info and flashes the built-in WS2818.

## Notes

* On some boards you may need to replug USB / upload while holding the boot button to get it into bootloader mode / 
  upload successfully.
* Put your loop code in `while (true) {}` in the setup function and leave the loop function blank. Using the loop 
  function crashes the chip. 
* There is a regular LED and a WS2818 GRB LED on the board. Both are on GPIO 48, so use `digitalWrite`, etc. to use the 
  regular LED and `FastLED` to use the WS2818 GRB LED.
* Change `monitor_speed` if desired. (default is baud rate of 115200)
