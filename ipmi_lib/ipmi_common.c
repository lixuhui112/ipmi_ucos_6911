//*****************************************************************************
//
// ipmi_common.c - IPMI Command for Common
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************
#include <stdlib.h>
#include "ipmi.h"
#include "ucos_ii.h"
#include "app/lib_common.h"
#include "ipmi_lib/ipmi_common.h"

//*****************************************************************************
//
// Define global variable for ipmi system
//
//*****************************************************************************
ipmi_global_t ipmi_global;

ipmi_board_table_t ipmi_board_table[BOARD_MAX_COUNT] =
{
    {   /* bit0 bmc_index */                            /* map to ipmi_global.alive_bmc_map */
        .slot_addr          = IPMI_SLOT_ADDR_POWER_1,
        .board_type         = BOARD_TYPE_POWER,
        .ipmb_self_addr     = 0x71,                     /* 7-bit slaver i2c address */
        .ipmb_timestamp     = 0,
        .board_type_str     = "power-1",
    },
    {   /* bit1 bmc_index */
        .slot_addr          = IPMI_SLOT_ADDR_POWER_2,
        .board_type         = BOARD_TYPE_POWER,
        .ipmb_self_addr     = 0x72,
        .ipmb_timestamp     = 0,
        .board_type_str     = "power-2",
    },
    {   /* bit2 bmc_index */
        .slot_addr          = IPMI_SLOT_ADDR_SWITCH_1,
        .board_type         = BOARD_TYPE_SWITCH,
        .ipmb_self_addr     = 0x73,
        .ipmb_timestamp     = 0,
        .board_type_str     = "switch-1",
    },
    {   /* bit3 bmc_index */
        .slot_addr          = IPMI_SLOT_ADDR_SWITCH_2,
        .board_type         = BOARD_TYPE_SWITCH,
        .ipmb_self_addr     = 0x74,
        .ipmb_timestamp     = 0,
        .board_type_str     = "switch-2",
    },
    {   /* bit4 bmc_index */
        .slot_addr          = IPMI_SLOT_ADDR_FABRIC_1,
        .board_type         = BOARD_TYPE_FABRIC,
        .ipmb_self_addr     = 0x75,
        .ipmb_timestamp     = 0,
        .board_type_str     = "fabric-1",
    },
    {   /* bit5 bmc_index */
        .slot_addr          = IPMI_SLOT_ADDR_SWITCH10G,
        .board_type         = BOARD_TYPE_SWITCH10G,
        .ipmb_self_addr     = 0x76,
        .ipmb_timestamp     = 0,
        .board_type_str     = "siwtch10g",
    },
    {   /* bit6 bmc_index */
        .slot_addr          = IPMI_SLOT_ADDR_FABRIC_2,
        .board_type         = BOARD_TYPE_FABRIC,
        .ipmb_self_addr     = 0x77,
        .ipmb_timestamp     = 0,
        .board_type_str     = "fabric-2",
    },
    {   /* bit7 bmc_index */
        .slot_addr          = IPMI_SLOT_ADDR_SWITCH_3,
        .board_type         = BOARD_TYPE_SWITCH,
        .ipmb_self_addr     = 0x78,
        .ipmb_timestamp     = 0,
        .board_type_str     = "switch-3",
    },
    {   /* bit8 bmc_index */
        .slot_addr          = IPMI_SLOT_ADDR_SWITCH_4,
        .board_type         = BOARD_TYPE_SWITCH,
        .ipmb_self_addr     = 0x79,
        .ipmb_timestamp     = 0,
        .board_type_str     = "switch-4",
    },
    {   /* bit9 bmc_index */
        .slot_addr          = IPMI_SLOT_ADDR_FAN_1,
        .board_type         = BOARD_TYPE_FAN,
        .ipmb_self_addr     = 0x7a,
        .ipmb_timestamp     = 0,
        .board_type_str     = "fan-1",
    },
    {   /* bit10 bmc_index */
        .slot_addr          = IPMI_SLOT_ADDR_FAN_2,
        .board_type         = BOARD_TYPE_FAN,
        .ipmb_self_addr     = 0x7b,
        .ipmb_timestamp     = 0,
        .board_type_str     = "fan-2",
    },
};

