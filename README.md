# CH32V00x_lib_i2c          Ver 5.5

## Library Description
`lib_i2c` is a fully featured, but lightweight library for the I2C peripheral
on the CH32V003 with the following features:
* Support for All 3 Alternative Pinouts
* Support 7-bit Addresses (7-bit aligned, eg `0bx1101000 - 0x68`)
* Support 8-bit Registers
* Up to 1MHz Bus Frequency has been tested. Can be set higher.
* Easy to use I2C Error Status'
* Funcion to Scan the Interface for devices
* Master Mode Only

## Links
[CNLohr's ch32fun](https://github.com/cnlohr/ch32fun)
[PlatformIO Template](https://github.com/mattybigback/CH32V003_lib_i2c)

## Usage

Copy the content of the github repository under the ``ch32fun/examples`` subdirectory
and ``make`` to compile from ``ch32fun/examples\CH32V003_lib_i2c``.

## Changelog
* `v5.5` - Adapted funconfig.h to modern ch32fun. Slight change to demo. Tested on CH32V006
* `v5.4` - Added a `read_reg_delay()` function for when clock-stretching is inadequate
* `v5.3` - Added an easier user-definable clock-stretch timout variable
* `v5.2` - Fixed a define bug in the header - Testing complete of multi-byte registers
* `v5.1` - Refactored a few things to be more efficient, added support for 10Bit
           Addresses
* `v5.0` - Added Multi-Byte Register support (up to 4 bytes) with config in i2c_device_t  
           Changed init() to use the device pointer, multiple devices can still be used at once  
           Added read & write raw functions - without use of registers
* `v4.3` - Began refactor to allow 7bit, 10bit and 16bit addresses. Only supports 7bit currently
* `v4.2` - Protected most loops with timeout and error propegation
* `v4.1` - Added i2c_wait to simplify waiting for bus to be free
* `v4.0` - Refactored using inlined functions for expandability (i2c_start(), i2c_stop() etc)
* `v3.5` - Converted from ch32v003fun to ch32fun
* `v3.4` - Fixed an off-by-one error in i2c_read


## TODO
* Neaten library so there are less repeated chunks of code
* Test on other MCU Variants:
	* CH32V003 ✔️
	* CH32V006 ✔️


## Thanks
Thank you [niansa](https://github.com/niansa) for continued help refining the library  
Thank you [Johannes JKT](https://github.com/DeadBugEngineering) for fixing an i2c_read off_by_one bug  

----
Copyright (c) 2024 - 2025 ADBeta
