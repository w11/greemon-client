#include "c_types.h"
#include "spi_flash.h"
#include "osapi.h"
#include "user_interface.h"
#include "mem.h"
#include "myConfig.h"


/* SEE: http://www.esp8266.com/viewtopic.php?f=13&t=2506&sid=9531563d4d1565af88d4a8330e884622&start=5 */
size_t fs_size() { // returns the flash chip's size, in BYTES
  uint32_t id = spi_flash_get_id(); 
  uint8_t mfgr_id = id & 0xff;
  uint8_t type_id = (id >> 8) & 0xff; // not relevant for size calculation
  uint8_t size_id = (id >> 16) & 0xff; // lucky for us, WinBond ID's their chips as a form that lets us calculate the size
  if(mfgr_id != 0xEF) // 0xEF is WinBond; that's all we care about (for now)
    return 0;
  return 1 << size_id;
}


void ICACHE_FLASH_ATTR
config_print(config_t* config){
#ifdef DEV_VERSION
  uint8_t i;
	DBG_OUT("========= GREEMON ==========");
  DBG_OUT("FLASH: \t%d Bytes", fs_size());
  DBG_OUT("CONFIG: \t%d Bytes", sizeof(*config));
  DBG_OUT("BUID-DATE: \t%s", &__GM_BUILD_DATE);
  DBG_OUT("DEVICERID: \t%s", &__GM_BUILD_RAND_NUMBER);
  DBG_OUT("======= CONFIG START =======");
  DBG_OUT("ADDR: \t%x", config);
  DBG_OUT("MAGC: \t%x", config->magic);
  DBG_OUT("VERS: \t%d", config->version);
  DBG_OUT("RAND: \t%d", config->random);
  DBG_OUT("----------------------------");
  DBG_OUT("SSID: \t%s", config->gm_apn_data.ssid);
  DBG_OUT("PASS: \t%s", config->gm_apn_data.pass);
  DBG_OUT("RSRV: \t%d.%d.%d.%d:%d", 
      config->gm_auth_data.srv_address[0], 
      config->gm_auth_data.srv_address[1], 
      config->gm_auth_data.srv_address[2], 
      config->gm_auth_data.srv_address[3],
      config->gm_auth_data.srv_port );
  DBG_OUT("======= CONFIG END =========");
  DBG_OUT("====== SAVED VALUES ========");
  for (i = 0; i<=config->storedData-1; i++){
    DBG_OUT("TIME: \t%x", config->gm_data[i].timestamp );
    DBG_OUT("MOIST:\t%d.%d", 
        (uint8_t) ((0x0F & config->gm_data[i].dht22_moisture) >> 8), 
        (uint8_t) (0xF0 & config->gm_data[i].dht22_moisture));
    DBG_OUT("TEMP: \t%d.%d", 1,1);
    DBG_OUT("LIGHT:\t%d.%d", 1,1);
    DBG_OUT("ADC:  \t%d.%d", 1,1);
    DBG_OUT("--------------------------")
  }
  DBG_OUT("======= END VALUES =========");
#endif
}


SpiFlashOpResult ICACHE_FLASH_ATTR
config_write(config_t* config) {
  DBG_OUT("writing new configuration data to flash memory");

  ETS_GPIO_INTR_DISABLE();

  SpiFlashOpResult r = SPI_FLASH_RESULT_ERR;

  r = spi_flash_write( CONFIG_ADDRESS_START,                // dest
      (uint32_t*)config,                // src
      sizeof(config_t) );               // length
  if (SPI_FLASH_RESULT_OK == r) 
  { 
    DBG_OUT("successfully wrote config into flash memory");
  }
  ETS_GPIO_INTR_ENABLE();
  return r;

}

  SpiFlashOpResult ICACHE_FLASH_ATTR
config_read(config_t* config)
{ 
  DBG_OUT("reading configuration file from flash memory");

  ETS_GPIO_INTR_DISABLE();

  SpiFlashOpResult r = SPI_FLASH_RESULT_ERR;

  r = spi_flash_read(  (uint32_t)CONFIG_ADDRESS_START,		// source 
      (uint32_t*)config,                	// dest
      sizeof(config_t) );               	// data length

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
  //uint8_t num_sec_del;  // number of sectors to delete
  uint8_t isec;    // sector to delte in for..
  uint8_t num_errors_occured = 0;

  uint16_t config_size = sizeof(config_t);
  uint16_t num_sectors = 1;
  uint32_t end_sector = CONFIG_SECTOR_START;
  uint32_t psector = CONFIG_SECTOR_START;

  if (config_size < SPI_FLASH_SEC_SIZE) 
    num_sectors = ( config_size / SPI_FLASH_SEC_SIZE ) +1;

  end_sector += num_sectors;

  DBG_OUT("cfg-size: %d bytes, sector-size: %d bytes, sector-start: 0x%x, sector-end: 0x%x", config_size, SPI_FLASH_SEC_SIZE, CONFIG_SECTOR_START, end_sector-1);
  DBG_OUT("erasing %d bytes ( %d sectors ) in flash for the configuration data", num_sectors*SPI_FLASH_SEC_SIZE, num_sectors);

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
    return SPI_FLASH_RESULT_ERR;
  } else {
    DBG_OUT("flash erase success");
    return SPI_FLASH_RESULT_OK;
  }
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


