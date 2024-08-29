/******************************************************************************
* Basic Example of using lib_i2c on the CH32V003 Microcontroller
*
* Demo Version 1.0.0    25 Aug 2024
* See GitHub Repo for more information: 
* https://github.com/ADBeta/CH32V000x-lib_i2c
*
* Released under the MIT Licence
* Copyright ADBeta (c) 2024
******************************************************************************/
#include "ch32v003fun.h"
#include "lib_i2c.h"

#include <stdio.h>

// I2C Scan Callback example function. Prints the address which responded
void i2c_scan_callback(const uint8_t addr)
{
	printf("Address: 0x%02X Responded.\n", addr);
}


int main() 
{
	SystemInit();
	
	// Initialise the I2C Interface on the selected pins, at the specified Hz.
	// Enter a clock speed in Hz (Weirdness happens below 10,000), or use one
	// of the pre-defined clock speeds:
	// I2C_CLK_10KHZ
	// I2C_CLK_50KHZ
	// I2C_CLK_100KHZ
	// I2C_CLK_400KHZ
	if(i2c_init(I2C_CLK_400KHZ) != I2C_OK) printf("Failed to init the I2C Bus\n");

	// Scan the I2C Bus, prints any devices that respond
	printf("----Scanning I2C Bus for Devices---\n");
	i2c_scan(i2c_scan_callback);
	printf("----Done Scanning----\n\n");

	/*** Example ***/
	// This example is specifically for the DS3231 I2C RTC Module.
	// Use this as an example for generic devices, changing Address, speed etc
	i2c_err_t i2c_stat;

	// Write to the -Seconds- Register (Reg 0x00, 0x00 Seconds, one byte)
	i2c_stat = i2c_write(0x68, 0x00, (uint8_t[]){0x00}, 1);
	if(i2c_stat != I2C_OK) { printf("Error Using the I2C Bus\n"); return -1; }

	// Example of writing an array to a register.
	uint8_t array[3] = {0x00, 0x01, 0x02};
	i2c_stat = i2c_write(0x68, 0x00, array, 3);
	if(i2c_stat != I2C_OK) { printf("Error Using the I2C Bus\n"); return -1; }

	// Example to read from the I2C Device
	uint8_t seconds = 0;    // Just Seconds (Read as Hex instead od Decimal)
	uint8_t time[3] = {0};  // Time in Sec, Min, Hrs (Hex not Decimal)
	
	// Loop forever
	while(1)
	{
		// Example reading just one byte
		i2c_stat = i2c_read(0x68, 0x00, &seconds, 1);
		if(i2c_stat != I2C_OK) printf("Error Using the I2C Bus\n");
		// Print Seconds as a single hex byte
		printf("Seconds: %02X\n", seconds);

		
		// Example reading multiple bytes
		i2c_stat = i2c_read(0x68, 0x00, time, 3);
		if(i2c_stat != I2C_OK) printf("Error Using the I2C Bus\n");
		// Print Time as Hrs Min Sec
		printf("Time: %02X:%02X:%02X\n\n", time[2], time[1], time[0]);


		// Wait 1 Second
		Delay_Ms(1000);
	}

	return 0;
}
