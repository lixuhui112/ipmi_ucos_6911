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
#include "ipmi_lib/ipmi.h"
#include "app/lib_spi.h"
#include "app/lib_i2c.h"
#include "app/lib_uart.h"
#include "app/lib_eth.h"
#include "app/lib_io.h"

#if (defined(BOARD_6911_SWITCH) || defined(BOARD_6911_FABRIC))
#define IPMI_INTF_SSIF                  IPMI_CH_NUM_SYS_INTERFACE
#define IPMI_INTF_ICMB                  IPMI_CH_NUM_ICMB
#define IPMI_INTF_IPMB                  IPMI_CH_NUM_PRIMARY_IPMB
#define IPMI_INTF_ETH                   IPMI_CH_NUM_LAN
#define IPMI_INTF_DEBUG                 IPMI_CH_NUM_CONSOLE
#endif
#if (defined(BOARD_6911_FAN) || defined(BOARD_6911_POWER))
#define IPMI_INTF_IPMB                  IPMI_CH_NUM_PRIMARY_IPMB
#define IPMI_INTF_DEBUG                 IPMI_CH_NUM_SYS_INTERFACE
#endif

// IPMI帧开始标记字符
#define IPMI_FRAME_CHAR_SIZE            3
#define IPMI_FRAME_CMD_READ             0x01
#define IPMI_FRAME_CMD_WRITE            0x02
#define IPMI_FRAME_CMD_STATE            0x03

extern uint8_t IPMI_FRAME_CHAR[IPMI_FRAME_CHAR_SIZE];


// IPMI接口结构体
typedef struct ipmi_intf {
	char *name;
	uint32_t flags;

	//struct ipmi_session * session;
	//struct ipmi_oem_handle * oem;
	//struct ipmi_cmd * cmdlist;
	uint32_t my_addr;
	uint32_t target_addr;
	uint8_t target_lun;
	uint8_t target_channel;
	uint32_t transit_addr;
	uint8_t transit_channel;
	uint8_t channel_buf_size;

	//uint8_t devnum;

	//int (*setup)(struct ipmi_intf * intf);
	//int (*open)(struct ipmi_intf * intf);
	//void (*close)(struct ipmi_intf * intf);
	//struct ipmi_rs *(*sendrecv)(struct ipmi_intf * intf, struct ipmi_rq * req);
	//int (*sendrsp)(struct ipmi_intf * intf, struct ipmi_rs * rsp);
	//struct ipmi_rs *(*recv_sol)(struct ipmi_intf * intf);
	//struct ipmi_rs *(*send_sol)(struct ipmi_intf * intf, struct ipmi_v2_payload * payload);
	//int (*keepalive)(struct ipmi_intf * intf);
} ipmi_intf_t;


// SSIF接口读写函数
#define IPMI_SSIF_READ(buf, size)           SPI_spi1_read(buf, size)
#define IPMI_SSIF_WRITE(buf, size)          SPI_spi1_write(buf, size)

// CPLD接口读写函数
#define IPMI_LOGIC_READ(raddr, buf)         SPI_spi0_xfer(raddr, 2, buf, 1)
#define IPMI_LOGIC_WRITE(raddr, buf)        SPI_spi0_xfer(buf, 3, 0, 0)

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
void ipmi_intf_recv_post(uint8_t intf);

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

