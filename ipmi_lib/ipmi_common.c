//*****************************************************************************
//
// ipmi_common.c - IPMI Command for Common
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#include "ipmi.h"
#include "ucos_ii.h"
#include "app/lib_common.h"

int ipmi_cmd_err(struct ipmi_ctx *ctx_cmd, uint8_t error)
{
    ctx_cmd->rsp.msg.ccode = error;
    ctx_cmd->rsp.msg.data_len = 0;
    return 0;
}

int ipmi_cmd_invalid(struct ipmi_ctx *ctx_cmd)
{
    return ipmi_cmd_err(ctx_cmd, IPMI_CC_INV_CMD);
}

int ipmi_cmd_ok(struct ipmi_ctx *ctx_cmd, uint8_t datalen)
{
    ipmi_cmd_err(ctx_cmd, IPMI_CC_OK);
    ctx_cmd->rsp.msg.data_len = datalen;
    return 0;
}


int ipmi_common_power_onoff(int on)
{
    return 0;
}

void ipmi_led_blink(int delay)
{
    switch (delay)
    {
        case IPMI_LED_ON:           // always on
            led_delay_ms = IPMI_LED_SYS_FORCE;
            led_delay_s = IPMI_LED_SYS_FORCE;
            break;

        case IPMI_LED_SYS:
            led_delay_ms = IPMI_LED_SYS_OK;
            led_delay_s = 0;
            break;

        case IPMI_LED_ERR:
            led_delay_ms = IPMI_LED_SYS_ERR;
            led_delay_s = 0;
            break;

        default:
            led_delay_ms = 0;
            led_delay_s = IPMI_LED_SYS_IDENTIFY * delay;
            break;
    }
}

void ipmi_sensor_init(void)
{
#ifdef IPMI_CHIP_MAX6635
    max6635_init();
#endif

#ifdef IPMI_CHIP_ADT7470
    adt7470_init();
#endif

#ifdef IPMI_CHIP_KEY
    key_init();
#endif

#if 0
#ifdef IPMI_CHIP_MC_LOCATOR
    mc_locator_init();
#endif

#ifdef IPMI_CHIP_UCD9081
    ucd9081_init();
#endif

#ifdef IPMI_CHIP_AT24CXX
    at24xx_init();
#endif

#ifdef IPMI_CHIP_INA230
    ina230_init();
#endif
#endif
}

#if defined(IPMI_MODULES_SPI1_SSIF) && 0
char ipmi_common_get_device_id(void)
{
    unsigned char devid = 0;
    unsigned char regaddr[2] = {0x00, CPLD_DEVICE_ID_REG};

    return 0x01;
    IPMI_CPLD_READ(&regaddr[0], (unsigned char*)&devid, 1);

    return devid;
}

char ipmi_common_get_device_revision(void)
{
    unsigned char devrev = 0;
    unsigned char regaddr[2] = {0x00, CPLD_DEVICE_REV_REG};

    IPMI_CPLD_READ(&regaddr[0], (unsigned char*)&devrev, 1);

    return devrev;
}

char ipmi_common_get_product_id(void)
{
    unsigned char pdid = 0;
    unsigned char regaddr[2] = {0x00, CPLD_PRODUCT_ID_REG};

    IPMI_CPLD_READ(&regaddr[0], (unsigned char*)&pdid, 1);

    return pdid;
}

char ipmi_common_get_poh(unsigned long *poh)
{
    unsigned char regaddr[2] = {0x00, CPLD_POH_REG};

    IPMI_CPLD_READ(&regaddr[0], (unsigned char*)&poh, 4);

    return 0;
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

char ipmi_common_get_poh(unsigned long *poh)
{
    unsigned long power_on_seconds;

    power_on_seconds = OSTimeGet() / SYSTICKHZ;

    *poh = power_on_seconds;

    return 0;
}

#endif



