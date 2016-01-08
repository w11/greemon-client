#include "myGreemonStateMachine.h"

#define GM_SERVER_TRANSMIT_INTERVAL (16*1000)

LOCAL greemon_state_t gm_in_state = _STATE_STARTUP;
LOCAL gm_error_counter = 0;

os_timer_t timer_server_wait;


/******************************************************************************
 * FunctionName : gm_state_get
 * Description  : 
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR 
timer_server_wait_cb(){
	os_timer_disarm(&timer_server_wait);
	if ( !gm_server_connect() ) gm_state_set(_STATE_DEEP_SLEEP);
}


/******************************************************************************
 * FunctionName : gm_state_get
 * Description  : 
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
greemon_state_t ICACHE_FLASH_ATTR 
gm_state_get(){
	return gm_in_state;
}

/******************************************************************************
 * FunctionName : gm_state_set
 * Description  : 
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR 
gm_state_set(greemon_state_t gm_new_state){
	INFO("\r\n[--- Changed state: %d -> %d ---]\r\n", gm_in_state, gm_new_state);
	gm_in_state = gm_new_state;
	gm_state_run(gm_in_state);
}

/******************************************************************************
 * FunctionName : gm_state_run
 * Description  : 
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
gm_state_run(greemon_state_t gm_run_state){
	switch (gm_run_state) {

// --------------------------------------------------------------------------

		case (_STATE_STARTUP):
		/* Initialize UART, 
		 * Register Callback for lwip
		 * set wifi handler
		 * do not connect to wifi at this point
		 */
			gm_error_counter = 0;
			if ( !gm_startup() ) gm_state_set(_STATE_ERROR);
			INFO("Waiting for init callback");
		break;

// --------------------------------------------------------------------------

		case (_STATE_INIT):	
		/* Fired after LWIP callback,
		 * Register interrupt for reset
		 * Load Configuration
		 */
			INFO("System initialized. Greemon startup...");
			if ( !gm_init() ) gm_state_set(_STATE_ERROR);
		break;

// --------------------------------------------------------------------------

		case (_STATE_RESET):

		break;

// --------------------------------------------------------------------------

		case (_STATE_ERROR):
		/* 
		 * this state only happens if there was an error
		 */
			INFO("Wooooooops... there was an ERROR.");
			INFO("restart is the only exit")
		break;

// --------------------------------------------------------------------------

		case (_STATE_INITIALIZED):
			/* System is initialized,
			 * we can load the config before we startup our greemon system
			*/
			if ( !gm_load_config() ) gm_state_set(_STATE_ERROR);
		break;

// --------------------------------------------------------------------------

		case (_STATE_CONFIG_USER):
		/*	magic word has been found in the flash data,
		 *	we start with the user configuration
		 *	but first read sensor data
		 */
			INFO("user config has been loaded");
			if ( !gm_read_sensors() ) gm_state_set(_STATE_ERROR);
			gm_state_set(_STATE_SENSOR_DATA_SAVED);
		break;

// --------------------------------------------------------------------------

		case (_STATE_CONFIG_DEFAULT):
		/*	magic word has not been found in the flash data,
		 *	we start with the default configuration
		 */
			INFO("loaded standard configuration");
			INFO("webserver starting");
			if ( !gm_webserver_start() ) gm_state_set(_STATE_ERROR);
			gm_state_set(_STATE_WEBSERVER_RUNNING);
		break;

// --------------------------------------------------------------------------

		case (_STATE_CONFIG_RECIEVED):
		INFO("config recieved - please restart the controller");
		break;

// --------------------------------------------------------------------------

		case (_STATE_WEBSERVER_PARSING):
			
		break;

// --------------------------------------------------------------------------

		case (_STATE_WEBSERVER_RUNNING):
		/* Webserver is running as long as 
		 * we did not recieve a new configuration
		 */
			INFO("webserver running");
			INFO("waiting for HTTP requests");

// TEST FOR BH1750
//TODO: DELETEME
//		i2ctest();	


		break;

// --------------------------------------------------------------------------

		case (_STATE_SAVED_CONFIGURATION):
			INFO("configuration has been saved");
			wifi_station_disconnect();
			gm_webserver_stop();
			INFO("restarting");
			system_restart();
			while(1) os_delay_us(100);
		break;

// --------------------------------------------------------------------------

		case (_STATE_TIME_RECIEVED):
			INFO("start connecting with server");
			if ( !gm_server_connect() ) gm_state_set(_STATE_DEEP_SLEEP);
		break;

// --------------------------------------------------------------------------

		case (_STATE_SENSOR_DATA_SAVED):
			INFO("Sensor data saved");
			if ( !gm_connect_ap() ) gm_state_set(_STATE_ERROR);
			INFO("connecting.. waiting for wifi-handler");
		break;

// --------------------------------------------------------------------------

		case (_STATE_WIFI_CONNECTED):
		/* Connected to AP and recieved an IP Adress
		 * Starting SNTP Client
		 */
			if ( !gm_sntp_start() ) gm_state_set(_STATE_ERROR);
			user_sntp_wait_valid_time();
			INFO("recieved time from server");
			// NEXT STATE ->_STATE_TIME_RECIEVED
			gm_state_set(_STATE_TIME_RECIEVED);
		break;

// --------------------------------------------------------------------------

		case (_STATE_CONNECTION_CLOSED):
		
		break;

// --------------------------------------------------------------------------

		case (_STATE_CONNECTION_OPEN):
			INFO("sending data..");
			if ( !gm_server_send_data() ) gm_state_set(_STATE_ERROR);
		break;

// --------------------------------------------------------------------------

		case (_STATE_CONNECTION_TRANSMITTED):
			INFO("transmitted data successfully")
			// A dataset has been transmitted. 
			// Remove from the flash happend automatically when we popped the data
			// Start 16 secs timer because ts only acceps data every 15 secs
			if ( 0 == global_cfg.storedData ) {
				INFO("all data has been sent");
				gm_state_set(_STATE_DEEP_SLEEP);
			} else {
				INFO("still data(%d) remaining, starting timer", global_cfg.storedData);
				os_timer_setfn(&timer_server_wait,timer_server_wait_cb,NULL);
				os_timer_arm(&timer_server_wait,GM_SERVER_TRANSMIT_INTERVAL,1);
			}
		break;

// --------------------------------------------------------------------------

		case (_STATE_DEEP_SLEEP):
			INFO("Finished work. Deep sleep, yay.");
			system_deep_sleep(60*1000*1000); // uint32_t time in us 
		break;

// --------------------------------------------------------------------------
	}
}