uint8_t ipmi_board_type(uint8_t slot_addr)
{
    for (uint8_t i = 0; i < BOARD_MAX_COUNT; i++) {
        if (slot_addr == ipmi_board_table[i].slot_addr) {
            return ipmi_board_table[i].board_type;
        }
    }

    return BOARD_TYPE_NONE;
}

char *ipmi_board_type_str(uint8_t board_type)
{
    for (uint8_t i = 0; i < BOARD_MAX_COUNT; i++) {
        if (board_type == ipmi_board_table[i].board_type) {
            return ipmi_board_table[i].board_type_str;
        }
    }

    return "none";
}

uint8_t ipmi_slotaddr_to_index(uint8_t slot_addr)
{
    uint8_t i;

    for (i = 0; i < BOARD_MAX_COUNT; i++) {
        if (slot_addr == ipmi_board_table[i].slot_addr) {
            return i;
        }
    }

    return 0;
}

uint8_t ipmi_ipmbaddr_to_index(uint8_t ipmb_addr)
{
    uint8_t i;

    for (i = 0; i < BOARD_MAX_COUNT; i++) {
        if (ipmb_addr == ipmi_board_table[i].ipmb_self_addr) {
            return i;
        }
    }

    return 0;
}

void ipmi_alive_bmc_map_set(uint8_t bmc_index)
{
    if (bmc_index > BOARD_MAX_COUNT) {
        return;
    }
    ipmi_global.alive_bmc_map |= (1 << bmc_index);
}

void ipmi_alive_bmc_map_clr(uint8_t bmc_index)
{
    if (bmc_index > BOARD_MAX_COUNT) {
        return;
    }
    ipmi_global.alive_bmc_map &= ~(1 << bmc_index);
}

uint8_t ipmi_alive_bmc_map_get(uint8_t bmc_index)
{
    if (bmc_index > BOARD_MAX_COUNT) {
        return 0;
    }
    return (ipmi_global.alive_bmc_map & (1 << bmc_index)) ? 1 : 0;
}


struct ipmi_ctx *ipmi_get_free_ctx_entry(void)
{
    uint8_t error;

    OSSemPend(ipmi_global.ipmi_sem, 0, &error);
    if (error) {
        return NULL;
    }

    for (uint8_t i = 0; i < IPMI_CTX_POOL_SIZE; i++)
    {
        if ((ipmi_global.ipmi_ctx_pool[i].flags & IPMI_CTX_ENABLE) == 0)
        {
            ipmi_global.ipmi_ctx_pool[i].flags |= IPMI_CTX_ENABLE;
            OSSemPost(ipmi_global.ipmi_sem);
            return &ipmi_global.ipmi_ctx_pool[i];
        }
    }

    OSSemPost(ipmi_global.ipmi_sem);
    return NULL;    // the entry is full
}

struct ipmi_ctx *ipmi_get_bdg_ctx_entry(uint8_t rq_seq)
{
    uint8_t error;

    OSSemPend(ipmi_global.ipmi_sem, 0, &error);
    if (error) {
        return NULL;
    }

    for (uint8_t i = 0; i < IPMI_CTX_POOL_SIZE; i++)
    {
        if ((ipmi_global.ipmi_ctx_pool[i].flags & IPMI_CTX_BRIDGE) &&
            (ipmi_global.ipmi_ctx_pool[i].rq_seq == rq_seq))
        {
            OSSemPost(ipmi_global.ipmi_sem);
            return &ipmi_global.ipmi_ctx_pool[i];
        }
    }

    OSSemPost(ipmi_global.ipmi_sem);
    return NULL;
}

void ipmi_put_free_ctx_entry(struct ipmi_ctx *ctx)
{
    memset(ctx, 0, sizeof(struct ipmi_ctx));
}


