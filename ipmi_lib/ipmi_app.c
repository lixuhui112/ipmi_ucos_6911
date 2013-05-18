//*****************************************************************************
//
// ipmi_app.c - IPMI Command for Application
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#include "ipmi.h"
#include "driverlib/sysctl.h"
#include "app/lib_common.h"

extern uint8_t ipmi_get_channel_medium_type(uint8_t channel_number);
extern uint8_t ipmi_get_channel_protocol_type(uint8_t channel_number);
extern uint8_t ipmi_get_channel_session_support(uint8_t channel_number);

void ipmi_bmc_get_device_id(struct ipmi_ctx *ctx_cmd)
{
    struct ipmi_devid_rsp *devid_rsp;

    devid_rsp = (struct ipmi_devid_rsp *)&ctx_cmd->rsp.data[0];

    devid_rsp->device_id = ipmi_common_get_device_id();
    devid_rsp->device_revision = (((IPMI_DEV_SDR|IPMI_DEV_SENSOR) ? IPM_DEV_DEVICE_ID_SDR_MASK : 0) |
                                  (ipmi_common_get_device_revision() & IPM_DEV_DEVICE_ID_REV_MASK));
    devid_rsp->fw_rev1 = (ipmi_global.device_available << 7) | IPMI_FIRMWARE_VER_MAJOR;
    devid_rsp->fw_rev2 = IPMI_FIRMWARE_VER_MINOR;
    devid_rsp->ipmi_version = IPMI_VERSION;
    devid_rsp->adtl_device_support = IPMI_DEV_DEVICE_ADT;
    devid_rsp->manufacturer_id[0] = IPMI_DEV_MANUFACTURER_ID_2;
    devid_rsp->manufacturer_id[1] = IPMI_DEV_MANUFACTURER_ID_1;
    devid_rsp->manufacturer_id[2] = IPMI_DEV_MANUFACTURER_ID_0;
    devid_rsp->product_id[0] = ipmi_common_get_product_id();
    devid_rsp->product_id[1] = 0;
    devid_rsp->aux_fw_rev[0] = IPMI_AUXILIARY_VERSION & 0xff;
    devid_rsp->aux_fw_rev[1] = (IPMI_AUXILIARY_VERSION >> 8) & 0xff;
    devid_rsp->aux_fw_rev[2] = (IPMI_AUXILIARY_VERSION >> 16) & 0xff;
    devid_rsp->aux_fw_rev[3] = (IPMI_AUXILIARY_VERSION >> 24) & 0xff;

    ipmi_cmd_ok(ctx_cmd, sizeof(struct ipmi_devid_rsp));
}

void ipmi_bmc_set_global_enables(struct ipmi_ctx *ctx_cmd)
{
    if (ctx_cmd->req.data[0] & EN_RCV_MSG_QUE_INT) {
        BIT_SET(ipmi_global.bmc_global_enable, EN_RCV_MSG_QUE_INT);
    }
    if (ctx_cmd->req.data[0] & EN_EVT_MSG_BUF_FUL_INT) {
        BIT_SET(ipmi_global.bmc_global_enable, EN_EVT_MSG_BUF_FUL_INT);
    }
    if (ctx_cmd->req.data[0] & EN_EVT_MSG_BUF) {
        BIT_SET(ipmi_global.bmc_global_enable, EN_EVT_MSG_BUF);
    }
    if (ctx_cmd->req.data[0] & EN_SYS_EVT_LOG) {
        BIT_SET(ipmi_global.bmc_global_enable, EN_SYS_EVT_LOG);
    }
    ipmi_cmd_ok(ctx_cmd, 0);
}

void ipmi_bmc_get_global_enables(struct ipmi_ctx *ctx_cmd)
{
    ctx_cmd->rsp.data[0] = ipmi_global.bmc_global_enable;
    ipmi_cmd_ok(ctx_cmd, 1);
}

void ipmi_bmc_clear_message_flags(struct ipmi_ctx *ctx_cmd)
{
    ipmi_global.bmc_message_flags &= ~(ctx_cmd->req.data[0] | (7 << 2));
    ipmi_cmd_ok(ctx_cmd, 0);
}

