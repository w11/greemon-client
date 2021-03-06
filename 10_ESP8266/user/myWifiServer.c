#include "myWifiServer.h"

/******************************************************************************
 * FunctionName : user_esp_platform_check_ip
 * Description  : check whether get ip addr or not. 
 *								Function is called by timer test_station_ip.
 *								Timer is startet by user_set_station_config.
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_esp_platform_check_ip(void)
{
    struct ip_info ipconfig;

   //disarm timer first
    os_timer_disarm(&test_station_ip);

   //get ip info of ESP8266 station
    wifi_get_ip_info(STATION_IF, &ipconfig);
    if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0) {
      DBG_OUT("DHCP-Client got IP-Address");
    } else {
        if ((wifi_station_get_connect_status() == STATION_WRONG_PASSWORD ||
                wifi_station_get_connect_status() == STATION_NO_AP_FOUND ||
                wifi_station_get_connect_status() == STATION_CONNECT_FAIL)) {            
        	DBG_OUT("Connection failed");
        } else {
           //re-arm timer to check ip
            os_timer_setfn(&test_station_ip, (os_timer_func_t *)user_esp_platform_check_ip, NULL);
            os_timer_arm(&test_station_ip, 100, 0);
        }
    }
}


/******************************************************************************
 * FunctionName : user_set_station_config
 * Description  : set the router info which ESP8266 station will connect to
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_set_station_config(gm_APN_t* apn)
{
	info("connect to access point");
	wifi_station_disconnect();

	// save to config
	config_write_apn(apn);

	// Create the struc the esp needs
	struct station_config stationConf;
  // need not mac address
  stationConf.bssid_set = 0;
   
	//Set ap settings
	os_memcpy(&stationConf.ssid, apn->ssid, CONFIG_SIZE_SSID);
	os_memcpy(&stationConf.password, apn->pass, CONFIG_SIZE_PASS);
	DBG_OUT("Loading ssid and password into esp");
	wifi_station_set_config(&stationConf);

	if (true==wifi_station_connect()) {
		DBG_OUT("Connected to AP %s",&stationConf.ssid);
	} else {
		DBG_OUT("Not Connected to AP %s, with password: %s",&stationConf.ssid, &stationConf.password);
	}

   //set a timer to check whether got ip from router succeed or not.
   os_timer_disarm(&test_station_ip);
   os_timer_setfn(&test_station_ip, (os_timer_func_t *)user_esp_platform_check_ip, NULL);
   os_timer_arm(&test_station_ip, 100, 0);
}


/******************************************************************************
 * FunctionName : webserver_parse_post_content_length
 * Description  : parse the content and return an array
 * Parameters   : pusrdata - Pointer to the sent content
 *								length - recieved data length
 * Returns      : bool - config could be saved or not
*******************************************************************************/
uint16_t ICACHE_FLASH_ATTR 
webserver_parse_post_content_length(char *pusrdata, unsigned short *pLength)
{
	uint16_t 	sp = 0; // start parsing
	uint8_t 	noc = 0; // number of characters to copy
	uint16_t 	content_length = 0;
	char*			pLenTemp;	
	uint16_t	post_length = *pLength;
	uint16_t 	i = 0; 
	INFO("reading content length");

	// FIND CONTENT LENGTH
	while ( i < post_length )
 	{
		if (0 == os_strncmp(pusrdata+i,"Content-Length", 14 )) {
			sp = i+16; // skip "Content-Length: "
			noc = 0;
			// Find \r\n
			if ('\r' == pusrdata[sp+1] && '\n' == pusrdata[sp+2]) {
				noc = 1;
				break;
			} else
			if ('\r' == pusrdata[sp+2] && '\n' == pusrdata[sp+3]) {
				noc = 2;
				break;
			} else
			if ('\r' == pusrdata[sp+3] && '\n' == pusrdata[sp+4]) {
				noc = 3;
				break;
			} else {
				ERR_OUT("Did not find \r\n")
				// else \r\n comes toooo late for us or never
				return false;
			}
		}
		i++;
	}

	// found \r\n after value of noc characters
	pLenTemp = (char*)os_zalloc(sizeof(char) * noc+1);
	if (NULL == pLenTemp) return false; // TODO return Code
	//DBG_OUT("number of chars: %d", noc);
	os_memcpy(pLenTemp,pusrdata+sp,noc);
	pLenTemp[noc] = '\0'; // add termination
	//DBG_OUT("pLenTemp: %s", pLenTemp);
	content_length = atoi(pLenTemp);
	//DBG_OUT("length: %u",content_length);

	os_free(pLenTemp);
	return content_length;
}


