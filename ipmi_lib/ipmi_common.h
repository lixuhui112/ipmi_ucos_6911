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
#include <ucos_ii.h>

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

#define BIT_CLR(x,b)            (x = ((x) & (~b)))
#define BIT_SET(x,b)            (x = ((x) | (b)))
#define BIT_MASK_SET(x,m,b)     (x = ((x) & (~m) | ((m) & (b))))
#define BIT_TST(x,b)            ((x) | (b))

//*****************************************************************************
//
// Defines for Type of BaseBoard.
//
//*****************************************************************************
#define BOARD_MAX_COUNT         11          // 最大板卡数量

#define BOARD_TYPE_MAJOR_MASK   0xf0        // 板卡类型掩码
#define BOARD_TYPE_MINOR_MASK   0x0f

#define BOARD_TYPE_FABRIC       0x10        // 板卡主类型
#define BOARD_TYPE_SWITCH       0x20
#define BOARD_TYPE_FAN          0x30
#define BOARD_TYPE_POWER        0x40
#define BOARD_TYPE_SWITCH10G    0x50
#define BOARD_TYPE_NONE         0x60

#define BOARD_FABRIC_MAJOR      0x01        // FABRIC板卡子类型
#define BOARD_FABRIC_MINOR      0x02
#define BOARD_FAN_7             0x01        // 风扇板卡子类型
#define BOARD_FAN_4             0x02
#define BOARD_POWER_600W        0x01        // 电源板卡子类型
#define BOARD_POWER_300W        0x02

uint8_t ipmi_board_type(uint8_t slot_addr);
char *ipmi_board_type_str(uint8_t board_type);



//*****************************************************************************
// Global variable for device
//*****************************************************************************
#define IPMI_CTX_POOL_SIZE      4
#define IPMI_RCV_MESG_SIZE      4
#define IPMI_MSG_QUE_SIZE       4

typedef struct ipmi_global_st {
    OS_EVENT *ipmi_sem;                 // IPMI全局信号量
    uint8_t slot_addr;                  // 本地槽位地址
    uint8_t ipmb_addr;                  // 本地IPMB地址，7位地址
    uint8_t event_recv_addr;            // 消息接收者的地址，0xff表示禁用此功能
    uint8_t event_recv_lun;             // 消息接收者的单元号
    uint8_t board_type;                 // 本地板卡类型，板类型+子类型
    uint8_t present_ok;                 // 本地在位信号
    uint8_t bmc_global_enable;          // BMC全局使能
    uint8_t bmc_message_flags;          // BMC消息状态
    uint32_t channel_msg_rev_en;        // 通道消息接收状态
    uint32_t flags;                     // 本地标志位
    uint32_t adt_support;               // 本地板卡功能使能 @ipmi_modules.h
    uint32_t timestamp;                 // 本地当前的时间戳，从1970/1/1
    uint32_t alive_bmc_map;             // 本地活动板卡位图 ~ipmi_board_table_st
    uint32_t device_available;          // 设备可用状态
    OS_EVENT *ipmi_req_que;             // 请求命令队列
    OS_EVENT *ipmi_prs_que;             // 处理命令队列
    OS_EVENT *ipmi_rsp_que;             // 响应命令队列
    OS_EVENT *ipmi_msg_que;             // 事件消息队列
    void *ipmi_req_que_pool[IPMI_MSG_QUE_SIZE];         // 请求队列内存池
    void *ipmi_prs_que_pool[IPMI_MSG_QUE_SIZE];         // 处理队列内存池
    void *ipmi_rsp_que_pool[IPMI_MSG_QUE_SIZE];         // 响应队列内存池
    void *ipmi_msg_que_pool[IPMI_MSG_QUE_SIZE];         // 事件消息队列内存池
    struct ipmi_ctx ipmi_ctx_pool[IPMI_CTX_POOL_SIZE];  // IPMI消息内存池
    struct ipmi_ctx *recv_msg_que[IPMI_RCV_MESG_SIZE];  // SMS接收消息队列
} ipmi_global_t;

