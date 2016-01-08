#include "myThingspeakClient.h"
#include "myConfig.h"



LOCAL bool waiting_ack = false;
LOCAL bool connected = false;

uint8_t failures = 0;

struct espconn * 
ICACHE_FLASH_ATTR user_ts_get_pConnection(void){
	return &user_tcp_conn;
}

bool ICACHE_FLASH_ATTR user_ts_is_connected(void)
{
	return connected;
}
bool ICACHE_FLASH_ATTR user_ts_is_waiting(void)
{
	return waiting_ack;
}


/******************************************************************************
 * FunctionName : user_ts_connect
 * Description  : etablishes connection to thingspeak server
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR 
user_ts_connect() {
	connected = true;

	failures = 0;

	user_tcp_conn.proto.tcp = &user_tcp;
  user_tcp_conn.type = ESPCONN_TCP;
  user_tcp_conn.state = ESPCONN_NONE;

	//const char esp_tcp_server_ip[4];

/* = {
		global_cfg.gm_auth_data.srv_address[0], 
		global_cfg.gm_auth_data.srv_address[1],
		global_cfg.gm_auth_data.srv_address[2], 
		global_cfg.gm_auth_data.srv_address[3]
	}; // remote IP of TCP server
*/
	DBG_OUT("loading remote ip");
	IP4_ADDR(&tcp_server_ip, 
		global_cfg.gm_auth_data.srv_address[0], 
		global_cfg.gm_auth_data.srv_address[1], 
		global_cfg.gm_auth_data.srv_address[2],
		global_cfg.gm_auth_data.srv_address[3]);

	// uint8 remote_ip[4];
	//os_memcpy(user_tcp_conn.proto.tcp->remote_ip, &tcp_server_ip.addr, sizeof(uint32_t));
	user_tcp_conn.proto.tcp->remote_ip[0] = ip4_addr1(&tcp_server_ip.addr);
	user_tcp_conn.proto.tcp->remote_ip[1] =	ip4_addr2(&tcp_server_ip.addr);
	user_tcp_conn.proto.tcp->remote_ip[2] =	ip4_addr3(&tcp_server_ip.addr);
	user_tcp_conn.proto.tcp->remote_ip[3] =	ip4_addr4(&tcp_server_ip.addr);

	DBG_OUT("REMOTE IP: %u.%u.%u.%u", 
		user_tcp_conn.proto.tcp->remote_ip[0], 
		user_tcp_conn.proto.tcp->remote_ip[1], 
		user_tcp_conn.proto.tcp->remote_ip[2], 
		user_tcp_conn.proto.tcp->remote_ip[3]);

	user_tcp_conn.proto.tcp->remote_port = 80;  // remote port

	user_tcp_conn.proto.tcp->local_port = espconn_port(); //local port of ESP8266

	espconn_regist_connectcb(&user_tcp_conn, user_tcp_connect_cb); // register connect callback
	espconn_regist_reconcb(&user_tcp_conn, user_tcp_recon_cb); // register reconnect callback as error handler

	INFO("connecting...");
	espconn_connect(&user_tcp_conn);
}


