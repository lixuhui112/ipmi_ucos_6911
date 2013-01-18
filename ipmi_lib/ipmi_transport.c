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

    switch (ctx_cmd->req.msg.cmd)
    {
        /* LAN Device Commands **********************************************/
        case SET_LAN_CONFIG_PARAM:                          /* 0x10 */
        case GET_LAN_CONFIG_PARAM:                          /* 0x02 */
        case SUSPEND_BMC_ARPS:                              /* 0x03 */
        case GET_IP_UDP_RMCP_STATISTICS:                    /* 0x04 */

        /* Serial/Modem Device Commands *************************************/
        case SET_SERIAL_MODEM_CONFIG:                       /* 0x10 */
        case GET_SERIAL_MODEM_CONFIG:                       /* 0x11 */
        case SET_SERIAL_MODEM_MUX:                          /* 0x12 */
        case GET_TAP_RESPONSE_CODES:                        /* 0x13 */
        case SET_PPP_UDP_PROXY_TRANSMIT_DATA:               /* 0x14 */
        case GET_PPP_UDP_PROXY_TRANSMIT_DATA:               /* 0x15 */
        case SEND_PPP_UDP_PROXY_PACKET:                     /* 0x16 */
        case GET_PPP_UDP_PROXY_RECEIVE_DATA:                /* 0x17 */
        case SERIAL_MODEM_CONNECTION_ACTIVE:                /* 0x18 */
        case CALLBACK:                                      /* 0x19 */
        case SET_USER_CALLBACK_OPTIONS:                     /* 0x1A */
        case GET_USER_CALLBACK_OPTIONS:                     /* 0x1B */
        case SET_SERIAL_ROUTING_MUX:                        /* 0x1C */
        case SOL_ACTIVATING:                                /* 0x20 */
        case SET_SOL_CONFIG_PARAM:                          /* 0x21 */
        case GET_SOL_CONFIG_PARAM:                          /* 0x22 */

        /* Command Forwarding Commands **************************************/
        case FORWARDED_COMMAND:                             /* 0x30 */
        case SET_FORWARDED_COMMANDS:                        /* 0x31 */
        case GET_FORWARDED_COMMANDS:                        /* 0x32 */
        case ENABLE_FORWARDED_COMMANDS:                     /* 0x33 */

        default:
            ipmi_cmd_invalid(ctx_cmd);
            break;
    }

    return 0;
}



