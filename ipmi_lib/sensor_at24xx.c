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
#include "ipmi_lib/ipmi_cfg.h"
#include "app/lib_common.h"
#include "app/lib_i2c.h"
#include <string.h>
#include <stdio.h>

#ifdef IPMI_CHIP_AT24CXX

#define MAX_AT24XX_COUNT            1

#define AT24C64_MAX_SIZE            8192    /* 总共8k大小 */
#define AT24C64_PAGE_SIZE           32      /* 每页32个字节 */
#define AT24C64_PAGE_NUM            256     /* 总共256个页面 */
#define AT24C64_ADDR_SIZE           2

const uint8_t at24xx_i2c_addr[MAX_AT24XX_COUNT] = {AT24CXX_SLAVE_ADDR};   /* 8bit address */

I2C_DEVICE at24xx_dev[MAX_AT24XX_COUNT];

void at24xx_init_chip(void)
{
    I2C_i2c1_slave_dev_init(&at24xx_dev[0], at24xx_i2c_addr[0], AT24C64_ADDR_SIZE);
}

void at24xx_init(void)
{
    at24xx_init_chip();
}

uint8_t at24xx_read(uint32_t addr, uint8_t *buffer, uint32_t size)
{
    if (addr >= AT24C64_MAX_SIZE || size > AT24C64_MAX_SIZE)
    {
        return -1;
    }

    I2C_i2c1_slave_dev_set(&at24xx_dev[0], addr, (uint8_t*)&buffer[0], size);
    I2C_i2c1_master_read(&at24xx_dev[0]);

    return 0;
}

uint8_t at24xx_write(uint32_t addr, uint8_t *buffer, uint32_t size)
{
    if (addr >= AT24C64_MAX_SIZE || size > AT24C64_MAX_SIZE)
    {
        return -1;
    }

    I2C_i2c1_slave_dev_set(&at24xx_dev[0], addr, (uint8_t*)&buffer[0], size);
    I2C_i2c1_master_write(&at24xx_dev[0]);

    return 0;
}

#endif  // IPMI_CHIP_AT24CXX

