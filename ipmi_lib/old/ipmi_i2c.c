//*****************************************************************************
//
// ipmi_i2c.c - IPMI use i2c driver
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#include <stdlib.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/i2c.h"
#include "driverlib/debug.h"
#include "driverlib/interrupt.h"
#include "third_party/ustdlib.h"
#include "third_party/list.h"
#include "ipmi_i2c.h"
#include "ipmi_debug.h"

//*****************************************************************************
//
// Defines ipmi_i2c root
//
//*****************************************************************************
static ipmi_i2c_bus_head ipmi_i2c_bus_root;
static ipmi_i2c_dev_head ipmi_i2c_dev_root;
static ipmi_i2c_ram_dev_head ipmi_i2c_ram_dev_root;


//*****************************************************************************
//
//! Initialization the I2C bus module.
//!
//! \param bus specifies the i2c bus data structure.
//!
//! This function initialization the I2C hardware bus, it will open the i2c peripheral
//! and config the I2C gpio
//!
//! \return 0 is OK.
//
//*****************************************************************************
int ipmi_i2c_bus_init(ipmi_i2c_bus *bus)
{
    //
    // Check the arguments.
    //
    ASSERT(bus->sys_peripheral);
    ASSERT(bus->i2c_scl_periph);
    ASSERT(bus->i2c_sda_periph);
    ASSERT(bus->i2c_scl_gpio_port);
    ASSERT(bus->i2c_sda_gpio_port);
    ASSERT(bus->i2c_scl_gpio_pin);
    ASSERT(bus->i2c_sda_gpio_pin);
    ASSERT(bus->i2c_hw_master_base);

    // ��ʼ������ṹ
    INIT_LIST_HEAD(&bus->list);

    // ��ʼ��ϵͳӲ������
    SysCtlPeripheralEnable(bus->sys_peripheral);

    // ��ʼ��SCL�ܽ�Ӳ������
    SysCtlPeripheralEnable(bus->i2c_scl_periph);
    if (bus->i2c_scl_gpio_mux)
        GPIOPinConfigure(bus->i2c_scl_gpio_mux);
    GPIOPinTypeI2C(bus->i2c_scl_gpio_port, bus->i2c_scl_gpio_pin);

    // ��ʼ��SDA�ܽ�Ӳ������
    SysCtlPeripheralEnable(bus->i2c_sda_periph);
    if (bus->i2c_sda_gpio_mux)
        GPIOPinConfigure(bus->i2c_sda_gpio_mux);
    GPIOPinTypeI2C(bus->i2c_sda_gpio_port, bus->i2c_sda_gpio_pin);

    // �豸ʹ�ܣ����ж�
    I2CMasterInit(bus->i2c_hw_master_base, false);
    if (bus->i2c_int)
    {
        //I2CIntRegister(bus->i2c_hw_master_base);
        IntEnable(bus->i2c_int);
        I2CMasterIntEnable(bus->i2c_hw_master_base);
    }
    I2CMasterEnable(bus->i2c_hw_master_base);

    list_add(&bus->list, &ipmi_i2c_bus_root.head);
    ipmi_i2c_bus_root.count++;

    return 0;
}

//*****************************************************************************
//
//! Initialization the I2C device module.
//!
//! \param dev specifies the i2c device data structure.
//! \param bus specifies the i2c bus data structure.
//! \param slave_addr specifies the device on the I2C bus address
//!
//! This function initialization the I2C hardware slave device
//!
//! \return 0 is OK.
//
//*****************************************************************************
int ipmi_i2c_dev_init(ipmi_i2c_dev *dev, ipmi_i2c_bus *bus, unsigned char slave_addr)
{
    //
    // Check the arguments.
    //
    ASSERT(dev);
    ASSERT(bus);
    ASSERT(slave_addr);

    // ��ʼ������ṹ
    INIT_LIST_HEAD(&dev->list);

    dev->bus = bus;
    dev->slave_addr = slave_addr;
    dev->data_addr  = 0;
    dev->data_size  = 0;
    dev->buf_addr   = NULL;

    list_add(&dev->list, &ipmi_i2c_dev_root.head);
    ipmi_i2c_dev_root.count++;

    return 0;
}

//*****************************************************************************
//
//! Initialization the I2C virtual RAM slave device.
//!
//! \param dev specifies the i2c virtual ram slave device data structure.
//!
//! This function initialization the RAM based I2C virtual slave device
//!
//! \return 0 is OK.
//
//*****************************************************************************

