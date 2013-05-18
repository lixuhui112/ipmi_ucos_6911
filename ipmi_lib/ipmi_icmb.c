//*****************************************************************************
//
// ipmi_icmb.c - IPMI Intelligent Chassis Management Bus Interface
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#include "ipmi.h"

#ifdef IPMI_MODULES_UART0_DEBUG
void ipmi_debug_init(void)
{
}

int ipmi_debug_recv(struct ipmi_ctx *ctx_cmd)
{
    unsigned long size;

    IPMI_DBGU_READ((char*)(&ctx_cmd->req), &size);

    return 0;
}

int ipmi_debug_send(struct ipmi_ctx *ctx_cmd)
{
    IPMI_DBGU_WRITE((char*)(&ctx_cmd->rsp),
        ctx_cmd->rsp.msg.data_len + sizeof(struct _ipmi_rsp_cmd));

    return 0;
}
#endif


#ifdef IPMI_MODULES_UART1_ICMB
void ipmi_icmb_init(void)
{
}

int ipmi_icmb_recv(struct ipmi_ctx *ctx_cmd)
{
    unsigned long size;

    IPMI_ICMB_READ((char*)(&ctx_cmd->req), &size);

    return 0;
}

int ipmi_icmb_send(struct ipmi_ctx *ctx_cmd)
{
    IPMI_ICMB_WRITE((char*)(&ctx_cmd->rsp),
        ctx_cmd->rsp.msg.data_len + sizeof(struct _ipmi_rsp_cmd));

    return 0;
}
#endif      // IPMI_MODULES_UART1_ICMB

