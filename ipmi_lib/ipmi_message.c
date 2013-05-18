//*****************************************************************************
//
// ipmi_message.c - IPMI Messaging Interfaces
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************
#include <stdlib.h>
#include "ipmi.h"
#include "ucos_ii.h"
#include "app/lib_common.h"

struct event_request_message g_evm;

uint8_t ipmi_evt_msg_generic(struct event_request_message *evt_msg)
{
    if (!BIT_TST(ipmi_global.bmc_global_enable, EN_EVT_MSG_BUF)) {
        return 0;
    }

    memcpy((char*)&g_evm, (char*)evt_msg, sizeof(struct event_request_message));
    g_evm.evm_rev = 0x4;

    return OSQPost(ipmi_global.ipmi_msg_que, (void*)&g_evm);
}



