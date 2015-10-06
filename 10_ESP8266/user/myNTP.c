#include "myNTP.h"

/******************************************************************************
 * FunctionName : user_ntp_test
 * Description  : testcase
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
bool ICACHE_FLASH_ATTR user_ntp_test(void) {
#ifdef DEV_VERSION
  // Connect to wifi if theres no connection
    
  // check for timestamp
  
  // get time
#endif
}

/******************************************************************************
 * FunctionName : user_sntp_timer_callback
 * Description  : checks if the sntp client has recieved a value
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_sntp_timer_callback(void){
  DBG_OUT("timer callback for ntp client");
  // Timer callback for sntp client
  // Check for time
  user_ntp_timestamp = sntp_get_current_timestamp();
  user_ntp_valid_time = false;
  user_ntp_tries = 0;

  if (0 == user_ntp_timestamp)
  {
    // Did not recieve timestamp yet
    if ( USER_NTP_MAX_TRIES <=  user_ntp_tries) {
      // break, because we tried for too long
      ERR_OUT("error recieving time from ntp server, after %d retries", user_ntp_tries);
      os_timer_disarm(&user_timer_sntp);
    } else {
      // next try
      user_ntp_tries =+ 1;
      os_timer_disarm(user_timer_sntp);
      os_timer_arm(&user_timer_sntp, USER_NTP_POLLING_INTERVAL, false);
    }
  } else {
    // timestamp recieved
    DBG_OUT("successfully recieved time from ntp server");
    user_ntp_valid_time = true;
    user_ntp_tries = 0;
    os_timer_disarm(&user_timer_sntp);
  }
}

/******************************************************************************
 * FunctionName : user_sntp_start
 * Description  : Feeds the arguments and starts the sntpclient
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
bool ICACHE_FLASH_ATTR 
user_sntp_start(void) {
  user_ntp_tries = 0;
  // Set the Server-Names of the NTP Server
	DBG_OUT("set ntp server names");
  sntp_setservername(0,USER_NTP_SRV1);
	sntp_setservername(1,USER_NTP_SRV2);
	sntp_setservername(2,USER_NTP_SRV3);
  DBG_OUT("set timezone");
  sntp_set_timezone(USER_NTP_TIMEZONE); // UTC+1 MID EUROPE
	// Start SNTP Client.
	// This may need some time. Just call the function to get the
	// time periodically
  // arm the timer
  DBG_OUT("start timer");
  //start ntp
	sntp_init();
  os_timer_setfn(&user_timer_sntp, user_sntp_timer_callback, NULL);
  os_timer_disarm(&user_timer_sntp);
  os_timer_arm(&user_timer_sntp, USER_NTP_POLLING_INTERVAL, false);
}
