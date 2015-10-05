#ifndef __MY_CONFIG_H__
#define __MY_CONFIG_H__

#include "spi_flash.h"

/* NOTICE !!! ---this is for 512KB spi flash.*/
/* You can change to other sector if you use other size spi flash. */
/* Refer to the documentation about OTA support and flash mapping*/
#define CONFIG_SECTOR_START 0x3C
#define CONFIG_ADDRESS_START (SPI_FLASH_SEC_SIZE * CONFIG_SECTOR_START)


#define MAX_CHARACTERS 64
#define CONFIG_MAGIC 0xCAFEC0DE
#define CONFIG_MAGIC_RESET 0xDEADBEEF
#define CONFIG_VERSION 1


typedef enum config_error_t {
  CONFIG_ALLOCATION_FAILED,
  CONFIG_MAGIC_NOT_FOUND = -1,	// No Config available
	CONFIG_INITIAL = 0,				// Initial Error value
	CONFIG_MAGIC_FOUND = 1,			// Config found
} config_error_t;


typedef struct {
	uint32_t 	magic;  // INITIAL: CONFIG_MAGIC_RESET
	uint16_t	version;
  uint16_t  random;
  //uint16_t	config_padding;
	//uint32_t 	config_deep_sleep_time; // !0 = ENABLE DEEPSLEEP MODUS 
	uint8_t 	remote_srv_addr[3]; // IPADDR OF THE SERVER
	uint16_t  remote_srv_port;// PORT OF THE SERVER
	uint8_t 	ssid[32]; // SSID of the network
	uint8_t 	pass[32]; // passwort for the network
} config_t;


config_t user_global_cfg;


void config_print(config_t* config){
  DBG_OUT("======= CONFIG START =======");
  DBG_OUT("ADDR: \t%x", config);
  DBG_OUT("MAGC: \t%x", config->magic);
  DBG_OUT("VERS: \t%d", config->version);
  DBG_OUT("RAND: \t%d", config->random);
  DBG_OUT("----------------------------");
  DBG_OUT("SSID: \t%s", config->ssid);
  DBG_OUT("PASS: \t%s", config->pass);
  DBG_OUT("RSRV: \t%d.%d.%d.%d:%d", 
      config->remote_srv_addr[0], 
      config->remote_srv_addr[1], 
      config->remote_srv_addr[2], 
      config->remote_srv_addr[3], 
      config->remote_srv_port );
  DBG_OUT("======= CONFIG END =========");
}



SpiFlashOpResult ICACHE_FLASH_ATTR
config_write(config_t* config) {
  DBG_OUT("writing new configuration data to flash memory");
  
  ETS_GPIO_INTR_DISABLE();

  SpiFlashOpResult r = SPI_FLASH_RESULT_ERR;

  r = spi_flash_write( CONFIG_ADDRESS_START,                // dest
                          (uint32_t*)config,                // src
                          sizeof(config_t) );               // length
  if (SPI_FLASH_RESULT_OK == r) DBG_OUT("successfully wrote config into flash memory");
  ETS_GPIO_INTR_ENABLE();
  return r;

}

SpiFlashOpResult ICACHE_FLASH_ATTR
config_read(config_t* config)
{ 
  DBG_OUT("reading configuration file from flash memory");

  ETS_GPIO_INTR_DISABLE();

  SpiFlashOpResult r = SPI_FLASH_RESULT_ERR;

  r = spi_flash_read(  (uint32_t)CONFIG_ADDRESS_START,   // source 
                          (uint32_t*)config,                // dest
                          sizeof(config_t) );               // data length

  ETS_GPIO_INTR_ENABLE();
  return r; 
}

