//*****************************************************************************
//
// ipmi_transport.h - IPMI Transport Header File
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#ifndef __IPMI_TRANSPORT_H__
#define __IPMI_TRANSPORT_H__

#include "ipmi_lib/ipmi.h"
#include "app/lib_common.h"


struct mc_locator_notify_req {          //
    uint32_t flags;                     // 标志位               @see@ ipmi_global_t.flags
    uint32_t timestamp;                 // 本地时间             from 1970/1/1
	uint32_t alive_bmc_map;             // 活动BMC设备映射表    @see@ ipmi_global_t.alive_bmc_map
};

void ipmi_mc_locator_notify(struct ipmi_ctx *ctx_cmd);


#endif  // __IPMI_TRANSPORT_H__

