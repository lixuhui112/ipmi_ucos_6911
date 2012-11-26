//*****************************************************************************
//
// ipmi_cpld.h - IPMI CPLD define
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#ifndef __IPMI_CPLD_H__
#define __IPMI_CPLD_H__

#define CPLD_DEVICE_ID_REG              0x00        /* 设备ID           */
#define CPLD_DEVICE_REV_REG             0x01        /* 设备版本号       */
#define CPLD_PRODUCT_ID_REG             0x02        /* 产品ID           */
#define CPLD_SITOK_REG                  0x03        /* 在位状态         */
#define CPLD_POH_REG                    0x04        /* 系统运行时间     */

#endif  // __IPMI_CPLD_H__
