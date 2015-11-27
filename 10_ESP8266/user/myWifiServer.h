#ifndef __MY_WIFI_SERVER_H__
#define __MY_WIFI_SERVER_H__

#include "c_types.h"
#include "os_type.h"
#include "osapi.h"
#include "user_interface.h"
#include "ip_addr.h"
#include "espconn.h" // FOR WEBSERVER
#include "mem.h"
#include "spi_flash.h"
#include "myConfig.h"
#include "myGreemonStateMachine.h"

#define HTTP_VERSION "0.1"
#define HTTP_CONNECTION_MAX 2
#define HTTP_PORT 80
#define HTTP_SEND_BUFFER 4096
#define HTTP_POST_BUFFER 1024
#define HTTP_RECV_BUFFER 1024
//#define WEBSRV_DEBUG_PRINT_CLIENTS


typedef struct _HTTPConnection{
	struct espconn *conn;
  uint8_t id;
//  char requestType;
//	uint8_t 	*sBuf;
//	uint16_t  	sBufLen;
} HTTPConnection_t;


HTTPConnection_t client[HTTP_CONNECTION_MAX]; // connection pool
struct espconn webserver_conn;								// server structure
esp_tcp webserver_tcp;												// tcp structure for the server
os_timer_t test_station_ip;										// testing for ip adr

LOCAL const char indexPage[] = "<html>\
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

LOCAL const char aboutPage[] = "<html>\
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


LOCAL const char savePage[] = "<html>\
<head>\
<link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\" />\
<link href=\"data:image/x-icon;base64,AAABAAEAEBAQAAEABAAoAQAAFgAAACgAAAAQAAAAIAAAAAEABAAAAAAAgAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAAAIIjAAA2DgCMYRYAYz8AAABUFwDHnFIAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACIiVQAAAABENVURMwAABDM1URZmMABDMzMRFmZjAEMzM2ZmEWUEM1VRYWEREVQzNVFhERERVDVTFhERERFUNTMWEREREVQzNRZmYRERVDMzZhYRERFQQzNmEREWYwBDM2ZhERFjAAQzZmEWZjAAAEQ2ZmYzAAAAAEQzMwAAD4HwAA4AcAAMADAACAAQAAgAEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAIABAACAAQAAwAMAAOAHAAD4HwAA\" rel=\"icon\" type=\"image/x-icon\" />\
</head>\
<body>\
	<h1>Greemon Interface</h1>\
	<p>Settings saved!</p>\
	<p>Greemon is restarting</p>\
</body>\
</html>"; 


// Info: According to the documentation of the standard, the length of an SSID should be a maximum of 32 characters
// Password 8-63 (64?)
LOCAL const char configPage[] = "<!DOCTYPE html>\
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
<label for=\"ip\">Server Adresse</label>\
<input name=\"ip\" type=\"text\" id=\"ip\" maxlength=\"50\">\
<br/>\
<label for=\"port\">Server Port</label>\
<input name=\"port\" type=\"text\" id=\"port\" maxlength=\"5\">\
<br/>\
<label for=\"token\">Token</label>\
<input name=\"token\" type=\"text\" id=\"token\" maxlength=\"32\">\
<br/>\
<h3>WLAN konfiguration</h3>\
<label for=\"ssid\">SSID</label>\
<input name=\"ssid\" type=\"text\" id=\"ssid\" maxlength=\"32\">\
<br/>\
<label for=\"pass\">Passwort</label>\
<input name=\"pass\" type=\"password\" id=\"pass\" maxlength=\"64\">\
<button type=\"reset\">Zur&uuml;cksetzen</button>\
<button type=\"submit\">Absenden</button>\
<footer>\
</footer>\
</form>\
</section>\
</body>\
</html>"; 

LOCAL const char styleSheet[] = "\
/* Reset CSS, opensource.736cs.com MIT */\
html,body,div,span,applet,object,iframe,h1,h2,h3,h4,h5,h6,p,blockquote,pre,a,abbr,acronym,address,big,cite,code,del,dfn,em,font,img,ins,kbd,q,s,samp,small,strike,strong,sub,sup,tt,var,b,i,center,dl,dt,dd,ol,ul,li,fieldset,form,label,legend,table,caption,tbody,tfoot,thead,tr,th,td,article,aside,audio,canvas,details,figcaption,figure,footer,header,hgroup,mark,menu,meter,nav,output,progress,section,summary,time,video{border:0;outline:0;font-size:100%;vertical-align:baseline;background:transparent;margin:0;padding:0;}body{line-height:1;}article,aside,dialog,figure,footer,header,hgroup,nav,section,blockquote{display:block;}nav ul{list-style:none;}ol{list-style:decimal;}ul{list-style:disc;}ul ul{list-style:circle;}blockquote,q{quotes:none;}blockquote:before,blockquote:after,q:before,q:after{content:none;}ins{text-decoration:underline;}del{text-decoration:line-through;}mark{background:none;}abbr[title],dfn[title]{border-bottom:1px dotted #000;cursor:help;}table{border-collapse:collapse;border-spacing:0;}hr{display:block;height:1px;border:0;border-top:1px solid #ccc;margin:1em 0;padding:0;}input[type=submit],input[type=button],button{margin:0!important;padding:0!important;}input,select,a img{vertical-align:middle;}\
body{ font-family: Helvetica, sans-serif; color: #222; }\
";

LOCAL const char *httpNotFoundHeader = "HTTP/1.0 404 Not Found\r\n\
Server: greemon-httpd/"HTTP_VERSION"\r\n\
Connection: close\r\n\
Content-Type: text/plain\r\n\
Content-Length: 12\r\n\r\n\
Not Found.\r\n";
LOCAL const char *httpNotImplementedHeader = "HTTP/1.0 501 Not Implemented\r\n\
Server: greemon-httpd/"HTTP_VERSION"\r\n\
Connection: close\r\n\
Content-Type: text/plain\r\n\
Content-Length: 28\r\n\r\n\
Not Implemented, sorry :(.\r\n";

#ifndef ARRAY_SIZE
# define ARRAY_SIZE(a) (uint16_t*)(sizeof(a) / sizeof((a)[0]))
#endif


uint8_t ICACHE_FLASH_ATTR webserver_init(uint32_t port);
void ICACHE_FLASH_ATTR webserver_regist_connect(void *arg);
void ICACHE_FLASH_ATTR webserver_discon(void *arg);
void ICACHE_FLASH_ATTR webserver_recon(void *arg, sint8 err);
void ICACHE_FLASH_ATTR webserver_print_clients();
void ICACHE_FLASH_ATTR webserver_recv(void *arg, char *pusrdata, unsigned short length);
bool ICACHE_FLASH_ATTR webserver_parse_post_content(char *pusrdata, unsigned short *pLength);
uint16_t ICACHE_FLASH_ATTR webserver_parse_post_content_length(char *pusrdata, unsigned short *pLength);
void ICACHE_FLASH_ATTR user_set_station_config(gm_APN_t* apn);
void ICACHE_FLASH_ATTR user_esp_platform_check_ip(void);

#endif
