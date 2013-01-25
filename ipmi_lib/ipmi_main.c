//*****************************************************************************
//
// ipmi_main.c - IPMI system main process file
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#include <inttypes.h>
#include <inc/hw_types.h>
#include <string.h>
#include "app/lib_common.h"
#include "app/lib_i2c.h"
#include "ipmi_lib/ipmi.h"
#include "ucos_ii.h"
#include "third_party/ustdlib.h"

//*****************************************************************************
//
// Defines stack for the task
//
//*****************************************************************************
#define STK_SIZE    128
static OS_STK ipmi_task_stk[STK_SIZE];
static OS_STK recv_task_stk[STK_SIZE];
static OS_STK proc_task_stk[STK_SIZE];
static OS_STK send_task_stk[STK_SIZE];
static OS_STK test_task_stk[STK_SIZE];

//*****************************************************************************
//
// Defines semaphore for the task
//
//*****************************************************************************
static OS_EVENT *ipmi_recv_sem;
//static OS_EVENT *ipmi_send_sem;


//*****************************************************************************
//
// Defines semaphore for the task
//
//*****************************************************************************
#define IPMI_CMD_QUE_SIZE       4
static OS_EVENT *ipmi_req_que;
static OS_EVENT *ipmi_rsp_que;
void *ipmi_req_que_pool[IPMI_CMD_QUE_SIZE];
void *ipmi_rsp_que_pool[IPMI_CMD_QUE_SIZE];


//*****************************************************************************
//
// Defines context of IPMI Message
//
//*****************************************************************************
#define IPMI_CTX_POOL_SIZE      4
struct ipmi_ctx ipmi_ctx_pool[IPMI_CTX_POOL_SIZE];

/* TODO:
 * 注意临界区问题
 */
struct ipmi_ctx *ipmi_get_free_ctx_entry(void)
{
    int i;

    for (i = 0; i < IPMI_CTX_POOL_SIZE; i++)
    {
        if ((ipmi_ctx_pool[i].flags & IPMI_CTX_ENABLE) == 0)
            break;
    }

    if (i == IPMI_CTX_POOL_SIZE)
    {
        return NULL;    // the entry is full
    }

    ipmi_ctx_pool[i].flags |= IPMI_CTX_ENABLE;
    return &ipmi_ctx_pool[i];
}

void ipmi_put_free_ctx_entry(struct ipmi_ctx *ctx)
{
    memset(ctx, 0, sizeof(struct ipmi_ctx));
}

//*****************************************************************************
//
// Defines Front Character Frame of IPMI Message
//
//*****************************************************************************
uint8_t IPMI_FRAME_CHAR[IPMI_FRAME_CHAR_SIZE] = {0x0f, 0xf0, 0x5a};