void ipmi_bmc_get_message_flags(struct ipmi_ctx *ctx_cmd)
{
    ctx_cmd->req.data[0] = ipmi_global.bmc_message_flags;
    ipmi_cmd_ok(ctx_cmd, 1);
}

void ipmi_get_channel_info_command(struct ipmi_ctx *ctx_cmd)
{
    struct ipmi_get_channel_info_st *rsp = (struct ipmi_get_channel_info_st *)&ctx_cmd->rsp.data[0];
    uint8_t ch_num;

    ch_num = ctx_cmd->req.data[0] & 0x0f;

    switch (ch_num) {
        case IPMI_CH_NUM_PRESENT_INTERFACE:
            ch_num = ctx_cmd->from_channel;
            break;

        case IPMI_CH_NUM_PRIMARY_IPMB:
        case IPMI_CH_NUM_CONSOLE:
        case IPMI_CH_NUM_ICMB:
        case IPMI_CH_NUM_PMB:
        case IPMI_CH_NUM_LAN:
        case IPMI_CH_NUM_SYS_INTERFACE:
            // use provide ch number
            break;

        default:
            ipmi_cmd_err(ctx_cmd, IPMI_CC_PARAM_OUT_OF_RANGE);
            return;
    }

    rsp->actual_channel_number = ch_num;
    rsp->channel_medium_type = ipmi_get_channel_medium_type(ch_num);
    rsp->channel_protocol_type = ipmi_get_channel_protocol_type(ch_num);
    rsp->session_support = ipmi_get_channel_session_support(ch_num);
    rsp->vendor_id[0] = 0xf2;
    rsp->vendor_id[1] = 0x1b;
    rsp->vendor_id[2] = 0x00;
    rsp->auxiliary_channel_info[0] = 0;
    rsp->auxiliary_channel_info[1] = 0;

    ipmi_cmd_ok(ctx_cmd, sizeof(struct ipmi_get_channel_info_st));
}

void ipmi_enable_msg_ch_recv(struct ipmi_ctx *ctx_cmd)
{
    uint8_t ch_num, op;

    ch_num = ctx_cmd->req.data[0] & 0xf;
    op = ctx_cmd->req.data[1] & 0x3;

    ctx_cmd->rsp.data[0] = ch_num;
    switch (op) {
        case 0x00:      // 00b = disable channel
            ipmi_global.channel_msg_rev_en &= ~(1 << ch_num);
            ctx_cmd->rsp.data[1] = op & 0x1;
            break;

        case 0x01:      // 01b = enable channel
            ipmi_global.channel_msg_rev_en |= (1 << ch_num);
            ctx_cmd->rsp.data[1] = op & 0x1;
            break;

        case 0x02:      // 10b = get channel enable/disable state
            ctx_cmd->rsp.data[1] = ipmi_global.channel_msg_rev_en & (1 << ch_num) ? 1 : 0;
            break;

        case 0x03:      // 11b = reserved
        default:
            ipmi_cmd_err(ctx_cmd, IPMI_CC_PARAM_OUT_OF_RANGE);
            return;
    }
    ipmi_cmd_ok(ctx_cmd, 2);
}

void ipmi_get_message(struct ipmi_ctx *ctx_cmd)
{
    struct ipmi_ctx *old_ctx;
    uint8_t data_len;

    if (ipmi_global.bmc_message_flags) {
        // have message
        old_ctx = ipmi_msg_queue_pull();
        ctx_cmd->rsp.data[0] = (IPMI_PV_LEVEL_NONE << 4) | (old_ctx->to_channel & 0x0f);
        data_len = old_ctx->rsp.msg.data_len + sizeof(struct _ipmi_rsp_cmd);

        memcpy(&ctx_cmd->rsp.data[1], (char*)&old_ctx->rsp, data_len);

        // free old_ctx;
        ipmi_put_free_ctx_entry(old_ctx);

        ipmi_cmd_ok(ctx_cmd, data_len + 1);
    } else {
        // no message
        ipmi_cmd_err(ctx_cmd, IPMI_CC_CANT_RET_NUM_REQ_BYTES);
    }
}

