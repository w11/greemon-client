#include "myTest.h"

#define DBG_USE_TESTCONFIG false
#define DBG_USE_CONNECTION_DATA true

bool ICACHE_FLASH_ATTR test_data_add(void){
	gm_Data_t d[5];

	INFO("Adding testdata for demo");
/*
	uint32_t  	timestamp;          // timestamp
	int16_t   	dht22_moisture;     // DHT22
	uint16_t  	dht22_temperature;  // DHT22
	uint16_t  	bh1750_light;       // BH1750
	uint16_t  	adc_moisture;       // ADC
*/

	d[0].timestamp = 1452168000;
	d[0].dht22_moisture = (int16_t)1;
	d[0].dht22_temperature = (int16_t)-2;
	d[0].bh1750_light = (uint16_t)1;
	d[0].adc_moisture = (uint16_t)1;

	d[1].timestamp = 1452168060;
	d[1].dht22_moisture = (int16_t)0;
	d[1].dht22_temperature = (int16_t)-1;
	d[1].bh1750_light = (uint16_t)2;
	d[1].adc_moisture = (uint16_t)2;

	d[2].timestamp = 1452168120;
	d[2].dht22_moisture = (int16_t)1;
	d[2].dht22_temperature = (int16_t)0;
	d[2].bh1750_light = (uint16_t)3;
	d[2].adc_moisture = (uint16_t)3;

	d[3].timestamp = 1452168180;
	d[3].dht22_moisture = (int16_t)0;
	d[3].dht22_temperature = (int16_t)1;
	d[3].bh1750_light = (uint16_t)2;
	d[3].adc_moisture = (uint16_t)2;

	d[4].timestamp = 1452168240;
	d[4].dht22_moisture = (int16_t)1;
	d[4].dht22_temperature = (int16_t)2;
	d[4].bh1750_light = (uint16_t)1;
	d[4].adc_moisture = (uint16_t)1;

	d[5].timestamp = 1452168300;
	d[5].dht22_moisture = (int16_t)0;
	d[5].dht22_temperature = (int16_t)1;
	d[5].bh1750_light = (uint16_t)1;
	d[5].adc_moisture = (uint16_t)1;

	d[6].timestamp = 1452168360;
	d[6].dht22_moisture = (int16_t)1;
	d[6].dht22_temperature = (int16_t)0;
	d[6].bh1750_light = (uint16_t)1;
	d[6].adc_moisture = (uint16_t)1;

	d[7].timestamp = 1452168420;
	d[7].dht22_moisture = (int16_t)0;
	d[7].dht22_temperature = (int16_t)-1;
	d[7].bh1750_light = (uint16_t)1;
	d[7].adc_moisture = (uint16_t)1;

	config_write_dataset(&global_cfg, 7, d);
	config_save(&global_cfg);
	config_print(&global_cfg);
	DBG_OUT("Test data has been added.");
}

void ICACHE_FLASH_ATTR test_connection_settings(void) {
#if DBG_USE_CONNECTION_DATA 
	gm_Srv_t Srv_data;
	gm_APN_t APN_data;

	Srv_data.srv_address[0] = 144;
	Srv_data.srv_address[1] = 212;
	Srv_data.srv_address[2] = 80;
	Srv_data.srv_address[3] = 11;
	
	os_memcpy(APN_data.ssid,"Julia\0",CONFIG_SIZE_SSID);
	os_memcpy(APN_data.pass,"scheka123\0",CONFIG_SIZE_PASS);

	os_memcpy(Srv_data.token,"A8HR63UB82X72MG0\0",CONFIG_SIZE_TKN);
	os_memcpy(Srv_data.srv_port,80,sizeof(uint16_t));


	config_write_srv(&Srv_data);
 	config_write_apn(&APN_data);

	system_restart();
#endif
}


/******************************************************************************
 * FunctionName : test_config
 * Description  : Test routine for configuration file
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR test_config(void) {
	INFO("Generating Test-Configuration File");
	config_t test_cfg;	
	config_t loaded_cfg;

#if DBG_USE_TESTCONFIG
  gm_Data_t test_data[3];
  gm_APN_t apn_data;
  gm_Srv_t srv_data;

//144.212.80.11
  srv_data.srv_address[0] = 144;
  srv_data.srv_address[1] = 212;
  srv_data.srv_address[2] = 80;
  srv_data.srv_address[3] = 11;
  srv_data.srv_port = 80;

  os_memcpy(srv_data.id, "GMID213\0",sizeof(srv_data.token));
  os_memcpy(srv_data.token, "A8HR63UB82X72MG0\0",sizeof(srv_data.token));

	os_memcpy(apn_data.ssid,"EvoraIT\0",sizeof(apn_data.ssid));
	os_memcpy(apn_data.pass,"Mobile123123\0",sizeof(apn_data.pass));

	test_cfg.magic = CONFIG_MAGIC;	
	test_cfg.version = CONFIG_VERSION;
  test_cfg.random = 12345;
  test_cfg.storedData = 0;

	//os_memcpy(test_cfg.gm_apn_data.pass,"MY_OLD_PASS\0",sizeof(test_cfg.gm_apn_data.pass));
	os_memcpy(test_cfg.gm_apn_data.ssid,"MY_OLD_SSID\0",sizeof(test_cfg.gm_apn_data.ssid));

	test_cfg.gm_auth_data.srv_address[0] = 192;
	test_cfg.gm_auth_data.srv_address[1] = 168;
	test_cfg.gm_auth_data.srv_address[2] = 10;
	test_cfg.gm_auth_data.srv_address[3] = 11;
	test_cfg.gm_auth_data.srv_port 	= 8080;

	DBG_OUT("CFG:ERASE");
	config_erase();
	DBG_OUT("CFG:WRITE");
	config_write(&test_cfg);
	DBG_OUT("CFG:READ");
	config_read(&loaded_cfg);
	DBG_OUT("CFG:PRINT");
	config_print(&loaded_cfg);	
#endif

  config_init();
	test_cfg.magic = CONFIG_MAGIC;	
	test_cfg.version = CONFIG_VERSION;
  test_cfg.random = 12345;
  test_cfg.storedData = 0;
	config_write(&test_cfg);

#if DBG_USE_TESTCONFIG
  config_write_dataset(3,test_data);
  config_write_apn(&apn_data);
  config_write_srv(&srv_data);
#endif

  config_read(&loaded_cfg);
  config_print(&loaded_cfg);
}
