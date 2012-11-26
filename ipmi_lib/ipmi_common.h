//*****************************************************************************
//
// ipmi_common.h - IPMI Common Command Header File
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#include "ipmi.h"

#ifndef __IPMI_COMMON_H__
#define __IPMI_COMMON_H__

#define BSWAP_16(x)             ((((x) & 0xff00) >> 8) | (((x) & 0x00ff) << 8))
#define BSWAP_32(x)             ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >> 8) |\
                                 (((x) & 0x0000ff00) << 8) | (((x) & 0x000000ff) << 24))

#define IPMI_LED_ON                     0xffffffff
#define IPMI_LED_SYS                    0x00000000
#define IPMI_LED_ERR                    0x00ff0000
#define IPMI_LED_SYS_OK                 0x0200          /* 512ms    */
#define IPMI_LED_SYS_ERR                0x0100          /* 256ms    */
#define IPMI_LED_SYS_IDENTIFY           0x0f            /* 15s      */
#define IPMI_LED_SYS_FORCE              0xff            /* 256      */

extern unsigned long led_delay_ms;
extern unsigned long led_delay_s;

extern void max6635_init(void);
extern void ucd9081_init(void);
extern void ina230_init(void);
extern void adt7470_init(void);
extern void at24xx_init(void);
extern void key_init(void);
extern void mc_locator_init(void);

int ipmi_cmd_err(struct ipmi_ctx *ctx_cmd, uint8_t error);
int ipmi_cmd_invalid(struct ipmi_ctx *ctx_cmd);
int ipmi_cmd_ok(struct ipmi_ctx *ctx_cmd, uint8_t datalen);

char ipmi_common_get_device_id(void);
char ipmi_common_get_device_revision(void);
char ipmi_common_get_product_id(void);
char ipmi_common_get_poh(unsigned long *poh);

void ipmi_sensor_init(void);
int ipmi_common_power_onoff(int on);
void ipmi_led_blink(int delay);

#endif  // __IPMI_COMMON_H__

