#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#define DEV_OUTPUT_ENABLE
#define FULL_VERSION
#define DEV_VERSION

#ifdef DEV_VERSION
#undef FULL_VERSION
#endif


//#define USE_OPTIMIZE_PRINTF

#ifdef DEV_OUTPUT_ENABLE
#define DBG_OUT_ENABLE
#define ERR_OUT_ENABLE
//#define CONSOLE_CLEAR_ENABLE
#define INFO(...) os_printf("[INFO@%s:%s]",__FILE__,__func__); DBG_OUT( __VA_ARGS__ )
#endif   /* DEV_OUTPUT_ENABLE */

#ifdef DBG_OUT_ENABLE
#define DBG_OUT(...) os_printf( __VA_ARGS__ );os_printf("\r\n");
#else
#define DBG_OUT
#endif   /* DBG_OUT_ENABLE */

#ifdef ERR_OUT_ENABLE
#define ERR_OUT(...) DBG_OUT( __VA_ARGS__ );os_printf("(!) [ERROR @ %s]",__func__);system_print_meminfo();os_printf("\r\n");
#else
#define ERR_OUT
#endif   /* ERR_OUT_ENABLE */

#define ICACHE_STORE_TYPEDEF_ATTR __attribute__((aligned(4),packed))
#define ICACHE_STORE_ATTR __attribute__((aligned(4)))
#define ICACHE_RAM_ATTR __attribute__((section(".iram0.text")))
//#define ICACHE_RODATA_ATTR __attribute__((section(".rodata2.text")))


//#define GPIO_INTERRUPT_ENABLE

//#define PRINTF_LONG_SUPPORT



#endif   /* __USER_CONFIG_H__ */
