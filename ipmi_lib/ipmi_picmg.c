//*****************************************************************************
//
// ipmi_picmg.c - IPMI Command for PICMG commands
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#include "ipmi_lib/ipmi.h"
#include "app/lib_common.h"

int ipmi_cmd_picmg(struct ipmi_ctx *ctx_cmd)
{
    DEBUG("ipmi_cmd_picmg\r\n");
    ipmi_cmd_invalid(ctx_cmd);
    return 0;
}



