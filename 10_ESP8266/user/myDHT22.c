#include "myDHT22.h"
#include "pin_map.h"

/* Datasheets:
 * http://www.adafruit.com/datasheets/DHT22.pdf
 * http://akizukidenshi.com/download/ds/aosong/AM2302.pdf
 * last one is better.
 */

LOCAL inline ICACHE_FLASH_ATTR delay_ms(uint8_t ms){ 

	os_delay_us(1000 * ms); 
}


LOCAL inline bool calculateChecksum( uint8_t *data )
{
	//TODO
	uint8_t checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
	if (data[4] == checksum)  {
		DHT_DBG_MSG("SUCCESS: CHECKSUM %x == %x\r\n",checksum, data[4]);
		return true;
	} else {
		DHT_DBG_MSG("ERROR: CHECKSUM %x != %x\r\n",checksum, data[4]);
		return false;
	}
}

LOCAL inline float calculateHumidity(uint8_t *data) {
		int32_t buffer = 0;
    float humidity = 0; 
		buffer = data[0] * 256 + data[1];
		DBG_OUT("Buffer h: %d", buffer);
		humidity = buffer/10;
    return humidity;
}

LOCAL inline float calculateTemperature(uint8_t *data) {
    float temperature = 0;
		int32_t buffer = data[2] & 0x7f;
    buffer *= 256;
    buffer += data[3];
    if (data[2] & 0x80) buffer *= -1;
		DBG_OUT("calculated t: %d", buffer);
    temperature = buffer/10;
    return temperature; 
}

ICACHE_FLASH_ATTR bool DHT22_init(void){

	PIN_FUNC_SELECT(pin_mux[DHT_PIN], pin_func[DHT_PIN]);

	GPIO_REG_WRITE(GPIO_PIN_ADDR(GPIO_ID_PIN(DHT_PIN)), GPIO_REG_READ(GPIO_PIN_ADDR(GPIO_ID_PIN(DHT_PIN))) | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_ENABLE)); //open drain;
	GPIO_REG_WRITE(GPIO_ENABLE_ADDRESS, GPIO_REG_READ(GPIO_ENABLE_ADDRESS) | (1 << DHT_PIN));

	PIN_PULLUP_DIS(pin_mux[DHT_PIN]);
	GPIO_OUTPUT_SET(DHT_PIN, 1);

#ifdef DHT22_DEBUG_PIN_OUT // FOR TESTING AT PIN WITH OSC
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12); 
	DHT_DBG_PULLUP(DBG_PIN);
#endif
	return true;
}


