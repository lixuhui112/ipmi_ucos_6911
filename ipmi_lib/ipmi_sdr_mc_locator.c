//*****************************************************************************
//
// ipmi_sdr_mc_locator.c - The Management Controller Device Locator Record
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

/******************************************************************************
TODO:
    2012/10/9

History:
    2012/10/9

******************************************************************************/


#include "ipmi_lib/ipmi.h"
#include "app/lib_common.h"
#include "app/lib_i2c.h"
#include "app/lib_io.h"
#include <string.h>
#include <stdio.h>

SDR_RECORD_MC_LOCATOR mc_locator_sr[BOARD_MAX_COUNT];
sensor_data_t mc_locator_sd[BOARD_MAX_COUNT];
I2C_DEVICE mc_locator_dev[BOARD_MAX_COUNT];

void mc_locator_scan_function(void *arg)
{
    /* TODO -V2.0
     * 通过IPMB总线遍历所有板卡,并标识其在位状态 mc_locator_sd.unavailable
     */
    uint8_t idx;
    uint32_t error;
    struct ipmi_req *req;
    struct mc_locator_notify_req *req_data;
    sensor_data_t *sd = (sensor_data_t *)arg;
    char mc_info[32] = {0};

    // test for blink led0
    {
        static uint8_t onoff = 0;
        IO_led0_set(onoff);
        onoff = !onoff;
    }

    // set default sensor data attribute
    sd->unavailable = 1;
    sd->retry = 0;

    // notify the other bmc
	req = (struct ipmi_req *)&mc_info[0];

    // fill the message header
    req->msg.data_len = sizeof(struct _ipmi_req_cmd) + sizeof(struct mc_locator_notify_req);
	req->msg.rs_sa = SDR_MC_LOCATOR_ADDR(sd->sdr_record->record_ptr);   // mc_locator_device_id
    req->msg.netfn = IPMI_NETFN_REQ_TRANSPORT;
    req->msg.rs_lun = 0x0;
    req->msg.checksum1 = 0xff;
    req->msg.rq_sa = I2C_i2c0_ipmb_self_addr_get();
    req->msg.rq_lun = 0x0;
    req->msg.rq_seq = 0x01;
    req->msg.cmd = NOTIFY_RMC_INFO;

    // fill the message content data
    req_data = (struct mc_locator_notify_req *)&req->data[0];
    req_data->alive_bmc_map = ipmi_global.alive_bmc_map;
    req_data->timestamp = ipmi_global.timestamp;
    req_data->flags = (ipmi_global.flags & BMC_SYNC_TIME_MASK);

    if (req->msg.rs_sa == req->msg.rq_sa) {
        return;
    }

    error = I2C_i2c0_ipmb_write(req->msg.rs_sa, mc_info, req->msg.data_len);
    DEBUG("mc_locator_scan rq_sa=0x%x, rs_sa=0x%x err=0x%x\r\n", req->msg.rq_sa, req->msg.rs_sa, error);
    DEBUG("old ipmi_global.alive_bmc_map=0x%08x\r\n", ipmi_global.alive_bmc_map);

    idx = ipmi_ipmbaddr_to_index(req->msg.rs_sa);

    if (error) {
        // if IIC bus lost
        if (error & I2C_MASTER_ERR_ARB_LOST) {
            sd->retry = 1;
        }

        // new state with bmc_locator, generic a event message
        if (ipmi_alive_bmc_map_get(idx)) {
            struct event_request_message evt_msg;

            evt_msg.sensor_type = SENSOR_TYPE_MODUBRD;
            evt_msg.sensor_num = req->msg.rs_sa;
            evt_msg.event_type = EVENT_TYPE_ASS_DRI;
            evt_msg.event_dir = EVENT_DIR_ASSERT;
            evt_msg.event_data[0] = 0;
            evt_msg.event_data[1] = 0;
            evt_msg.event_data[2] = 0;

            ipmi_evt_msg_generic(&evt_msg);
            DEBUG("generic event message sensor_num#0x%x\r\n", evt_msg.sensor_num);
        }

        // clear the alive/map bit
        SDR_MC_LOCATOR_ALIVE(sd->sdr_record->record_ptr, 0);
        ipmi_alive_bmc_map_clr(idx);

    } else {
        // rs_sa ack ok, remote bmc is alive
        sd->unavailable = 0;
        SDR_MC_LOCATOR_ALIVE(sd->sdr_record->record_ptr, 1);
        ipmi_alive_bmc_map_set(idx);
    }
    DEBUG("new ipmi_global.alive_bmc_map=0x%08x\r\n", ipmi_global.alive_bmc_map);
}

void mc_locator_sett_function(void *arg)
{
}

