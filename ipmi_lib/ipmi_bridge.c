//*****************************************************************************
//
// ipmi_bridge.c - IPMI Command for Bridge
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#include "ipmi_lib/ipmi.h"
#include "app/lib_common.h"

int ipmi_cmd_bridge(struct ipmi_ctx *ctx_cmd)
{
    DEBUG("ipmi_cmd_bridge\r\n");
    ipmi_cmd_invalid(ctx_cmd);
    return 0;
}

