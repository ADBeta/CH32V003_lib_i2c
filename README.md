# CH32Vxxx_lib_i2c
**This is a simplified library for I2C, used with [ch32v003_fun](https://github.com/cnlohr/ch32v003fun)**

## Library Description
Based on an [example library](https://github.com/hexeguitar/ch32v003fun_libs/blob/main/ch32v003_i2c.h),
but with better comments, better error codes and simpler implimentation.  

This Library provides an architecture native, peripheral I2C interface. It has
the following features:  
* Support 8-bit Addresses (8-bit aligned, eg `0b1101000x - 0xD0`)
* Support 8-bit Registers
* Easily used I2C Error Status Returns
* Scan Interface for devices
* Master Mode Only

## TODO
* Test on other MCU Variants:
	* CH32V003 ✔️


If you want to use ANY pins for I2C, see [lib_swi2c](https://github.com/ADBeta/CH32Vxxx_lib_swi2c)

----
ADBeta (c) 2024