void mc_location_alive(uint8_t rq_sa, struct mc_locator_notify_req *req)
{
    DEBUG("bmc rq_sa=0x%x flags=0x%x, timestamp=0x%x, alive_bmc_map=0x%08x\r\n",
        rq_sa, req->flags, req->timestamp, req->alive_bmc_map);

    if ((ipmi_global.flags & BMC_SYNC_TIME_MASK) > (req->flags & BMC_SYNC_TIME_MASK)) {
        ipmi_global.timestamp = req->timestamp;
    }

    for (uint8_t i = 0; i < BOARD_MAX_COUNT; i++) {

        // set sensor data alive bit
        if (mc_locator_sr[i].key.dev_slave_addr == rq_sa) {
            DEBUG("mc_locator_%d (addr=0x%x) is alive\r\n", i, rq_sa);
            mc_locator_sr[i].key.alive = 1;
        }

        // set alive_bmc_map bit
        if (ipmi_board_table[i].ipmb_self_addr == rq_sa) {
            ipmi_board_table[i].ipmb_timestamp = ipmi_global.timestamp;
            ipmi_alive_bmc_map_set(i);
        }
    }
}

void mc_locator_init_dev(uint8_t i)
{
    I2C_i2c0_slave_dev_init(&mc_locator_dev[i], ipmi_board_table[i].ipmb_self_addr, 1);
}

void mc_locator_init_sensor_record(uint8_t i)
{
    char id_string[16];

	/* SENSOR RECORD HEADER */
	mc_locator_sr[i].header.record_id           = 1;
	mc_locator_sr[i].header.sdr_version         = 0x51;
	mc_locator_sr[i].header.record_type         = SDR_RECORD_TYPE_MC_DEVICE_LOCATOR;
	mc_locator_sr[i].header.record_len          = sizeof(SDR_RECORD_MC_LOCATOR);

	/* RECORD KEY BYTES */
	mc_locator_sr[i].key.dev_slave_addr         = ipmi_board_table[i].ipmb_self_addr;
	mc_locator_sr[i].key.channel_num            = 0;

	if (mc_locator_sr[i].key.dev_slave_addr == ipmi_global.ipmb_addr) {
	    mc_locator_sr[i].key.alive = 1;
	} else {
	    mc_locator_sr[i].key.alive = 0;
	}

    mc_locator_sr[i].pow_state_noti.acpi_sys_pwr_st_notify_req = 0;
    mc_locator_sr[i].pow_state_noti.acpi_dev_pwr_st_notify_req = 0;
    mc_locator_sr[i].pow_state_noti.ctrl_logs_init_errs = 0;
    mc_locator_sr[i].pow_state_noti.log_init_agent_errs = 0;
    mc_locator_sr[i].pow_state_noti.ctrl_init = 0;

    mc_locator_sr[i].dev_cap.dev_sup_chassis        = IPMI_DEV_CHASSIS;
    mc_locator_sr[i].dev_cap.dev_sup_bridge         = IPMI_DEV_BRIDGE;
    mc_locator_sr[i].dev_cap.dev_sup_ipmb_evt_gen   = IPMI_DEV_IPMB_EG;
    mc_locator_sr[i].dev_cap.dev_sup_ipmb_evt_rcv   = IPMI_DEV_IPMB_ER;
    mc_locator_sr[i].dev_cap.dev_sup_fru_inv        = IPMI_DEV_FRU;
    mc_locator_sr[i].dev_cap.dev_sup_sel            = IPMI_DEV_SEL;
    mc_locator_sr[i].dev_cap.dev_sup_sdr_rep        = IPMI_DEV_SDR;
    mc_locator_sr[i].dev_cap.dev_sup_sensor         = IPMI_DEV_SENSOR;

    mc_locator_sr[i].entity_id.id = ENTITY_ID_SYS_MGMT_MODULE;
    mc_locator_sr[i].entity_id.entity_type = 0;
    mc_locator_sr[i].entity_id.entity_instance_num = i;

    mc_locator_sr[i].oem = 0;
    mc_locator_sr[i].id_type_length.type = 3;                               /* 11 = 8-bit ASCII + Latin 1. */
	snprintf(id_string, 16, "bmc%d-%s", i, ipmi_board_table[i].board_type_str);
    mc_locator_sr[i].id_type_length.length = strlen(id_string);             /* length of following data, in characters */
    memcpy(mc_locator_sr[i].id_string_bytes, id_string, strlen(id_string));


    mc_locator_sd[i].sensor_id = 0;
    mc_locator_sd[i].last_sensor_reading = 0;
    mc_locator_sd[i].scan_period = 30;                                     /* scan mc device per 300 second */
    mc_locator_sd[i].scan_function = mc_locator_scan_function;
    mc_locator_sd[i].sett_function = mc_locator_sett_function;
    mc_locator_sd[i].event_messages_enabled = 1;
    mc_locator_sd[i].sensor_scanning_enabled = 1;
    mc_locator_sd[i].unavailable = 1;                                       /* default state is unavailable */
    mc_locator_sd[i].local_sensor_id = i;
    mc_locator_sd[i].recv1 = 0;
    mc_locator_sd[i].recv2 = 0;
    mc_locator_sd[i].recv3 = 0;

	sensor_add(&mc_locator_sd[i], SDR_RECORD_TYPE_MC_DEVICE_LOCATOR, &mc_locator_sr[i]);
}

void mc_locator_init(void)
{
    uint8_t i;

    for (i = 0; i < BOARD_MAX_COUNT; i++)
    {
        mc_locator_init_dev(i);
        mc_locator_init_sensor_record(i);
    }
}


