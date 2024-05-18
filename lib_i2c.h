/******************************************************************************
* Simplified CH32V000x I2C Library.
* Only supports 8-Bit addresses and 8-Bit registers. 
*
* Has simple to use functions for detection, reading and writing of I2C Devices
* SCL = PC2 
* SDA = PC1
*
* See original GitHub Repo here: https://github.com/ADBeta/CH32V000x-lib_i2c
*
* ADBeta (c) 2024
******************************************************************************/
#ifndef CH32_LIB_I2C_H
#define CH32_LIB_I2C_H

#include "ch32v003fun.h"

/*** Hardware Definitions ****************************************************/
// Common Clock Speeds
#define I2C_CLK_400KHZ 400000
#define I2C_CLK_100KHZ 100000

// Hardware CLK Prerate and timeout
#define I2C_PRERATE 2000000
#define I2C_TIMEOUT 2000

// Default PORT Configuration
#define I2C_PORT_RCC	RCC_APB2Periph_GPIOC
#define I2C_PORT		GPIOC 
#define I2C_PIN_SCL 	2
#define I2C_PIN_SDA 	1

// Error Code Definitons
typedef enum {
	I2C_OK           = 0,
	I2C_ERR_TIMEOUT  = 1,
	I2C_ERR_NACK     = 2,
	I2C_ERR_NO_DATA  = 3,

} i2c_err_t;

/*** Static Functions ********************************************************/
/// @breif Checks the I2C Status against a mask value, returns 1 if it matches
/// @param Status To match to
/// @return uint32_t masked status value - 1 if mask and status match
static inline uint32_t i2c_status(const uint32_t status_mask)
{
	uint32_t status = (uint32_t)I2C1->STAR1 | (uint32_t)(I2C1->STAR2 << 16);
	return (status & status_mask) == status_mask; 
}


/*** Functions ***************************************************************/
/// @breif Initialise the I2C Peripheral on the default pins, in Master Mode
/// @param clk_rate that the I2C Bus should use in Hz. Max 400000
/// @return i2c_err_t - I2C_OK On success
i2c_err_t i2c_init(const uint32_t clk_rate);

/// @breif Pings a specific I2C Address, and returns a i2c_err_t status
/// @param addr I2C Device Address, MUST BE 7Bit
/// @return i2c_err_t, I2C_OK if the device responds, 
/// I2C_ERR_TIMOUT or I2C_ERR_NO_ACK if not
i2c_err_t i2c_ping(const uint8_t addr);

/// @breif Scans through all 7bit addresses, prints any that respond
/// @param None
/// @return None
void i2c_scan(void);

/// @breif reads [len] bytes from [addr]s [reg] register into [buf]
/// @param addr, address of I2C Device to Read from
/// @param buf, buffer to read to
/// @param len, number of bytes to read
/// @return 12c_err_t. I2C_OK on Success, TIMEOUT or NO_DATA on error
i2c_err_t i2c_read(const uint8_t addr,         const uint8_t reg,
				                                     uint8_t *buf,
                                               const uint8_t len);

/// @breif writes [len] bytes from [buf], to the [reg] of [addr]
/// @param addr, Address of the I2C Device to Write to
/// @param buf, Buffer to write from
/// @param len, number of bytes to read
/// @return i2c_err_t. I2C_OK On Success. TIMEOUT or TODO:
i2c_err_t i2c_write(const uint8_t addr,        const uint8_t reg,
					                           const uint8_t *buf,
                                               const uint8_t len);

#endif
