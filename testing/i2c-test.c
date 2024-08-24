/******************************************************************************
*
*
*
*
*
******************************************************************************/
#include "ch32v003fun.h"
#include "lib_i2c.h"

#include <stdio.h>

int main() 
{
	SystemInit();

	Delay_Ms(1000);
	
	// Initialise the I2C Interface on the selected pins, at the specified Hz.
	// I2C_CLK_400KHZ and I2C_CLK_100KHZ are predefined values, 
	// 400000 or 100000 will work equally.
	i2c_init(I2C_CLK_100KHZ);

	// Scan the I2C Bus, prints any devices that respond
	//i2c_scan();










	uint8_t secs = 0;

	i2c_write(0x68, 0x00, (uint8_t[]){0x18}, 1);
	while(1)
	{
		Delay_Ms(100);
		i2c_read(0x68, 0x00, &secs, 1);
		printf("secs: %2X\n", secs);
	}

	return 0;
}