#define BMC_SYNC_TIME_MASK  0x0000000f  // ipmi_global_t.flags
#define BMC_SYNC_TIME_SEC   0x00000001
#define BMC_SYNC_TIME_MIN   0x00000002
#define BMC_SYNC_TIME_HOUR  0x00000003
#define BMC_SYNC_TIME_NONE  0x0000000f
#define BMC_SOL_MASK        0x00000010  // ipmi_global_t.flags
#define BMC_SOL_ON          0x00000010
#define BMC_HOTSWAP_MASK    0x00000020  // ipmi_global_t.flags
#define BMC_HOTSWAP_ON      0x00000020
#define BMC_FAB_INIT_MASK   0x00000040  // ipmi_global_t.flags
#define BMC_FAB_INIT_ON     0x00000040
#define BMC_BRIDGE_MASK     0x00000080  // ipmi_global_t.flags
#define BMC_BRIDGE_ON       0x00000080

#define EN_RCV_MSG_QUE_INT              (1 << 0)    // ipmi_global_t.bmc_global_enable, ipmi_global_t.bmc_message_flags
#define EN_EVT_MSG_BUF_FUL_INT          (1 << 1)
#define EN_EVT_MSG_BUF                  (1 << 2)
#define EN_SYS_EVT_LOG                  (1 << 3)
#define OEM_0_EN                        (1 << 5)
#define OEM_1_EN                        (1 << 6)
#define OEM_2_EN                        (1 << 7)
#define WDT_PRE_TO_INT                  (1 << 3)    // ipmi_global_t.bmc_message_flags

typedef struct ipmi_board_table_st {
    uint8_t slot_addr;              // 板卡槽位地址
    uint8_t board_type;             // 板卡类型
    uint8_t ipmb_self_addr;         // 板卡IPMB地址
    uint32_t ipmb_timestamp;        // 最后一次响应IPMB总线的时戳
    char *board_type_str;           // 板卡描述
} ipmi_board_table_t;

void ipmi_alive_bmc_map_set(uint8_t bmc_index);
void ipmi_alive_bmc_map_clr(uint8_t bmc_index);
uint8_t ipmi_alive_bmc_map_get(uint8_t bmc_index);
uint8_t ipmi_slotaddr_to_index(uint8_t slot_addr);
uint8_t ipmi_ipmbaddr_to_index(uint8_t ipmb_addr);

extern void max6635_init(void);
extern void ucd9081_init(void);
extern void ina230_init(void);
extern void adt7470_init(void);
extern void key_init(void);
extern void mc_locator_init(void);


void ipmi_cmd_err(struct ipmi_ctx *ctx_cmd, uint8_t error);
void ipmi_cmd_invalid(struct ipmi_ctx *ctx_cmd);
void ipmi_cmd_ok(struct ipmi_ctx *ctx_cmd, uint8_t datalen);
void ipmi_cmd_set_flags(struct ipmi_ctx *ctx_cmd, uint8_t flags);
void ipmi_cmd_sendto_ch(struct ipmi_ctx *ctx_cmd, uint8_t to_channel);
void ipmi_cmd_response(struct ipmi_ctx *ctx_cmd);

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

extern ipmi_global_t ipmi_global;
extern ipmi_board_table_t ipmi_board_table[BOARD_MAX_COUNT];

struct ipmi_ctx *ipmi_get_free_ctx_entry(void);
struct ipmi_ctx *ipmi_get_bdg_ctx_entry(uint8_t rq_seq);
void ipmi_put_free_ctx_entry(struct ipmi_ctx *ctx);

void ipmi_msg_queue_push(struct ipmi_ctx *ctx_cmd);
struct ipmi_ctx *ipmi_msg_queue_pull(void);

#endif  // __IPMI_COMMON_H__

