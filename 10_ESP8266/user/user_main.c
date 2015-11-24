/*
	GREEMON

	 GREEN UP YOUR LIFE
	 DAVID HUBER
	 HOCHSCHULE PFORZHEIM

	INITIAL DOCUMENT: 2015-08-19 19:18
*/
#include "myDHT22.h"
#include "myBH1750.h"

#include "user_config.h"


#include "/opt/esp-open-sdk/sdk/include/ets_sys.h"
#include "/opt/esp-open-sdk/sdk/include/osapi.h"
#include "/opt/esp-open-sdk/sdk/include/gpio.h"
#include "c_types.h"

#include "user_interface.h"

#include "driver/uart.h"
#include "driver/uart_register.h"
#include "spi_flash.h"
#include "myWifiServer.h"
#include "myConfig.h"
#include "myNTP.h"

#include "gpio.h"

//#define MEMLEAK_DEBUG

#define BH1750_PIN_SDA 2
#define BH1750_PIN_SCL 14

#define DBG_USE_TESTCONFIG false

typedef enum greemon_state {
	_STATE_INITIAL = 0,
	_STATE_STARTUP = 1,
	_STATE_LOAD_CONFIG,
	_STATE_START_WEBSERVER,
	_STATE_WIFI_SCAN_RUNNING,
	_STATE_WIFI_SCAN_FINISHED,
	_STATE_RESTART,
	_STATE_IN_ERROR
} greemon_state_t;

LOCAL bool WifiScanComplete = false;
LOCAL bool init_done = false;

LOCAL uint32_t COUNTER = 0;
os_timer_t timerDHT;
os_timer_t i2ctest_timer;
os_timer_t deepsleep_timer;
os_timer_t earthprobe_timer;

void intr_handle_cb(void);
void enable_reset_interrupt(void);

void user_set_softap_config(void);
/******************************************************************************
 * FunctionName : test_config
 * Description  : Test routine for configuration file
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void test_config(void) {
	INFO("Generating Test-Configuration File");
	config_t test_cfg;	
	config_t loaded_cfg;

#if DBG_USE_TESTCONFIG
  gm_Data_t test_data[3];
  gm_APN_t apn_data;
  gm_Srv_t srv_data;

  srv_data.srv_address[0] = 172;
  srv_data.srv_address[1] = 141;
  srv_data.srv_address[2] = 1;
  srv_data.srv_address[3] = 2;
  srv_data.srv_port = 1337;

  os_memcpy(srv_data.id, "GMID213\0",sizeof(srv_data.token));
  os_memcpy(srv_data.token, "ABC-TOKEN\0",sizeof(srv_data.token));

  test_data[0].timestamp = 0xDEADBEEF;
  test_data[1].timestamp = 0xCAFEC0DE;
  test_data[2].timestamp = 0xF000BAAA;

	os_memcpy(apn_data.ssid,"MY_NEW_SSID\0",sizeof(apn_data.ssid));
	os_memcpy(apn_data.pass,"MY_NEW_PASS\0",sizeof(apn_data.pass));

	test_cfg.magic = CONFIG_MAGIC;	
	test_cfg.version = CONFIG_VERSION;
  test_cfg.random = 12345;
  test_cfg.storedData = 0;

	os_memcpy(test_cfg.gm_apn_data.pass,"MY_OLD_PASS\0",sizeof(test_cfg.gm_apn_data.pass));
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


void i2ctest(){
	DBG_OUT("=== READ BH1750 ===");
	myBH1750_Init(BH1750_PIN_SDA,BH1750_PIN_SCL);
	myBH1750_SendRequest(BH1750_ONE_TIME_HIGH_RES_MODE);
	os_printf("wait..\r\n");	
	os_delay_us(BH1750_CONVERSION_TIME*1000);
	myBH1750_ReadValue();
	DBG_OUT("===================");
}


void DHT_timerCallback(void *pArg) {
	error_t e;
	DBG_OUT("=== READ DHT22 ===");	
	e = DHT22_recieveData();
	if ( e != E_SUCCESS ) {
		ERR_OUT("ERROR CODE: %d\r\n",e); 
	} else {
		DBG_OUT("SUCCESS")	
	}
	GPIO_OUTPUT_SET(DHT_PIN, 1);
	DBG_OUT("==================");	
} // End of timerCallback



/*
* TESTING DEEP SLEEP MODE
* CONNECT RST -> GPIO16, needed for Wakeup Event
* Light-Sleep: 0.9 mA
* Deep-Sleep:   10 uA
*/
void deep_sleep_test_cb(void *pArg) {

	os_printf("Start\n");
	int res, i;
	for (i = 0; i < 2; i++) {
	  if (spi_flash_erase_sector(0x20+i) != 0) {
		  os_printf("Erase error");
	   }
	}
	static char text_to_write[] = "1234567812345678123456781234567812345678";
	static char buf[sizeof(text_to_write)];
	uint32_t addr = 0x3C000;
	for (i = 0; i < 100; i++) {
	  addr += sizeof(text_to_write) + 42;
	  if (spi_flash_write(addr, (uint32_t*)text_to_write, sizeof(text_to_write)-1) != 0) {    
		      os_printf("%d: Write error: addr: %p\n", i, addr);
		       continue;
	  }
	  memset(buf, 0, sizeof(buf));
	  if (spi_flash_read(addr, (uint32_t*)buf,  sizeof(text_to_write)-1) != 0) {
		  os_printf("%d: Read error: addr: %d\n", i, addr);
		  continue;
	  }
	  if (memcmp(text_to_write, buf, sizeof(buf)) != 0) {
		  os_printf("%d: addr: %p, In != Out\n", i, addr);
		  continue;
	  }
	  if (memcmp(text_to_write, buf, sizeof(buf)) == 0) {
		  os_printf("%d: addr: %p, In == Out\n", i, addr);
		  continue;
	  }
	}
	os_printf("Finish\n");

	os_printf("\r\n---DEEP SLEEP TEST---\r\n");	
	system_deep_sleep(10000000); // uint32_t time in us 
}

