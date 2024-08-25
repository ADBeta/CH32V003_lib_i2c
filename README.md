# CH32V003_lib_i2c
**This is a simplified library for I2C, used with [ch32v003fun](https://github.com/cnlohr/ch32v003fun)**

## Library Description
`lib_i2c` is a fully featured, but lightweight library for the I2C peripheral
on the CH32V003 with the following features:
* Support for All 3 Alternative Pinouts
* Support 7-bit Addresses (7-bit aligned, eg `0bx1101000 - 0x68`)
* Support 8-bit Registers
* Easy to use I2C Error Status'
* Funcion to Scan the Interface for devices
* Master Mode Only

## TODO
* Test on other MCU Variants:
	* CH32V003 ✔️

If you want to use ANY pins for I2C, see [lib_swi2c](https://github.com/ADBeta/CH32Vxxx_lib_swi2c)

----
MIT License
Copyright (c) 2024 ADBeta