ICACHE_FLASH_ATTR error_t DHT22_recieveData(void){
	uint8_t 	ByteNumber = 1;
	uint8_t 	BitNumber = 1;
	uint8_t 	Recieved[5] = {0,0,0,0,0}; 
	uint16_t 	dht_wait_time = 0;
	uint16_t 	BitsRecieved = 0;
  	error_t 	e = E_UNKNOWN;
		// CALCULATE 
		uint32_t calculatedHumidity   = 0; 	//   0-100 %
		int32_t calculatedTemperature = 0; 	//  -40-80°C


	// Step 1: MCU send out start signal to AM2302 and AM2302 send response signal to MCU
	DHT_DBG_MSG("---START DHT READING---\r\n"); 	

	GPIO_OUTPUT_SET(DHT_PIN, 0);

#ifdef DHT22_DEBUG_OUT
	// This is for debuging purposes via Oszcilloscope
	DHT_DBG_PULLDN(DBG_PIN);
	os_delay_us(10);

	GPIO_OUTPUT_SET(DHT_PIN, 1);
	os_delay_us(20);

	GPIO_OUTPUT_SET(DHT_PIN, 0);
	os_delay_us(10);

	GPIO_OUTPUT_SET(DHT_PIN, 1);
	os_delay_us(30);

	GPIO_OUTPUT_SET(DHT_PIN, 0);
	os_delay_us(10);

	GPIO_OUTPUT_SET(DHT_PIN, 1);
	os_delay_us(40);
#endif
	GPIO_OUTPUT_SET(DHT_PIN, 0);
	os_delay_us(10);

	GPIO_OUTPUT_SET(DHT_PIN, 1);
	delay_ms(1);
	
	// Hold low for 2ms, 1ms minimum (see:datasheet)
	GPIO_OUTPUT_SET(DHT_PIN, 0);
	DHT_DBG_MSG("WAIT FOR DHT RESPONSE\r\n"); 
	delay_ms(2);							
#ifdef DHT22_DEBUG_OUT							
	DHT_DBG_PULLDN(DBG_PIN);
#endif
	// Disable output. Pin should raise high, because of the pullup
    // DHT should answer within 20-40us
	GPIO_DIS_OUTPUT(DHT_PIN);
	// Wait for the pin to raise
	os_delay_us(5);	//TODO: DO I NEED YOU?			
	DHT_DBG_PULLUP(DBG_PIN);

	// Wait for pin to drop = 0, because DHT pulls down as an answer for 80us
	// exit the function if it takes too long
	dht_wait_time = DHT_MAX_WAIT_TIME; //TODO: TEST 80us
	while ( (1 == GPIO_INPUT_GET(DHT_PIN)) && (dht_wait_time > DHT_REQ_TIMEOUT)  ) 
	{
		dht_wait_time--;
		os_delay_us(1); 
	}
	if (dht_wait_time <= 0) return E_NO_RESPONSE;
#ifdef DHT22_DEBUG_OUT	
	DHT_DBG_PULLDN(DBG_PIN);
#endif
	//DHT_DBG_MSG("Waited: %d us\r\n", DHT_MAX_WAIT_TIME-dht_wait_time);

	// Wait for pin to raise, because DHT pulls up as an answer for 80us
	// exit the function if it takes too long
	dht_wait_time = DHT_MAX_WAIT_TIME; //TODO: TEST 80us
	while ( (0 == GPIO_INPUT_GET(DHT_PIN)) && (dht_wait_time > DHT_REQ_TIMEOUT)  ) 
	{
		dht_wait_time--;
		os_delay_us(1); 
	}
	if (dht_wait_time <= 0) return E_NO_RESPONSE;
#ifdef DHT22_DEBUG_OUT	
	DHT_DBG_PULLUP(DBG_PIN);
#endif
	//DHT_DBG_MSG("Waited: %d us\r\n", DHT_MAX_WAIT_TIME-dht_wait_time);
	//DHT_DBG_MSG("DHT OK\r\n"); 

	//DHT responded with 0-1, continue: Start Data transmission	
	//Starting with 1 to 0 drop, that lasts 50us.
	// 0 = 26-28 us raise until drop
	// 1 = 70    us raise until drop
	// START DATA TRANSMISSION DHT->ESP
	
	//DHT_DBG_MSG("START DATA TRANSMISSION\r\n");
	// Wait for a pin drop
	dht_wait_time = DHT_MAX_WAIT_TIME;
#ifdef DHT22_DEBUG_OUT	
	DHT_DBG_PULLDN(DBG_PIN);
#endif
	while ( (1 == GPIO_INPUT_GET(DHT_PIN)) && (dht_wait_time > DHT_REQ_TIMEOUT)  ) 
	{
		dht_wait_time--;
	}
#ifdef DHT22_DEBUG_OUT	
	DHT_DBG_PULLUP(DBG_PIN);
#endif
	if (dht_wait_time <= 0) return E_ALWAYS_HIGH;
	// Pin dropped, start next bit	 	

	// Read 40 Bits, 5 Bytes*8Bit
	for (ByteNumber = 0; ByteNumber <= DHT_DATA_BYTES-1; ByteNumber++) // 0-4 Bytes
	{
		//DHT_DBG_MSG("BYTE NUMBER :%d\r\n", ByteNumber); 
		// Start Byte reading. Every Bit starts with an 0 for ~50us
		for (BitNumber = 0; BitNumber <= 7; BitNumber++) // 0-7 Bits
		{	
#ifdef DHT22_DEBUG_OUT	
			DHT_DBG_PULLDN(DBG_PIN);
#endif
			// WAIT FOR START SEQUENCE TO END (~50US)
			
			dht_wait_time = 60; //60us 
			while ( (0 == GPIO_INPUT_GET(DHT_PIN)) && (dht_wait_time > DHT_REQ_TIMEOUT)  ) {
				dht_wait_time--;
				os_delay_us(1); //TODO: Use a hw timer?
			}
			if (0 == dht_wait_time) return E_STOPPED_DATA;
			

			// NOW THE PIN IS HIGH	
#ifdef DHT22_DEBUG_OUT			
			DHT_DBG_PULLUP(DBG_PIN);
#endif
			//DHT_DBG_MSG("BIT:%d", BitNumber); 

			// Wait max DHT_MAX_BIT_TRANSMISSION_US * 1us
			dht_wait_time = 1; 
			while ( (1 == GPIO_INPUT_GET(DHT_PIN)) && (dht_wait_time < DHT_MAX_BIT_TRANSMISSION_US)  ) {
				dht_wait_time++;
				os_delay_us(1); //TODO: Use a hw timer?
			}
			if (dht_wait_time >= DHT_MAX_BIT_TRANSMISSION_US) return E_ALWAYS_HIGH;
			// else Bit finished
			if ( (dht_wait_time / 30) >= 1 ) Recieved[ByteNumber] + 1;
			if ( (dht_wait_time / 30) == 0 ) Recieved[ByteNumber] + 0; 

			Recieved[ByteNumber] = Recieved[ByteNumber] + (dht_wait_time / 30) ; // SCHWELLE für 1 oder 0, addiere Ergebnis zu dem Array.
			//DHT_DBG_MSG("[%u %u]",ByteNumber, dht_wait_time);
			//DHT_DBG_MSG("%u,",Recieved[ByteNumber]);
			BitsRecieved++;
			Recieved[ByteNumber] = Recieved[ByteNumber] << 1; // Um eine Stelle verschieben

			// Wait for a pin drop
			
			dht_wait_time = DHT_MAX_WAIT_TIME;
			while ( (1 == GPIO_INPUT_GET(DHT_PIN)) && (dht_wait_time > 0)  ) dht_wait_time--;
			if (dht_wait_time <= 0) return E_ALWAYS_HIGH;

			// Pin dropped, start next bit	 
		}
	}
	DHT_DBG_PULLDN(DBG_PIN);

	// Recieved not enough data.
	if (40 != BitsRecieved) {		
		ERR_OUT("NOT ENOUGH DATA RECV (%u)", BitsRecieved);
		return E_STOPPED_DATA;	
	}
	//Calculate the checksum and check with 5th byte
	if (false == calculateChecksum(Recieved)) {
		ERR_OUT("WRONG CHECKSUM"); 
		return E_WRONG_CHECKSUM;
	}

	// Output the 4x8 bits recieved.	LH LH  T T  PAR
	DBG_OUT("Recieved (HEX): [0x%x\t 0x%x]\t [0x%x\t 0x%x]\t", Recieved[0], Recieved[1], Recieved[2], Recieved[3] );



	/* 
	 * Calculation Routines
	 * TODO: Functionize this!
	 */
	// Calculate Humidity
	calculatedHumidity = 0;
	calculatedHumidity = Recieved[0] << 8; // 0000 1111  => 1111 0000 
	calculatedHumidity = calculatedHumidity | Recieved[1]; // 1111 0000 | 0000 1010 = 1111 1010
	DBG_OUT("Calculated Humidity: %d.%d Percent", calculatedHumidity / 10, (calculatedHumidity % 10) );

	// Calculate Temperature
	calculatedTemperature = 0;
	calculatedTemperature = (Recieved[2] & 0x7F) * 256;  // 1111 1100 & 0x7F (0111 1111) = 0111 1100 
	calculatedTemperature = calculatedTemperature + Recieved[3]; // 0111 1100 + 0000 0010 = 0111 1110
	if ((Recieved[2] & 0x80) != 0)  calculatedTemperature *= -1; // 1000 0000 (0x80) & 0111 1110 = 0000 0000
	DBG_OUT("Calculated Temperature: %d.%d Degrees", calculatedTemperature /10, calculatedTemperature % 10);

	DBG_OUT("READ DHT22 RETURN SUCCESS");
	return E_SUCCESS;

	

}

