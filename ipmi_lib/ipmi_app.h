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

#pragma pack(1)
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

struct ipmi_guid {
	uint32_t  time_low;	                /* timestamp low field */
	uint16_t  time_mid;	                /* timestamp middle field */
	uint16_t  time_hi_and_version;      /* timestamp high field and version number */
	uint8_t   clock_seq_hi_variant;     /* clock sequence high field and variant */
	uint8_t   clock_seq_low;            /* clock sequence low field */
	uint8_t   node[6];	                /* node */
};

struct ipmi_get_channel_info_st {
#ifdef __LITTLE_ENDIAN__
    uint8_t actual_channel_number:4,
            reserved1:4;
#else
    uint8_t reserved1:4,
            actual_channel_number:4;
#endif
#ifdef __LITTLE_ENDIAN__
    uint8_t channel_medium_type:7,
            reserved2:1;
#else
    uint8_t reserved2:1,
            channel_medium_type:7;
#endif
#ifdef __LITTLE_ENDIAN__
    uint8_t channel_protocol_type:5,
            reserved:3;
#else
    uint8_t reserved:3,
            channel_protocol_type:5;
#endif
#ifdef __LITTLE_ENDIAN__
    uint8_t active_session_count:6,
            session_support:2;
#else
    uint8_t session_support:2,
            active_session_count:6;
#endif
    uint8_t vendor_id[3];
    uint8_t auxiliary_channel_info[2];
};

struct get_message_rsp {
#ifdef __LITTLE_ENDIAN__
    uint8_t ch_num:4,
            infr_priv_level:4;
#else
    uint8_t infr_priv_level:4,
            ch_num:4;
#endif
    uint8_t msg_data[IPMI_BUF_SIZE-1];
};

struct send_message_req {
#ifdef __LITTLE_ENDIAN__
    uint8_t send_ch_num:4,
            with_auth:1,
            with_encrpt:1,
            track:2;
#else
    uint8_t track:2,
            with_encrpt:1,
            with_auth:1,
            send_ch_num:4;
#endif
    uint8_t msg_data[IPMI_BUF_SIZE-2];
};
#pragma pack()

#endif  // __IPMI_APP_H__

