# romonit

# ft232r_prog-1.24
Is a tool to reflash the product id of the romonit FDTI-cables. 
To use it you need to compile it and rewrite the eeprom on the FTDI-chip.

- aptitude install libftdi-dev
- make
- sudo ./ft232r_prog --old-vid 0x0403 --old-pid 0xbaf9 --new-pid 0x6001

After that, it's a normal USB-serial cable

# fw-mm
Contains a small firmware for romonit devices, which blinks the LED once 
and puts the AVR in powerdown mode. Current consumption is below 10uA @3.3V.

