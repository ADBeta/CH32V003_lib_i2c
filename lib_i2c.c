/******************************************************************************
* Lightweight and simple CH32V0003 I2C Library.
*
* This library provides functions to init, read and write to the hardware I2C
* Bus - in Default, and Alternative Pinout Modes.
* Default:	SCL = PC2		SDA = PC1
* Alt 1:	SCL = PD1		SDA = PD0
* Alt 2:	SCL = PC5		SDA = PC6
*
* See GitHub Repo for more information: 
* https://github.com/ADBeta/CH32V000x-lib_i2c
*
* Released under the MIT Licence
* Copyright ADBeta (c) 2024
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to
* deal in the Software without restriction, including without limitation the 
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
* sell copies of the Software, and to permit persons to whom the Software is 
* furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in 
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
* OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE 
* USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/
#include "lib_i2c.h"
#include <stddef.h>

/*** Static Functions ********************************************************/
/// @brief Checks the I2C Status against a mask value, returns 1 if it matches
/// @param Status To match to
/// @return uint32_t masked status value: 1 if mask and status match
__attribute__((always_inline))
static inline uint32_t i2c_status(const uint32_t status_mask)
{
	uint32_t status = (uint32_t)I2C1->STAR1 | (uint32_t)(I2C1->STAR2 << 16);
	return (status & status_mask) == status_mask; 
}

/// @brief Gets and returns any error state on the I2C Interface, and resets
/// the bit flags
/// @param none
/// @return i2c_err_t error value
__attribute__((always_inline))
static inline i2c_err_t i2c_error(void)
{
	// BERR
	if(I2C1->STAR1 & I2C_STAR1_BERR) {I2C1->STAR1 &= ~I2C_STAR1_BERR; return I2C_ERR_BERR;}
	// NACK
	if(I2C1->STAR1 & I2C_STAR1_AF) {I2C1->STAR1 &= ~I2C_STAR1_AF; return I2C_ERR_NACK;}
	// ARLO
	if(I2C1->STAR1 & I2C_STAR1_ARLO) {I2C1->STAR1 &= ~I2C_STAR1_ARLO; return I2C_ERR_ARLO;}
	// OVR
	if(I2C1->STAR1 & I2C_STAR1_OVR) {I2C1->STAR1 &= ~I2C_STAR1_OVR; return I2C_ERR_OVR;}

	return I2C_OK;
}

/// @brief Checks the current I2C Status, if it does not have an error state,
/// it defaults to I2C_ERR_BUSY
/// @param None
/// @return i2c_err_t error value
__attribute__((always_inline))
static inline uint32_t i2c_get_busy_error(void)
{
	i2c_err_t i2c_err = i2c_error();
	if(i2c_err == I2C_OK) i2c_err = I2C_ERR_BUSY;
	return i2c_err;
}



/*** API Functions ***********************************************************/
i2c_err_t i2c_init(uint32_t clk_rate)
{
	// Toggle the I2C Reset bit to init Registers
	RCC->APB1PRSTR |=  RCC_APB1Periph_I2C1;
	RCC->APB1PRSTR &= ~RCC_APB1Periph_I2C1;

	// Enable the I2C Peripheral Clock
	RCC->APB1PCENR |= RCC_APB1Periph_I2C1;

	// Enable the selected I2C Port, and the Alternate Function enable bit
	RCC->APB2PCENR |= I2C_PORT_RCC | RCC_APB2Periph_AFIO;

	// Reset the AFIO_PCFR1 register, then set it up
	AFIO->PCFR1 &= ~(0x04400002);
	AFIO->PCFR1 |= I2C_AFIO_REG;

	// Clear, then set the GPIO Settings for SCL and SDA, on the selected port
	I2C_PORT->CFGLR &= ~(0x0F << (4 * I2C_PIN_SDA));
	I2C_PORT->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_OD_AF) << (4 * I2C_PIN_SDA);	
	I2C_PORT->CFGLR &= ~(0x0F << (4 * I2C_PIN_SCL));
	I2C_PORT->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_OD_AF) << (4 * I2C_PIN_SCL);

	// Set the Prerate frequency
	uint16_t i2c_conf = I2C1->CTLR2 & ~I2C_CTLR2_FREQ;
	i2c_conf |= (FUNCONF_SYSTEM_CORE_CLOCK / I2C_PRERATE) & I2C_CTLR2_FREQ;
	I2C1->CTLR2 = i2c_conf;

	// Set I2C Clock
	if(clk_rate <= 100000)
	{
		i2c_conf = (FUNCONF_SYSTEM_CORE_CLOCK / (2 * clk_rate)) & I2C_CKCFGR_CCR;
	} else {
		// Fast mode. Default to 33% Duty Cycle
		i2c_conf = (FUNCONF_SYSTEM_CORE_CLOCK / (3 * clk_rate)) & I2C_CKCFGR_CCR;
		i2c_conf |= I2C_CKCFGR_FS;
	}
	I2C1->CKCFGR = i2c_conf;

	// Enable the I2C Peripheral
	I2C1->CTLR1 |= I2C_CTLR1_PE;

	//TODO:
	// Check error states
	if(I2C1->STAR1 & I2C_STAR1_BERR) 
	{
		I2C1->STAR1 &= ~(I2C_STAR1_BERR); 
		return I2C_ERR_BERR;
	}

	return I2C_OK;
}


