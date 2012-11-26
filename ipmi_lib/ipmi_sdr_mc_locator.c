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
#include <string.h>
#include <stdio.h>

#define MAX_MC_COUNT                11

const unsigned char mc_locator_addr[MAX_MC_COUNT] = {
    0x80, 0x82, 0x84, 0x86, 0x88, 0x8a, 0x8c, 0x8e, 0x90, 0x92, 0x94
};

SDR_RECORD_MC_LOCATOR mc_locator_sr[MAX_MC_COUNT];
sensor_data_t mc_locator_sd[MAX_MC_COUNT];
I2C_DEVICE mc_locator_dev[MAX_MC_COUNT];

void mc_locator_scan_function(void *arg)
{
    /* TODO -V2.0
     * 通过IPMB总线遍历所有板卡,并标识其在位状态 mc_locator_sd.unavailable
     */
}

void mc_locator_sett_function(void *arg)
{
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
	mc_locator_sr[i].key.dev_slave_addr         = (mc_locator_addr[i] >> 1);
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
    mc_locator_sr[i].entity_id.entity_instance_num = 0;

    mc_locator_sr[i].oem = 0;
    mc_locator_sr[i].id_type_length.type = 3;                               /* 11 = 8-bit ASCII + Latin 1. */
	snprintf(id_string, 16, "mc-locator-%d", i);
    mc_locator_sr[i].id_type_length.length = strlen(id_string);             /* length of following data, in characters */
    memcpy(mc_locator_sr[i].id_string_bytes, id_string, strlen(id_string));


    mc_locator_sd[i].sensor_id = 0;
    mc_locator_sd[i].last_sensor_reading = 0;
    mc_locator_sd[i].scan_period = 10;                                      /* scan mc device per 10 second */
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


