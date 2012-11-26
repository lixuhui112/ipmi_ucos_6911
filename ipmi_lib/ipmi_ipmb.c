//*****************************************************************************
//
// ipmi_ipmb.c - IPMI Intelligent Platform Management Bus Interface
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#include "ipmi.h"

void ipmi_ipmb_init()
{
    // init the ipmb interface for master/slaver
}

int ipmi_ipmb_recv(struct ipmi_ctx *ctx_cmd)
{
    unsigned long size;

    IPMI_IPMB_READ((char*)(&ctx_cmd->req), &size);

    return 0;
}

int ipmi_ipmb_send(struct ipmi_ctx *ctx_cmd)
{
    IPMI_IPMB_WRITE(ctx_cmd->channel, (char*)(&ctx_cmd->rsp),
        ctx_cmd->rsp.msg.data_len + sizeof(ctx_cmd->rsp.msg));

    return 0;
}


