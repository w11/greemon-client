#ifndef __MYNTP_H__
#define __MYNTP_H__
/* 
 * Implementation for GREEMON Client
 * David Huber
 */

#include "c_types.h"
#include "user_interface.h"
#include "osapi.h"

#define USER_NTP_TIMEZONE +1
#define USER_NTP_SRV1 "0.europe.pool.ntp.org"
#define USER_NTP_SRV2 "1.europe.pool.ntp.org"
#define USER_NTP_SRV3 "2.europe.pool.ntp.org"

#define USER_NTP_POLLING_INTERVAL 10000
#define USER_NTP_MAX_TRIES 5



LOCAL os_timer_t user_timer_sntp;

uint32_t ICACHE_FLASH_ATTR user_sntp_latest_timestamp(void);
bool ICACHE_FLASH_ATTR user_ntp_test(void);
void ICACHE_FLASH_ATTR user_sntp_wait_valid_time(void);
uint32_t ICACHE_FLASH_ATTR user_sntp_latest_timestamp(void); 
void ICACHE_FLASH_ATTR user_sntp_timer_callback(void);
void ICACHE_FLASH_ATTR user_sntp_start(void);
bool ICACHE_FLASH_ATTR user_sntp_valid(void);

#endif
