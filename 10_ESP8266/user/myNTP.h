/* Implementation for GREEMON Client
 * David Huber
 */

#define USER_NTP_TIMEZONE +1
#define USER_NTP_SRV1 "0.europe.pool.ntp.org"
#define USER_NTP_SRV2 "1.europe.pool.ntp.org"
#define USER_NTP_SRV3 "2.europe.pool.ntp.org"



/******************************************************************************
 * FunctionName : user_sntp_start
 * Description  : Feeds the arguments and starts the sntpclient
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
bool ICACHE_FLASH_ATTR 
user_sntp_start(void) {
	// Set the Server-Names of the NTP Server
	sntp_setservername(0,USER_NTP_SRV1);
	sntp_setservername(1,USER_NTP_SRV2);
	sntp_setservername(2,USER_NTP_SRV3);
  sntp_set_timezone(USER_NTP_TIMEZONE); // UTC+1 MID EUROPE
	// Start SNTP Client.
	// This may need some time. Just call the function to get the
	// time periodically
	sntp_init();
}


