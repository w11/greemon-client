#include "c_types.h"
#include "spi_flash.h"
#include "osapi.h"
#include "user_interface.h"
#include "mem.h"
#include "myConfig.h"


uint16_t getConfigSize(void){
	return sizeof(config_t);
}

/******************************************************************************
 * FunctionName : config_print
 * Description  : Prints the content of the given config_t file out
 * Parameters   : config_t
 * Returns      : void
 *******************************************************************************/
void ICACHE_FLASH_ATTR
config_print(config_t* config){
#ifdef DEV_VERSION
	char timestamp[50];
		
  uint8_t i;
	DBG_OUT("========= GREEMON ==========");
  DBG_OUT("CONF: \t%d Bytes", sizeof(*config));
  DBG_OUT("DATE: \t%s", &__GM_BUILD_DATE);
  DBG_OUT("GMID: \t%s", &__GM_BUILD_RAND_NUMBER);
	DBG_OUT("CHIP: \t%u", system_get_chip_id());
	DBG_OUT("======= CONFIG START =======");
	DBG_OUT("ADDR: \t%x", &config);
	DBG_OUT("MADR: \t%x", CONFIG_ADDRESS_START);
	DBG_OUT("MAGC: \t%x", config->magic);
	DBG_OUT("VERS: \t%d", config->version);
	DBG_OUT("RAND: \t%d", config->random);
	DBG_OUT("----------------------------");
	DBG_OUT("SSID: \t%s", config->gm_apn_data.ssid);
	DBG_OUT("PASS: \t%s", config->gm_apn_data.pass);
	DBG_OUT("----------------------------");
	DBG_OUT("ADDR: \t%d.%d.%d.%d:%d", 
	    config->gm_auth_data.srv_address[0], 
	    config->gm_auth_data.srv_address[1], 
	    config->gm_auth_data.srv_address[2], 
	    config->gm_auth_data.srv_address[3],
	    config->gm_auth_data.srv_port );
	DBG_OUT("TOKN: \t%s", config->gm_auth_data.token);
	DBG_OUT("======= CONFIG END =========");
	DBG_OUT("====== SAVED VALUES ========");
	for (i = 0; i<=config->storedData-1; i++){
		os_sprintf(timestamp, epoch_to_str(config->gm_data[i].timestamp));		
	  DBG_OUT("TIME: \t%s", timestamp );
	  DBG_OUT("MOIST:\t%d", config->gm_data[i].dht22_moisture);
	  DBG_OUT("TEMP: \t%d", config->gm_data[i].dht22_temperature);
	  DBG_OUT("LIGHT:\t%d", config->gm_data[i].bh1750_light); 
		DBG_OUT("ADC:  \t%d", config->gm_data[i].adc_moisture); 
	  DBG_OUT("--------------------------");
	}
	DBG_OUT("======= END VALUES =========");
#endif
}

/******************************************************************************
 * FunctionName : config_write
 * Description  : Writes the configuration data into flash memory 
 *								at CONFIG_ADDRESS_START. 
 *								you need to call config_erase before! 								
 * Parameters   : config_t
 * Returns      : SpiFlashOpResult
 *******************************************************************************/
SpiFlashOpResult ICACHE_FLASH_ATTR
config_write(config_t* config) {
	DBG_OUT("got config at addr: %x",&config);
	DBG_OUT("destination: %x",CONFIG_ADDRESS_START);
  ETS_GPIO_INTR_DISABLE();

  volatile SpiFlashOpResult r = SPI_FLASH_RESULT_ERR;

  r = spi_flash_write( 
			(uint32_t)CONFIG_ADDRESS_START,  	// dest
      (uint32_t*)config,								// src               
      (uint32_t)sizeof(config_t) 				// length
	);               

  if (SPI_FLASH_RESULT_OK == r) {	
		INFO("write success");
	} else {
		INFO("(!) write failed");
	}

  ETS_GPIO_INTR_ENABLE();
  return r;
}