//*****************************************************************************
//
// Defines sequence of IPMI Message
//
//*****************************************************************************
uint8_t seq_array[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

//*****************************************************************************
//
// sequence number generator
//
//*****************************************************************************
uint8_t ipmi_get_next_seq(uint8_t *seq)
{
	uint8_t i;

	/* return the first free sequence number */
	for (i = 0; i < 16; i++)
	{
		if (!seq_array[i])
		{
			seq_array[i] = 1;
			*seq = i;
			return (1);
		}
	}
	return (0);
}

void ipmi_seq_free(uint8_t seq)
{
	seq_array[seq] = 0;
}


//*****************************************************************************
//
// Defines IPMI Device available
//
//*****************************************************************************
unsigned long device_available = IPMI_DEVICE_NORMAL_OPERATION;


//*****************************************************************************
//
// Defines receive flag of interrupt
//
//*****************************************************************************
unsigned long ipmi_recv_intf;


//*****************************************************************************
//
// Defines receive semaphore process for interrupt
//
//*****************************************************************************
void ipmi_intf_recv_post(int intf)
{
    ipmi_recv_intf = intf;
    OSSemPost(ipmi_recv_sem);
}

//*****************************************************************************
//
// Defines IPMI Command Receive Task
//
//*****************************************************************************
void ipmi_cmd_recv_task(void *args)
{
    INT8U err;
    struct ipmi_ctx *ctx_cmd;

    while (1)
    {
        ctx_cmd = ipmi_get_free_ctx_entry();
        if (ctx_cmd == NULL)
        {
            ipmi_err();
            continue;
        }

        OSSemPend(ipmi_recv_sem, 0, &err);

        if (err)
        {
            ipmi_err();
            continue;
        }

        switch (ipmi_recv_intf)
        {
#ifdef IPMI_MODULES_UART0_DEBUG
            case IPMI_INTF_DEBUG:
                DEBUG("recv from debug\r\n");
                err = ipmi_debug_recv(ctx_cmd);
                break;
#endif

#ifdef IPMI_MODULES_SPI1_SSIF
            case IPMI_INTF_SSIF:
                DEBUG("recv from ssif\r\n");
                err = ipmi_ssif_recv(ctx_cmd);
                break;
#endif

#ifdef IPMI_MODULES_UART1_ICMB
            case IPMI_INTF_ICMB:
                DEBUG("recv from icmb\r\n");
                err = ipmi_icmb_recv(ctx_cmd);
                break;
#endif

#ifdef IPMI_MODULES_I2C0_IPMB
            case IPMI_INTF_IPMB:
                DEBUG("recv from ipmb\r\n");
                err = ipmi_ipmb_recv(ctx_cmd);
                break;
#endif

#ifdef IPMI_MODULES_ETH_LAN
            case IPMI_INTF_ETH:
                DEBUG("recv from eth\r\n");
                err = ipmi_eth_recv(ctx_cmd);
                break;
#endif
            default:
                // err no interface
                break;
        }

        if (err)
        {
            ipmi_err();
            ipmi_put_free_ctx_entry(ctx_cmd);
            continue;
        }

        ctx_cmd->channel = ipmi_recv_intf;
        ctx_cmd->rq_seq = ctx_cmd->req.msg.rq_seq;
        ctx_cmd->req_len = ctx_cmd->req.msg.data_len - sizeof(struct _ipmi_req_cmd);

        ipmi_recv_intf = 0;

        OSQPost(ipmi_req_que, (void*)ctx_cmd);
    }
}


//*****************************************************************************
//
// Defines IPMI Command Process Task
//
//*****************************************************************************
void ipmi_cmd_proc_task(void *args)
{
    INT8U err = 0;
    struct ipmi_ctx *ctx_cmd;

    while (1)
    {
        ctx_cmd = (struct ipmi_ctx*)OSQPend(ipmi_req_que, 0, &err);
        if (err)
        {
            ipmi_err();
            continue;
        }

        DEBUG("rs_sa=0x%x\r\n", ctx_cmd->req.msg.rs_sa);
        DEBUG("netfn=0x%x\r\n", ctx_cmd->req.msg.netfn);
        DEBUG("rs_lun=0x%x\r\n", ctx_cmd->req.msg.rs_lun);
        DEBUG("checksum=0x%x\r\n", ctx_cmd->req.msg.checksum1);
        DEBUG("rq_sa=0x%x\r\n", ctx_cmd->req.msg.rq_sa);
        DEBUG("rq_seq=0x%x\r\n", ctx_cmd->req.msg.rq_seq);
        DEBUG("rq_lun=0x%x\r\n", ctx_cmd->req.msg.rq_lun);
        DEBUG("cmd=0x%x\r\n", ctx_cmd->req.msg.cmd);
        DEBUG("datalen=%d\r\n", ctx_cmd->req_len);

        switch (ctx_cmd->req.msg.netfn)
        {
            case IPMI_NETFN_CHASSIS:
                err = ipmi_cmd_chassis(ctx_cmd);
                break;

            case IPMI_NETFN_BRIDGE:
                err = ipmi_cmd_bridge(ctx_cmd);
                break;

            case IPMI_NETFN_SE:
                err = ipmi_cmd_se(ctx_cmd);
                break;

            case IPMI_NETFN_APP:
                err = ipmi_cmd_app(ctx_cmd);
                break;

            case IPMI_NETFN_FIRMWARE:
                err = ipmi_cmd_firmware(ctx_cmd);
                break;

            case IPMI_NETFN_STORAGE:
                err = ipmi_cmd_storage(ctx_cmd);
                break;

            case IPMI_NETFN_TRANSPORT:
                err = ipmi_cmd_transport(ctx_cmd);
                break;

            case IPMI_NETFN_PICMG:
                err = ipmi_cmd_picmg(ctx_cmd);
                break;

            default:
                ipmi_cmd_invalid(ctx_cmd);
                break;
        }

        DEBUG("proc netfn, err=%d\r\n", err);

        if (err)
        {
            ipmi_err();
            ipmi_put_free_ctx_entry(ctx_cmd);
            continue;
        }

        if (ctx_cmd->flags & IPMI_CTX_NO_RESPONSE)
        {
            ipmi_put_free_ctx_entry(ctx_cmd);
        }
        else
        {
            OSQPost(ipmi_rsp_que, (void*)ctx_cmd);
        }
    }
}


//*****************************************************************************
//
// Defines IPMI Command Send Task
//
//*****************************************************************************
void ipmi_cmd_send_task(void *args)
{
    INT8U err;
    struct ipmi_ctx *ctx_cmd;

    while (1)
    {
        ctx_cmd = (struct ipmi_ctx *)OSQPend(ipmi_rsp_que, 0, &err);
        if (err)
        {
            ipmi_err();
            continue;
        }

        DEBUG("ipmi sendto=%d\r\n", ctx_cmd->channel);

        switch (ctx_cmd->channel)
        {
#ifdef IPMI_MODULES_UART0_DEBUG
            case IPMI_INTF_DEBUG:
                err = ipmi_debug_send(ctx_cmd);
                // write to uart direct
                break;
#endif

#ifdef IPMI_MODULES_SPI1_SSIF
            case IPMI_INTF_SSIF:
                err = ipmi_ssif_send(ctx_cmd);
                // add to ssif queue
                break;
#endif

#ifdef IPMI_MODULES_I2C0_IPMB
            case IPMI_INTF_IPMB:
                err = ipmi_ipmb_send(ctx_cmd);
                // master write to ipmb
                break;
#endif

#ifdef IPMI_MODULES_UART1_ICMB
            case IPMI_INTF_ICMB:
                err = ipmi_icmb_send(ctx_cmd);
                // write to uart direct
                break;
#endif

#ifdef IPMI_MODULES_ETH_LAN
            case IPMI_INTF_ETH:
                // write to ethernet direct
                break;
#endif

            default:
                // log error
                break;
        }
        ipmi_put_free_ctx_entry(ctx_cmd);
    }
#if 0
    while (1) {
        OSTimeDlyHMSM(0, 1, 0, 0);
    }
#endif
}

void ipmi_cmd_test_task(void *args)
{
#if 0
    // test for eeprom

    uint8_t val[32] = { 0 };
    uint8_t i;
    uint32_t error, addr;
    I2C_DEVICE at24xx_dev;

    I2C_i2c1_slave_dev_init(&at24xx_dev, 0x50, 2);

#if 0
    // set val to i
    for (i = 0; i < 32; i++) {
        val[i] = i;
    }

    // write to eeprom
    I2C_i2c1_slave_dev_set(&at24xx_dev, 0x1800, (uint8_t*)&val[0], 32);
    error = I2C_i2c1_master_write(&at24xx_dev);
    if (error)
    {
        DEBUG("write error=0x%x\r\n", error);
    }
    else
    {
        DEBUG("write to eeprom ok\r\n");
    }

    OSTimeDlyHMSM(0, 0, 1, 0);
#endif

#if 0
    // clear eeprom
    error = at24xx_clear(0x1800, 0x800);
    if (error)
    {
        DEBUG("clear error=0x%x\r\n", error);
    }
    else
    {
        DEBUG("clear OK\r\n");
    }
#endif

    // read eeprom
    for (addr = 0x1800; addr < 0x2000; addr += 32) {

        // clear val
        for (i = 0; i < 32; i++) {
            val[i] = 0;
        }

        // read from eeprom
        error = at24xx_read(addr, val, 32);
        //I2C_i2c1_slave_dev_set(&at24xx_dev, addr, (uint8_t*)&val[0], 32);
        //error = I2C_i2c1_master_read(&at24xx_dev);
        if (error)
        {
            DEBUG("read error=0x%x\r\n", error);
        }
        else
        {
            // show the read
            for (i = 0; i < 32; i++) {
                if (i % 8 == 0) {
                    DEBUG("\r\n0x%04x ", addr+i);
                }
                DEBUG("0x%02x ", val[i]);
            }
        }
    }


    while (1)
    {
        OSTimeDlyHMSM(0, 0, 1, 0);
    }
#endif


#if 0
    // test for i2c1/ipmb
    I2C_DEVICE ipmb_dev;
    uint32_t error;
    //uint8_t addr = 0;
    uint8_t buffer[4] = {0x00, 0x00, 0x00, 0x00};

    I2C_i2c1_slave_dev_init(&ipmb_dev, 0x41, 1);

#if 1
    while (1)
    {
        // write to i2c0
        //addr = (addr + 1) & 0xff;

        I2C_i2c1_slave_dev_set(&ipmb_dev, 0x00, (uint8_t*)&buffer[0], 2);
        error = I2C_i2c1_master_read(&ipmb_dev);
        if (error)
        {
            DEBUG("i2c0 error=0x%x\r\n", error);
        }
        else
        {
            DEBUG("ina230 cfg=0x%x\r\n", (buffer[0] << 8) | buffer[1]);
        }

        I2C_i2c1_slave_dev_set(&ipmb_dev, 0x01, (uint8_t*)&buffer[0], 2);
        error = I2C_i2c1_master_read(&ipmb_dev);
        if (error)
        {
            DEBUG("i2c0 error=0x%x\r\n", error);
        }
        else
        {
            DEBUG("ina230 shunt=0x%x\r\n", (buffer[0] << 8) | buffer[1]);
        }

        I2C_i2c1_slave_dev_set(&ipmb_dev, 0x02, (uint8_t*)&buffer[0], 2);
        error = I2C_i2c1_master_read(&ipmb_dev);
        if (error)
        {
            DEBUG("i2c0 error=0x%x\r\n", error);
        }
        else
        {
            DEBUG("ina230 bus=0x%x\r\n", (buffer[0] << 8) | buffer[1]);
        }

        buffer[0] = 0x2e;
        buffer[1] = 0xcf;
        I2C_i2c1_slave_dev_set(&ipmb_dev, 0x05, (uint8_t*)&buffer[0], 2);
        error = I2C_i2c1_master_write(&ipmb_dev);
        if (error)
        {
            DEBUG("i2c0 error=0x%x\r\n", error);
        }
        else
        {
            DEBUG("ina230 cal=0x%x\r\n", (buffer[0] << 8) | buffer[1]);
        }

        I2C_i2c1_slave_dev_set(&ipmb_dev, 0x04, (uint8_t*)&buffer[0], 2);
        error = I2C_i2c1_master_read(&ipmb_dev);
        if (error)
        {
            DEBUG("i2c0 error=0x%x\r\n", error);
        }
        else
        {
            DEBUG("ina230 cur=0x%x\r\n", (buffer[0] << 8) | buffer[1]);
        }

        I2C_i2c1_slave_dev_set(&ipmb_dev, 0x03, (uint8_t*)&buffer[0], 2);
        error = I2C_i2c1_master_read(&ipmb_dev);
        if (error)
        {
            DEBUG("i2c0 error=0x%x\r\n", error);
        }
        else
        {
            DEBUG("ina230 pow=0x%x\r\n", (buffer[0] << 8) | buffer[1]);
        }

        OSTimeDlyHMSM(0, 0, 3, 0);
    }
#endif

#if 0
    while (1)
    {
        // read device id
        temp_val[0] = temp_val[1] = 0;
        I2C_i2c1_slave_dev_set(&adt7470_dev, 0x3d, (uint8_t*)&temp_val[0], 1);
        error = I2C_i2c1_master_read(&adt7470_dev);
        if (error)
        {
            DEBUG("error=0x%x\r\n", error);
        }
        else
        {
            DEBUG("device id=0x%x\t", temp_val[0]);
        }
        OSTimeDlyHMSM(0, 0, 1, 0);

        DEBUG("\r\n");
    }
#endif
#endif

#if 1
    // test empty
    while (1)
    {
        OSTimeDlyHMSM(0, 0, 1, 0);
    }
#endif
}


//*****************************************************************************
//
// Defines IPMI Main Task
//
//*****************************************************************************
void ipmi_task_main(void *args)
{
    // 读写任务信号量
    ipmi_recv_sem = OSSemCreate(0);
    //ipmi_send_sem = OSSemCreate(0);

    // IPMI请求相应消息队列
    ipmi_req_que = OSQCreate((void**)&ipmi_req_que_pool, IPMI_CMD_QUE_SIZE);
    ipmi_rsp_que = OSQCreate((void**)&ipmi_rsp_que_pool, IPMI_CMD_QUE_SIZE);

    // IPMI上下文缓冲池
    memset(ipmi_ctx_pool, 0, sizeof(ipmi_ctx_pool));

    ipmi_modules_init();
    ipmi_sensors_init();

    // IPMI任务
    OSTaskCreate(ipmi_cmd_recv_task, (void*)0, (OS_STK*)&recv_task_stk[STK_SIZE-1], (INT8U)6);
    OSTaskCreate(ipmi_cmd_proc_task, (void*)0, (OS_STK*)&proc_task_stk[STK_SIZE-1], (INT8U)5);
    OSTaskCreate(ipmi_cmd_send_task, (void*)0, (OS_STK*)&send_task_stk[STK_SIZE-1], (INT8U)4);
    OSTaskCreate(ipmi_cmd_test_task, (void*)0, (OS_STK*)&test_task_stk[STK_SIZE-1], (INT8U)7);

    // IPMI定时器
    ipmi_timer_init();

    while (1)
    {
        OSTimeDlyHMSM(0, 1, 0, 0);
    }

}

void ipmi_main_start(void)
{
    // 操作系统初始化
    OSInit();

    // 创建任务并运行
    OSTaskCreate(ipmi_task_main, (void*)0, (OS_STK*)&ipmi_task_stk[STK_SIZE-1], (INT8U)1);
    OSTimeSet(0);
    OSStart();
}


