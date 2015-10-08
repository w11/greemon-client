#ifndef __MY_CONFIG_H__
#define __MY_CONFIG_H__



/* NOTICE !!! ---this is for 512KB spi flash.*/
/* You can change to other sector if you use other size spi flash. */
/* Refer to the documentation about OTA support and flash mapping*/
#define CONFIG_SECTOR_START 0x3C
#define CONFIG_ADDRESS_START (SPI_FLASH_SEC_SIZE * CONFIG_SECTOR_START)


#define CONFIG_MAX_CHAR 64
#define CONFIG_MAGIC 0xCAFEC0DE
#define CONFIG_MAGIC_RESET 0xDEADBEEF
#define CONFIG_VERSION 1
#define CONFIG_MAX_DATASETS 30
#define CONFIG_SIZE_SSID 32
#define CONFIG_SIZE_TKN 32
#define CONFIG_SIZE_PASS 32

#define CONFIG_STD_APNAME "GREEMON"
#define CONFIG_STD_PASS ""

#ifndef __GM_BUILD_DATE
#define __GM_BUILD_DATE "unknown"
#else
//extern char __GM_BUILD_DATE
#endif

typedef enum config_error_t {
  CONFIG_INIT_TIMESTAMP_NULL,
  CONFIG_ALLOCATION_FAILED,
  CONFIG_MAGIC_NOT_FOUND = -1,		// No Config available
  CONFIG_INITIAL = 0,							// Initial Error value
  CONFIG_MAGIC_FOUND = 1,					// Config found
} config_error_t;

typedef struct {
  uint32_t  timestamp;          // timestamp
  int16_t   dht22_moisture;     // DHT22
  uint16_t  dht22_temperature;  // DHT22
  uint16_t  bh1750_light;       // BH1750
  uint16_t  adc_moisture;       // ADC
} gm_Data_t;
// 96 Bytes

typedef struct {
  uint8_t     id[32];               // unique recieved from server
  uint8_t     token[CONFIG_SIZE_TKN];            // unique recieved from server
  uint8_t     srv_address[3];       // adress for the server
  uint16_t    srv_port;             // port for the server
} gm_Srv_t;

typedef struct {
  uint8_t     ssid[CONFIG_SIZE_SSID];             // Access Point
  uint8_t     pass[CONFIG_SIZE_PASS];             // Authentication Data
} gm_APN_t;

typedef struct {
  uint32_t 		magic;  		            // INITIAL: CONFIG_MAGIC_RESET
  uint16_t		version;                // version of the configuration file
  uint16_t  	random;                 // Random number, used for validation
  uint32_t 	  config_deep_sleep_time; // !0 = ENABLE DEEPSLEEP MODUS 
  uint8_t     storedData;             // number of stored values
  gm_Srv_t    gm_auth_data;
  gm_APN_t    gm_apn_data;
  gm_Data_t   gm_data[CONFIG_MAX_DATASETS]; // Save last Datasets
} config_t;
// 208 bytes + 96*CONFIG_MAX_DATASETS + gm_Auth_t


LOCAL config_t* pUser_global_cfg;

void ICACHE_FLASH_ATTR config_print(config_t* config);
SpiFlashOpResult ICACHE_FLASH_ATTR config_write(config_t* config);
SpiFlashOpResult ICACHE_FLASH_ATTR config_read(config_t* config);
SpiFlashOpResult ICACHE_FLASH_ATTR config_erase(void);
config_error_t ICACHE_FLASH_ATTR config_save(config_t* unsaved_config);
bool ICACHE_FLASH_ATTR config_write_apn(gm_APN_t* apn);
bool ICACHE_FLASH_ATTR config_write_srv(gm_Srv_t* srv_data);
bool ICACHE_FLASH_ATTR config_write_dataset(uint8_t len, gm_Data_t* data);
config_error_t ICACHE_FLASH_ATTR config_init();

#endif
