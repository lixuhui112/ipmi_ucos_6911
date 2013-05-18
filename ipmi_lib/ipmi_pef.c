//*****************************************************************************
//
// ipmi_pef.c - IPMI PEF and Alerting Commands
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#include "ipmi_lib/ipmi.h"
#include "app/lib_common.h"

void ipmi_get_pef_capabilities(struct ipmi_ctx *ctx_cmd)
{
    DEBUG("ipmi_get_pef_capabilities\r\n");
    ipmi_cmd_invalid(ctx_cmd);
}



