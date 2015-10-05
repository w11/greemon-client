#ifndef __MYDHT22_H__
#define __MYDHT22_H__
/*
	GREEMON

	 ESP 8266 LIBARY FOR DHT22
	 DAVID HUBER
	 HOCHSCHULE PFORZHEIM

	INITIAL DOCUMENT: 2015-08-19 19:18
*/

/*
	DHT22 	- Single Bus Data Communication.
		-> Costs 5ms for single time communication (Datasheet)

	 DATA=8 bit integral RH data
		+8 bit decimal RH data
		+8 bit integral T data
		+8 bit decimal T data
		+8 bit check-sum
		If the data transmission is right: 
	Check-sum should be the last 8 bit of 
		"8 bit integral RH data
		+8 bit decimal RH data
		+8 bit integral T data
		+8 bit decimal T data".

		response signal of 40-bit data 
*/



#include "c_types.h"
#include "user_interface.h"
#include "gpio.h"
#include "osapi.h"

/*
#esp8266@Freenode IRC
[17:36] <mattbrejza> yea it outputs its clock
[17:36] <mattbrejza> or at least the crystal freq
 !!! DO NOT USE GPIO5 OR GPIO4, 
 !!! BECAUSE THEY ONLY OUTPUT -0.8V to +0.8V (Think thats the frequency of arounf 28MHz) 
 !!! Signal is too low for the DHT22. Lost a lot of time, because of that.
*/
#define DHT_PIN 12
#define DBG_PIN 2

/*
	GPIO0 -
 	GPIO1 - NO PROG/UART
	GPIO2 - YES
	GPIO3 - NO PROG/UART
	GPIO4 - NO?
	GPIO5 - NO?
	GPIO6
	GPIO7
	GPIO8
	GPIO9
	GPIO10
	GPIO11
	GPIO12 - YES
	GPIO13
	GPIO14
	GPIO15
	GPIO16

 You can have three states:

	output 1 or 0
	Code: Select all
	gpio.mode(8,gpio.OUTPUT)
	gpio.write(8,gpio.HIGH)
	gpio.write(8,gpio.LOW)

	input with pullup
	Code: Select all
	gpio.mode(9,gpio.INPUT)
	gpio.write(9,gpio.HIGH)

	input without pullup
	Code: Select all
	gpio.mode(9,gpio.INPUT)
	gpio.write(9,gpio.LOW)
*/


#define DHT_MAX_WAIT_TIME 32000
#define DHT_MAX_TIMING 10000
#define DHT_MAX_BIT_TRANSMISSION_US 100
#define DHT_DATA_BYTES 5
#define DHT_REQ_TIMEOUT 0

#define DHT22_DEBUG_OUT

#ifdef DHT22_DEBUG_OUT
#define DHT_DBG_MSG(...) os_printf(__VA_ARGS__);
#define DHT_DBG_PULLUP(...) GPIO_OUTPUT_SET(__VA_ARGS__, 1);
#define DHT_DBG_PULLDN(...) GPIO_OUTPUT_SET(__VA_ARGS__, 0);
#else
#define DHT_DBG_MSG
#define DHT_DBG_PULLUP
#define DHT_DBG_PULLDN
#endif


enum _dht22_error_codes
{
	E_SUCCESS 		=  0,
	E_UNKNOWN 		= -1,
	E_NO_RESPONSE 	= -2,
	E_NO_DATA		= -3,
	E_ALWAYS_HIGH	= -4,
	E_STOPPED_DATA	= -5,
	E_WRONG_CHECKSUM =-6
};

typedef enum _dht22_error_codes error_t;

LOCAL struct _errordesc {
	int code;
	char *msg;
} errordesc[] = {
	{ E_SUCCESS, 		"Reading successfull" },
	{ E_UNKNOWN, 		"Initial Error state" },
	{ E_NO_RESPONSE, 	"No response from DHT" },
	{ E_NO_DATA,		"No data recieved" },
	{ E_ALWAYS_HIGH, 	"Sensor not working properly, check the electical connection status" },
	{ E_STOPPED_DATA,	"Sensor started sending, but did not finish" },
	{ E_WRONG_CHECKSUM, "Checksum not matched" }
};

LOCAL const char *byte_to_binary(int x);
LOCAL inline delay_ms(uint8_t ms);
LOCAL inline bool calculateChecksum( uint8_t *data );
LOCAL inline float calculateHumidity(uint8_t *data);
LOCAL inline float calculateTemperature(uint8_t *data);
ICACHE_FLASH_ATTR bool DHT22_init(void);
ICACHE_FLASH_ATTR error_t DHT22_recieveData(void);



#endif /* __MYDHT22_H_ */


