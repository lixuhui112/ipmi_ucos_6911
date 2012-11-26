//*****************************************************************************
//
// ipmi_app.h - IPMI Command Header File
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#ifndef __IPMI_APP_H__
#define __IPMI_APP_H__

#include <stdint.h>

//*****************************************************************************
//
// Defines command BMC_GET_DEVICE_ID response data structure
//
//*****************************************************************************
#define IPM_DEV_DEVICE_ID_SDR_MASK      (0x80)      /* 1 = provides SDRs      */
#define IPM_DEV_DEVICE_ID_REV_MASK      (0x0F)      /* bin-enoded             */

#define IPMI_DEVICE_NORMAL_OPERATION    0
#define IPMI_DEVICE_FIRMWARE            1


#define IPM_DEV_DEVICE_ADT       \
        (IPMI_DEV_CHASSIS << 7 | \
         IPMI_DEV_BRIDGE  << 6 | \
         IPMI_DEV_IPMB_EG << 5 | \
         IPMI_DEV_IPMB_ER << 4 | \
         IPMI_DEV_FRU     << 3 | \
         IPMI_DEV_SEL     << 2 | \
         IPMI_DEV_SDR     << 1 | \
         IPMI_DEV_SENSOR)

#define IPMI_DEV_MANUFACTURER_ID_0      0x00        /* Manufacturer ID is 26067 65d3h */
#define IPMI_DEV_MANUFACTURER_ID_1      0x65
#define IPMI_DEV_MANUFACTURER_ID_2      0xd3

struct ipmi_devid_rsp {
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

#endif  // __IPMI_APP_H__