/******************************************************************************
 * FunctionName : config_read
 * Description  : Reads flash memory at position CONFIG_ADDRESS_START and copies
 *								the content to the memory given as parameter 						
 * Parameters   : config_t
 * Returns      : SpiFlashOpResult
 *******************************************************************************/
SpiFlashOpResult ICACHE_FLASH_ATTR
config_read(config_t* config)
{ 
	DBG_OUT("got config at addr: %x",&config);
  ETS_GPIO_INTR_DISABLE();

  volatile SpiFlashOpResult r = SPI_FLASH_RESULT_ERR;

  r = spi_flash_read(  
		(uint32_t)CONFIG_ADDRESS_START,		// source 
    (uint32_t*)config,                // dest
    (uint32_t)sizeof(config_t) 									// data length
	);               	

  if (SPI_FLASH_RESULT_OK == r) {	
		INFO("read success");
	} else {
		INFO("(!) read failed");
	}
  ETS_GPIO_INTR_ENABLE();
  return r; 
}

/******************************************************************************
 * FunctionName : config_erase
 * Description  : This function clears the sector used for the configuration 
 *								file. You can only erase sector-wise. A sector has a size of
 * 								SPI_FLASH_SEC_SIZE. So therefore we need to calculate how much
 *								sectors we have to erase for the config to write.
 * Parameters   : none
 * Returns      : config_error_t
 *******************************************************************************/
SpiFlashOpResult ICACHE_FLASH_ATTR
config_erase(void)
{
  uint8_t isec;    // sector to delte in for..
  uint8_t num_errors_occured = 0;

  uint16_t config_size = sizeof(config_t);
  uint16_t num_sectors = 1;
  uint32_t end_sector = CONFIG_SECTOR_START;
  uint32_t psector = CONFIG_SECTOR_START;

  if (config_size < SPI_FLASH_SEC_SIZE) num_sectors = ( config_size / SPI_FLASH_SEC_SIZE ) +1;

  end_sector += num_sectors;

  ETS_GPIO_INTR_DISABLE();
  SpiFlashOpResult r = SPI_FLASH_RESULT_ERR;

  for (isec = 0; isec < num_sectors; isec++)
  { 
    psector = CONFIG_SECTOR_START + isec;
    DBG_OUT("del: sector 0x%x", psector);
    // Delete sector
    r = spi_flash_erase_sector( psector );
    // get result
    if ( SPI_FLASH_RESULT_OK != r ) {
      num_errors_occured++;
      ERR_OUT("sector %x could not be deleted!", psector);
    } else {
      DBG_OUT("sector %x deleted", psector);
    }
  }
  ETS_GPIO_INTR_ENABLE();
  if (num_errors_occured > 0) {
    ERR_OUT("ERRORS OCCURED: %d", num_errors_occured);
		INFO("(!) failed");
    return SPI_FLASH_RESULT_ERR;
  } else {
    DBG_OUT("flash erase success");
		INFO("success");
    return SPI_FLASH_RESULT_OK;
  }
}


/******************************************************************************
 * FunctionName : config_save
 * Description  : Wrapper for config_write. So we can add some features here.
 * Parameters   : config_t
 * Returns      : SpiFlashOpResult
 ********************************************************************************/
SpiFlashOpResult ICACHE_FLASH_ATTR
config_save(config_t* unsaved_config)
{
	INFO("setting things up");
	//DBG_OUT("got config at addr: %x",&unsaved_config);
  unsaved_config->version = CONFIG_VERSION;
  // unsaved_config->magic = CONFIG_MAGIC;
  // Erase configuration sector
  if (SPI_FLASH_RESULT_OK == config_erase() ){
	
    // write new config in flash
    if (SPI_FLASH_RESULT_OK == config_write(unsaved_config)){
      DBG_OUT("wrote new configuration file to flash memory");
			INFO("success");
			return SPI_FLASH_RESULT_OK;
      // TODO COMPARE NEW CONFIG WITHUNSAVED_CONFIG        
    } else {
      ERR_OUT("error writing to flash");
			return SPI_FLASH_RESULT_ERR;
    }
  } else {
    ERR_OUT("could not erase config file sector in flash memory");
		return SPI_FLASH_RESULT_ERR;
  }
}

