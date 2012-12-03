//*****************************************************************************
//
// sensor_at24xx.c - The Atmel24xx EEProm
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

/******************************************************************************
TODO:
    2012/11/27

History:
    2012/11/27
                sensor reading
                sensor get
                sensor thresh
                sensor list
******************************************************************************/


#include "ipmi_lib/ipmi.h"
#include "app/lib_common.h"
#include "app/lib_i2c.h"
#include <string.h>
#include <stdio.h>

#define MAX_AT24XX_COUNT            1

const uint8_t at24xx_i2c_addr[MAX_AT24XX_COUNT] = {AT24C_SLAVE_ADDR};   /* 8bit address */

I2C_DEVICE at24xx_dev[MAX_AT24XX_COUNT];

void at24xx_init_chip(void)
{
    I2C_i2c1_slave_dev_init(&at24xx_dev[0], at24xx_i2c_addr[0], 1);
}

void at24xx_init(void)
{
    at24xx_init_chip();
}


