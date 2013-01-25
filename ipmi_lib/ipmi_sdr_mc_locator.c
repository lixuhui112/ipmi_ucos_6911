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

#define MAX_MC_COUNT                11

const unsigned char mc_locator_addr[MAX_MC_COUNT] = {       /* 7-bit slaver i2c address */
    0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b
};

SDR_RECORD_MC_LOCATOR mc_locator_sr[MAX_MC_COUNT];
sensor_data_t mc_locator_sd[MAX_MC_COUNT];
I2C_DEVICE mc_locator_dev[MAX_MC_COUNT];

void mc_locator_scan_function(void *arg)
{
    /* TODO -V2.0
     * 通过IPMB总线遍历所有板卡,并标识其在位状态 mc_locator_sd.unavailable
     */
    static uint8_t onoff = 0;

    uint32_t error;
    char mc_info[32] = {0};
    struct ipmi_req *req;
    struct mc_locator_notify_req *req_data;
    sensor_data_t *sd = (sensor_data_t *)arg;

    // test for blink led0
    IO_led0_set(onoff);
    onoff = !onoff;

    // set default sensor data attribute
    sd->unavailable = 1;
    sd->retry = 0;

    // notify the other bmc
	req = (struct ipmi_req *)&mc_info[0];

    // fill the message header
    req->msg.data_len = 7 + sizeof(struct mc_locator_notify_req);
	req->msg.rs_sa = ((SDR_RECORD_MC_LOCATOR *)(sd->sdr_record->record_ptr))->key.dev_slave_addr; // mc_locator_device_id
    req->msg.netfn = IPMI_NETFN_TRANSPORT;
    req->msg.rs_lun = 0x0;
    req->msg.checksum1 = 0xff;
    req->msg.rq_sa = I2C_i2c0_ipmb_self_addr_get();
    req->msg.rq_lun = 0x0;
    req->msg.rq_seq = 0x01;
    req->msg.cmd = NOTIFY_RMC_INFO;

    // fill the message content data
    req_data = (struct mc_locator_notify_req *)&req->data[0];
    req_data->device_id = ipmi_common_get_device_id();
    req_data->device_revision = (((IPMI_DEV_SDR|IPMI_DEV_SENSOR) ? IPM_DEV_DEVICE_ID_SDR_MASK : 0) |
                                  (ipmi_common_get_device_revision() & IPM_DEV_DEVICE_ID_REV_MASK));
    req_data->fw_rev1 = (device_available << 7) | IPMI_FIRMWARE_VER_MAJOR;
    req_data->fw_rev2 = IPMI_FIRMWARE_VER_MINOR;
    req_data->ipmi_version = IPMI_VERSION;
    req_data->adtl_device_support = IPM_DEV_DEVICE_ADT;
    req_data->manufacturer_id[0] = IPMI_DEV_MANUFACTURER_ID_2;
    req_data->manufacturer_id[1] = IPMI_DEV_MANUFACTURER_ID_1;
    req_data->manufacturer_id[2] = IPMI_DEV_MANUFACTURER_ID_0;
    req_data->product_id[0] = ipmi_common_get_product_id();
    req_data->product_id[1] = 0;
    req_data->aux_fw_rev[0] = IPMI_AUXILIARY_VERSION & 0xff;
    req_data->aux_fw_rev[1] = (IPMI_AUXILIARY_VERSION >> 8) & 0xff;
    req_data->aux_fw_rev[2] = (IPMI_AUXILIARY_VERSION >> 16) & 0xff;
    req_data->aux_fw_rev[3] = (IPMI_AUXILIARY_VERSION >> 24) & 0xff;

    if (req->msg.rs_sa == req->msg.rq_sa) {
        return;
    }

    DEBUG("mc_locator_scan addr=0x%x ", req->msg.rs_sa);
    error = I2C_i2c0_ipmb_write(req->msg.rs_sa, mc_info, req->msg.data_len);

    DEBUG(" error=0x%x\r\n", error);
    if (error) {
        if (error == I2C_MASTER_ERR_ARB_LOST) {
            sd->retry = 1;
        }
    } else {
        sd->unavailable = 0;
    }
}

void mc_locator_sett_function(void *arg)
{
}

void mc_location_alive(uint8_t addr)
{
    for (int i = 0; i < MAX_MC_COUNT; i++) {
        if (mc_locator_sr[i].key.dev_slave_addr == addr) {
            DEBUG("mc_locator_%d (addr=0x%x) is alive\r\n", i, addr);
            mc_locator_sr[i].key.alive = 1;
        } else if (mc_locator_sr[i].key.dev_slave_addr > addr) {    // clear behind alive status
            mc_locator_sr[i].key.alive = 0;
        }
    }
}

void mc_locator_init_dev(uint8_t i)
{
    I2C_i2c0_slave_dev_init(&mc_locator_dev[i], mc_locator_addr[i], 1);
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
	mc_locator_sr[i].key.dev_slave_addr         = mc_locator_addr[i];
	mc_locator_sr[i].key.channel_num            = 0;

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
	snprintf(id_string, 16, "mc-locator-%d", i);
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

    for (i = 0; i < MAX_MC_COUNT; i++)
    {
        mc_locator_init_dev(i);
        mc_locator_init_sensor_record(i);
    }
}


