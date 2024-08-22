/******************************************************************************
* Lightweight and simple CH32V0003 I2C Library.
*
* This library provides functions to init, read and write to the hardware I2C
* Bus - in Default, and Alternative Pinout Modes.
* Default:		SCL = PC2		SDA = PC1
* Alternative:	SCL = P			SDA = P
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
#include <stdio.h>

/*** API Functions ***********************************************************/
i2c_err_t i2c_init(uint32_t clk_rate)
{
	// Enable the I2C Peripheral Clock
	RCC->APB1PCENR |= RCC_APB1Periph_I2C1;


	// TODO: set i2c port on, change afio if needed
	// R32_AFIO_PCFR1
	RCC->APB2PCENR |= I2C_PORT_RCC | RCC_APB2Periph_AFIO;

	// Clear, then set the GPIO Settings for SCL and SDA, on the selected port
	I2C_PORT->CFGLR &= ~(0x0F<<(4*I2C_PIN_SDA));
	I2C_PORT->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_OD_AF)<<(4*I2C_PIN_SDA);	
	I2C_PORT->CFGLR &= ~(0x0F<<(4*I2C_PIN_SCL));
	I2C_PORT->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_OD_AF)<<(4*I2C_PIN_SCL);

	// Toggle the I2C Reset bit to init Registers
	RCC->APB1PRSTR |=  RCC_APB1Periph_I2C1;
	RCC->APB1PRSTR &= ~RCC_APB1Periph_I2C1;

	// Set the Prerate frequency
	uint16_t i2c_conf = I2C1->CTLR2 & ~I2C_CTLR2_FREQ;
	i2c_conf |= (FUNCONF_SYSTEM_CORE_CLOCK / I2C_PRERATE) & I2C_CTLR2_FREQ;
	I2C1->CTLR2 = i2c_conf;

	// Set I2C Clock
	if(clk_rate <= 100000)
	{
		i2c_conf = (FUNCONF_SYSTEM_CORE_CLOCK / (2*clk_rate)) & I2C_CKCFGR_CCR;
	} else {
		// Fast mode. Default to 33% Duty Cycle
		i2c_conf = (FUNCONF_SYSTEM_CORE_CLOCK / (3*clk_rate)) & I2C_CKCFGR_CCR;
		i2c_conf |= I2C_CKCFGR_FS;
	}
	I2C1->CKCFGR = i2c_conf;

	// Enable the I2C Peripheral
	I2C1->CTLR1 |= I2C_CTLR1_PE;

	// TODO: Get error states before this
	return I2C_OK;
}


i2c_err_t i2c_ping(const uint8_t addr)
{
	i2c_err_t ret_sta = I2C_OK;

	// Wait for the bus to become not busy - return I2C_ERR_TIMEOUT on failure
	int32_t timeout = I2C_TIMEOUT;
	while(I2C1->STAR2 & I2C_STAR2_BUSY) if(--timeout < 0) return I2C_ERR_TIMEOUT;

	// Send a START Signal and wait for it to assert
	I2C1->CTLR1 |= I2C_CTLR1_START;
	while(!i2c_status(I2C_EVENT_MASTER_MODE_SELECT));

	// Send the Address and wait for it to finish transmitting
	timeout = I2C_TIMEOUT;
	I2C1->DATAR = (addr << 1) & 0xFE;
	while(!i2c_status(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
		if(--timeout < 0) { ret_sta = I2C_ERR_NACK; break; }
	}

	// Send the STOP Signal, return OK or NACK depending
	I2C1->CTLR1 |= I2C_CTLR1_STOP;
	return ret_sta;
}


void i2c_scan(void)
{
	printf("--Scanning I2C Bus--\n");
	// Scan through every address, getting a ping() response
	for(uint8_t addr = 0x00; addr < 0x7F; addr++)
	{
		// If the address responds, set the return status, and print
		uint8_t real_addr = addr << 1; // Actual 7bit address being scanned
		if(i2c_ping(real_addr) == I2C_OK) printf("\tDevice 0x%02X Responded\n", real_addr);
	}
	printf("--Done Scanning--\n");
}


i2c_err_t i2c_read(const uint8_t addr,    const uint8_t reg,
				                          uint8_t *buf,
                                          const uint8_t len)
{
	// Wait for the bus to become not busy - return I2C_ERR_TIMEOUT on failure
	int32_t timeout = I2C_TIMEOUT;
	while(I2C1->STAR2 & I2C_STAR2_BUSY) if(--timeout < 0) return I2C_ERR_TIMEOUT;

	// Send a START Signal and wait for it to assert
	I2C1->CTLR1 |= I2C_CTLR1_START;
	while(!i2c_status(I2C_EVENT_MASTER_MODE_SELECT));

	// Send the Address and wait for it to finish transmitting
	timeout = I2C_TIMEOUT;
	I2C1->DATAR = (addr << 1) & 0xFE;
	while(!i2c_status(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
		if(--timeout < 0) return I2C_ERR_NACK;
	}

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
	while(!i2c_status(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) {
		if(--timeout < 0) return I2C_ERR_NACK;
	}

	// Read bytes
	uint8_t cbyte = 0;
	while(cbyte < len)
	{
		// If this is the last byte, send the NACK Bit
		if(cbyte == len) I2C1->CTLR1 &= ~I2C_CTLR1_ACK;

		// Wait until the Read Register isn't empty
		while(!(I2C1->STAR1 & I2C_STAR1_RXNE));
		buf[cbyte] = I2C1->DATAR;

		++cbyte;
	}

	// Send the STOP Signal
	I2C1->CTLR1 |= I2C_CTLR1_STOP;

	return I2C_OK;
}


i2c_err_t i2c_write(const uint8_t addr,    const uint8_t reg,
					                       const uint8_t *buf,
                                           const uint8_t len)
{
	// Wait for the bus to become not busy - return I2C_ERR_TIMEOUT on failure
	int32_t timeout = I2C_TIMEOUT;
	while(I2C1->STAR2 & I2C_STAR2_BUSY) if(--timeout < 0) return I2C_ERR_TIMEOUT;

	// Send a START Signal and wait for it to assert
	I2C1->CTLR1 |= I2C_CTLR1_START;
	while(!i2c_status(I2C_EVENT_MASTER_MODE_SELECT));

	// Send the Write Address and wait for it to finish transmitting
	timeout = I2C_TIMEOUT;
	I2C1->DATAR = (addr << 1) & 0xFE;
	while(!i2c_status(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
		if(--timeout < 0) return I2C_ERR_NACK;
	}

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

		++cbyte;
	}

	// Wait for the bus to finish transmitting, then send the STOP Signal
	while(!i2c_status(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	I2C1->CTLR1 |= I2C_CTLR1_STOP;

	return I2C_OK;
}