/******************************************************************************
 * FunctionName : webserver_parse_post_content
 * Description  : parse the content and return an array
 * Parameters   : pusrdata - Pointer to the sent content
 *								length - recieved data length
 * Returns      : bool - config could be saved or not
*******************************************************************************/
bool ICACHE_FLASH_ATTR 
webserver_parse_post_content(char *pusrdata, unsigned short *pLength)
{
	bool exitStatus = false;

	// These settings are going to be filled
	gm_Base_t Base_data;
	gm_Srv_t	Srv_data;
	gm_APN_t	APN_data;

	uint16_t	post_length = *pLength;
	static ip_addr_t ipaddr_temp;

	const char 	delimiter = '&';
	uint32_t*	pKeyValuePair = NULL;
	uint8_t 	countKeyValuePairs = 0;
	uint16_t 	pos = 0;
	uint16_t  temp_Port = 80;
	uint8_t		i; // for splitting ip

	char* 		pPost_content;
	uint16_t 	clen = 0;

	INFO("start parsing");
	//DBG_OUT(pusrdata);

	clen = webserver_parse_post_content_length(pusrdata, pLength);
	INFO("content-length: %u",clen);
	if (0 == clen) return false; // TODO err code.

	// 2 		Allocate Memory for x Size of Content Length
	if (clen > HTTP_POST_BUFFER) return false; // TODO err code.
	pPost_content = (char*)os_zalloc(sizeof(char) * clen+1);
	if (NULL != pPost_content) {
		// 4 		Copy Characters into allocated memory
		os_memcpy(pPost_content,pusrdata+post_length-clen,clen);
		pPost_content[clen] = '\0'; // add termination

		INFO("content: %s", pPost_content);

		// Read Parameter and Values
		// Count number of Key and Value pairs
		pos = 0; // current char
		while (pos < clen) {
			if (delimiter == pPost_content[pos]) countKeyValuePairs++;
			++pos;
		}
		countKeyValuePairs++; // THIS IS A BUGFIX /TODO EVALUATE
		//DBG_OUT("found %u Key-Pairs", countKeyValuePairs+1);

		// Allocate pointer array
		pKeyValuePair = (uint32_t*)os_zalloc((sizeof(uint32_t)) * (countKeyValuePairs));
		INFO("allocated memory for %u Pointer",(countKeyValuePairs));

		if (NULL == pKeyValuePair) return false; // TODO err code

		// now split the c string - means we change & to \0		
		pos = 0;
		countKeyValuePairs = 0;
		pKeyValuePair[countKeyValuePairs] = (uint32_t)pPost_content+pos; // first key-value-pair
		countKeyValuePairs++;		
		while (pos < clen) {
			if (delimiter == pPost_content[pos]) {
				pPost_content[pos] = '\0';
				// copy pointer to array.
				pKeyValuePair[countKeyValuePairs] = (uint32_t)pPost_content+pos+1; 
				countKeyValuePairs++;
			}
			pos++;
		}		

		// Find Server Configuration
		for (pos = 0; pos < countKeyValuePairs; pos++){
		system_soft_wdt_feed();
			// Output the string from the pointer of the array
			// DBG_OUT("KEYVALUE: %s", pKeyValuePair[pos] );	
			// parse content and map to config
			if (0 == os_strncmp(pKeyValuePair[pos],"ip=",3)) {
				//DBG_OUT("Found IP: %s",pKeyValuePair[pos]+3);
				if (1 == ipaddr_aton(pKeyValuePair[pos]+3, &ipaddr_temp) ) {
					INFO("ip extracted");
					//os_memcpy(&Srv_data.srv_address,&ipaddr_temp,sizeof(uint32_t));
					Srv_data.srv_address[0] = ip4_addr1(&ipaddr_temp.addr);
					Srv_data.srv_address[1] = ip4_addr2(&ipaddr_temp.addr);
					Srv_data.srv_address[2] = ip4_addr3(&ipaddr_temp.addr);
					Srv_data.srv_address[3] = ip4_addr4(&ipaddr_temp.addr);
				}	else {
					ERR_OUT("extracting IP address failed")
				}				
			} else if (0 == os_strncmp(pKeyValuePair[pos],"ssid=",5)) {
				//DBG_OUT("Found ssid: %s",pKeyValuePair[pos]+5);
				os_memcpy(APN_data.ssid,pKeyValuePair[pos]+5,CONFIG_SIZE_SSID);
				uint8_t j=0;
				for (j=0; j < os_strlen(APN_data.ssid); j++)
				{
					if ('+' == APN_data.ssid[j]) APN_data.ssid[j] = ' ';
				}

			} else if (0 == os_strncmp(pKeyValuePair[pos],"token=",6)) {
				//DBG_OUT("Found token: %s",pKeyValuePair[pos]+6);
				os_memcpy(Srv_data.token,pKeyValuePair[pos]+6,CONFIG_SIZE_TKN);

			} else if (0 == os_strncmp(pKeyValuePair[pos],"pass=",5)) {
				//DBG_OUT("Found pass: %s",pKeyValuePair[pos]+5);
				os_memcpy(APN_data.pass,pKeyValuePair[pos]+5,CONFIG_SIZE_PASS);

			} else if (0 == os_strncmp(pKeyValuePair[pos],"port=",5)) {
				//DBG_OUT("Found port: <%s>",pKeyValuePair[pos]+5);
				temp_Port = (uint16_t)atoi(pKeyValuePair[pos]+5);
				//DBG_OUT("Temp Port: <%u>",temp_Port);
				//Srv_data.srv_port = (uint16_t)temp_Port;
				os_memcpy(&Srv_data.srv_port,&temp_Port,sizeof(uint16_t));
			}
		}		

		//DBG_OUT("ip: %s", (char*)ipaddr_ntoa( ipaddr_temp.addr ));

		DBG_OUT("APN Data ADDR: %x",&APN_data);
		DBG_OUT("SRV Data ADDR: %x",&Srv_data);
		DBG_OUT("Saved Address: %u.%u.%u.%u",Srv_data.srv_address[0],Srv_data.srv_address[1],Srv_data.srv_address[2],Srv_data.srv_address[3]);
		DBG_OUT("Saved Port: %u",		Srv_data.srv_port);
		DBG_OUT("Saved Token: %s",	Srv_data.token);
		DBG_OUT("Saved SSID: %s",		APN_data.ssid);
		DBG_OUT("Saved Pass: %s",		APN_data.pass);

		DBG_OUT("Trying to save into config");

		if ( config_write_srv(&Srv_data) && config_write_apn(&APN_data) ) INFO("successfully saved parsed config");
		

		// Free Heap
		INFO("Free Content");
		os_free(pPost_content);
		INFO("Free Key Value")
		os_free(pKeyValuePair);
		exitStatus = true;

	} else {
		ERR_OUT("malloc failed");
		exitStatus = false; // TODO return Code	
	}
	
	// Everything went ok
	INFO("parsing success");
	return exitStatus;		

}