/********************************************************************************
 * FunctionName : config_write_apn
 * Description  : Loads the curent configuration and changes the section for the
 *								access point.
 * Parameters   : gm_APN_t
 * Returns      : bool
 ********************************************************************************/
bool ICACHE_FLASH_ATTR
config_write_apn(gm_APN_t* apn){
	INFO("writing apn data to config");
	config_read(&global_cfg);
	global_cfg.magic = CONFIG_MAGIC;
	os_memcpy(&global_cfg.gm_apn_data, apn, sizeof(gm_APN_t));
	config_print(&global_cfg);
	if (SPI_FLASH_RESULT_OK == config_save(&global_cfg))
	{
		INFO("success");
		return true;
	} else {
		ERR_OUT("An error occured");
		return false;
	}
}

/********************************************************************************
 * FunctionName : config_write_srv
 * Description  : Loads the curent configuration and changes the section for the
 *								server configuration.
 * Parameters   : gm_Srv_t
 * Returns      : bool
 ********************************************************************************/
bool ICACHE_FLASH_ATTR
config_write_srv(gm_Srv_t* srv_data){
	INFO("writing auth data to config");
	config_read(&global_cfg);
	global_cfg.magic = CONFIG_MAGIC;
	os_memcpy(&global_cfg.gm_auth_data, srv_data, sizeof(gm_Srv_t));
	config_print(&global_cfg);
	if (SPI_FLASH_RESULT_OK == config_save(&global_cfg))
	{
		INFO("success");
		return true;
	} else {
		ERR_OUT("An error occured");
		return false;
	}
}

/********************************************************************************
 * FunctionName : config_write_dataset
 * Description  : Loads the curent configuration and changes the section for the
 *								saved sensor data. Adds len number of data structures.
 * Parameters   : len - Number of data
 *								data - datasets 
 * Returns      : bool
 ********************************************************************************/
bool ICACHE_FLASH_ATTR
config_write_dataset(config_t* pConfig, uint8_t len, gm_Data_t* data){
	INFO("writing datasets to config");
	uint8_t i = 0;

	config_read(pConfig);
  DBG_OUT("number of data before: %d", pConfig->storedData);

	for (i=0; i < len; i++) config_push_data(pConfig, &data[i]);
		
	DBG_OUT("number of data after: %d", pConfig->storedData);

	if (SPI_FLASH_RESULT_OK == config_save(pConfig))
	{
		INFO("flash write success");
		return true;
	} else {
		ERR_OUT("An error occured");
		return false;
	} 

}

/********************************************************************************
 * FunctionName : config_push_data
 * Description  : 
 * Parameters   : 
 *								
 * Returns      : bool
 ********************************************************************************/
bool ICACHE_FLASH_ATTR
config_push_data(config_t* pConfig, gm_Data_t* data){
	INFO("adding data to config");
	DBG_OUT("current data in storage: %d", pConfig->storedData);

  if ( CONFIG_MAX_DATASETS == (pConfig->storedData) )
	{
    ERR_OUT("cannot write, full");
    return false;
  } else {
		DBG_OUT("PUSH Data[%d]",pConfig->storedData);
		os_memcpy(pConfig->gm_data+(pConfig->storedData), data, sizeof(gm_Data_t));
		pConfig->storedData++;
		DBG_OUT("data now: %u", pConfig->storedData);
		INFO("remember to save it");
		return true;
	} 
}

/********************************************************************************
 * FunctionName : config_pop_data
 * Description  : 
 * Parameters   : 
 *								
 * Returns      : gm_Data_t*
 ********************************************************************************/
