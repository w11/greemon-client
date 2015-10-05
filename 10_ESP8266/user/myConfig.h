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

typedef enum config_error_t {
		CONFIG_MAGIC_NOT_FOUND = -1,	// No Config available
		CONFIG_INITIAL = 0,				// Initial Error value
		CONFIG_MAGIC_FOUND = 1,			// Config found
} config_error_t;


struct config_t {
		uint32_t 	magic;
		uint32_t  	test;
		uint8_t  	test2[MAX_CHARACTERS];
};

struct config_t config;
struct config_t config_read;

void config_erase()
{


}



/******************************************************************************
 * FunctionName : config_init
 * Description  : Entry point for config initialization.
 *								Searches for the magic inside the flash memory.
 * Parameters   : none
 * Returns      : config_error_t
*******************************************************************************/
config_error_t config_init(void) {

		uint8_t i;
		uint8_t temp[MAX_CHARACTERS];


		if ( CONFIG_MAGIC_FOUND == config_load() ) {

				os_memcpy(temp, config_read.test2, MAX_CHARACTERS);
				DBG_OUT("Testmessage: %s",temp);

				// ERASE
				spi_flash_erase_sector(PRIV_PARAM_START_SEC + PRIV_PARAM_SAVE);
				config.magic = CONFIG_MAGIC_RESET;

				os_memcpy(config.magic, CONFIG_MAGIC, sizeof(config.magic));

				spi_flash_write((PRIV_PARAM_START_SEC + PRIV_PARAM_SAVE) * SPI_FLASH_SEC_SIZE,
								(uint32 *)&config, sizeof(struct config_t));

		} else {


				config.magic = CONFIG_MAGIC;

				os_memcpy(config.magic, CONFIG_MAGIC, sizeof(config.magic));
				os_memcpy(config.test2,"DAVE IS A GENIUS, BECAUSE IT FUCKING FITS INSIDE?\0",MAX_CHARACTERS);

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
