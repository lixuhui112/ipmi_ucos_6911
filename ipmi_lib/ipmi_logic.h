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

#define LOGIC_DEVICE_ID_REG             0x0000      /* �豸ID           */
#define LOGIC_DEVICE_REV_REG            0x0001      /* �豸�汾��       */
#define LOGIC_PRODUCT_ID_REG            0x0002      /* ��ƷID           */
#define LOGIC_SITOK_REG                 0x0003      /* ��λ״̬         */
#define LOGIC_POH_REG                   0x0004      /* ϵͳ����ʱ��     */

#define FPGA_R_BIT                      0x0000      /* ����־λ         */
#define FPGA_W_BIT                      0x8000      /* д��־λ         */

#define LOGIC_2_R_FPGA(addr)    (BSWAP_16(addr | FPGA_R_BIT))
#define LOGIC_2_W_FPGA(addr)    (BSWAP_16(addr | FPGA_W_BIT))

#endif  // __IPMI_LOGIC_H__

