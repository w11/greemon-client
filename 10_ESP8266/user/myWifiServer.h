#ifndef __MY_WIFI_SERVER_H__
#define __MY_WIFI_SERVER_H__

#define HTTP_VERSION "0.1"
#define HTTP_CONNECTION_MAX 5
#define HTTP_PORT 80
//#define HTTP_SEND_BUFFER 2048
//#define HTTP_POST_BUFFER 1024

#include "espconn.h" // FOR WEBSERVER
#include "mem.h"

os_timer_t test_station_ip;

LOCAL uint16_t requestNum = 0;

LOCAL uint8_t indexPage[] = "<html>\
<head>\
<link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\" />\
<link href=\"data:image/x-icon;base64,AAABAAEAEBAQAAEABAAoAQAAFgAAACgAAAAQAAAAIAAAAAEABAAAAAAAgAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAAAIIjAAA2DgCMYRYAYz8AAABUFwDHnFIAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACIiVQAAAABENVURMwAABDM1URZmMABDMzMRFmZjAEMzM2ZmEWUEM1VRYWEREVQzNVFhERERVDVTFhERERFUNTMWEREREVQzNRZmYRERVDMzZhYRERFQQzNmEREWYwBDM2ZhERFjAAQzZmEWZjAAAEQ2ZmYzAAAAAEQzMwAAD4HwAA4AcAAMADAACAAQAAgAEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAIABAACAAQAAwAMAAOAHAAD4HwAA\" rel=\"icon\" type=\"image/x-icon\" />\
</head>\
<body>\
<h1>Greemon Interface</h1>\
<ul>\
<li><a href=\"/\">Startseite</a></li>\
<li><a href=\"config.html\">Konfiguration</a></li>\
<li><a href=\"about.html\">&Uuml;ber den Entwickler</a></li>\
</ul>\
<p></p>\
</body>\
</html>"; 

LOCAL uint8_t aboutPage[] = "<html>\
<head>\
<link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\" />\
<link href=\"data:image/x-icon;base64,AAABAAEAEBAQAAEABAAoAQAAFgAAACgAAAAQAAAAIAAAAAEABAAAAAAAgAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAAAIIjAAA2DgCMYRYAYz8AAABUFwDHnFIAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACIiVQAAAABENVURMwAABDM1URZmMABDMzMRFmZjAEMzM2ZmEWUEM1VRYWEREVQzNVFhERERVDVTFhERERFUNTMWEREREVQzNRZmYRERVDMzZhYRERFQQzNmEREWYwBDM2ZhERFjAAQzZmEWZjAAAEQ2ZmYzAAAAAEQzMwAAD4HwAA4AcAAMADAACAAQAAgAEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAIABAACAAQAAwAMAAOAHAAD4HwAA\" rel=\"icon\" type=\"image/x-icon\" />\
</head>\
<body>\
<h1>Greemon Interface</h1>\
<ul>\
<li><a href=\"/\">Startseite</a></li>\
<li><a href=\"config.html\">Konfiguration</a></li>\
<li><a href=\"about.html\">&Uuml;ber den Entwickler</a></li>\
</ul>\
<h2>&Uuml;ber den Entwickler</h2>\
<p>David Huber - Hochschule Pforzheim</p>\
</body>\
</html>"; 

