#ifndef __MY_WIFI_SCAN_H__
#define __MY_WIFI_SCAN_H__




/******************************************************************************
 * FunctionName : scan_done
 * Description  : scan done callback
 * Parameters   :  arg: contain the aps information;
                          status: scan over status
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
scan_done(void *arg, STATUS status)
{
  uint8_t scanCounter = 0;

  uint8 ssid[33];
  char temp[128];

  if (status == OK)
  {
    struct bss_info *bss_link = (struct bss_info *)arg;
    bss_link = bss_link->next.stqe_next;//ignore the first one , it's invalid.

    while (bss_link != NULL)
    {
      os_memset(ssid, 0, 33);
      if (os_strlen(bss_link->ssid) <= 32)
      {
        os_memcpy(ssid, bss_link->ssid, os_strlen(bss_link->ssid));
      }
      else
      {
        os_memcpy(ssid, bss_link->ssid, 32);
      }
      os_printf("(%d, %d,\"%s\",%d,\""MACSTR"\",%d)\r\n",
                 	scanCounter++, 
				 	bss_link->authmode, 
					ssid, 
					bss_link->rssi,
                 	MAC2STR(bss_link->bssid),
					bss_link->channel
	  );
      bss_link = bss_link->next.stqe_next;
    }
  }
  else
  {
     os_printf("scan fail !!!\r\n");
  }

}

	
void ICACHE_FLASH_ATTR
scan_start(void){
	wifi_station_scan(NULL,scan_done);
}


/******************************************************************************
 * FunctionName : user_scan
 * Description  : wifi scan, only can be called after system init done.
 * Parameters   :  none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_scan(void)
{
   if(wifi_get_opmode() == SOFTAP_MODE)
   {
     os_printf("ap mode can't scan !!!\r\n");
     return;
   }
   wifi_station_scan(NULL,scan_done);

}


/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{
    os_printf("SDK version:%s\n", system_get_sdk_version());
   


}

#endif