bool ICACHE_FLASH_ATTR
config_write_apn(gm_APN_t* apn){
  config_t * old_config;
  old_config = (config_t*)os_zalloc(sizeof(config_t));

  if (NULL == old_config){ 
    ERR_OUT("memory allocation failed");
    return false;
  }

  config_read(old_config);

  // TODO check sizes fit?
  os_memcpy(old_config->gm_apn_data.ssid, apn->ssid, sizeof(apn->ssid));
  os_memcpy(old_config->gm_apn_data.pass, apn->pass, sizeof(apn->pass));   

  if ( SPI_FLASH_RESULT_OK == config_save(old_config) )
  {
    return true;
  } else {
    return false;
  }
}

bool ICACHE_FLASH_ATTR
config_write_srv(gm_Srv_t* srv_data){
  config_t * old_config;
  old_config = (config_t*)os_zalloc(sizeof(config_t));

  if (NULL == old_config){
    ERR_OUT("memory allocation failed");
    return false;
  }

  config_read(old_config);


  old_config->gm_auth_data.srv_address[0] = srv_data->srv_address[0];
  old_config->gm_auth_data.srv_address[1] = srv_data->srv_address[1];
  old_config->gm_auth_data.srv_address[2] = srv_data->srv_address[2];
  old_config->gm_auth_data.srv_address[3] = srv_data->srv_address[3];
  old_config->gm_auth_data.srv_port = srv_data->srv_port;

  //id token

  os_memcpy(old_config->gm_auth_data.id, srv_data->id, sizeof(srv_data->id));
  os_memcpy(old_config->gm_auth_data.token, srv_data->id, sizeof(srv_data->token));

  if (SPI_FLASH_RESULT_OK == config_save(old_config)){
    os_free(old_config);
    return true;
  } else {
    os_free(old_config);
    return false;
  }
}


bool ICACHE_FLASH_ATTR
config_write_dataset(uint8_t len, gm_Data_t* data){
  uint8_t i;

  config_t* old_config;
  old_config = (config_t*)os_zalloc(sizeof(config_t)); 

  if (NULL == old_config) {
    ERR_OUT("memory allocation failed");
    os_free(old_config);
    return false;
  }

  config_read(old_config);
  DBG_OUT("number of data: %d", old_config->storedData);

  if ( (old_config->storedData + len) >= CONFIG_MAX_DATASETS){
    ERR_OUT("cannot write, full");
    return false;
  }

  for (i=0; i < len; i++) {
    old_config->gm_data[i].timestamp = data[i].timestamp;
    //TODO REST  
  }
  old_config->storedData = len;

  if (SPI_FLASH_RESULT_OK == config_save(old_config) ) {
    os_free(old_config);
    return true;
  } else {
    os_free(old_config);
    return false;
  }
}

/******************************************************************************
 * FunctionName : config_init
 * Description  : Entry point for config initialization.
 *		  Searches for the magic inside the flash memory.
 *		  Loads the config from the flash memory.
 * Parameters   : none
 * Returns      : config_error_t
 *******************************************************************************/
config_error_t ICACHE_FLASH_ATTR 
config_init() {
  // Allocate memory on heap for the global configuration file
  pUser_global_cfg = (config_t*)os_zalloc(sizeof(config_t)); 
  if (NULL == pUser_global_cfg) {
    ERR_OUT("allocation for config failed");
    return CONFIG_ALLOCATION_FAILED;
    // uh oh thats bad... what now?
  }

  // Loading config from flash to memory
  config_read(pUser_global_cfg);

  // Find magic word
  if (CONFIG_MAGIC == pUser_global_cfg->magic) {
    DBG_OUT("Config magic found");
    // its okay to use this configuration file 
    // TODO Version
    if ( pUser_global_cfg->version < CONFIG_VERSION ){
      //HERE WHAT TO DO WITH OLDER VERSION
      DBG_OUT("older version found");
    }
    return CONFIG_MAGIC_FOUND;
  } else {  
    // no valid config file. Saving new config.
    DBG_OUT("Config magic not found. Creating new configuration file");
    pUser_global_cfg->magic = CONFIG_MAGIC;
    pUser_global_cfg->version = CONFIG_VERSION;
    pUser_global_cfg->storedData = 0;
    config_save(pUser_global_cfg);  
    return CONFIG_INITIAL;
  }


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
  }
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
     uint8_t temp[CONFIG_MAX_CHAR];

  // TODO: MALLOC and save values.	

  if ( CONFIG_MAGIC_FOUND == config_load() ) {


  if (CONFIG_VERSION == config_read.config_version) {
// This config version fits keep using it
os_memcpy(temp, config_read.test2, CONFIG_MAX_CHAR);
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
os_memcpy(config.test2,"DAVE IS A GENIUS, BECAUSE IT FUCKING FITS INSIDE\0",CONFIG_MAX_CHAR);

spi_flash_erase_sector(PRIV_PARAM_START_SEC + PRIV_PARAM_SAVE);

spi_flash_write((PRIV_PARAM_START_SEC + PRIV_PARAM_SAVE) * SPI_FLASH_SEC_SIZE,
(uint32 *)&config, sizeof(struct config_t));

}
*/

}