// Info: According to the documentation of the standard, the length of an SSID should be a maximum of 32 characters
// Password 8-63 (64?)
LOCAL uint8_t configPage[] = "<!DOCTYPE html>\
<html>\
<head>\
<meta charset=\"utf-8\">\
<link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\" />\
<link href=\"data:image/x-icon;base64,AAABAAEAEBAQAAEABAAoAQAAFgAAACgAAAAQAAAAIAAAAAEABAAAAAAAgAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAAAIIjAAA2DgCMYRYAYz8AAABUFwDHnFIAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACIiVQAAAABENVURMwAABDM1URZmMABDMzMRFmZjAEMzM2ZmEWUEM1VRYWEREVQzNVFhERERVDVTFhERERFUNTMWEREREVQzNRZmYRERVDMzZhYRERFQQzNmEREWYwBDM2ZhERFjAAQzZmEWZjAAAEQ2ZmYzAAAAAEQzMwAAD4HwAA4AcAAMADAACAAQAAgAEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAIABAACAAQAAwAMAAOAHAAD4HwAA\" rel=\"icon\" type=\"image/x-icon\" />\
</head>\
<body>\
<header><h1>Greemon Interface</h1></header>\
<nav>\
<ul>\
<li><a href=\"/\">Startseite</a></li>\
<li><a href=\"config.html\">Konfiguration</a></li>\
<li><a href=\"about.html\">&Uuml;ber den Entwickler</a></li>\
</ul>\
</nav>\
<section>\
<h2>Konfiguration</h2>\
<form action=\"saveSettings.cgi\" method=\"POST\" accept-charset=\"UTF-8\">\
<h3>Server Konfiguration</h3>\
<label for=\"server-ip\">Server Adresse</label>\
<input name=\"server-ip\" type=\"text\" id=\"server-ip\" maxlength=\"50\">\
<br/>\
<label for=\"server-port\">Server Port</label>\
<input name=\"server-port\" type=\"text\" id=\"server-port\" maxlength=\"5\">\
<br/>\
<h3>WLAN konfiguration</h3>\
<label for=\"wlan-ssid\">SSID</label>\
<input name=\"wlan-ssid\" type=\"text\" id=\"wlan-ssid\" maxlength=\"32\">\
<br/>\
<label for=\"wlan-password\">Passwort</label>\
<input name=\"wlan-password\" type=\"password\" id=\"wlan-password\" maxlength=\"64\">\
<br/>\
<h3>Greemon Info</h3>\
<label for=\"chipid\">Chip-ID</label>\
<input name=\"chipid\" type=\"text\" id=\"chipid\" maxlength=\"32\">\
<br/>\
<hr/>\
<button type=\"reset\">Zur&uuml;cksetzen</button>\
<button type=\"submit\">Absenden</button>\
<footer>\
</footer>\
</form>\
</section>\
</body>\
</html>"; 

LOCAL uint8_t styleSheet[] = "\
/* Reset CSS, opensource.736cs.com MIT */\
html,body,div,span,applet,object,iframe,h1,h2,h3,h4,h5,h6,p,blockquote,pre,a,abbr,acronym,address,big,cite,code,del,dfn,em,font,img,ins,kbd,q,s,samp,small,strike,strong,sub,sup,tt,var,b,i,center,dl,dt,dd,ol,ul,li,fieldset,form,label,legend,table,caption,tbody,tfoot,thead,tr,th,td,article,aside,audio,canvas,details,figcaption,figure,footer,header,hgroup,mark,menu,meter,nav,output,progress,section,summary,time,video{border:0;outline:0;font-size:100%;vertical-align:baseline;background:transparent;margin:0;padding:0;}body{line-height:1;}article,aside,dialog,figure,footer,header,hgroup,nav,section,blockquote{display:block;}nav ul{list-style:none;}ol{list-style:decimal;}ul{list-style:disc;}ul ul{list-style:circle;}blockquote,q{quotes:none;}blockquote:before,blockquote:after,q:before,q:after{content:none;}ins{text-decoration:underline;}del{text-decoration:line-through;}mark{background:none;}abbr[title],dfn[title]{border-bottom:1px dotted #000;cursor:help;}table{border-collapse:collapse;border-spacing:0;}hr{display:block;height:1px;border:0;border-top:1px solid #ccc;margin:1em 0;padding:0;}input[type=submit],input[type=button],button{margin:0!important;padding:0!important;}input,select,a img{vertical-align:middle;}\
body{ font-family: Helvetica, sans-serif; color: #222; }";


#ifndef ARRAY_SIZE
# define ARRAY_SIZE(a) (uint16_t*)(sizeof(a) / sizeof((a)[0]))
#endif


typedef struct _HTTPConnection{
	struct espconn *conn;
//	uint8_t connectionID;
//  char requestType;
//	uint8_t 	*sBuf;
//	uint16_t  	sBufLen;
} HTTPConnection_t;

static HTTPConnection_t HTTPConnection[HTTP_CONNECTION_MAX]; // Initializes a connection pool.


