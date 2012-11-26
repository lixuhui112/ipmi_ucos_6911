//*****************************************************************************
//
// ipmi_transport.c - IPMI Command for Transport
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#include "ipmi_lib/ipmi.h"
#include "app/lib_common.h"

int ipmi_cmd_transport(struct ipmi_ctx *ctx_cmd)
{
    DEBUG("ipmi_cmd_transport\r\n");
    ipmi_cmd_invalid(ctx_cmd);
    return 0;
}



