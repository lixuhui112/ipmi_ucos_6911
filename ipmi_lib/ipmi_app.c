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

#pragma pack(1)
struct ipmi_guid {
	uint32_t  time_low;	                /* timestamp low field */
	uint16_t  time_mid;	                /* timestamp middle field */
	uint16_t  time_hi_and_version;      /* timestamp high field and version number */
	uint8_t   clock_seq_hi_variant;     /* clock sequence high field and variant */
	uint8_t   clock_seq_low;            /* clock sequence low field */
	uint8_t   node[6];	                /* node */
};
#pragma pack()

int ipmi_cmd_app(struct ipmi_ctx *ctx_cmd)
{
    DEBUG("ipmi_cmd_app\r\n");

    switch (ctx_cmd->req.msg.cmd)
    {
        /* IPM Device ¡°Global¡± Commands ***********************************/
        case BMC_GET_DEVICE_ID:                             /* 0x01 */
            {
                struct ipmi_devid_rsp *devid_rsp;

                devid_rsp = (struct ipmi_devid_rsp *)&ctx_cmd->rsp.data[0];

                devid_rsp->device_id = ipmi_common_get_device_id();
                devid_rsp->device_revision = ((IPMI_DEV_SENSOR ? IPM_DEV_DEVICE_ID_SDR_MASK : 0) |
                                              (ipmi_common_get_device_revision() & IPM_DEV_DEVICE_ID_REV_MASK));
                devid_rsp->fw_rev1 = (device_available << 7) | IPMI_FIRMWARE_VER_MAJOR;
                devid_rsp->fw_rev2 = IPMI_FIRMWARE_VER_MINOR;
                devid_rsp->ipmi_version = IPMI_VERSION;
                devid_rsp->adtl_device_support = IPM_DEV_DEVICE_ADT;
                devid_rsp->manufacturer_id[0] = IPMI_DEV_MANUFACTURER_ID_2;
                devid_rsp->manufacturer_id[1] = IPMI_DEV_MANUFACTURER_ID_1;
                devid_rsp->manufacturer_id[2] = IPMI_DEV_MANUFACTURER_ID_0;
                devid_rsp->product_id[0] = ipmi_common_get_product_id();
                devid_rsp->product_id[1] = 0;
                devid_rsp->aux_fw_rev[0] = 0;
                devid_rsp->aux_fw_rev[1] = 0;
                devid_rsp->aux_fw_rev[2] = 0;
                devid_rsp->aux_fw_rev[3] = 0;

                ipmi_cmd_ok(ctx_cmd, sizeof(struct ipmi_devid_rsp));
            }
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

        /* BMC Watchdog Timer Commands **************************************/
        case BMC_RESET_WATCHDOG_TIMER:                      /* 0x22 */
        case BMC_SET_WATCHDOG_TIMER:                        /* 0x24 */
        case BMC_GET_WATCHDOG_TIMER:                        /* 0x25 */

        /* BMC Device and Messaging Commands ********************************/
        case BMC_SET_GLOBAL_ENABLES:                        /* 0x2e */
        case BMC_GET_GLOBAL_ENABLES:                        /* 0x2f */
        case CLEAR_MESSAGE_FLAGS:                           /* 0x30 */
        case GET_MESSAGE_FLAGS:                             /* 0x31 */
        case ENABLE_MESSAGE_CHANNEL_RECEIVE:                /* 0x32 */
        case GET_MESSAGE:                                   /* 0x33 */
        case SEND_MESSAGE:                                  /* 0x34 */
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
        case GET_CHANNEL_INFO_COMMAND:                      /* 0x42 */
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


