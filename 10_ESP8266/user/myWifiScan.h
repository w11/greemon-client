#ifndef __MY_WIFISCAN_H_
#define __MY_WIFISCAN_H_

/* Greemon Lib for Scanning WiFi Networks.
 * Huber David
 */
// currently not used
#define WIFI_SCAN_MAX_APS 20

void user_wifi_scan_done(void *arg, STATUS status);
void user_wifi_scan_start(struct scan_config *config);

#endif