void ipmi_send_message(struct ipmi_ctx *ctx_cmd)
{
    struct send_message_req *req = (struct send_message_req *)&ctx_cmd->req.data[0];
    struct ipmi_req *brg_req = (struct ipmi_req *)&req->msg_data[0];
    struct ipmi_ctx *new_ctx;

    if (ipmi_global.channel_msg_rev_en & (1 << req->send_ch_num)) {

        // allow send to the channel
        new_ctx = ipmi_get_free_ctx_entry();
        if (new_ctx == NULL)
        {
            ipmi_cmd_err(ctx_cmd, IPMI_CC_OUT_OF_SPACE);
            return;
        }

        new_ctx->from_channel = ctx_cmd->from_channel;
        new_ctx->to_channel = req->send_ch_num;
        new_ctx->rq_seq = brg_req->msg.rq_seq;
        ipmi_cmd_set_flags(new_ctx, IPMI_CTX_BRIDGE);

        new_ctx->req.msg.data_len = brg_req->msg.data_len;
        new_ctx->req.msg.rs_sa = brg_req->msg.rs_sa;
        new_ctx->req.msg.rs_lun = brg_req->msg.rs_lun;
        new_ctx->req.msg.netfn = brg_req->msg.netfn;
        new_ctx->req.msg.rq_sa = ipmi_global.ipmb_addr;
        new_ctx->req.msg.rq_lun = brg_req->msg.rq_lun;
        new_ctx->req.msg.rq_seq = brg_req->msg.rq_seq;  // todo: change to self sequence mechanism, rand()
        new_ctx->req.msg.cmd = brg_req->msg.cmd;
        new_ctx->req.msg.checksum1 = 0xff;              // todo
        memcpy(&new_ctx->req.data[0], &brg_req->data[0], brg_req->msg.data_len - sizeof(struct _ipmi_req_cmd));

        // send new message to channel
        ipmi_cmd_sendto_ch(new_ctx, new_ctx->to_channel);

        // old message response OK
        ipmi_cmd_ok(ctx_cmd, 0);

    } else {

        // dest channel is closed
        ipmi_cmd_err(ctx_cmd, IPMI_CC_DESTINATION_UNAVAILABLE);
    }
}