static const uint8_t *httpNotFoundHeader = "HTTP/1.0 404 Not Found\r\n\
Server: greemon-httpd/"HTTP_VERSION"\r\n\
Connection: close\r\n\
Content-Type: text/plain\r\n\
Content-Length: 12\r\n\r\n\
Not Found.\r\n";
static const uint8_t *httpNotImplementedHeader = "HTTP/1.0 501 Not Implemented\r\n\
Server: greemon-httpd/"HTTP_VERSION"\r\n\
Connection: close\r\n\
Content-Type: text/plain\r\n\
Content-Length: 28\r\n\r\n\
Not Implemented, sorry :(.\r\n";



/******************************************************************************
 * FunctionName : webserver_parse_post_content
 * Description  : parse the content and return an array
 * Parameters   : pusrdata - Pointer to the sent content
 * Returns      : none
*******************************************************************************/
bool ICACHE_FLASH_ATTR
webserver_parse_post_content(char *pusrdata, )
{
	uint8_t* 		pContent;

	// allocate memory for the settings	
	// These settings are going to be filled
	gm_Base_t* 	pBase_data;
	gm_Srv_t*		pSrv_data;
	gm_APN_t*		pAPN_data
	// search for content length 

	// allocate memory for the string

	//TODO ENOUGH

	//pContent = (uint8_t*)os_zalloc(<SIZE>);
	if ((NULL == pContent) || (NULL == pBase_data) || (NULL == pSrv_data ) || NULL == pAPN_data) {
		ERR_OUT("allocation for memory failed");
		return false;
	} else {
		// We can go on.
	}
	return true;
}


