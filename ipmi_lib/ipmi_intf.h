//*****************************************************************************
//
// ipmi_intf.h - IPMI Interface Header File
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************


#ifndef _IPMI_INTF_H_
#define _IPMI_INTF_H_

#include <inttypes.h>

#define IPMI_INTF_SSIF                  0x01
#define IPMI_INTF_ICMB                  0x02
#define IPMI_INTF_IPMB                  0x04
#define IPMI_INTF_ETH                   0x08
#define IPMI_INTF_DEBUG                 0x10


// IPMI帧开始标记字符
#define IPMI_FRAME_CHAR_SIZE            2
#define IPMI_FRAME_CMD_READ             0x01
#define IPMI_FRAME_CMD_WRITE            0x02
#define IPMI_FRAME_CMD_STATE            0x03

extern uint8_t IPMI_FRAME_CHAR[IPMI_FRAME_CHAR_SIZE];


#include "ipmi.h"

#include "lib_spi.h"
#include "lib_i2c.h"
#include "lib_uart.h"
#include "lib_eth.h"
#include "lib_io.h"
#include "lib_slot.h"


// SSIF接口读写函数
#define IPMI_SSIF_READ(buf, size)           SPI_spi1_read(buf, size)
#define IPMI_SSIF_WRITE(buf, size)          SPI_spi1_write(buf, size)

// CPLD接口读写函数
#define IPMI_CPLD_READ(regaddr, buf, size)  SPI_spi0_transfer(regaddr, 1, buf, size)
#define IPMI_CPLD_WRITE(regaddr, buf, size) SPI_spi0_transfer(buf, size, 0, 0)

// IPMB接口读写函数
#define IPMI_IPMB_READ(buf, size)           I2C_i2c0_ipmb_read(buf, size)
#define IPMI_IPMB_WRITE(addr, buf, size)    I2C_i2c0_ipmb_write(addr, buf, size)

// ICMB接口读写函数
#define IPMI_ICMB_READ(buf, size)           UART_uart1_read(buf, size)
#define IPMI_ICMB_WRITE(buf, size)          UART_uart1_write(buf, size)

// DBGU接口读写函数
#define IPMI_DBGU_READ(buf, size)           UART_uart0_read(buf, size)
#define IPMI_DBGU_WRITE(buf, size)          UART_uart0_write(buf, size)

// 接口读信号函数
void ipmi_intf_recv_post(int intf);

void ipmi_debug_init(void);
int ipmi_debug_recv(struct ipmi_ctx *ctx_cmd);
int ipmi_debug_send(struct ipmi_ctx *ctx_cmd);

void ipmi_ssif_init(void);
int ipmi_ssif_recv(struct ipmi_ctx *ctx_cmd);
int ipmi_ssif_send(struct ipmi_ctx *ctx_cmd);

void ipmi_ipmb_init(void);
int ipmi_ipmb_recv(struct ipmi_ctx *ctx_cmd);
int ipmi_ipmb_send(struct ipmi_ctx *ctx_cmd);

void ipmi_icmb_init(void);
int ipmi_icmb_recv(struct ipmi_ctx *ctx_cmd);
int ipmi_icmb_send(struct ipmi_ctx *ctx_cmd);

void ipmi_eth_init(void);
int ipmi_eth_recv(struct ipmi_ctx *ctx_cmd);
int ipmi_eth_send(struct ipmi_ctx *ctx_cmd);


#endif  // _IPMI_INTF_H_