/******************************************************************************
 * FunctionName : wifi_scan_done
 * Description  : scan done callback
 * Parameters   :  arg: contain the aps information;
                          status: scan over status
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
wifi_scan_done(void *arg, STATUS status)
{
  uint8 ssid[33];
  char temp[128];

  if (status == OK)
  {
    struct bss_info *bss_link = (struct bss_info *)arg;
    bss_link = bss_link->next.stqe_next; //ignore the first one , it's invalid.

    while (bss_link != NULL)
    {
      os_memset(ssid, 0, 33);
      if (os_strlen(bss_link->ssid) <= 32)
      {
        os_memcpy(ssid, bss_link->ssid, os_strlen(bss_link->ssid));
      }
      else
      {
        os_memcpy(ssid, bss_link->ssid, 32);
      }
      os_printf("(%d,\"%s\",%d,\""MACSTR"\",%d)\r\n",bss_link->authmode, ssid,bss_link->rssi,MAC2STR(bss_link->bssid),bss_link->channel);
      bss_link = bss_link->next.stqe_next;
    }
  }
  else
  {
     os_printf("scan fail !!!\r\n");
  }

}

/******************************************************************************
 * FunctionName : wifi_scan
 * Description  : wifi scan, only can be called after system init done.
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
wifi_scan(void)
{
   if(wifi_get_opmode() == SOFTAP_MODE)
   {
     os_printf("ap mode can't scan !!!\r\n");
     return;
   }
   wifi_station_scan(NULL,wifi_scan_done);
}


/******************************************************************************
 * FunctionName : user_init
 * Description  : Entry point for the esp program. Provides initialization.
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
uint16_t earthprobe_adc_read(void) {
	uint16_t adcValue = system_adc_read();
	os_printf("adc = %d\n", adcValue);
	return adcValue;
} // End of user_init

/******************************************************************************
 * FunctionName : 
 * Description  : 
 * Parameters   : 
 * Returns      : 
*******************************************************************************/
void user_rf_pre_init(void) { }

/******************************************************************************
 * FunctionName : 
 * Description  : 
 * Parameters   : 
 * Returns      : 
*******************************************************************************/
void system_init_done(void){
	
	INFO("system starting up");
	system_get_flash_size_map();

	// Register interrupt handler
	ETS_GPIO_INTR_ATTACH(intr_handle_cb, NULL);
	enable_reset_interrupt();
	
  switch (config_init())
  {
    case CONFIG_INITIAL:
      INFO("generated initial configuration");
			INFO("loading default ap settings");
			user_set_softap_config();
			DBG_OUT("starting webserver");
			webserver_init(HTTP_PORT);
    break;
    case CONFIG_MAGIC_FOUND:
      INFO("configuration loaded");
			config_print(&global_cfg);
			// search for ap name in config
			// connect if found
			// else try again after a few seconds
			// startup webconfig if not connected
			INFO("scanning available access points");
			wifi_station_ap_number_set(20);
			//wifi scan has to after system init done.
			wifi_scan();
    break;
  }
	init_done = true;
  //DHT22_init();
	//os_timer_setfn(&timerDHT, DHT_timerCallback, NULL);
	//os_timer_arm(&timerDHT, 5000, 1);

	//DBG_OUT("INIT: BH1750 initialization");
	//os_timer_setfn(&i2ctest_timer, i2ctest, NULL);
	//os_timer_arm(&i2ctest_timer, 20000, 1);
}

