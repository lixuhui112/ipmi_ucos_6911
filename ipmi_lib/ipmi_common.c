//*****************************************************************************
//
// ipmi_common.c - IPMI Command for Common
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************
#include <stdlib.h>
#include "ipmi.h"
#include "ucos_ii.h"
#include "app/lib_common.h"
#include "ipmi_lib/ipmi_common.h"

void ipmi_cmd_err(struct ipmi_ctx *ctx_cmd, uint8_t error)
{
    ctx_cmd->rsp.msg.ccode = error;
    ctx_cmd->rsp.msg.data_len = 0;
}

void ipmi_cmd_invalid(struct ipmi_ctx *ctx_cmd)
{
    ipmi_cmd_err(ctx_cmd, IPMI_CC_INV_CMD);
}

void ipmi_cmd_ok(struct ipmi_ctx *ctx_cmd, uint8_t datalen)
{
    ipmi_cmd_err(ctx_cmd, IPMI_CC_OK);
    ctx_cmd->rsp.msg.data_len = datalen;
}

void ipmi_cmd_set_flags(struct ipmi_ctx *ctx_cmd, uint8_t flags)
{
    ctx_cmd->flags |= flags;
}



//*****************************************************************************
//
// Defines a timer for IPMI_LED
//
//*****************************************************************************
OS_TMR *led_timer;
uint8_t led_period = 1;

extern void IO_led1_set(tBoolean bOn);
void led_blink(void *ptmr, void *param)
{
    static uint8_t led1;

    led1 = !led1;

    IO_led1_set(led1);
}

void led_start(void)
{
    uint8_t err;
    uint32_t period;

    switch (led_period) {
        case IPMI_LED_ON:
            period = 10;
            break;
        case IPMI_LED_ERR:
            period = 5;
            break;
        default:
            period = 10 * led_period;
            break;
    }
    led_timer = OSTmrCreate(0, period, OS_TMR_OPT_PERIODIC, led_blink, NULL, "led_timer", &err);
    if (err == OS_ERR_NONE)
    {
        OSTmrStart(led_timer, &err);
    }
}

void led_change(uint8_t period)
{
    uint8_t err;

    led_period = period;
    OSTmrDel(led_timer, &err);
    led_start();
}

void ipmi_modules_init(void)
{
#if defined(IPMI_CHIP_AT24CXX)
    at24xx_init();
#endif
#if (IPMI_DEV_SEL)
    ipmi_sel_init();
#endif
#if (IPMI_DEV_SDR)
    ipmi_sdr_init();
#endif
}

void ipmi_sensors_init(void)
{
#if defined(IPMI_CHIP_MAX6635)
    max6635_init();
#endif

#if defined(IPMI_CHIP_UCD9081)
    ucd9081_init();
#endif

#if defined(IPMI_CHIP_ADT7470)
    adt7470_init();
#endif

#if defined(IPMI_CHIP_INA230)
    ina230_init();
#endif

#if defined(IPMI_CHIP_MC_LOCATOR)
    mc_locator_init();
#endif

#if 0
#ifdef IPMI_CHIP_KEY
    key_init();
#endif
#endif
}

#if (defined(IPMI_MODULES_SPI0_CPLD) && 0)
char ipmi_common_get_device_id(void)
{
    uint8_t devid = 0;
    uint8_t *regaddr = (uint8_t*)LOGIC_2_R_FPGA(LOGIC_DEVICE_ID_REG);

    IPMI_LOGIC_READ(regaddr, (uint8_t*)&devid);

    return devid;
}

char ipmi_common_get_device_revision(void)
{
    uint8_t devrev = 0;
    uint8_t *regaddr = (uint8_t*)LOGIC_2_R_FPGA(LOGIC_DEVICE_REV_REG);

    IPMI_LOGIC_READ(regaddr, (uint8_t*)&devrev);

    return devrev;
}

char ipmi_common_get_product_id(void)
{
    uint8_t pdid = 0;
    uint8_t *regaddr = (uint8_t*)LOGIC_2_R_FPGA(LOGIC_PRODUCT_ID_REG);

    IPMI_LOGIC_READ(regaddr, (uint8_t*)&pdid);

    return pdid;
}

char ipmi_common_get_poh(uint32_t *poh)
{
    uint8_t *regaddr = (uint8_t*)LOGIC_2_R_FPGA(LOGIC_POH_REG);

    IPMI_LOGIC_READ(regaddr, (uint8_t*)&poh);

    return 0;
}

void ipmi_common_power_onoff(uint8_t on)
{
    return;
}

void ipmi_common_test_self(void)
{
    uint8_t buf[] = {0x5a, 0xa5, 0x5a};

    SPI_spi0_xfer(buf, 3, 0, 0);
}

#else   // IPMI_MODULES_SPI1_SSIF

char ipmi_common_get_device_id(void)
{
    return 0x01;
}

char ipmi_common_get_device_revision(void)
{
    return 0x01;
}

char ipmi_common_get_product_id(void)
{
    return 0x01;
}

char ipmi_common_get_poh(uint32_t *poh)
{
    uint32_t power_on_seconds;

    power_on_seconds = OSTimeGet() / SYSTICKHZ;

    *poh = power_on_seconds;

    return 0;
}

void ipmi_common_power_onoff(uint8_t on)
{
    return;
}

void ipmi_common_test_self(void)
{
}
#endif




