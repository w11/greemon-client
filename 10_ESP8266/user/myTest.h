#ifndef __MYTEST_H__
#define __MYTEST_H__

#include "c_types.h"
#include "ip_addr.h"
#include "espconn.h"
#include "user_interface.h"
#include "osapi.h"
#include "myConfig.h"
#include "mem.h"
#include "myNTP.h"

/*
 * CONFIG FOR TESTING
 */

#define TEST_APNAME "GreemonTestsuite"
#define TEST_APPASS "testpass123"
#define TEST_STANAME "Julia"
#define TEST_STAPASS "scheka123"
#define TEST_DEVICENAME "Greemon TestSuite"

bool ICACHE_FLASH_ATTR test_data_add(void);
void ICACHE_FLASH_ATTR test_config(void);
void ICACHE_FLASH_ATTR test_connection_settings(void);



































#ifdef DEV_VERSION

#endif

#endif