/******************************************************************************
 * FunctionName : webserver_recv
 * Description  : Processing the received data from the server
 * Parameters   : arg -- Additional argument to pass to the callback function
 *                pusrdata -- The received data (or NULL when the connection has been closed!)
 *                length -- The length of received data
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
webserver_recv(void *arg, char *pusrdata, unsigned short length)
{
	struct espconn *pClient = arg;
	uint8_t responseBuffer[HTTP_SEND_BUFFER]; //Because the StyleSheet is this large
	uint16_t l=0;
	uint8_t headerParser = 0;
	bool validRequest = false;
	uint16_t charstocopy = 0;
	bool save_config_success = false;

	//TODO REMOVE TEST DATA
	gm_APN_t apn_test;

	INFO("Recieved Message from Client");

// PARSE GET  COMMAND
	switch (pusrdata[0]) {
		case 'G': 
			DBG_OUT("Parsing - Found GET");
			// Parse Header	
			if ( 0 == os_strncmp(pusrdata,"GET / ", 6 ) )
			{
			// Insert the contents
				l = os_sprintf(responseBuffer, 
"HTTP/1.1 200 OK\r\n\
Content-Type: text/html\r\n\
Server: Greemon/0.0.1\r\n\
Content-Length: %u\r\n\
Connection: Keep-Alive\r\n\r\n\
%s",ARRAY_SIZE(indexPage)-1,indexPage);
			} else if (os_strncmp(pusrdata,"GET /about.html ", 16 )==0) //GET /about.html HTTP/1.1
			{
				// Insert the contents
				l = os_sprintf(responseBuffer, 
"HTTP/1.1 200 OK\r\n\
Content-Type: text/html\r\n\
Server: Greemon/0.0.1\r\n\
Content-Length: %u\r\n\
Connection: Keep-Alive\r\n\r\n\
%s",ARRAY_SIZE(aboutPage)-1,aboutPage);
			} else if (os_strncmp(pusrdata,"GET /config.html ", 17 )==0)
			{
				//TODO: DOCUMENTATION application/x-www-form-urlencoded
				l = os_sprintf(responseBuffer, 
"HTTP/1.1 200 OK\r\n\
Content-Type: text/html\r\n\
Server: Greemon/0.0.1\r\n\
Accept: application/x-www-form-urlencoded\r\n\
Content-Length: %u\r\n\
Connection: Keep-Alive\r\n\r\n\
%s",ARRAY_SIZE(configPage)-1,configPage);	
			} else if (os_strncmp(pusrdata,"GET /style.css ", 15 )==0)
			{
				l = os_sprintf(responseBuffer, 
"HTTP/1.1 200 OK\r\n\
Content-Type: text/css\r\n\
Server: Greemon/0.0.1\r\n\
Content-Length: %u\r\n\
Connection: Keep-Alive\r\n\r\n\
%s",ARRAY_SIZE(styleSheet)-1,styleSheet);
			} else {
				// FOR OTHERS: Error Code: 404 - Not found 	
				DBG_OUT("Not Implemented :(");
				l = os_sprintf(responseBuffer, httpNotFoundHeader); 
			}
		break;
		case 'P': 
			DBG_OUT("Parsing - Found POST");
      save_config_success == webserver_parse_post_content(pusrdata,&length);
// Insert the contents of the saving Page
			os_bzero(&responseBuffer, sizeof(responseBuffer));
			l = os_sprintf(responseBuffer, 
"HTTP/1.1 200 OK\r\n\
Content-Type: text/html\r\n\
Server: Greemon/0.0.1\r\n\
Content-Length: %u\r\n\
Connection: Keep-Alive\r\n\r\n\
%s",ARRAY_SIZE(savePage)-1,savePage);
		break;			
		default: 
				DBG_OUT("Parsing - Found NOT IMPLEMENTED");
				// FOR OTHERS: Error Code: 501 - Not Implemented 	
				l = os_sprintf(responseBuffer, httpNotImplementedHeader); 
		break;
	}


	if ((0 != l) || (NULL != responseBuffer)) { // Check if theres something in the buffer to send.
		INFO("Sending response...");
		INFO("free heap: %u Bytes",system_get_free_heap_size());

		
		if (0==espconn_sent((struct espconn*)arg, (uint8_t*)responseBuffer, l) ) {
			DBG_OUT("Response sent!");
		} else {
			DBG_OUT("Something went wrong :(");
		}
	} else {
		INFO("empty transmit buffer");
	}

	espconn_disconnect(pClient);

	if (save_config_success) gm_state_set(_STATE_SAVED_CONFIGURATION);
}


/******************************************************************************
 * FunctionName : webserver_print_clients
 * Description  : prints the array of connections with its data
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
webserver_print_clients(){
#ifdef WEBSRV_DEBUG_PRINT_CLIENTS
	DBG_OUT("=== CONNECTION POOL STATUS ===");	
	uint8_t i = 0;
	for (i=0; i < HTTP_CONNECTION_MAX; i++) {
		if (NULL == client[i].conn) {
			DBG_OUT(">> SLOT:%u\tSTATUS:FREE\t",i);
		} else {
			// Slot is reserved, but no active tcp connection
			if (NULL == client[i].conn->proto.tcp) {
				DBG_OUT(">> SLOT:%u\tSTATUS:%u\tLINK_CNT:%u\tPORT:NULL",
						i,client[i].conn->state,client[i].conn->link_cnt);
			} else {
			// Slot is reserved and has active tcp connection
				DBG_OUT(">> SLOT:%u\tSTATUS:%u\tLINK_CNT:%u\tPORT:%d",
						i,
						client[i].conn->state,
						client[i].conn->link_cnt,
						client[i].conn->proto.tcp->remote_port);		
			}	
		}
	}
#endif

}


/******************************************************************************
 * FunctionName : webserver_recon
 * Description  : the connection has been err, reconnection
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
webserver_recon(void *arg, sint8 err)
{
    struct espconn *pesp_conn = arg;

    INFO("client %d.%d.%d.%d:%d reconnected", 
			pesp_conn->proto.tcp->remote_ip[0],
    	pesp_conn->proto.tcp->remote_ip[1],
			pesp_conn->proto.tcp->remote_ip[2],
    	pesp_conn->proto.tcp->remote_ip[3],
			pesp_conn->proto.tcp->remote_port
		);
		DBG_OUT("with error code: %u", err);
}

/******************************************************************************
 * FunctionName : webserver_discon
 * Description  : the connection has been disconncted - Remove client from the pool.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
webserver_discon(void *arg)
{
	volatile bool foundClient = false;
	volatile uint8_t i = 0;
  struct espconn* disconnect_conn = arg;

  INFO("client %d.%d.%d.%d:%d disconnected", 
			disconnect_conn->proto.tcp->remote_ip[0],
      disconnect_conn->proto.tcp->remote_ip[1],
			disconnect_conn->proto.tcp->remote_ip[2],
      disconnect_conn->proto.tcp->remote_ip[3],
			disconnect_conn->proto.tcp->remote_port
	);

	for (i=0; i<HTTP_CONNECTION_MAX; i++) { 
		if ( client[i].conn->link_cnt == disconnect_conn->link_cnt ) {
			foundClient = true;
			client[i].conn = NULL;
			INFO("slot %u cleared",i);			
			break;
		} 
	}
	if (false == foundClient) {
		ERR_OUT("Woops. Did not found that Client. What do?");
		//TODO: WHAT DO?
	}
	
	webserver_print_clients();
}

/******************************************************************************
 * FunctionName : webserver_regist_connect
 * Description  : server found a new client, register to pool.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
webserver_regist_connect(void *arg)
{
	volatile bool foundFreeSlot = false;
	volatile uint8_t i = 0;
  struct espconn *listening_conn = arg;

	INFO("client %d.%d.%d.%d:%d connected", 
				listening_conn->proto.tcp->remote_ip[0],
        listening_conn->proto.tcp->remote_ip[1],
				listening_conn->proto.tcp->remote_ip[2],
        listening_conn->proto.tcp->remote_ip[3],
				listening_conn->proto.tcp->remote_port);

	// Search for a free slot to handle incoming connection
	for (i=0; i<HTTP_CONNECTION_MAX; i++) {
		if (NULL == client[i].conn) {
			// Found a free slot to handle the client
			INFO("registered at slot: %u", i);
			foundFreeSlot = true;
			break; // no need to look for other free slots
		}
	}

	if ( true == foundFreeSlot )
	{
		//TODO ALLOCATE MEMORY FOR SENDING BUFFER
				
		// Register Connection in Pool
		client[i].conn = listening_conn; // save connection-ptr
 		client[i].id = i;
		
/*
		espconn_regist_recvcb(listening_conn, webserver_recv);
		espconn_regist_reconcb(listening_conn, webserver_recon);
		espconn_regist_disconcb(listening_conn, webserver_discon);
*/
// TODO CHECK BECAUSE EXPERIMENTAL
//	We register the callback for the specific connection.
		espconn_regist_recvcb(client[i].conn, webserver_recv);
		espconn_regist_reconcb(client[i].conn, webserver_recon);
		espconn_regist_disconcb(client[i].conn, webserver_discon);
		INFO("registered callbacks");

		//TODO: REGISTER espconn_regist_sentcb();
	} else {
		ERR_OUT("(!) error. No free slot available.");
		//TODO SEND BUSY HEADER
	}
	webserver_print_clients();
}

