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
#include <string.h>
#include "app/lib_common.h"
#include "app/lib_i2c.h"
#include "ipmi_lib/ipmi.h"
#include "ucos_ii.h"

//*****************************************************************************
// Defines stack for the task
//*****************************************************************************
#define STK_SIZE    128
static OS_STK ipmi_task_stk[STK_SIZE];
static OS_STK recv_task_stk[STK_SIZE];
static OS_STK proc_task_stk[STK_SIZE];
static OS_STK send_task_stk[STK_SIZE];
static OS_STK test_task_stk[STK_SIZE];
static OS_STK mesg_task_stk[STK_SIZE];


//*****************************************************************************
// Defines receive semaphore process for interrupt
//*****************************************************************************
void ipmi_intf_recv_post(uint8_t intf)
{
    OSQPost(ipmi_global.ipmi_req_que, (void*)intf);
}


//*****************************************************************************
// Defines IPMI Command Receive Task
//*****************************************************************************
void ipmi_cmd_recv_task(void *args)
{
    uint8_t err;
    uint32_t recv_intf;
    struct ipmi_ctx *ctx_cmd;

    while (1)
    {
        ctx_cmd = ipmi_get_free_ctx_entry();
        if (ctx_cmd == NULL)
        {
            ipmi_err();
            continue;
        }

        recv_intf = (uint32_t)OSQPend(ipmi_global.ipmi_req_que, 0, &err);
        if (err)
        {
            ipmi_err();
            ipmi_put_free_ctx_entry(ctx_cmd);
            continue;
        }

        ctx_cmd->from_channel = (uint8_t)recv_intf;

        switch (recv_intf)
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

        ctx_cmd->rq_seq = ctx_cmd->req.msg.rq_seq;
        ctx_cmd->req_len = ctx_cmd->req.msg.data_len - sizeof(struct _ipmi_req_cmd);

        OSQPost(ipmi_global.ipmi_prs_que, (void*)ctx_cmd);
    }
}


//*****************************************************************************
// Defines IPMI Command Process Task
//*****************************************************************************
void ipmi_cmd_proc_task(void *args)
{
    uint8_t err = 0;
    struct ipmi_ctx *ctx_cmd;

    while (1)
    {
        ctx_cmd = (struct ipmi_ctx*)OSQPend(ipmi_global.ipmi_prs_que, 0, &err);
        if (err)
        {
            ipmi_err();
            continue;
        }

        DEBUG("proc netfn=0x%x\r\n", ctx_cmd->req.msg.netfn);
        DEBUG("proc cmd=0x%x\r\n", ctx_cmd->req.msg.cmd);
        DEBUG("proc rs_sa=0x%x\r\n", ctx_cmd->req.msg.rs_sa);
        DEBUG("proc rs_lun=0x%x\r\n", ctx_cmd->req.msg.rs_lun);
        DEBUG("proc checksum=0x%x\r\n", ctx_cmd->req.msg.checksum1);
        DEBUG("proc rq_sa=0x%x\r\n", ctx_cmd->req.msg.rq_sa);
        DEBUG("proc rq_seq=0x%x\r\n", ctx_cmd->req.msg.rq_seq);
        DEBUG("proc rq_lun=0x%x\r\n", ctx_cmd->req.msg.rq_lun);
        DEBUG("proc datalen=%d\r\n", ctx_cmd->req_len);

        switch (ctx_cmd->req.msg.netfn)
        {
            case IPMI_NETFN_REQ_CHASSIS:
                err = ipmi_cmd_chassis(ctx_cmd);
                break;

            case IPMI_NETFN_REQ_BRIDGE:
                err = ipmi_cmd_bridge(ctx_cmd);
                break;

            case IPMI_NETFN_REQ_SE:
                err = ipmi_cmd_se(ctx_cmd);
                break;

            case IPMI_NETFN_REQ_APP:
                err = ipmi_cmd_app(ctx_cmd);
                break;

            case IPMI_NETFN_REQ_FIRMWARE:
                err = ipmi_cmd_firmware(ctx_cmd);
                break;

            case IPMI_NETFN_REQ_STORAGE:
                err = ipmi_cmd_storage(ctx_cmd);
                break;

            case IPMI_NETFN_REQ_TRANSPORT:
                err = ipmi_cmd_transport(ctx_cmd);
                break;

            case IPMI_NETFN_PICMG:
                err = ipmi_cmd_picmg(ctx_cmd);
                break;

            case IPMI_NETFN_RSP_CHASSIS:
            case IPMI_NETFN_RSP_BRIDGE:
            case IPMI_NETFN_RSP_SE:
            case IPMI_NETFN_RSP_APP:
            case IPMI_NETFN_RSP_FIRMWARE:
            case IPMI_NETFN_RSP_STORAGE:
            case IPMI_NETFN_RSP_TRANSPORT:
                ipmi_cmd_response(ctx_cmd);
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
            OSQPost(ipmi_global.ipmi_rsp_que, (void*)ctx_cmd);
        }
    }
}


