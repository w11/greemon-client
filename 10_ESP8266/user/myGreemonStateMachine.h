#ifndef __GREEMON_STATE_MACHINE_H 
#define	__GREEMON_STATE_MACHINE_H

#include "c_types.h"
#include "osapi.h"
#include "os_type.h" // Timer, Signals
#include "user_config.h" // DBG_OUT, ERR_OUT, et
#include "ip_addr.h"
#include "espconn.h"
#include "myWifiServer.h"

typedef enum greemon_state {
	_STATE_STARTUP = 0,
	_STATE_INIT = 1,
	_STATE_RESET,
	_STATE_ERROR,
	_STATE_INITIALIZED,
	_STATE_CONFIG_USER,
	_STATE_CONFIG_DEFAULT,
	_STATE_CONFIG_RECIEVED,
	_STATE_WEBSERVER_PARSING,
	_STATE_WEBSERVER_RUNNING,
	_STATE_SAVED_CONFIGURATION,
	_STATE_TIME_RECIEVED,
	_STATE_SENSOR_DATA_SAVED,
	_STATE_WIFI_CONNECTED,
	_STATE_CONNECTION_CLOSED,
	_STATE_CONNECTION_OPEN,
	_STATE_CONNECTION_TRANSMITTED,
	_STATE_DEEP_SLEEP,
} greemon_state_t;

greemon_state_t ICACHE_FLASH_ATTR gm_state_get();
void ICACHE_FLASH_ATTR gm_state_set(greemon_state_t gm_new_state);
LOCAL void ICACHE_FLASH_ATTR gm_state_run(greemon_state_t gm_run_state);

#endif
