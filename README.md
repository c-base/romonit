# romonit

We got these thingies from:
https://web.archive.org/web/20100121014842/http://www.romonit.de/index.php?id=7

Some more info is here:
http://www.conrad.de/ce/de/product/108020/Romonit-econom-climat-Datenlogger-Messschreiber-5-bis-35-C

# ft232r_prog-1.24
Is a tool from Mark Lord to reflash the product id of the romonit FDTI-cables.
To use it you need to compile it and rewrite the eeprom on the FTDI-chip.

http://rtr.ca/ft232r/

- sudo aptitude install libftdi-dev
- make
- sudo ./ft232r_prog --old-vid 0x0403 --old-pid 0xbaf9 --new-pid 0x6001

After that, it's a normal USB-serial cable

# fw-mm
Contains a small firmware for romonit devices to test the components.
What works:
- LCD
- 32kHz timer
- LED
- Button
- Battery gauge
- Temperatur and humidity display

Current consumtion is 10uA for the LCD, 5uA (average) for measuring humidity
and temperature every 8s and about 2uA for processing on the AVR.