int ipmi_i2c_ram_dev_init(ipmi_i2c_ram_dev *dev, ipmi_i2c_bus *bus,
        unsigned char addr, unsigned long reg_size, char *reg_buf)
{
    ASSERT(dev);
    ASSERT(dev->bus);

    if ((addr == 0) ||                  // �����ǹ㲥��ַ
        (addr & 0x80 == 0x80) ||        // ������7λ��ַ��������8λ��ַ
        (addr & 0x78 == 0x78))          // �����Ǳ�����ַ
    {
        return IPMI_I2C_ERR_ADDR;
    }

    INIT_LIST_HEAD(&dev->list);
    dev->bus            = bus;
    dev->my_addr        = addr;
    dev->ram_reg_size   = reg_size;
    dev->ram_reg        = reg_buf;
    dev->read_lock      = OSSemCreate(0);
    dev->write_lock     = OSSemCreate(0);

    if (dev->read_lock == (void*)0 || dev->write_lock == (void*)0)
    {
        return IPMI_I2C_ERR_INTER;
    }


    // ���ô��豸��ַ
    I2CSlaveInit(dev->bus->i2c_hw_slave_base, dev->my_addr);

    // ���ж�
    I2CSlaveIntEnable(dev->bus->i2c_hw_slave_base);
    IntEnable(dev->bus->i2c_int);
    IntMasterEnable();

    // ���豸ʹ��
    I2CSlaveEnable(dev->bus->i2c_hw_slave_base);

    list_add(&dev->list, &ipmi_i2c_ram_dev_root.head);
    ipmi_i2c_ram_dev_root.count++;

    return 0;
}

//*****************************************************************************
//
//! I2C Ram virtual slave device interrupt service module.
//!
//! \param dev specifies the i2c device data structure.
//! \param bus specifies the i2c bus data structure.
//! \param slave_addr specifies the device on the I2C bus address
//!
//! This function initialization the I2C hardware slave device
//!
//
//*****************************************************************************

void ipmi_i2c_ram_dev_service(ipmi_i2c_ram_dev *dev, unsigned long status)
{
    unsigned long slave_act_status;

    if (status)
    {
        slave_act_status = I2CSlaveStatus(dev->bus->i2c_hw_slave_base);     // ��ȡ�ӻ�״̬
        switch (slave_act_status)
        {
            case I2C_SLAVE_ACT_NONE:                            // ����û�������κζ���
                break;

            case I2C_SLAVE_ACT_RREQ_FBR:                        // �����ѷ��ʹӻ���ַ�����ݵ�ַ���ֽ�
                dev->ram_reg_addr = I2CSlaveDataGet(dev->bus->i2c_hw_slave_base);
                dev->ram_reg_addr &= dev->ram_reg_size;
                break;

            case I2C_SLAVE_ACT_RREQ:                            // �����Ѿ��������ݵ��ӻ�
                dev->ram_reg[dev->ram_reg_addr++]= I2CSlaveDataGet(dev->bus->i2c_hw_slave_base);
                dev->ram_reg_addr &= dev->ram_reg_size;
                break;

            case I2C_SLAVE_ACT_TREQ:                            // ��������ӻ���������
                if (0)
                {
                    OSSemPend(dev->read_lock);                  // �ȴ��ӻ�����׼����
                }
                I2CSlaveDataPut(dev->bus->i2c_hw_slave_base, dev->ram_reg[dev->ram_reg_addr++]);
                dev->ram_reg_addr &= dev->ram_reg_size;
                break;

            default:
                break;
        }
    }

    if (0)  /* TODO: �ӻ����յ�ȫ�����ݣ��������һ��д��ɺ�ʹ�ö�ʱ�� */
    {
        OSSemPost(dev->write_lock);
    }
}

void ipmi_i2c_module_init(void)
{
    INIT_LIST_HEAD(&ipmi_i2c_bus_root.head);
    ipmi_i2c_bus_root.count = 0;

    INIT_LIST_HEAD(&ipmi_i2c_dev_root.head);
    ipmi_i2c_dev_root.count = 0;

    INIT_LIST_HEAD(&ipmi_i2c_ram_dev_root.head);
    ipmi_i2c_ram_dev_root.count = 0;
}