/******************************************************************************
 * FunctionName : user_esp_platform_check_ip
 * Description  : check whether get ip addr or not
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
user_set_station_config(void)
{
	DBG_OUT("INIT: Set Station Config")
	wifi_station_disconnect();
   // Wifi configuration
   char ssid[32] = "osiris";
   char password[64] = "scheka123";
   struct station_config stationConf;

   //need not mac address
   stationConf.bssid_set = 0;
   
   //Set ap settings
   os_memcpy(&stationConf.ssid, ssid, 32);
   os_memcpy(&stationConf.password, password, 64);
   wifi_station_set_config(&stationConf);

   	if (trhue==wifi_station_connect()) {
		DBG_OUT("Connected to AP %s",&stationConf.ssid);
	} else {
		DBG_OUT("Not Connected to AP %s",&stationConf.ssid);
	}

   //set a timer to check whether got ip from router succeed or not.
   os_timer_disarm(&test_station_ip);
   os_timer_setfn(&test_station_ip, (os_timer_func_t *)user_esp_platform_check_ip, NULL);
   os_timer_arm(&test_station_ip, 100, 0);

}

/******************************************************************************
 * FunctionName : webserver_recv
 * Description  : Processing the received data from the server
 * Parameters   : arg -- Additional argument to pass to the callback function
 *                pusrdata -- The received data (or NULL when the connection has been closed!)
 *                length -- The length of received data
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
webserver_recv(void *arg, char *pusrdata, unsigned short length)
{
    struct espconn *clientConnectionPtr = arg;
	uint8_t responseBuffer[4096]; //Because the StyleSheet is this large
	uint16_t l=0;
	uint8_t headerParser = 0;
	uint8_t *parsedRequest = NULL;
	bool validRequest = false;
	

	INFO("Recieved Message from Client");
	//DBG_OUT("--- Message Request No.%u Start ---",requestNum++);
	//DBG_OUT(pusrdata);
	//DBG_OUT("--- Message End ---");

// PARSE GET  COMMAND
	switch (pusrdata[0]) {
		case 'G': 
			DBG_OUT("Parsing - Found GET");
			// Find end of get request               vvvv
			// GET /index.html?this=0&that=1 HTTP/1.1\r\n
			// After that we dont need additional 13 Characters:
			// We dont need "GET /" and " HT T  P  1  .  1   
			//				 12345       789 10 11 12 13 14  
			// We allocate (Character to End of Line - 14 Characters) memory
			// to store: index.html?this=0&that=1 (25 chars)
			// for that we need to copy the source at the 5th position x characters long. 
			for (headerParser=0; headerParser<=255; headerParser++) {
				if ('\r' == pusrdata[headerParser] && '\n' == pusrdata[headerParser+1]) {
					parsedRequest = (uint8_t*) os_malloc(headerParser-14);  // TODO EVALUATE
					os_memcpy(parsedRequest, pusrdata+5, headerParser-14);
					if (NULL == parsedRequest) {
						DBG_OUT("Malloc failed");
						validRequest = true;
					} else {
						DBG_OUT("Client requested: '%s'", parsedRequest);		
						validRequest = false;		
					}								
 					os_free(parsedRequest);
					break;
				}	
				if (255==headerParser) {
					validRequest=false; //TODO: Eleganter? Did not find  \r\n	
					ERR_OUT("Could not parse header");
				} 	
			} 
		break;
		case 'P': 
			DBG_OUT("Parsing - Found POST");
			DBG_OUT(pusrdata);
      webserver_parse_post_content(pusrdata);
			user_set_station_config();
		break;			
		default: 
			DBG_OUT("Parsing - Found NOT IMPLEMENTED");
		break;
	}



// Parse Header	
	if (os_strncmp(pusrdata,"GET / ", 6 )==0)
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
		DBG_OUT("Not Implemented :(");
		// FOR OTHERS: Error Code: 501 - Not Implemented 	
		l = os_sprintf(responseBuffer, httpNotImplementedHeader); 
	}


	DBG_OUT("Starting response.");
	if (0==espconn_sent(clientConnectionPtr, (uint8_t*)responseBuffer, l) ) {
		DBG_OUT("Response sent!");
	} else {
		DBG_OUT("Something went wrong :(");
	}

	espconn_disconnect(clientConnectionPtr);
}


/******************************************************************************
 * FunctionName : webserver_print_clients
 * Description  : prints the array of connections with its data
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
webserver_print_clients(){

	DBG_OUT("=== CONNECTION POOL STATUS ===");	
	uint8_t i = 0;
	for (i=0; i<HTTP_CONNECTION_MAX; i++) {
		if (NULL == HTTPConnection[i].conn) {
			//HTTPConnection[i].conn->link_cnt
			DBG_OUT(">> SLOT:%u\tSTATUS:FREE\t",i);
		} else {
			// Slot is reserved, but no active tcp connection
			if (NULL == HTTPConnection[i].conn->proto.tcp) {
				DBG_OUT(">> SLOT:%u\tSTATUS:%u\tLINK_CNT:%u\tPORT:NULL",
						i,HTTPConnection[i].conn->state,HTTPConnection[i].conn->link_cnt);
			} else {
			// Slot is reserved and has active tcp connection
				DBG_OUT(">> SLOT:%u\tSTATUS:%u\tLINK_CNT:%u\tPORT:%d",
						i,
						HTTPConnection[i].conn->state,
						HTTPConnection[i].conn->link_cnt,
						HTTPConnection[i].conn->proto.tcp->remote_port);		
			}	
		}
	}
}


/******************************************************************************
 * FunctionName : webserver_recon
 * Description  : the connection has been err, reconnection
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL ICACHE_FLASH_ATTR
void webserver_recon(void *arg, sint8 err)
{
    struct espconn *pesp_conn = arg;

    DBG_OUT(">>Client %d.%d.%d.%d:%d Error: %d reconnected\n", 
			pesp_conn->proto.tcp->remote_ip[0],
    	pesp_conn->proto.tcp->remote_ip[1],
			pesp_conn->proto.tcp->remote_ip[2],
    	pesp_conn->proto.tcp->remote_ip[3],
			pesp_conn->proto.tcp->remote_port, 
			err);
}

/******************************************************************************
 * FunctionName : webserver_discon
 * Description  : the connection has been disconncted - Remove client from the pool.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL ICACHE_FLASH_ATTR
void webserver_discon(void *arg)
{
	bool foundClient = false;
	uint8_t i = 0;
    struct espconn *disconnect_conn = arg;

    DBG_OUT(">>Client %d.%d.%d.%d:%d disconnected. LINK_CNT=%u Removing from client-pool.", 
				disconnect_conn->proto.tcp->remote_ip[0],
        		disconnect_conn->proto.tcp->remote_ip[1],
				disconnect_conn->proto.tcp->remote_ip[2],
        		disconnect_conn->proto.tcp->remote_ip[3],
				disconnect_conn->proto.tcp->remote_port,
				disconnect_conn->link_cnt);

	for (i=0; i<HTTP_CONNECTION_MAX; i++) { 
		DBG_OUT("SEARCHING SLOT: %u, LINK_CNT: %u",i,HTTPConnection[i].conn->link_cnt);
		if ( HTTPConnection[i].conn->link_cnt == disconnect_conn->link_cnt ) {
			foundClient = true;
			DBG_OUT(">>Connection %u has been removed from the pool. Pool's closed ;)",i);
			HTTPConnection[i].conn = NULL;
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
LOCAL void ICACHE_FLASH_ATTR
webserver_regist_connect(void *arg)
{
	bool foundFreeSlot = false;
    struct espconn *listening_conn = arg;
	uint8_t i = 0;

	INFO(">> Client %d.%d.%d.%d:%d connected\n", 
				listening_conn->proto.tcp->remote_ip[0],
        listening_conn->proto.tcp->remote_ip[1],
				listening_conn->proto.tcp->remote_ip[2],
        listening_conn->proto.tcp->remote_ip[3],
				listening_conn->proto.tcp->remote_port);

	// Search for a free slot to handle incoming connection
	for (i=0; i<HTTP_CONNECTION_MAX; i++) {
		if (NULL == HTTPConnection[i].conn) {
			// Found a free slot to handle the client
			DBG_OUT(">> Register Client at Slot %u", i);
			foundFreeSlot = true;
			break; // no need to look for other free slots
		}
	}

	if ( true == foundFreeSlot )
	{
		//TODO ALLOCATE MEMORY FOR SENDING BUFFER
				
		
		// Register Connection in Pool
		HTTPConnection[i].conn = listening_conn;
		// Register Callbacks
		DBG_OUT("INIT: Register TCP Callbacks for client");
		espconn_regist_recvcb(listening_conn, webserver_recv);
		espconn_regist_reconcb(listening_conn, webserver_recon);
		espconn_regist_disconcb(listening_conn, webserver_discon);
		//TODO: REGISTER espconn_regist_sentcb();
	} else {
		ERR_OUT("INIT: No free tcp-slot avaiable");
		//TODO SEND BUSY HEADER
	}
	webserver_print_clients();
}

/******************************************************************************
 * FunctionName : user_webserver_init
 * Description  : parameter initialize as a server
 * Parameters   : port -- server port
 * Returns      : Status/Error Code
*******************************************************************************/
uint8_t webserver_init(uint32_t port) {
	uint8_t i = 0;

  LOCAL struct espconn webserver_conn;
  LOCAL esp_tcp webserver_tcp;

	//INFO();
	DBG_OUT("--- Initializing Webserver ---");

	// Reset Webserver Client Pool
	for (i=0; i<HTTP_CONNECTION_MAX; i++) {
		DBG_OUT("INIT: Reset HTTPConnection[%u] = NULL", i);
		HTTPConnection[i].conn = NULL;
	}

	// Init Webserver TCP Listening Configuration
  webserver_conn.type = ESPCONN_TCP;
  webserver_conn.state = ESPCONN_NONE;
  webserver_conn.proto.tcp = &webserver_tcp;
  webserver_conn.proto.tcp->local_port = port;

	//This timeout interval is not very precise, only as reference.
	//If timeout is set to 0, timeout will be disable and ESP8266 TCP server will
	//not disconnect TCP clients has stopped communication. This usage of
	//timeout=0, is deprecated.
	espconn_regist_time(&webserver_conn, 10000, 0);

	//Set the maximum number of TCP clients allowed to connect to ESP8266 TCP Server.
	espconn_tcp_set_max_con_allow(&webserver_conn, HTTP_CONNECTION_MAX);

	// Register Callback function for connection clients at tcp server
	DBG_OUT("INIT: Register Callback for incoming clients. Addr: %p", &webserver_conn);
	espconn_regist_connectcb(&webserver_conn, webserver_regist_connect);

	DBG_OUT("INIT: Starting TCP Server");
	switch (espconn_accept(&webserver_conn))
	{
		case ESPCONN_OK:
			DBG_OUT("INIT: Webserver listening at Port %u", port);
			DBG_OUT("Free Heap: %u",system_get_free_heap_size());
			//DBG_OUT("=== MEMINFO ===");
			//system_print_meminfo();	
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
			ERR_OUT("Error - Eebserver is already connected");
			return ESPCONN_ISCONN;
		break;
	}
	
}


#endif