//*****************************************************************************
// Defines Front Character Frame of IPMI Message
//*****************************************************************************
uint8_t IPMI_FRAME_CHAR[IPMI_FRAME_CHAR_SIZE] = {0x0f, 0xf0, 0x5a};


//*****************************************************************************
// Defines sequence of IPMI Message
//*****************************************************************************
uint8_t seq_array[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


//*****************************************************************************
// sequence number generator
//*****************************************************************************
uint8_t ipmi_get_next_seq(uint8_t *seq)
{
	uint8_t i;

    uint8_t error;

    OSSemPend(ipmi_global.ipmi_sem, 0, &error);
    if (error) {
        return NULL;
    }

	/* return the first free sequence number */
	for (i = 0; i < 16; i++)
	{
		if (!seq_array[i])
		{
			seq_array[i] = 1;
			*seq = i;
            OSSemPost(ipmi_global.ipmi_sem);
			return (1);
		}
	}

    OSSemPost(ipmi_global.ipmi_sem);
	return (0);
}

void ipmi_seq_free(uint8_t seq)
{
	seq_array[seq] = 0;
}


//*****************************************************************************
// IPMI SMS message queue push/pull
//*****************************************************************************
void ipmi_msg_queue_push(struct ipmi_ctx *ctx_cmd)
{
    uint8_t error;

    OSSemPend(ipmi_global.ipmi_sem, 0, &error);
    if (error) {
        return;
    }

    for (uint8_t i = 0; i < IPMI_RCV_MESG_SIZE; i++) {
        if (ipmi_global.recv_msg_que[i] == NULL) {
            ipmi_global.recv_msg_que[i] = ctx_cmd;
            break;
        }
    }

    BIT_SET(ipmi_global.bmc_message_flags, EN_RCV_MSG_QUE_INT);

    OSSemPost(ipmi_global.ipmi_sem);
}

struct ipmi_ctx *ipmi_msg_queue_pull(void)
{
    uint8_t error, count, i;
    struct ipmi_ctx *que_ctx;

    OSSemPend(ipmi_global.ipmi_sem, 0, &error);
    if (error) {
        return NULL;
    }

    count = 0;
    que_ctx = ipmi_global.recv_msg_que[0];

    for (i = 0; i < IPMI_RCV_MESG_SIZE-1; i++) {
        ipmi_global.recv_msg_que[i] = ipmi_global.recv_msg_que[i+1];
        if (ipmi_global.recv_msg_que[i]) {
            count++;
        }
    }
    ipmi_global.recv_msg_que[i] = NULL;

    if (!count) {
        BIT_CLR(ipmi_global.bmc_message_flags, EN_RCV_MSG_QUE_INT);
    }

    OSSemPost(ipmi_global.ipmi_sem);

    return que_ctx;
}

//*****************************************************************************
//
// Define generic ipmi response function
//
//*****************************************************************************
void ipmi_cmd_err(struct ipmi_ctx *ctx_cmd, uint8_t error)
{
    ctx_cmd->rsp.msg.ccode = error;
    ctx_cmd->rsp.msg.data_len = 0;
    ctx_cmd->rsp.msg.cmd = ctx_cmd->req.msg.cmd;
    ctx_cmd->rsp.msg.rq_sa = ctx_cmd->req.msg.rq_sa;
    ctx_cmd->rsp.msg.rs_sa = ctx_cmd->req.msg.rs_sa;
    ctx_cmd->rsp.msg.netfn = ctx_cmd->req.msg.netfn + 1;
    ctx_cmd->rsp.msg.rq_lun = ctx_cmd->req.msg.rq_lun;
    ctx_cmd->rsp.msg.rs_lun = ctx_cmd->req.msg.rs_lun;
    ctx_cmd->rsp.msg.rq_seq = ctx_cmd->req.msg.rq_seq;
    ctx_cmd->to_channel = ctx_cmd->from_channel;
}

void ipmi_cmd_invalid(struct ipmi_ctx *ctx_cmd)
{
    ipmi_cmd_err(ctx_cmd, IPMI_CC_INV_CMD);
}

void ipmi_cmd_ok(struct ipmi_ctx *ctx_cmd, uint8_t datalen)
{
    ipmi_cmd_err(ctx_cmd, IPMI_CC_OK);
    ctx_cmd->rsp.msg.data_len = datalen;
}

void ipmi_cmd_set_flags(struct ipmi_ctx *ctx_cmd, uint8_t flags)
{
    ctx_cmd->flags |= flags;
}

void ipmi_cmd_sendto_ch(struct ipmi_ctx *ctx_cmd, uint8_t to_channel)
{
    ctx_cmd->to_channel = to_channel;
    OSQPost(ipmi_global.ipmi_rsp_que, (void*)ctx_cmd);
}

void ipmi_cmd_response(struct ipmi_ctx *ctx_cmd)
{
    struct ipmi_ctx *old_ctx;
    struct ipmi_rsp *rsp;

    rsp = (struct ipmi_rsp *)&ctx_cmd->req;

    old_ctx = ipmi_get_bdg_ctx_entry(rsp->msg.rq_seq);
    if (old_ctx)
    {
        old_ctx->rsp.msg.data_len = rsp->msg.data_len;
        old_ctx->rsp.msg.rq_sa = IPMI_REMOTE_SWID;
        old_ctx->rsp.msg.rs_sa = IPMI_BMC_SLAVE_ADDR;
        old_ctx->rsp.msg.rq_lun = 0x2;
        old_ctx->rsp.msg.rs_lun = 0x0;
        old_ctx->rsp.msg.netfn = rsp->msg.netfn;
        old_ctx->rsp.msg.cmd = rsp->msg.cmd;
        old_ctx->rsp.msg.checksum1 = rsp->msg.checksum1;
        old_ctx->rsp.msg.rq_seq = rsp->msg.rq_seq;
        old_ctx->rsp.msg.ccode = rsp->msg.ccode;
        memcpy(&old_ctx->rsp.data[0], &rsp->data[0], rsp->msg.data_len - sizeof(struct _ipmi_rsp_cmd));

        // add old_ctx to ipmi_global.recv_msg_que
        ipmi_msg_queue_push(old_ctx);
    }

    ipmi_cmd_set_flags(ctx_cmd, IPMI_CTX_NO_RESPONSE);
}


//*****************************************************************************
//
// Defines a timer for IPMI_LED
//
//*****************************************************************************
OS_TMR *led_timer;
uint8_t led_period = 1;

extern void IO_led1_set(tBoolean bOn);
void led_blink(void *ptmr, void *param)
{
    static uint8_t led1;

    led1 = !led1;

    IO_led1_set(led1);
}

void led_start(void)
{
    uint8_t err;
    uint32_t period;

    switch (led_period) {
        case IPMI_LED_ON:
            period = 10;
            break;
        case IPMI_LED_ERR:
            period = 5;
            break;
        default:
            period = 10 * led_period;
            break;
    }
    led_timer = OSTmrCreate(0, period, OS_TMR_OPT_PERIODIC, led_blink, NULL, "led_timer", &err);
    if (err == OS_ERR_NONE)
    {
        OSTmrStart(led_timer, &err);
    }
}

void led_change(uint8_t period)
{
    uint8_t err;

    led_period = period;
    OSTmrDel(led_timer, &err);
    led_start();
}

void ipmi_common_init(void)
{
    // IPMIÉÏÏÂÎÄ»º³å³Ø
    memset(ipmi_global.ipmi_ctx_pool, 0, sizeof(ipmi_global.ipmi_ctx_pool));

    ipmi_global.flags = BMC_SYNC_TIME_NONE | (BMC_SOL_MASK & ~BMC_SOL_ON) | (BMC_HOTSWAP_MASK & BMC_HOTSWAP_ON);

    ipmi_global.adt_support = IPMI_DEV_DEVICE_ADT;
#ifdef IPMI_MODULES_SOL
    ipmi_global.adt_support |= IPMI_DEV_SOL;
#endif

    ipmi_global.timestamp = 0x00;

    ipmi_global.event_recv_addr = 0xff;     // disable event message receiver
    ipmi_global.event_recv_lun = 0x00;

    ipmi_global.bmc_global_enable = EN_SYS_EVT_LOG;
    ipmi_global.bmc_message_flags = 0x00;
    ipmi_global.channel_msg_rev_en = IPMI_CH_NUM_PRIMARY_IPMB | IPMI_CH_NUM_SYS_INTERFACE;

    ipmi_global.device_available = IPMI_DEVICE_NORMAL_OPERATION;

    ipmi_alive_bmc_map_set(ipmi_slotaddr_to_index(ipmi_global.slot_addr));
}

void ipmi_modules_init(void)
{
    ipmi_common_init();

#if defined(IPMI_CHIP_AT24CXX)
    at24xx_init();
#endif

#if (IPMI_DEV_SEL)
    ipmi_sel_init();
#endif

#if (IPMI_DEV_SDR)
    ipmi_sdr_init();
#endif
}

void ipmi_sensors_init(void)
{
#if defined(IPMI_CHIP_MAX6635)
    max6635_init();
#endif

#if defined(IPMI_CHIP_UCD9081)
    ucd9081_init();
#endif

#if defined(IPMI_CHIP_ADT7470)
    adt7470_init();
#endif

#if defined(IPMI_CHIP_INA230)
    ina230_init();
#endif

#if defined(IPMI_CHIP_MC_LOCATOR)
    mc_locator_init();
#endif

#if 0
#ifdef IPMI_CHIP_KEY
    key_init();
#endif
#endif
}

#if (defined(IPMI_MODULES_SPI0_CPLD) && 0)
char ipmi_common_get_device_id(void)
{
    uint8_t devid = 0;
    uint8_t *regaddr = (uint8_t*)LOGIC_2_R_FPGA(LOGIC_DEVICE_ID_REG);

    IPMI_LOGIC_READ(regaddr, (uint8_t*)&devid);

    return devid;
}

char ipmi_common_get_device_revision(void)
{
    uint8_t devrev = 0;
    uint8_t *regaddr = (uint8_t*)LOGIC_2_R_FPGA(LOGIC_DEVICE_REV_REG);

    IPMI_LOGIC_READ(regaddr, (uint8_t*)&devrev);

    return devrev;
}

char ipmi_common_get_product_id(void)
{
    uint8_t pdid = 0;
    uint8_t *regaddr = (uint8_t*)LOGIC_2_R_FPGA(LOGIC_PRODUCT_ID_REG);

    IPMI_LOGIC_READ(regaddr, (uint8_t*)&pdid);

    return pdid;
}

char ipmi_common_get_poh(uint32_t *poh)
{
    uint8_t *regaddr = (uint8_t*)LOGIC_2_R_FPGA(LOGIC_POH_REG);

    IPMI_LOGIC_READ(regaddr, (uint8_t*)&poh);

    return 0;
}

void ipmi_common_power_onoff(uint8_t on)
{
    return;
}

void ipmi_common_test_self(void)
{
    uint8_t buf[] = {0x5a, 0xa5, 0x5a};

    SPI_spi0_xfer(buf, 3, 0, 0);
}

#else   // IPMI_MODULES_SPI1_SSIF

char ipmi_common_get_device_id(void)
{
    return 0x01;
}

char ipmi_common_get_device_revision(void)
{
    return 0x01;
}

char ipmi_common_get_product_id(void)
{
    return 0x01;
}

char ipmi_common_get_poh(uint32_t *poh)
{
    uint32_t power_on_seconds;

    power_on_seconds = OSTimeGet() / SYSTICKHZ;

    *poh = power_on_seconds;

    return 0;
}

void ipmi_common_power_onoff(uint8_t on)
{
    return;
}

void ipmi_common_test_self(void)
{
}
#endif




