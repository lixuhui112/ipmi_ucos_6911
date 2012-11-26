//*****************************************************************************
//
// ipmi_init.c - IPMI Modules Initialization
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/14
// version: v1.0
//
//*****************************************************************************

#include "ipmi_interface.h"

#ifdef IPMI_MODULES_HARD_PMB
void ipmi_hard_pmb_init(void)
{
    ipmi_i2c_module_init();

    i2c0.sys_peripheral = SYSCTL_PERIPH_I2C0;
    i2c0.i2c_scl_periph = SYSCTL_PERIPH_GPIOB;
    i2c0.i2c_sda_periph = SYSCTL_PERIPH_GPIOB;
    i2c0.i2c_scl_gpio_port = GPIO_PORTB_BASE;
    i2c0.i2c_sda_gpio_port = GPIO_PORTB_BASE;
    i2c0.i2c_scl_gpio_pin = GPIO_PIN_2;
    i2c0.i2c_sda_gpio_pin = GPIO_PIN_3;
    i2c0.i2c_scl_gpio_mux = GPIO_PB2_I2C0SCL;
    i2c0.i2c_sda_gpio_mux = GPIO_PB3_I2C0SDA;
    i2c0.i2c_hw_master_base = I2C0_MASTER_BASE;
    i2c0.i2c_hw_slave_base = I2C0_SLAVE_BASE;
    i2c0.i2c_int = INT_I2C0;
    ipmi_i2c_bus_init(&i2c0);

    ipmi_i2c_dev_init(&at24c, &i2c0, AT24C_SLAVE_ADDR);
    ipmi_i2c_dev_init(&adt7470_1, &i2c0, ADT7470_SLAVE_ADDR_1);
    ipmi_i2c_dev_init(&adt7470_2, &i2c0, ADT7470_SLAVE_ADDR_2);
}
#endif

//*****************************************************************************
//
// IPMIÄ£¿é³õÊ¼»¯
//
//*****************************************************************************
void ipmi_init(void)
{
#ifdef IPMI_MODULES_HARD_PMB
    ipmi_hard_pmb_init();
#endif

#ifdef IPMI_MODULES_SOFT_PMB
    ipmi_soft_pmb_init();
#endif

#ifdef IPMI_MODULES_IPMB
    ipmi_ipmb_init();
#endif

#ifdef IPMI_MODULES_SSIF
    ipmi_ssif_init();
#endif

#ifdef IPMI_MODULES_UART
    ipmi_uart_init();
#endif

#ifdef IPMI_MODULES_ETHERNET
    ipmi_ethernet_init();
#endif

#ifdef IPMI_MODULES_SESSION
    ipmi_session_init();
#endif
}