//*****************************************************************************
// Defines IPMI Command Send Task
//*****************************************************************************
void ipmi_cmd_send_task(void *args)
{
    INT8U err;
    struct ipmi_ctx *ctx_cmd;

    while (1)
    {
        ctx_cmd = (struct ipmi_ctx *)OSQPend(ipmi_global.ipmi_rsp_que, 0, &err);
        if (err)
        {
            ipmi_err();
            continue;
        }

        DEBUG("ipmi sendto=%d\r\n", ctx_cmd->to_channel);
        DEBUG("rsp data_len=%d\r\n", ctx_cmd->rsp.msg.data_len);
        DEBUG("rsp netfn=%d\r\n", ctx_cmd->rsp.msg.netfn);
        DEBUG("rsp cmd=%d\r\n", ctx_cmd->rsp.msg.cmd);
        DEBUG("rsp rq_sa=%d\r\n", ctx_cmd->rsp.msg.rq_sa);
        DEBUG("rsp rs_sa=%d\r\n", ctx_cmd->rsp.msg.rs_sa);
        DEBUG("rsp rq_lun=%d\r\n", ctx_cmd->rsp.msg.rq_lun);
        DEBUG("rsp rs_lun=%d\r\n", ctx_cmd->rsp.msg.rs_lun);
        DEBUG("rsp rq_seq=%d\r\n", ctx_cmd->rsp.msg.rq_seq);

        switch (ctx_cmd->to_channel)
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

        if (!(ctx_cmd->flags & IPMI_CTX_BRIDGE)) {
            ipmi_put_free_ctx_entry(ctx_cmd);
        }
    }
}

void ipmi_msg_proc_task(void *args)
{
    uint8_t err;
    uint16_t new_sel_id;
    sel_event_record newsel;
    struct event_request_message *evm;
    struct ipmi_req *req;
    char msg_cmd[32] = {0};

    while (1)
    {
        evm = (struct event_request_message *)OSQPend(ipmi_global.ipmi_msg_que, 0, &err);
        if (err)
        {
            ipmi_err();
            continue;
        }

        newsel.record_type = 0x02;
        newsel.sel_type.standard_type.id_type = 0x1;
        newsel.sel_type.standard_type.ipmb_slave_addr = 0x0;
        newsel.sel_type.standard_type.ipmb_dev_lun = 0x0;
        newsel.sel_type.standard_type.channel_number = 0x0;
        newsel.sel_type.standard_type.evm_rev = evm->evm_rev;
        newsel.sel_type.standard_type.sensor_type = evm->sensor_type;
        newsel.sel_type.standard_type.sensor_num = evm->sensor_num;
        newsel.sel_type.standard_type.event_type = evm->event_type;
        newsel.sel_type.standard_type.event_dir = evm->event_dir;
        newsel.sel_type.standard_type.event_data[0] = evm->event_data[0];
        newsel.sel_type.standard_type.event_data[1] = evm->event_data[1];
        newsel.sel_type.standard_type.event_data[2] = evm->event_data[2];

        // add sel to eeprom
        new_sel_id = ipmi_add_sel(&newsel, newsel.record_type);
        if (new_sel_id == 0)
        {
            ipmi_err();
        }

        // event receiver is enable, send the event message to IPMB
        if (ipmi_global.event_recv_addr != 0xff)
        {
            req = (struct ipmi_req *)&msg_cmd[0];

            req->msg.data_len = sizeof(struct _ipmi_req_cmd) + sizeof(struct event_request_message);
            req->msg.rs_sa = ipmi_global.event_recv_addr;
            req->msg.rs_lun = ipmi_global.event_recv_lun;;
            req->msg.netfn = PLATFORM_EVENT;
            req->msg.cmd = IPMI_NETFN_REQ_SE;
            req->msg.rq_sa = ipmi_global.ipmb_addr;
            req->msg.rq_lun = 0x0;
            req->msg.rq_seq = 0x01;
            req->msg.checksum1 = 0xff;
            memcpy(&req->data[0], evm, sizeof(struct event_request_message));

            err = I2C_i2c0_ipmb_write(req->msg.rs_sa, msg_cmd, req->msg.data_len);
            DEBUG("send msg to receiver rq_sa=0x%x, rs_sa=0x%x err=0x%x\r\n", req->msg.rq_sa, req->msg.rs_sa, err);
        }
    }
}

