//*****************************************************************************
//
// ipmi_logic.h - IPMI FPGA/CPLD define
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#ifndef __IPMI_LOGIC_H__
#define __IPMI_LOGIC_H__

#include "ipmi_lib/ipmi_common.h"

#define LOGIC_DEVICE_ID_REG             0x0000      /* 设备ID           */
#define LOGIC_DEVICE_REV_REG            0x0001      /* 设备版本号       */
#define LOGIC_PRODUCT_ID_REG            0x0002      /* 产品ID           */
#define LOGIC_SITOK_REG                 0x0003      /* 在位状态         */
#define LOGIC_POH_REG                   0x0004      /* 系统运行时间     */

#define FPGA_R_BIT                      0x0000      /* 读标志位         */
#define FPGA_W_BIT                      0x8000      /* 写标志位         */

#define LOGIC_2_R_FPGA(addr)    (BSWAP_16(addr | FPGA_R_BIT))
#define LOGIC_2_W_FPGA(addr)    (BSWAP_16(addr | FPGA_W_BIT))

#endif  // __IPMI_LOGIC_H__