/******************************************************************************
 * FunctionName : config_erase
 * Description  : This function clears the sector used for the configuration 
									file.
 * Parameters   : none
 * Returns      : config_error_t
*******************************************************************************/
SpiFlashOpResult ICACHE_FLASH_ATTR
config_erase(void)
{
  ETS_GPIO_INTR_DISABLE();

  SpiFlashOpResult r = SPI_FLASH_RESULT_ERR;
  DBG_OUT("erasing config flash sector..");

  r = spi_flash_erase_sector( CONFIG_SECTOR_START );

  switch (r)
  {
    case
      SPI_FLASH_RESULT_ERR: 
        ERR_OUT("error erasing config sector");
        return r;
      break;
    case
      SPI_FLASH_RESULT_TIMEOUT:
        ERR_OUT("timeout erasing config sector");
        return r;
      break;
    case
      SPI_FLASH_RESULT_OK: 
        DBG_OUT("successfully erased config sector");
        return r;
      break;
  }

  ETS_GPIO_INTR_ENABLE();
}


/******************************************************************************
 * FunctionName : config_save
 * Description  : Write the configuration file to flash.
 * Parameters   : none
 * Returns      : config_error_t
 ********************************************************************************/
config_error_t ICACHE_FLASH_ATTR
config_save(config_t* unsaved_config)
{
  unsaved_config->version = CONFIG_VERSION;
  unsaved_config->magic = CONFIG_MAGIC;
  // Erase configuration sector
  if (SPI_FLASH_RESULT_OK == config_erase() ){
    // write new config in flash
    if (SPI_FLASH_RESULT_OK == config_write(unsaved_config)){
      DBG_OUT("wrote new configuration file to flash memory");
       // TODO COMPARE NEW CONFIG WITHUNSAVED_CONFIG        
    } else {
      ERR_OUT("error writing to flash");
    }
  } else {
    ERR_OUT("could not erase config file sector in flash memory");
  }
}



/******************************************************************************
 * FunctionName : config_init
 * Description  : Entry point for config initialization.
 *								Searches for the magic inside the flash memory.
 *								Loads the config from the flash memory.
 * Parameters   : none
 * Returns      : config_error_t
*******************************************************************************/
config_error_t ICACHE_FLASH_ATTR 
config_init(void) {
  // Allocate memory for the configuation file
  
  //global_cfg.magic = 0;
  //global_cfg.version = 1;
  //DBG_OUT("initializing configuration file");


  
  /*
  // Load flash sector into allocated memory 
  DBG_OUT("reading flash memory");
	if (SPI_FLASH_RESULT_OK == config_read(&global_cfg)){
		DBG_OUT("..OK");
	} else {
		DBG_OUT("..ERROR");
	}
  */
 /*
  // Read Content. Is it a configuration file? (config-magic)
  if (CONFIG_MAGIC != cfg->config_magic) {
    ERR_OUT("config magic not found");
    ERR_OUT("Found: %s",cfg->config_magic);
    return CONFIG_MAGIC_NOT_FOUND; 
  } 
  if (CONFIG_INITIAL == cfg->config_magic) {
    DBG_OUT("magic initial found");
    return CONFIG_INITIAL;
  }
  // Read Version of the configuration file
  if (CONFIG_VERSION == cfg->config_version) {
    DBG_OUT("config file has the same version as the firmware");
    load_failover = true;
  } else { // Load failover config
    ERR_OUT("loading failover configuration");
  } 
  // use config

  */


/*
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
*/

}

config_error_t config_load(){
		DBG_OUT("--- CHECKING FOR MAGIC ---");
/*
		spi_flash_read((PRIV_PARAM_START_SEC + PRIV_PARAM_SAVE) * SPI_FLASH_SEC_SIZE, 
						(uint32 *)&config_read, sizeof(struct config_t));

    if (CONFIG_MAGIC == config_read.magic) { 
				DBG_OUT(">Found Magic: %x",config_read.magic);	
				return CONFIG_MAGIC_FOUND;
		} else {
				ERR_OUT(">Magic not found: %x",config_read.magic);
				return CONFIG_MAGIC_NOT_FOUND;
		}
*/
}


#endif
