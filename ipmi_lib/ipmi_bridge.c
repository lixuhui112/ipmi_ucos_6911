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

    switch (ctx_cmd->req.msg.cmd)
    {
        /* Chassis Device Commands ******************************************/
        case GET_BRIDGE_STATE:                              /* 0x00 */
        case SET_BRIDGE_STATE:                              /* 0x01 */
        case GET_ICMB_ADDRESS:                              /* 0x02 */
        case SET_ICMB_ADDRESS:                              /* 0x03 */
        case SET_BRIDGE_PROXYADDRESS:                       /* 0x04 */
        case GET_BRIDGE_STATISTICS:                         /* 0x05 */
        case GET_ICMB_CAPABILITIES:                         /* 0x06 */
        case CLEAR_BRIDGE_STATISTICS:                       /* 0x08 */
        case GET_BRIDGE_PROXY_ADDRESS:                      /* 0x09 */
        case GET_ICMB_CONNECTOR_INFO:                       /* 0x0A */
        case GET_ICMB_CONNECTION_ID:                        /* 0x0B */
        case SEND_ICMB_CONNECTION_ID:                       /* 0x0C */
        case PREPAREFORDISCOVERY:                           /* 0x10 */
        case GETADDRESSES:                                  /* 0x11 */
        case SETDISCOVERED:                                 /* 0x12 */
        case GETCHASSISDEVICEID:                            /* 0x13 */
        case SETCHASSISDEVICEID:                            /* 0x14 */
        case BRIDGEREQUEST:                                 /* 0x20 */
        case BRIDGEMESSAGE:                                 /* 0x21 */
        case GETEVENTCOUNT:                                 /* 0x30 */
        case SETEVENTDESTINATION:                           /* 0x31 */
        case SETEVENTRECEPTIONSTATE:                        /* 0x32 */
        case SENDICMBEVENTMESSAGE:                          /* 0x33 */
        case GETEVENTDESTINATION:                           /* 0x34 */
        case GETEVENTRECEPTIONSTATE:                        /* 0x35 */
        case ERROR_REPORT:                                  /* 0xFF */
        default:
            ipmi_cmd_invalid(ctx_cmd);
            break;
    }
    return 0;
}

