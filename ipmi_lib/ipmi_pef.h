//*****************************************************************************
//
// ipmi_pef.h - IPMI PEF and Alerting Commands Header File
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#include "ipmi_lib/ipmi.h"
#include "app/lib_common.h"

struct get_pef_capabilities_rsp {
    uint8_t pef_version;
#ifdef __LITTLE_ENDIAN__
    uint8_t alert:1,
            powerdown:1,
            reset:1,
            powercycle:1,
            oemaction:1,
            diag_int:1,
            resv:2;
#else
    uint8_t resv:2,
            diag_int:1,
            oemaction:1,
            powercycle:1,
            reset:1,
            powerdown:1,
            alert:1;
#endif
    uint8_t num_of_event_filter_table_entries;
};

struct arm_pef_postpone_timer_req {
    uint8_t pef_postpone_timeout;
};

struct pef_config_param {
#ifdef __LITTLE_ENDIAN__
    uint8_t param_sel:7,
            reserved1:1;
#else
    uint8_t reserved1:1,
            param_sel:7;
#endif

};

void ipmi_get_pef_capabilities(struct ipmi_ctx *ctx_cmd);
void ipmi_arm_pef_postpone_timer(struct ipmi_ctx *ctx_cmd);
void ipmi_set_pef_config_param(struct ipmi_ctx *ctx_cmd);
void ipmi_get_pef_config_param(struct ipmi_ctx *ctx_cmd);
void ipmi_set_last_proc_event_id(struct ipmi_ctx *ctx_cmd);
void ipmi_get_last_proc_event_id(struct ipmi_ctx *ctx_cmd);
void ipmi_alert_immediate(struct ipmi_ctx *ctx_cmd);
void ipmi_pet_acknowledge(struct ipmi_ctx *ctx_cmd);


