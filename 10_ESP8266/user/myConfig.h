#ifndef __MY_CONFIG_H__
#define __MY_CONFIG_H__

#include "spi_flash.h"

/* NOTICE !!! ---this is for 512KB spi flash.*/
/* You can change to other sector if you use other size spi flash. */
/* Refer to the documentation about OTA support and flash mapping*/
#define PRIV_PARAM_START_SEC		0x3C
#define PRIV_PARAM_SAVE     0

#define MAX_CHARACTERS 64
#define CONFIG_MAGIC 0xCAFEC0DE
#define CONFIG_MAGIC_RESET 0xDEADBEEF
#define CONFIG_VERSION 1


typedef enum config_error_t {
		CONFIG_MAGIC_NOT_FOUND = -1,	// No Config available
		CONFIG_INITIAL = 0,				// Initial Error value
		CONFIG_MAGIC_FOUND = 1,			// Config found
} config_error_t;


struct config_t {
	uint32_t 	magic;  // INITIAL: CONFIG_MAGIC_RESET
	uint8_t		config_version;
	uint32_t  test;
	uint8_t  	test2[MAX_CHARACTERS];
	uint32_t 	deep_sleep_time; // !0 = ENABLE DEEPSLEEP MODUS 
	uint8_t 	remote_srv_addr[3]; // IPADDR OF THE SERVER
	uint16_t  remote_srv_port;// PORT OF THE SERVER
};

struct config_t config;
struct config_t config_read;


/******************************************************************************
 * FunctionName : config_erase
 * Description  : This function clears the sector used for the configuration 
									file.
 * Parameters   : none
 * Returns      : config_error_t
*******************************************************************************/
void config_erase()
{


}

/******************************************************************************
 * FunctionName : config_init
 * Description  : Entry point for config initialization.
 *								Searches for the magic inside the flash memory.
 *								Loads the config from the flash memory.
 * Parameters   : none
 * Returns      : config_error_t
*******************************************************************************/
config_error_t config_init(void) {

		uint8_t i;
		uint8_t temp[MAX_CHARACTERS];
	
	// TODO: MALLOC and save values.	

		if ( CONFIG_MAGIC_FOUND == config_load() ) {


				if (CONFIG_VERSION == config_read.config_version) {
					// This config version fits keep using it
					os_memcpy(temp, config_read.test2, MAX_CHARACTERS);
					DBG_OUT("Testmessage: %s",temp);
					DBG_OUT("Config Version: %i", config_read.config_version);
					
				} else {
					// This config version is on an old state
					ERR_OUT("Please update your firmware. Config version is too old.");
				}

				// ERASE
				spi_flash_erase_sector(PRIV_PARAM_START_SEC + PRIV_PARAM_SAVE);
				config.magic = CONFIG_MAGIC_RESET;

				os_memcpy(config.magic, CONFIG_MAGIC, sizeof(config.magic));

				spi_flash_write((PRIV_PARAM_START_SEC + PRIV_PARAM_SAVE) * SPI_FLASH_SEC_SIZE,
								(uint32 *)&config, sizeof(struct config_t));

		} else {


				config.magic = CONFIG_MAGIC;

				os_memcpy(config.magic, CONFIG_MAGIC, sizeof(config.magic));
				os_memcpy(config.test2,"DAVE IS A GENIUS, BECAUSE IT FUCKING FITS INSIDE\0",MAX_CHARACTERS);

				spi_flash_erase_sector(PRIV_PARAM_START_SEC + PRIV_PARAM_SAVE);

				spi_flash_write((PRIV_PARAM_START_SEC + PRIV_PARAM_SAVE) * SPI_FLASH_SEC_SIZE,
								(uint32 *)&config, sizeof(struct config_t));

		}

}

config_error_t config_load(){
		DBG_OUT("--- CHECKING FOR MAGIC ---");
		spi_flash_read((PRIV_PARAM_START_SEC + PRIV_PARAM_SAVE) * SPI_FLASH_SEC_SIZE, 
						(uint32 *)&config_read, sizeof(struct config_t));

		if (CONFIG_MAGIC == config_read.magic) { 
				DBG_OUT(">Found Magic: %x",config_read.magic);	
				return CONFIG_MAGIC_FOUND;
		} else {
				ERR_OUT(">Magic not found: %x",config_read.magic);
				return CONFIG_MAGIC_NOT_FOUND;
		}

}


#endif