int ipmi_cmd_app(struct ipmi_ctx *ctx_cmd)
{
    DEBUG("ipmi_cmd_app\r\n");

    switch (ctx_cmd->req.msg.cmd)
    {
        /* IPM Device ¡°Global¡± Commands ***********************************/
        case BMC_GET_DEVICE_ID:                             /* 0x01 */
            ipmi_bmc_get_device_id(ctx_cmd);
            break;

        case BMC_COLD_RESET:                                /* 0x02 */
            SysCtlReset();
            break;

        case BMC_WARM_RESET:                                /* 0x03 */
            /* warm reset need response */
            SysCtlReset();
            ipmi_cmd_ok(ctx_cmd, 0);
            break;

        case BMC_GET_SELF_TEST:                             /* 0x04 */
        case BMC_MANUFACTURING_TEST_ON:                     /* 0x05 */
        case SET_ACPI_POWER_STATE:                          /* 0x06 */
        case GET_ACPI_POWER_STATE:                          /* 0x07 */
        case GET_DEVICE_GUID:                               /* 0x08 */
        case GET_NETFN_SUPPORT:                             /* 0x09 */
        case GET_COMMAND_SUPPORT:                           /* 0x0A */
        case GET_COMMAND_SUB_FUN_SUPPORT:                   /* 0x0B */
        case GET_CONFIG_COMMANDS:                           /* 0x0C */
        case GET_CONFIG_COMMAND_SUB_FUNC:                   /* 0x0D */
            ipmi_cmd_invalid(ctx_cmd);
            break;

        /* BMC Watchdog Timer Commands **************************************/
        case BMC_RESET_WATCHDOG_TIMER:                      /* 0x22 */
        case BMC_SET_WATCHDOG_TIMER:                        /* 0x24 */
        case BMC_GET_WATCHDOG_TIMER:                        /* 0x25 */
            ipmi_cmd_invalid(ctx_cmd);
            break;

        /* BMC Device and Messaging Commands ********************************/
        case BMC_SET_GLOBAL_ENABLES:                        /* 0x2e */
            ipmi_bmc_set_global_enables(ctx_cmd);
            break;

        case BMC_GET_GLOBAL_ENABLES:                        /* 0x2f */
            ipmi_bmc_get_global_enables(ctx_cmd);
            break;

        case CLEAR_MESSAGE_FLAGS:                           /* 0x30 */
            ipmi_bmc_clear_message_flags(ctx_cmd);
            break;

        case GET_MESSAGE_FLAGS:                             /* 0x31 */
            ipmi_bmc_get_message_flags(ctx_cmd);
            break;

        case ENABLE_MESSAGE_CHANNEL_RECEIVE:                /* 0x32 */
            ipmi_enable_msg_ch_recv(ctx_cmd);
            break;

        case GET_MESSAGE:                                   /* 0x33 */
            ipmi_get_message(ctx_cmd);
            break;

        case SEND_MESSAGE:                                  /* 0x34 */
            ipmi_send_message(ctx_cmd);
            break;

        case READ_EVENT_MESSAGE_BUFFER:                     /* 0x35 */
        case GET_BT_INTERFACE_CAPABILITIES:                 /* 0x36 */
        case GET_SYSTEM_GUID:                               /* 0x37 */
        case GET_CHANNEL_AUTH_CAPABILITIES:                 /* 0x38 */
        case GET_SESSION_CHALLENGE:                         /* 0x39 */
        case ACTIVATE_SESSION:                              /* 0x3A */
        case SET_SESSION_PRIVILEGE_LEVEL:                   /* 0x3B */
        case CLOSE_SESSION:                                 /* 0x3C */
        case GET_SESSION_INFO:                              /* 0x3D */
        case GET_AUTHCODE:                                  /* 0x3F */
        case SET_CHANNEL_ACCESS:                            /* 0x40 */
        case GET_CHANNEL_ACCESS:                            /* 0x41 */
            break;

        case GET_CHANNEL_INFO_COMMAND:                      /* 0x42 */
            ipmi_get_channel_info_command(ctx_cmd);
            break;

        case SET_USER_ACCESS_COMMAND:                       /* 0x43 */
        case GET_USER_ACCESS_COMMAND:                       /* 0x44 */
        case SET_USER_NAME:                                 /* 0x45 */
        case GET_USER_NAME_COMMAND:                         /* 0x46 */
        case SET_USER_PASSWORD_COMMAND:                     /* 0x47 */
        case ACTIVATE_PAYLOAD:                              /* 0x48 */
        case DEACTIVATE_PAYLOAD:                            /* 0x49 */
        case GET_PAYLOAD_ACTIVATION_STATUS:                 /* 0x4A */
        case GET_PAYLOAD_INSTANCE_INFO:                     /* 0x4B */
        case SET_USER_PAYLOAD_ACCESS:                       /* 0x4C */
        case GET_USER_PAYLOAD_ACCESS:                       /* 0x4D */
        case GET_CHANNEL_PAYLOAD_SUPPORT:                   /* 0x4E */
        case GET_CHANNEL_PAYLOAD_VERSION:                   /* 0x4F */
        case GET_CHANNEL_OEM_PAYLOAD_INFO:                  /* 0x50 */
        case MASTER_WRITE_READ:                             /* 0x52 */
        case GET_CHANNEL_CIPHER_SUITES:                     /* 0x54 */
        case SR_PAYLOAD_ENCRYPTION:                         /* 0x55 */
        case SET_CHANNEL_SECURITY_KEYS:                     /* 0x56 */
        case GET_SYSTEM_INTF_CAPABILITIES:                  /* 0x57 */
        case SET_SYSTEM_INFO_PARAMETERS:                    /* 0x58 */
        case GET_SYSTEM_INFO_PARAMETERS:                    /* 0x59 */
        case SET_COMMAND_ENABLES:                           /* 0x60 */
        case GET_COMMAND_ENABLES:                           /* 0x61 */
        case SET_COMMAND_SUB_FUNC_ENABLES:                  /* 0x62 */
        case GET_COMMAND_SUB_FUNC_ENABLES:                  /* 0x63 */
        case GET_OEM_NETFN_IANA_SUPPORT:                    /* 0x64 */
        default:
            ipmi_cmd_invalid(ctx_cmd);
            break;
    }

    return 0;
}