/******************************************************************************
 * FunctionName : user_webserver_stop
 * Description  : parameter initialize as a server
 * Parameters   : port -- server port
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
webserver_stop() {
	espconn_disconnect(&webserver_conn);
	webserver_conn.type = ESPCONN_NONE;
}


/******************************************************************************
 * FunctionName : user_webserver_init
 * Description  : parameter initialize as a server
 * Parameters   : port -- server port
 * Returns      : Status/Error Code
*******************************************************************************/
uint8_t ICACHE_FLASH_ATTR
webserver_init(uint32_t port) {

	uint8_t i = 0;
	INFO("initializing webserver");

	// Reset Webserver Client Pool
	for (i=0; i<HTTP_CONNECTION_MAX; i++) client[i].conn = NULL;

	// Init Webserver TCP Listening Configuration
  webserver_conn.type = ESPCONN_TCP;
  webserver_conn.state = ESPCONN_NONE;
  webserver_conn.proto.tcp = &webserver_tcp;
  webserver_conn.proto.tcp->local_port = port;



	//Set the maximum number of TCP clients allowed to connect to ESP8266 TCP Server.
	espconn_tcp_set_max_con_allow(&webserver_conn, HTTP_CONNECTION_MAX);

	// Register Callback function for connection clients at tcp server
	espconn_regist_connectcb(&webserver_conn, webserver_regist_connect);

	INFO("starting server");
	switch (espconn_accept(&webserver_conn))
	{
		case ESPCONN_OK:
			//timeout=0, is deprecated. (seconds)
			espconn_regist_time(&webserver_conn, 120, 0);
			INFO("listening at port %u", port);
			return ESPCONN_OK;
		break;
		case ESPCONN_MEM:
			ERR_OUT("Error - ESP is out of memory");
			return ESPCONN_MEM;
		break;
		case ESPCONN_ARG:
			ERR_OUT("Error - Illegal argument provided");
			return ESPCONN_ARG;
		break;
		case ESPCONN_ISCONN:
			ERR_OUT("Error - Webserver is already running");
			return ESPCONN_ISCONN;
		break;
	}
	
}