/******************************************************************************
 * FunctionName : user_tcp_connect_cb
 * Description  : A new incoming tcp connection has been connected.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_tcp_connect_cb(void *arg)
{
	struct espconn *pespconn = &user_tcp_conn;
	INFO("--- CONNECTION ETABLISHED! ---\r\n");

	espconn_regist_recvcb(pespconn, user_tcp_recv_cb);
	espconn_regist_sentcb(pespconn, user_tcp_sent_cb);
	espconn_regist_disconcb(pespconn, user_tcp_discon_cb);
	gm_state_set(_STATE_CONNECTION_OPEN);
}

/******************************************************************************
 * FunctionName : user_tcp_discon_cb
 * Description  : disconnect callback.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_tcp_discon_cb(void *arg)
{
	INFO("tcp disconnect succeed !!! \r\n");
	connected = false;
	gm_state_set(_STATE_CONNECTION_TRANSMITTED);
}

/******************************************************************************
 * FunctionName : user_tcp_recon_cb
 * Description  : reconnect callback, error occured in TCP connection.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_tcp_recon_cb(void *arg, sint8 err)
{	
	//error occured , tcp connection broke. user can try to reconnect here.
	INFO("error code %d !!! \r\n",err);
}

/******************************************************************************
 * FunctionName : user_tcp_recv_cb
 * Description  : receive callback.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_tcp_recv_cb(void *arg, char *pusrdata, unsigned short length)
{
	//received some data from tcp connection
	INFO("Recieved Data: %s \r\n", pusrdata);
	waiting_ack = false;

	espconn_disconnect(&user_tcp_conn);
	connected = false;
}
/******************************************************************************
 * FunctionName : user_tcp_sent_cb
 * Description  : data sent callback.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_tcp_sent_cb(void *arg)
{
	//data sent successfully
	INFO("send success\r\n");
	waiting_ack = true;
}

/******************************************************************************
 * FunctionName : user_esp_platform_hsent
 * Description  : Processing the application data and sending it to the host
 * Parameters   : pespconn -- the espconn used to connetion with the host
 * Returns      : none
*******************************************************************************/
bool ICACHE_FLASH_ATTR
user_ts_send_data(gm_Data_t* pData)
{
		struct espconn *pespconn = &user_tcp_conn;
		char* pSendBuffer = NULL;
		char* pGenBuffer = NULL;
		char* pContBuffer = NULL;
		char timestamp[100];
		uint32_t err_code = 0;
		uint16_t content_length = 0;

		if (NULL == pData) {
			ERR_OUT("no data to send");
			return false;
		}

		pSendBuffer = (char *)os_zalloc(packet_size);
		pGenBuffer = (char *)os_zalloc(255);
		pContBuffer = (char *)os_zalloc(255);

		if (NULL == pSendBuffer ||
				NULL == pGenBuffer	||
				NULL == pContBuffer) {
			ERR_OUT("out of memory");
			return false;		
		}
		os_sprintf(timestamp, epoch_to_str(pData->timestamp));				
		DBG_OUT("datetime=%s",timestamp);


		os_sprintf(pSendBuffer,"POST /update.json?= HTTP/1.1\r\n");
		os_sprintf(pGenBuffer, "Host: api.thingspeak.com\r\n"
/*%d.%d.%d.%d\r\n",
				user_tcp_conn.proto.tcp->remote_ip[0],
				user_tcp_conn.proto.tcp->remote_ip[1],
				user_tcp_conn.proto.tcp->remote_ip[2],
				user_tcp_conn.proto.tcp->remote_ip[3]
*/
);
		os_strcat(pSendBuffer,pGenBuffer);
		os_strcat(pSendBuffer,"Connection: close\r\n");
		os_sprintf(pGenBuffer,"X-THINGSPEAKAPIKEY: %s\r\n", global_cfg.gm_auth_data.token);
		os_strcat(pSendBuffer,pGenBuffer);
		os_strcat(pSendBuffer,"Content-Type: application/x-www-form-urlencoded\r\n");
		content_length = os_sprintf(pContBuffer,"field1=%d&field2=%d",
				pData->dht22_moisture,
				pData->dht22_temperature
				//pData->bh1750_light,
				//pData->adc_moisture,
				//timestamp
);
		os_sprintf(pGenBuffer, "Content-Length: %d\r\n\r\n", content_length);
		os_strcat(pSendBuffer, pGenBuffer);
		os_strcat(pSendBuffer, pContBuffer);

/*
				pData->dht22_moisture,
				pData->dht22_temperature,
				pData->bh1750_light,
				pData->adc_moisture,
				timestamp,

*/

		DBG_OUT("pSendBuffer:\"%s\",characters: %u",pSendBuffer,os_strlen(pSendBuffer));

		err_code = espconn_send(pespconn, pSendBuffer, os_strlen(pSendBuffer));

		switch (err_code) {
			case ESPCONN_OK: 
				INFO("sending..");
				break;
			default:
				failures++;
				ERR_OUT("Error-Code: %d", err_code);
				return false;
				//config_push_data(&global_cfg, pData);
				//INFO("restored data");
			break;
		}

		os_free(pSendBuffer);
		os_free(pGenBuffer);
		os_free(pContBuffer);
		//espconn_disconnect(&user_tcp_conn);
		return true;
}

/******************************************************************************
 * FunctionName : user_esp_platform_sent
 * Description  : Processing the application data and sending it to the host
 * Parameters   : pespconn -- the espconn used to connetion with the host
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_ts_send_dataset(struct espconn *pespconn, gm_Data_t* data, uint16_t num)
{
	uint16_t i;
	for (i = 0; i < num; i++) {

	}
}
