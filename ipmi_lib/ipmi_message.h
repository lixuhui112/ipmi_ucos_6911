//*****************************************************************************
//
// ipmi_message.h - IPMI Messaging Interfaces Header File
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#ifndef __IPMI_MESSAGE_H__
#define __IPMI_MESSAGE_H__

#include <stdlib.h>
#include "ipmi.h"
#include "ucos_ii.h"
#include "app/lib_common.h"

struct event_request_message {
    uint8_t evm_rev;                /* Event Message Revision, 04h */
    uint8_t sensor_type;            /* see Table 42-3, Sensor Type Codes */
    uint8_t sensor_num;             /* A unique number within a given sensor device */
#ifdef __LITTLE_ENDIAN__
    uint8_t event_type:7,           /* See Section 42, Sensor and Event Code Tables */
            event_dir:1;            /* Indicates the event transition direction */
#else
    uint8_t event_dir:1,
            event_type:7;
#endif
    uint8_t event_data[3];          /* see Table 29-6, Event Request Message Event Data Field Contents */

};

uint8_t ipmi_evt_msg_generic(struct event_request_message *evt_msg);

#endif  // __IPMI_MESSAGE_H__

