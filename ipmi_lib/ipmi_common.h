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
#ifdef __LITTLE_ENDIAN__
#define B16_H2L(X)              (X)             /* 16 bit to little endian */
#define B32_H2L(X)              (X)             /* 32 bit to little endian */
#define B16_H2B(X)              BSWAP_16(X)     /* 16 bit to big endian */
#define B32_H2B(X)              BSWAP_32(X)     /* 32 bit to big endian */
#define B16_L2H(X)              (X)             /* 16 bit little to host */
#define B32_L2H(X)              (X)             /* 32 bit little to host */
#define B16_B2H(X)              BSWAP_16(X)     /* 16 bit big endian to host */
#define B32_B2H(X)              BSWAP_32(X)     /* 32 bit big endian to host */
#else
#define B16_H2L(X)              BSWAP_16(X)     /* 16 bit to little endian */
#define B32_H2L(X)              BSWAP_32(X)     /* 32 bit to little endian */
#define B16_H2B(X)              (X)             /* 16 bit to big endian */
#define B32_H2B(X)              (X)             /* 32 bit to big endian */
#define B16_L2H(X)              BSWAP_16(X)     /* 16 bit little to host */
#define B32_L2H(X)              BSWAP_32(X)     /* 32 bit little to host */
#define B16_B2H(X)              (X)             /* 16 bit big endian to host */
#define B32_B2H(X)              (X)             /* 32 bit big endian to host */
#endif

extern void max6635_init(void);
extern void ucd9081_init(void);
extern void ina230_init(void);
extern void adt7470_init(void);
extern void key_init(void);
extern void mc_locator_init(void);

int ipmi_cmd_err(struct ipmi_ctx *ctx_cmd, uint8_t error);
int ipmi_cmd_invalid(struct ipmi_ctx *ctx_cmd);
int ipmi_cmd_ok(struct ipmi_ctx *ctx_cmd, uint8_t datalen);

char ipmi_common_get_device_id(void);
char ipmi_common_get_device_revision(void);
char ipmi_common_get_product_id(void);
char ipmi_common_get_poh(uint32_t *poh);
void ipmi_common_power_onoff(uint8_t on);
void ipmi_common_test_self(void);

#define IPMI_LED_ON             0x00
#define IPMI_LED_SYS            0x01
#define IPMI_LED_ERR            0xff
#define IPMI_LED_IDENTIFY       0x0f

void led_start(void);
void led_change(uint8_t period);

extern void at24xx_init(void);
extern uint32_t at24xx_read(uint32_t addr, uint8_t *buffer, uint32_t size);
extern uint32_t at24xx_write(uint32_t addr, uint8_t *buffer, uint32_t size);
extern uint32_t at24xx_clear(uint32_t addr, uint32_t size);

extern void ipmi_sel_init(void);
extern void ipmi_sdr_init(void);

void ipmi_modules_init(void);
void ipmi_sensors_init(void);

#endif  // __IPMI_COMMON_H__

