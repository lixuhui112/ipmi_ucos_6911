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

struct mc_locator_notify_req {          // copy from 'struct ipmi_devid_rsp' from ipmi_app.h
	uint8_t device_id;                  // 设备ID
	uint8_t device_revision;            // 设备修订号，[7]=SDRS,[6:4]=0,[3:0]=revision
	uint8_t fw_rev1;
	uint8_t fw_rev2;
	uint8_t ipmi_version;
	uint8_t adtl_device_support;
	uint8_t manufacturer_id[3];
	uint8_t product_id[2];
	uint8_t aux_fw_rev[4];
};

void ipmi_mc_locator_notify(struct ipmi_ctx *ctx_cmd);


#endif  // __IPMI_TRANSPORT_H__

