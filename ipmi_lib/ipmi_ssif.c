//*****************************************************************************
//
// ipmi_ssif.c - IPMI System Interface
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#include "ipmi.h"

void ipmi_ssif_init()
{
    // init the spi interface for slaver
}

int ipmi_ssif_recv(struct ipmi_ctx *ctx_cmd)
{
    unsigned long size;

    IPMI_SSIF_READ((char*)(&ctx_cmd->req), &size);

    return 0;
}

int ipmi_ssif_send(struct ipmi_ctx *ctx_cmd)
{
    IPMI_SSIF_WRITE((char*)(&ctx_cmd->rsp),
        ctx_cmd->rsp.msg.data_len + sizeof(ctx_cmd->rsp.msg));

    return 0;
}