//*****************************************************************************
// Defines IPMI Test Task
//*****************************************************************************
void ipmi_cmd_test_task(void *args)
{
#if 0
    // SPI0 master to FPGA test
    {
        uint8_t spibuf[3] = {0};
        uint8_t i;

        while (1)
        {
#if 1
            for (i = 0; i < 15; i++)
            {
                spibuf[0] = 0x80;
                spibuf[1] = i;
                spibuf[2] = i;

                SPI_spi0_xfer(spibuf, 3, 0, 0);
                DEBUG("write_spi data=0x%x\r\n", spibuf[2]);
            }

            OSTimeDlyHMSM(0, 0, 1, 0);
#endif
            for (i = 0; i < 15; i++)
            {
                spibuf[0] = 0x00;
                spibuf[1] = i;
                spibuf[2] = 0;

                SPI_spi0_xfer(spibuf, 2, &spibuf[2], 1);
                DEBUG("read_spi data=0x%x\r\n", spibuf[2]);
            }

            OSTimeDlyHMSM(0, 0, 3, 0);
        }

    }
#endif

#if 0
    {
#define IPMB_SELF_ADDR  (0x71)
#define SLAVE_ADDR      (0x73)          // 定义从机地址
#if 0
        SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);                 // 使能I2C模块
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);                // 使能I2C管脚所在的GPIO模块
        GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_2 | GPIO_PIN_3);   // 配置相关管脚为I2C收发功能

        I2CSlaveInit(I2C0_SLAVE_BASE, SLAVE_ADDR);                  // I2C从机模块初始化
        I2CSlaveIntEnable(I2C0_SLAVE_BASE);                         // 使能I2C从机模块中断
        IntEnable(INT_I2C0);                                        // 使能I2C中断
        I2CSlaveEnable(I2C0_SLAVE_BASE);                            // 使能I2C从机
#endif
        {
            unsigned char ulDataTx[128] = {0};
            unsigned char i;
            unsigned long error;
            struct ipmi_req *req;

            ulDataTx[0] = IPMI_FRAME_CHAR[0];
            ulDataTx[1] = IPMI_FRAME_CHAR[1];
            ulDataTx[2] = IPMI_FRAME_CHAR[2];

            req = (struct ipmi_req *)&ulDataTx[3];

            req->msg.data_len = 7;
            req->msg.rs_sa = SLAVE_ADDR;
            req->msg.netfn = 0x06;
            req->msg.rs_lun = 0x0;
            req->msg.checksum1 = 0xff;
            req->msg.rq_sa = IPMB_SELF_ADDR;
            req->msg.rq_lun = 0x0;
            req->msg.rq_seq = 0x01;
            req->msg.cmd = 0x01;

            while (1) {
                OSTimeDlyHMSM(0, 0, 5, 0);
                DEBUG("netfn=0x%x cmd=0x%x rs_sa=0x%x rq_sa=0x%x ",
                    req->msg.netfn, req->msg.cmd, req->msg.rs_sa, req->msg.rq_sa);
                error = I2C_dev_write(I2C0_MASTER_BASE, SLAVE_ADDR, 0, 1, 20, &ulDataTx[0]);
                DEBUG("error=0x%x\r\n", error);
                //SysCtlDelay(10000000);
            }
        }
    }
#endif

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
// Defines IPMI Main Task
//*****************************************************************************
void ipmi_task_main(void *args)
{
    // 信号量初始化
    ipmi_global.ipmi_sem = OSSemCreate(1);

    // 消息队列初始化
    ipmi_global.ipmi_req_que = OSQCreate(&ipmi_global.ipmi_req_que_pool[0], IPMI_MSG_QUE_SIZE);
    ipmi_global.ipmi_prs_que = OSQCreate(&ipmi_global.ipmi_prs_que_pool[0], IPMI_MSG_QUE_SIZE);
    ipmi_global.ipmi_rsp_que = OSQCreate(&ipmi_global.ipmi_rsp_que_pool[0], IPMI_MSG_QUE_SIZE);
    ipmi_global.ipmi_msg_que = OSQCreate(&ipmi_global.ipmi_msg_que_pool[0], IPMI_MSG_QUE_SIZE);

    ipmi_modules_init();
    ipmi_sensors_init();

    // IPMI任务
    OSTaskCreate(ipmi_cmd_send_task, (void*)0, (OS_STK*)&send_task_stk[STK_SIZE-1], (INT8U)4);
    OSTaskCreate(ipmi_cmd_proc_task, (void*)0, (OS_STK*)&proc_task_stk[STK_SIZE-1], (INT8U)5);
    OSTaskCreate(ipmi_cmd_recv_task, (void*)0, (OS_STK*)&recv_task_stk[STK_SIZE-1], (INT8U)6);
    OSTaskCreate(ipmi_msg_proc_task, (void*)0, (OS_STK*)&mesg_task_stk[STK_SIZE-1], (INT8U)7);
    OSTaskCreate(ipmi_cmd_test_task, (void*)0, (OS_STK*)&test_task_stk[STK_SIZE-1], (INT8U)30);

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