gm_Data_t* ICACHE_FLASH_ATTR
config_pop_data(config_t* pConfig){
	gm_Data_t * pData = NULL;

	INFO("POP Data[%d]", pConfig->storedData-1);

	if ( -1 == pConfig->storedData ) {
		pData = NULL;
	} else {
		pData = &global_cfg.gm_data[pConfig->storedData-1];
	}

	if (NULL == pData) {
		ERR_OUT("empty data storage");
		return NULL;
	} else {
		INFO("returning pointer: %x for Data[%u]", pData, pConfig->storedData-1);
		pConfig->storedData--;
		DBG_OUT("data left in storage: %u", pConfig->storedData)
	}

	return pData; 
}

/******************************************************************************
 * FunctionName : config_init
 * Description  : Entry point for config initialization.
 *		  					Searches for the magic inside the flash memory.
 *		  					Loads the config from the flash memory into the global
 *								variable pUser_global_cfg
 * Parameters   : none
 * Returns      : config_error_t
 *******************************************************************************/
config_error_t ICACHE_FLASH_ATTR 
config_init() {
  // Allocate memory on heap for the global configuration file
  //pUser_global_cfg = (config_t*)os_zalloc(sizeof(config_t)); 
	/*  
	if (NULL == pUser_global_cfg) {
    ERR_OUT("Allocation for config failed.");
		ERR_OUT("Your Greemon is probably bricked.");
    return CONFIG_ALLOCATION_FAILED;
    // uh oh thats bad... what now?
  }
	*/

  // Loading config from flash to memory
  config_read(&global_cfg);

  // Find magic word
  if (CONFIG_MAGIC == global_cfg.magic) {
    DBG_OUT("Config magic found");
    // its okay to use this configuration file 
    // TODO Version
    if ( global_cfg.version < CONFIG_VERSION ){
      //HERE WHAT TO DO WITH OLDER VERSION
      DBG_OUT("older version found");
    }
    return CONFIG_MAGIC_FOUND;
  } else {  
    // no valid config file. Saving new config.
    DBG_OUT("Config magic not found. Creating new configuration file");
    global_cfg.magic = CONFIG_MAGIC_RESET;
    global_cfg.version = CONFIG_VERSION;

    global_cfg.storedData = 0;

		DBG_OUT("Saving Random");
		global_cfg.random = (uint32_t)__GM_BUILD_RAND_NUMBER;
		DBG_OUT("Saving Default Name");
		os_memcpy(global_cfg.gm_base_data.name,"GREEMON_DEFAULT",sizeof(global_cfg.gm_base_data.name));
		DBG_OUT("Saving Deep Sleep Time");
		global_cfg.gm_base_data.deep_sleep_us = 0;

		DBG_OUT("Saving APN Data");
		os_memcpy(global_cfg.gm_apn_data.ssid, CONFIG_STD_APNAME, sizeof(global_cfg.gm_apn_data.ssid));
		os_memcpy(global_cfg.gm_apn_data.pass, CONFIG_STD_PASS, sizeof(global_cfg.gm_apn_data.pass));

		DBG_OUT("Saving SRV Data");
		global_cfg.gm_auth_data.srv_address[0] = 0;
		global_cfg.gm_auth_data.srv_address[1] = 0;
		global_cfg.gm_auth_data.srv_address[2] = 0;
		global_cfg.gm_auth_data.srv_address[3] = 0;
		DBG_OUT("saving port");
		global_cfg.gm_auth_data.srv_port = 80;
		DBG_OUT("saving token");
		os_memcpy(global_cfg.gm_auth_data.token, "NoTokenSaved\0", CONFIG_SIZE_TKN * sizeof(uint8_t));

		DBG_OUT("Saving Config");
    config_save(&global_cfg); 
		config_read(&global_cfg); 
		config_print(&global_cfg);

		INFO("Config size: %u", getConfigSize() );

    return CONFIG_INITIAL;
  }
}



