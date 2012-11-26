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
        case BMC_GET_DEVICE_ID:
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

        case BMC_WARM_RESET:
            /* warm reset need response */
            ipmi_cmd_ok(ctx_cmd, 0);

        case BMC_COLD_RESET:
            SysCtlReset();
            break;

        case BMC_GET_SELF_TEST:
        case BMC_RESET_WATCHDOG_TIMER:
        case BMC_SET_WATCHDOG_TIMER:
        case BMC_GET_WATCHDOG_TIMER:
        case BMC_SET_GLOBAL_ENABLES:
        case BMC_GET_GLOBAL_ENABLES:
        case GET_SYSTEM_GUID:
        default:
            ipmi_cmd_invalid(ctx_cmd);
            break;
    }

    return 0;
}