i2c_err_t i2c_ping(const uint8_t addr)
{
	i2c_err_t i2c_ret = I2C_OK;

	// Wait for the bus to become not busy - return I2C_ERR_TIMEOUT on failure
	int32_t timeout = I2C_TIMEOUT;
	while(I2C1->STAR2 & I2C_STAR2_BUSY) 
		if(--timeout < 0) i2c_ret = I2C_ERR_BUSY;

	if(i2c_ret == I2C_OK)
	{
		// Send a START Signal and wait for it to assert
		I2C1->CTLR1 |= I2C_CTLR1_START;
		while(!i2c_status(I2C_EVENT_MASTER_MODE_SELECT));

		// Send the Address and wait for it to finish transmitting
		timeout = I2C_TIMEOUT;
		I2C1->DATAR = (addr << 1) & 0xFE;
		// If the device times out, get the error status - if status is okay,
		// return generic I2C_ERR_BUSY Flag
		while(!i2c_status(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
			if(--timeout < 0) {i2c_ret = i2c_get_busy_error(); break;}
	}

	// Send the STOP Signal, return i2c status
	I2C1->CTLR1 |= I2C_CTLR1_STOP;
	return i2c_ret;
}


void i2c_scan(void (*callback)(const uint8_t))
{
	// If the callback function is null, exit
	if(callback == NULL) return;

	// Scan through every address, getting a ping() response
	for(uint8_t addr = 0x00; addr < 0x7F; addr++)
	{
		// If the address responds, call the callback function
		if(i2c_ping(addr) == I2C_OK) callback(addr);
	}
}


i2c_err_t i2c_read(const uint8_t addr,		const uint8_t reg,
											uint8_t *buf,
											const uint8_t len)
{
	i2c_err_t i2c_ret = I2C_OK;

	// Wait for the bus to become not busy - set state to I2C_ERR_TIMEOUT on failure
	int32_t timeout = I2C_TIMEOUT;
	while(I2C1->STAR2 & I2C_STAR2_BUSY) 
		if(--timeout < 0) {i2c_ret = i2c_get_busy_error(); break;}
	
	if(i2c_ret == I2C_OK)
	{
		// Send a START Signal and wait for it to assert
		I2C1->CTLR1 |= I2C_CTLR1_START;
		while(!i2c_status(I2C_EVENT_MASTER_MODE_SELECT));

		// Send the Address and wait for it to finish transmitting
		timeout = I2C_TIMEOUT;
		I2C1->DATAR = (addr << 1) & 0xFE;
		while(!i2c_status(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
			if(--timeout < 0) {i2c_ret = i2c_get_busy_error(); break;}
	}

	if(i2c_ret == I2C_OK)
	{
		// Send the Register Byte
		I2C1->DATAR = reg;
		while(!(I2C1->STAR1 & I2C_STAR1_TXE));

		// If the message is long enough, enable ACK messages
		if(len > 1) I2C1->CTLR1 |= I2C_CTLR1_ACK;

		// Send a Repeated START Signal and wait for it to assert
		I2C1->CTLR1 |= I2C_CTLR1_START;
		while(!i2c_status(I2C_EVENT_MASTER_MODE_SELECT));

		// Send Read Address
		timeout = I2C_TIMEOUT;
		I2C1->DATAR = (addr << 1) | 0x01;
		while(!i2c_status(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
			if(--timeout < 0) {i2c_ret = i2c_get_busy_error(); break;}
	}

	if(i2c_ret == I2C_OK)
	{
		// Read bytes
		uint8_t cbyte = 0;
		while(cbyte < len)
		{
			// If this is the last byte, send the NACK Bit
			if(cbyte == len) I2C1->CTLR1 &= ~I2C_CTLR1_ACK;

			// Wait until the Read Register isn't empty
			while(!(I2C1->STAR1 & I2C_STAR1_RXNE));
			buf[cbyte] = I2C1->DATAR;

			// Make sure no errors occured
			if((i2c_ret = i2c_error()) != I2C_OK) break;

			++cbyte;
		}
	}

	// Send the STOP Condition to auto-reset for the next operation
	I2C1->CTLR1 |= I2C_CTLR1_STOP;

	return i2c_ret;
}


i2c_err_t i2c_write(const uint8_t addr,		const uint8_t reg,
											const uint8_t *buf,
											const uint8_t len)
{
	i2c_err_t i2c_ret = I2C_OK;

	// Wait for the bus to become not busy - set state to I2C_ERR_TIMEOUT on failure
	int32_t timeout = I2C_TIMEOUT;
	while(I2C1->STAR2 & I2C_STAR2_BUSY) 
		if(--timeout < 0) {i2c_ret = i2c_get_busy_error(); break;}

	if(i2c_ret == I2C_OK)
	{
		// Send a START Signal and wait for it to assert
		I2C1->CTLR1 |= I2C_CTLR1_START;
		while(!i2c_status(I2C_EVENT_MASTER_MODE_SELECT));

		// Send the Address and wait for it to finish transmitting
		timeout = I2C_TIMEOUT;
		I2C1->DATAR = (addr << 1) & 0xFE;
		while(!i2c_status(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
			if(--timeout < 0) {i2c_ret = i2c_get_busy_error(); break;}
	}


	if(i2c_ret == I2C_OK)
	{
		// Send the Register Byte
		I2C1->DATAR = reg;
		while(!(I2C1->STAR1 & I2C_STAR1_TXE));

		// Write bytes
		uint8_t cbyte = 0;
		while(cbyte < len)
		{
			// Write the byte and wait for it to finish transmitting
			while(!(I2C1->STAR1 & I2C_STAR1_TXE));
			I2C1->DATAR = buf[cbyte];

			// Make sure no errors occured
			if((i2c_ret = i2c_error()) != I2C_OK) break;

			++cbyte;
		}

		// Wait for the bus to finish transmitting
		while(!i2c_status(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	}

	// Send a STOP Condition, to aut-reset for the next operation
	I2C1->CTLR1 |= I2C_CTLR1_STOP;

	return i2c_ret;
}
