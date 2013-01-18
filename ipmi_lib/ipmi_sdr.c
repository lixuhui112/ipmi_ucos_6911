//*****************************************************************************
//
// ipmi_sdr.c - IPMI Command for Sensor Data Record Repository
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#include "ipmi_lib/ipmi.h"
#include "app/lib_common.h"
#include <string.h>

//sdr_event_header g_ipmi_sdr_header;


void ipmi_get_sdr_repository_info(struct ipmi_ctx *ctx_cmd)
{
    ipmi_cmd_err(ctx_cmd, IPMI_CC_INV_CMD);
}


void ipmi_sdr_init(void)
{
#if 0
    int error;

    error = at24xx_read(4096, (uint8_t*)&g_ipmi_sel_header, sizeof(sel_event_header));
    if (error) {
        g_ipmi_sel_header.number_of_entries = 0;
        g_ipmi_sel_header.free_space = 0;
        g_ipmi_sel_header.most_recent_addition_timestamp = 0;
        g_ipmi_sel_header.most_recent_erase_timestamp = 0;
    }
#endif
}


