/************************************************************
 *	Project: 			Greemon Client
 *	Author: 			David Huber
 *	Description: 	Libary for the BH1750 (Light-)Sensor
 * 								Open-Source. Feel free - Do what you want.
 *								No support.
 ***********************************************************/

#ifndef __MY_BH1750_H
#define	__MY_BH1750_H

#include "c_types.h"
#include "ets_sys.h"
#include "osapi.h"
#include "pin_map.h"
#include "driver/i2c_master.h"

//#define BH1750_ADDRESS 0x46 		//Address Pin = Low
//#define BH1750_READ_ADDRESS 0x47 	//Address Pin = Low
#define BH1750_ADDRESS 0xB8 		//Address Pin = High
#define BH1750_READ_ADDRESS 0xB9 	//Address Pin = High

#define BH1750_CONVERSION_TIME 150
#define BH1750_CONTINUOUS_HIGH_RES_MODE	0x10		// 1.0 lx resolution, 120ms 
#define BH1750_CONTINUOUS_HIGH_RES_MODE_2	0x11	// 0.5 lx resolution, 120ms 	
#define BH1750_CONTINUOUS_LOW_RES_MODE	0x13		// 4.0 lx resolution, 16ms.
#define BH1750_ONE_TIME_HIGH_RES_MODE	0x20
#define BH1750_ONE_TIME_HIGH_RES_MODE_2	0x21
#define BH1750_ONE_TIME_LOW_RES_MODE	0x23
#define BH1750_POWER_DOWN	0x00	// No active state
#define BH1750_POWER_ON	0x01		// Wating for measurment command
#define BH1750_RESET	0x07		// Reset data register value - not accepted in POWER_DOWN mode

bool myBH1750_Init(uint8_t sda, uint8_t scl);
bool myBH1750_Start(void);
bool myBH1750_SendRequest(uint8_t requestMode);
bool myBH1750_ReadValue(void);
uint16_t myBH1750_getValue(void);

LOCAL uint16 lastReadValue = 0;

#endif

uint16_t myBH1750_getValue(void){
	return lastReadValue;
}


bool myBH1750_Init(uint8_t sda, uint8_t scl){
	return i2c_master_gpio_init(sda, scl);
}

bool myBH1750_Start(void){
	uint8 	msg[1];	
	bool 	has_error = true;


	// SEND 0x46 0x01 (ADDRESS,POWER_ON)
	i2c_master_start();
	i2c_master_writeByte(BH1750_ADDRESS);
	if (!i2c_master_checkAck())
	{
		i2c_master_stop();
		os_printf("POWER_ON: Device not ACKed on address\n" );
		return false;
	}

	i2c_master_writeByte(BH1750_POWER_ON);
	if (!i2c_master_checkAck())
	{
		os_printf("POWER_ON: Device not ACKed on write\n" );
		i2c_master_stop();
		return false;
	
	}
	i2c_master_stop();

	os_delay_us(BH1750_CONVERSION_TIME*1000);

	// SEND 0x46 0x07 (ADDRESS,RESET)
	i2c_master_start();
	i2c_master_writeByte(BH1750_ADDRESS);
	if (!i2c_master_checkAck())
	{
		i2c_master_stop();
		os_printf("RESET: Device not ACKed on address\n" );
		return false;
	}

	i2c_master_writeByte(BH1750_RESET);
	if (!i2c_master_checkAck())
	{
		os_printf("RESET: Device not ACKed on write\n" );
		i2c_master_stop();
		return false;
	
	}
	i2c_master_stop();

	os_printf("BH1750_START OK\r\n");
	return true;
}

bool myBH1750_SendRequest(uint8_t requestMode)
{
	os_printf("READ_FUNC\r\n");
/*
	READING SEQUENCE
	SEND ONE TIME MODE
	START | {ADDR | +0} | ACK | MODE | ACK | STOP
	>>WAIT TO COMPLETE (MAX 24MS)
	SEND READ REQUEST
	START | {ADDR | +1} | ACK | HIGHBYTE | ACK(MASTER) | LOWBYTE | !ACK(MASTER) | STOP
*/		
	// START SEND 0x46 0x23 (ADDRESS,ONE_TIME_LOW_RES_MODE)
	i2c_master_start();
	// ADDR+0
	i2c_master_writeByte(BH1750_ADDRESS);
	// ACK
	if (!i2c_master_checkAck())
	{
		i2c_master_stop();
		os_printf("Device not ACKed on address\n" );
		return false;
	} 
	// MODE // CHANGE ME IF YOU WANT
	i2c_master_writeByte(requestMode); 
	// ACK	
	if (!i2c_master_checkAck())
	{
		os_printf("Device not ACKed on MODE\n" );
		i2c_master_stop();
		return false;
	
	}
	// STOP
	i2c_master_stop();
	return true;
}

bool myBH1750_ReadValue(void){

	LOCAL uint8_t readValHigh = 0;
	LOCAL uint8_t readValLow  = 0;
	// WAIT FOR COMPLETION (MAX 24MS)
	//os_printf("wait..\r\n");	
	//os_delay_us(BH1750_CONVERSION_TIME*1000);

	os_printf("Read: ");

	// START
	i2c_master_start();
	// ADDR+1
	i2c_master_writeByte(BH1750_READ_ADDRESS);		// +1 because of READ Instruction (7BIT ADDR + 1BIT MODE)
	// ACK
	if (!i2c_master_checkAck())
	{
		i2c_master_stop();
		os_printf("Device not ACKed on address + READ\n" );
		return false;
	}
	//i2c_master_wait(5);

	
	// HIGHBYTE
	readValHigh = i2c_master_readByte();
	// ACK(MASTER)
	i2c_master_send_ack();
	
	
	// LOWBYTE	
	readValLow = i2c_master_readByte();
	// !ACK(MASTER)
	i2c_master_send_nack();

	i2c_master_stop();
	DBG_OUT("%x",readValHigh);
	DBG_OUT("%x",readValLow);

	lastReadValue = readValHigh << 8 + readValLow;
	

	return true; //TODO IMPLEMENT ERROR CODES
}


