//*****************************************************************************
//
// lib_vol.c - Voltage Monitor Library for the IPMI application.
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/i2c.h"
#include "driverlib/debug.h"
#include "driverlib/interrupt.h"
#include "ipmi_lib/ipmi_modules.h"
#include "app/lib_gpio.h"
#include "app/lib_i2c.h"

#ifdef IPMI_CHIP_UCD9081
//*****************************************************************************
//
// The I2C Chip UCD9081 operation.  (Power Voltage Sensor)
//
// UCD9081电压监控芯片，主要作用于交换板与主控板卡中，负责监控板卡上的12V电源
// 转换为3.3/2.5/1.8/1.2/1.0电压后的情况，并依据预定义动作进行处理
//
//*****************************************************************************
void I2C_chip_ucd9081_power_chk_open(void)
{
    // Enable the peripherals used by the ucd9081 GPIO1 interrupt.
    SysCtlPeripheralEnable(GPIO_CHIP_UCD9081_GPIO1_PERIPH);

    // Configure the GPIO1 used to read the state.
    GPIOPinTypeGPIOInput(GPIO_CHIP_UCD9081_GPIO1_PORT, GPIO_CHIP_UCD9081_GPIO1_PIN);
    GPIOPadConfigSet(GPIO_CHIP_UCD9081_GPIO1_PORT, GPIO_CHIP_UCD9081_GPIO1_PIN,
            GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    // Configure the Interrupt for ucd9081 GPIO1.
    GPIOIntTypeSet(GPIO_CHIP_UCD9081_GPIO1_PORT, GPIO_CHIP_UCD9081_GPIO1_PIN, GPIO_FALLING_EDGE);
    GPIOPinIntEnable(GPIO_CHIP_UCD9081_GPIO1_PORT, GPIO_CHIP_UCD9081_GPIO1_PIN);
    IntEnable(GPIO_CHIP_UCD9081_GPIO1_INT);

    // Enable the peripherals used by the ucd9081 GPIO2 interrupt.
    SysCtlPeripheralEnable(GPIO_CHIP_UCD9081_GPIO2_PERIPH);

    // Configure the GPIO2 used to read the state.
    GPIOPinTypeGPIOInput(GPIO_CHIP_UCD9081_GPIO2_PORT, GPIO_CHIP_UCD9081_GPIO2_PIN);
    GPIOPadConfigSet(GPIO_CHIP_UCD9081_GPIO2_PORT, GPIO_CHIP_UCD9081_GPIO2_PIN,
            GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    // Configure the Interrupt for ucd9081 GPIO1.
    GPIOIntTypeSet(GPIO_CHIP_UCD9081_GPIO2_PORT, GPIO_CHIP_UCD9081_GPIO2_PIN, GPIO_FALLING_EDGE);
    GPIOPinIntEnable(GPIO_CHIP_UCD9081_GPIO2_PORT, GPIO_CHIP_UCD9081_GPIO2_PIN);
    IntEnable(GPIO_CHIP_UCD9081_GPIO2_INT);
}

void I2C_chip_ucd9081_reset(void)
{
    uint8_t reg = 0x00;

    I2C_i2c1_pmb_write(UCD9081_SLAVE_ADDR, RESTART, 1, (char*)&reg, 1);
}

void I2C_chip_ucd9081_shutdown(void)
{
    uint8_t reg = 0xc0;

    I2C_i2c1_pmb_write(UCD9081_SLAVE_ADDR, RESTART, 1, (char*)&reg, 1);
}

void I2C_chip_ucd9081_error(void)
{
    uint8_t status_9081;
    uint8_t rail_status_9081;
    uint8_t error[6];

    I2C_i2c1_pmb_read(UCD9081_SLAVE_ADDR, STATUS, 1, (char*)status_9081, 1);

    if (status_9081 == 0)
    {
        return;
    }

    if (status_9081 & STATUS_IIC_ERR)
    {
    }
    if (status_9081 & STATUS_RAIL_ERR)
    {
        I2C_i2c1_pmb_read(UCD9081_SLAVE_ADDR, RAILSTATUS2, 1, (char*)rail_status_9081, 1);
        I2C_i2c1_pmb_read(UCD9081_SLAVE_ADDR, ERROR1, 1, (char*)error[0], 6);

        //UARTprintf("rail %d error\r\n", rail_status_9081);
        //UARTprintf("rail \r\n", error[0]);

        chassis_power_state_set(IPMI_CHASSIS_POW_STATE_POW_OVER);
    }
    if (status_9081 & STATUS_NVERRLOG)
    {
    }
    if (status_9081 & STATUS_FW_ERR)
    {
    }
    if (status_9081 & STATUS_PARAM_ERR)
    {
    }
    if (status_9081 & STATUS_REG_W_ERR)
    {
    }
    if (status_9081 & STATUS_REG_R_ERR)
    {
    }
    if (status_9081 & STATUS_REG_A_ERR)
    {
    }

    /* TODO:
    SendMessage(CHANNEL_IPMB, "power fault");
     */
}

float I2C_chip_ucd9081_rail_read(uint8_t rail)
{
    uint8_t rail_h, rail_l;
    uint16_t rail_vol;
    float voltage;

    if (rail < 1 || rail > 8)
    {
        return 0;
    }

    I2C_i2c1_pmb_read(UCD9081_SLAVE_ADDR, (rail - 1) * 2 + 0, 1, (char*)rail_h, 1);
    I2C_i2c1_pmb_read(UCD9081_SLAVE_ADDR, (rail - 1) * 2 + 1, 1, (char*)rail_l, 1);
    rail_vol = ((rail_h & 0x03) << 8) | rail_l;

    switch (rail)
    {
        case 1:
        case 2:
            voltage = (rail_vol * RAIL_VOL_R / 1024) * ((RAIL_VOL_DIV_UP_1 + RAIL_VOL_DIV_DN_1) / RAIL_VOL_DIV_DN_1);
            break;

        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
            voltage = rail_vol * RAIL_VOL_R / 1024;
            break;

        default:
            voltage = 0;
            break;
    }

    return voltage;
}

void I2C_chip_ucd9081_int_handle(void)
{
    I2C_chip_ucd9081_error();
}

void I2C_chip_ucd9081_init(void)
{
    uint8_t reg;

    I2C_i2c1_pmb_read(UCD9081_SLAVE_ADDR, VERSION, 1, (char*)&reg, 1);
    UARTprintf("ucd9081 version: %d.%d\r\n", (reg & 0xf0) >> 4, (reg & 0x0f));

    I2C_chip_ucd9081_power_chk_open();
}

#endif

#ifdef IPMI_CHIP_INA230

#endif