/******************************************************************************
 * FunctionName : wifi_ipv4_setDefault
 * Description  : Sets the default values for the DHCP Server
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void wifi_ipv4_setDefault(void){
	struct ip_info info;

	wifi_softap_dhcps_stop();

	IP4_ADDR(&info.ip, 192, 168, 5, 1);
	IP4_ADDR(&info.gw, 192, 168, 5, 1);
	IP4_ADDR(&info.netmask, 255, 255, 255, 0);

	wifi_set_ip_info(SOFTAP_IF, &info);

	wifi_softap_dhcps_start();
}


/******************************************************************************
 * FunctionName : wifi_handle_event_cb
 * Description  : 
 * Parameters   : 
 * Returns      : 
*******************************************************************************/
void ICACHE_FLASH_ATTR
wifi_handle_event_cb(System_Event_t *evt)
{
	DBG_OUT(">>WiFi Event: %x", evt->event);
	switch (evt->event) {
		case EVENT_STAMODE_CONNECTED:
			DBG_OUT("Greemon: Connect to SSID %s. Channel: %d",
				evt->event_info.connected.ssid,
				evt->event_info.connected.channel);
		break;
		case EVENT_STAMODE_DISCONNECTED:
			DBG_OUT("Greemon: Disconnected from SSID %s. Reason: %d",
				evt->event_info.disconnected.ssid,
				evt->event_info.disconnected.reason);
		break;
		case EVENT_STAMODE_AUTHMODE_CHANGE:
			DBG_OUT("Greemon: Changed Authmode %d -> %d",
				evt->event_info.auth_change.old_mode,
				evt->event_info.auth_change.new_mode);
		break;
		case EVENT_STAMODE_GOT_IP:
			DBG_OUT("Greemon: Optained Address from DHCP Server."); 
			DBG_OUT(">> IP:" IPSTR "\tMask:" IPSTR "\tGW:" IPSTR,
				IP2STR(&evt->event_info.got_ip.ip),
				IP2STR(&evt->event_info.got_ip.mask),
				IP2STR(&evt->event_info.got_ip.gw));
        
        // We are starting the SNTP Timer, because we got an IP Address
        DBG_OUT("starting sntp client");
        user_sntp_start();
		break;
		case EVENT_SOFTAPMODE_STACONNECTED:
			DBG_OUT("New Client: " MACSTR " joined, AID = %d",
				MAC2STR(evt->event_info.sta_connected.mac),
				evt->event_info.sta_connected.aid);
		break;
		case EVENT_SOFTAPMODE_STADISCONNECTED:
			DBG_OUT("Old Client: " MACSTR " leaved, AID = %d",
				MAC2STR(evt->event_info.sta_disconnected.mac),
				evt->event_info.sta_disconnected.aid);
		break;
		case EVENT_MAX:
			ERR_OUT("MAX CONNECTIONS LIMIT REACHED");
		break;
		default:
		break;
	}
}

/******************************************************************************
 * FunctionName : user_set_softap_config
 * Description  : set SSID and password of ESP8266 softAP
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_set_softap_config(void)
{
	struct softap_config config;

	wifi_softap_get_config(&config); // Get config first.

	//TODO: LOAD Config from Flash Memory. 
	os_memset(config.ssid, 0, 32);
	os_memset(config.password, 0, 64);
	os_memcpy(config.ssid, "Greemon_default", 15);
	os_memcpy(config.password, "", 0);
	config.authmode = AUTH_OPEN;
	config.ssid_len = 0;		// or its actual length

	// how many stations can connect to ESP8266 softAP
	config.max_connection = HTTP_CONNECTION_MAX; 
	
	wifi_softap_set_config(&config);// Set ESP8266 softap config .
	
	wifi_ipv4_setDefault();
}

/*****************************************************************************
 * FunctionName	: reset_interrupt_cb
 * Description	: This is the interrupt handler, whenever the reset button is
 *		  Pressed. Checks if the button was pressed more than 
		  x seconds and sets the config to default values.
 * Parameters	: none
 * Returns	: none
 *****************************************************************************/
void intr_handle_cb(void){
	uint8_t i = 0;

	// Save current value for status register
	volatile uint32_t gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);	

	//Disable Interrup
  ETS_GPIO_INTR_DISABLE();
	
	// Clear status in interrupt register
	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status & BIT(0));	
	

	DBG_OUT("Interrupt detected.");
	
	// Count how long the button is pressed
	DBG_OUT("RESET - Detected reset");
  config_erase();

  // Reenable interrupt
  ETS_GPIO_INTR_ENABLE();
}

/******************************************************************************
 * FunctionName : enable_reset_interrupt
 * Description  : 
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void enable_reset_interrupt(){
	ETS_GPIO_INTR_DISABLE();
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
    	gpio_output_set(0, 0, 0, GPIO_ID_PIN(0));
    	gpio_register_set(GPIO_PIN_ADDR(0), GPIO_PIN_INT_TYPE_SET(GPIO_PIN_INTR_DISABLE)
          | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_DISABLE)
          | GPIO_PIN_SOURCE_SET(GPIO_AS_PIN_SOURCE));

    	//clear gpio status
    	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(0));

    	//enable interrupt
    	gpio_pin_intr_state_set(GPIO_ID_PIN(0), GPIO_PIN_INTR_NEGEDGE);
    	ETS_GPIO_INTR_ENABLE();
	INFO("interrupt enabled");
}

/******************************************************************************
 * FunctionName : user_init
 * Description  : Entry point for the esp program. Provides initialization.
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void) {
	system_init_done_cb(system_init_done);
	wifi_station_set_auto_connect(false);
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	wifi_set_event_handler_cb(wifi_handle_event_cb);
	INFO("waiting for wifi to start..")
} // End of user_init

