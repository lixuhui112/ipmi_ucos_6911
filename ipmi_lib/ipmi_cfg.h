//*****************************************************************************
//
// ipmi_cfg.h
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#ifndef __IPMI_CFG_H__
#define __IPMI_CFG_H__

//
// IPMI传感器地址
//
#define AT24C_SLAVE_ADDR        0x54        // EEPROM芯片地址

#define ADT7470_SLAVE_ADDR_1    0x2c        // 风扇状态监控芯片地址
#define ADT7470_SLAVE_ADDR_2    0x2f        //

#define MAX6635_SLAVE_ADDR_1    0x48        // 温度传感器芯片地址1
#define MAX6635_SLAVE_ADDR_2    0x49        //

#define INA230_SLAVE_ADDR       0x00        // 电源板电压电流监控芯片地址

#define UCD9081_SLAVE_ADDR      0x65        // 交换板电压监控芯片地址

#define PCF8563_SLAVE_ADDR      0x51        // 板载RTC芯片地址



#endif  // __IPMI_CFG_H__

