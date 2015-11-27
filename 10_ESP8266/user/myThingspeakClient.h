#ifndef __MY_THINGSPEAK_CLIENT_
#define __MY_THINGSPEAK_

#include "c_types.h"
#include "ip_addr.h"
#include "espconn.h"
#include "user_interface.h"
#include "osapi.h"
#include "myConfig.h"
#include "mem.h"
#include "myNTP.h"
#include "myGreemonStateMachine.h"

LOCAL struct espconn user_tcp_conn;
LOCAL struct _esp_tcp user_tcp;
ip_addr_t tcp_server_ip;

#define pheadbuffer "GET /%s HTTP/1.1\r\nUser-Agent: curl/7.37.0\r\nHost: %s.%s.%s.%s\r\n\r\n"
#define packet_size (2048)

// THINGSPEAK ONLY ACCEPTS VALUES EVERY 15 SECONDS 
#define USER_TS_TRANSMIT_INTERVAL (16*1000*1000)


void ICACHE_FLASH_ATTR user_ts_connect();
void ICACHE_FLASH_ATTR user_ts_send_data(gm_Data_t* pData);

bool ICACHE_FLASH_ATTR user_ts_is_connected(void);
bool ICACHE_FLASH_ATTR user_ts_is_waiting(void);

struct espconn * ICACHE_FLASH_ATTR user_ts_get_pConnection(void);

LOCAL void ICACHE_FLASH_ATTR user_tcp_connect_cb(void *arg);

LOCAL void ICACHE_FLASH_ATTR user_tcp_discon_cb(void *arg);
LOCAL void ICACHE_FLASH_ATTR user_tcp_recon_cb(void *arg, sint8 err);
LOCAL void ICACHE_FLASH_ATTR user_tcp_recv_cb(void *arg, char *pusrdata, unsigned short length);
LOCAL void ICACHE_FLASH_ATTR user_tcp_sent_cb(void *arg);

#endif
