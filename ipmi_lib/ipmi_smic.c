//*****************************************************************************
//
// ipmi_smic.c - IPMI SMIC (Server Management Interface Chip) interface
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#include <stdlib.h>
#include "ipmi_smic.h"
#include "ipmi_i2c.h"

ipmi_i2c_ram_dev ipmi_smic_dev;
ipmi_smic_intf_t ipmi_smic_intf_dev;

int ipmi_smic_intf_init(ipmi_i2c_bus *bus)
{
    // 初始化SMIC接口设备
    memset(&ipmi_smic_intf_dev, 0, sizeof(ipmi_smic_intf_t));
    ipmi_smic_intf_dev.smic_intf_slave_addr = SMIC_SLAVE_ADDR;
    ipmi_smic_intf_dev.req_buf = &ipmi_smic_intf_dev.reg_buf[0];
    ipmi_smic_intf_dev.rsp_buf = &ipmi_smic_intf_dev.reg_buf[SMIC_REG_SIZE];

    // 初始化I2C从虚拟设备
    ipmi_i2c_ram_dev_init(&ipmi_smic_dev, bus, ipmi_smic_intf_dev.smic_intf_slave_addr,
            SMIC_REG_SIZE*2, &ipmi_smic_intf_dev.reg_buf[0]);

    return 0;
}

int ipmi_smic_cmd_process()
{
    // 已经读取主设备发出的命令
    OSSemPost(ipmi_smic_dev->write_lock);

    // 处理主设备发出的命令，并将结果保存至rsp_buf中
    ipmi_cmd_dispatch(ipmi_smic_intf_dev.req_buf, ipmi_smic_intf_dev.rsp_buf);

    // 通知SMIC接口处理完毕
    OSSemPost(ipmi_smic_dev->read_lock);

    return 0;
}

