//*****************************************************************************
//
// ipmi_chassis.c - IPMI Command for Chassis
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#include "ipmi_lib/ipmi.h"
#include "app/lib_common.h"
#include "app/lib_i2c.h"

struct ipmi_chassis_capabilities_rsp {
	uint8_t capabilities_flag;                      // 板卡功能         __LITTLE_ENDIAN__
	uint8_t chassis_fru_dev_addr;                   // IPMB地址，7位地址，0位不用
	uint8_t sdr_dev_addr;                           // SDR设备地址
	uint8_t sel_dev_addr;                           // SEL设备地址
	uint8_t sys_dev_addr;                           // 系统管理设备地址
	uint8_t bridge_dev_addr;                        // 桥设备地址
};

struct ipmi_chassis_status_rsp {
#ifdef __LITTLE_ENDIAN__
    uint8_t cur_pow_state:5,
            pow_restore_policy:2,
            reserved:1;
#else
    uint8_t reserved:1,
            pow_restore_policy:2,                   // 当前电源状态
            cur_pow_state:5;                        // 电源恢复策略
#endif
    uint8_t last_pow_env;                           // 最后的电源事件
    uint8_t chassis_state;                          // 板卡状态
    uint8_t front_p_button_status;                  // 前面板按钮状态
};


// 全局变量
static char chassis_power_state = IPMI_CHASSIS_POW_STATE_DEFAULT;
static char chassis_power_restore_policy = IPMI_CHASSIS_POLICY_DEFAULT;
static char chassis_last_power_event;
static char chassis_state;

void chassis_power_policy_set(int policy)
{
    chassis_power_restore_policy = policy & 0x03;
}

void chassis_power_state_set(int state)
{
    chassis_power_state = state & 0x1f;
}

void chassis_last_power_event_set(int event)
{
    chassis_last_power_event = event;
}

void chassis_state_set(int state)
{
    chassis_state = (state & 0x0f);
}


int ipmi_cmd_chassis(struct ipmi_ctx *ctx_cmd)
{
    DEBUG("ipmi_cmd_chassis\r\n");

    switch (ctx_cmd->req.msg.cmd)
    {
        case CHASSIS_GET_CAPABILITIES:
            {
                struct ipmi_chassis_capabilities_rsp *capa_rsp;

                capa_rsp = (struct ipmi_chassis_capabilities_rsp*)&ctx_cmd->rsp.data[0];

                ctx_cmd->rsp.msg.data_len = sizeof(struct ipmi_chassis_capabilities_rsp);
                ctx_cmd->rsp.msg.ccode = IPMI_CC_OK;

                capa_rsp->capabilities_flag = 0;
                capa_rsp->chassis_fru_dev_addr = I2C_i2c0_ipmb_self_addr_get();
                capa_rsp->sdr_dev_addr = AT24C_SLAVE_ADDR;
                capa_rsp->sel_dev_addr = AT24C_SLAVE_ADDR;
                capa_rsp->sys_dev_addr = 0;
                capa_rsp->bridge_dev_addr = 0;
            }
            break;

        case CHASSIS_GET_STATUS:
            {
                struct ipmi_chassis_status_rsp *status_rsp;

                status_rsp = (struct ipmi_chassis_status_rsp*)&ctx_cmd->rsp.data[0];

                ctx_cmd->rsp.msg.data_len = sizeof(struct ipmi_chassis_status_rsp);
                ctx_cmd->rsp.msg.ccode = IPMI_CC_OK;

                status_rsp->pow_restore_policy =  chassis_power_restore_policy;
                status_rsp->cur_pow_state   = chassis_power_state;
                status_rsp->last_pow_env    = chassis_last_power_event;
                status_rsp->chassis_state   = chassis_state;
                status_rsp->front_p_button_status   = 0;
            }
            break;

        case CHASSIS_CONTROL:
            {
                char reqcmd = ctx_cmd->req.data[0];
                switch (reqcmd & 0x0f)
                {
                    case IPMI_CHASSIS_CTL_POWER_DOWN:
                        //ctx_cmd->flags |= IPMI_CTX_NO_RESPONSE;
                        ipmi_common_power_onoff(0);
                        chassis_power_state_set(IPMI_CHASSIS_POW_STATE_POW_OFF);    // FIXME:从芯片读取电源状态
                        chassis_last_power_event_set(IPMI_CHASSIS_EVN_ON_CMD);
                        ipmi_cmd_ok(ctx_cmd, 0);
                        break;

                    case IPMI_CHASSIS_CTL_POWER_UP:
                        ipmi_common_power_onoff(1);
                        chassis_power_state_set(IPMI_CHASSIS_POW_STATE_POW_ON);     // FIXME:从芯片读取电源状态
                        chassis_last_power_event_set(IPMI_CHASSIS_EVN_ON_CMD);
                        ipmi_cmd_ok(ctx_cmd, 0);
                        break;

                    case IPMI_CHASSIS_CTL_POWER_CYCLE:
                    case IPMI_CHASSIS_CTL_HARD_RESET:
                    case IPMI_CHASSIS_CTL_PULSE_DIAG:
                    case IPMI_CHASSIS_CTL_ACPI_SOFT:
                    default:
                        ipmi_cmd_invalid(ctx_cmd);
                        break;
                }
            }
            break;

        case CHASSIS_IDENTIFY:
            {
                switch (ctx_cmd->req_len)
                {
                    case 0:
                        led_change(IPMI_LED_IDENTIFY);
                        break;
                    case 1:
                        led_change(ctx_cmd->req.data[0]);
                        break;
                    case 2:
                        led_change(IPMI_LED_ON);
                        break;
                    default:
                        ipmi_cmd_err(ctx_cmd, IPMI_CC_REQ_DATA_INV_LENGTH);
                        return 0;
                }
                ipmi_cmd_ok(ctx_cmd, 0);
            }
            break;

        case CHASSIS_GET_POH_COUNTER:
            {
                unsigned long poh;

                ipmi_common_get_poh(&poh);

                poh /= 60;
                ctx_cmd->rsp.data[0] = 1;
                ctx_cmd->rsp.data[1] = poh & 0x000000ff;
                ctx_cmd->rsp.data[2] = (poh & 0x0000ff00) >> 8;
                ctx_cmd->rsp.data[3] = (poh & 0x00ff0000) >> 16;
                ctx_cmd->rsp.data[4] = (poh & 0xff000000) >> 24;

                ipmi_cmd_ok(ctx_cmd, 5);
            }
            break;

        case CHASSIS_RESET:
        case CHASSIS_SET_CAPABILITIES:
        case CHASSIS_SET_POW_RESTORE_POLICY:
        case CHASSIS_GET_RESTART_CAUSE:
        case CHASSIS_SET_SYS_BOOT_OPT:
        case CHASSIS_GET_SYS_BOOT_OPT:
        case CHASSIS_SET_FP_BUTTON_ENABLE:
        case CHASSIS_SET_POW_CYCLE_INTERVAL:
        default:
            // response C1h for no support cmd
            ipmi_cmd_invalid(ctx_cmd);
            break;
    }

    return 0;
}

