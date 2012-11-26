//*****************************************************************************
//
// lib_vol.h - Voltage Monitor Header for the IPMI application.
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#ifndef _LIB_VOL_H_
#define _LIB_VOL_H_

//*****************************************************************************
//
// Define The I2C Chip UCD9081 register.  (Power Voltage Sensor)
//
//*****************************************************************************
#define RAIL1H      0x00
#define RAIL1L      0x01
#define RAIL2H      0x02
#define RAIL2L      0x03
#define RAIL3H      0x04
#define RAIL3L      0x05
#define RAIL4H      0x06
#define RAIL4L      0x07
#define RAIL5H      0x08
#define RAIL5L      0x09
#define RAIL6H      0x0a
#define RAIL6L      0x0b
#define RAIL7H      0x0c
#define RAIL7L      0x0d
#define RAIL8H      0x0e
#define RAIL8L      0x0f
#define ERROR1      0x20
#define ERROR2      0x21
#define ERROR3      0x22
#define ERROR4      0x23
#define ERROR5      0x24
#define ERROR6      0x25
#define STATUS      0x26
#define VERSION     0x27
#define RAILSTATUS1 0x28
#define RAILSTATUS2 0x29
#define FLASHLOCK   0x2e
#define RESTART     0x2f
#define WADDR1      0x30
#define WADDR2      0x31
#define WDATA1      0x32
#define WDATA2      0x33

#define STATUS_IIC_ERR      0x80        // I2C PHY layer error
#define STATUS_RAIL_ERR     0x40        // RAIL error pending
#define STATUS_NVERRLOG     0x20        // ERROR points to non-volatile log (if held in RESET)
                                        //      and entries present in non-volatile log
#define STATUS_FW_ERR       0x10        // Device firmware error detected, device is idle
#define STATUS_PARAM_ERR    0x08        // Parameters invalid, last config loaded
#define STATUS_REG_W_ERR    0x03        // Write access error
#define STATUS_REG_R_ERR    0x02        // Read access error
#define STATUS_REG_A_ERR    0x01        // Invalid address

#define RAIL_VOL_R          2.5         // Voltage Reference

#define RAIL_VOL_DIV_UP_1   1.96
#define RAIL_VOL_DIV_DN_1   1.96
#define RAIL_VOL_DIV_UP_2   1.96
#define RAIL_VOL_DIV_DN_2   1.96
#define RAIL_VOL_DIV_UP_3   0
#define RAIL_VOL_DIV_DN_3   0
#define RAIL_VOL_DIV_UP_4   0
#define RAIL_VOL_DIV_DN_4   0
#define RAIL_VOL_DIV_UP_5   0
#define RAIL_VOL_DIV_DN_5   0
#define RAIL_VOL_DIV_UP_6   0
#define RAIL_VOL_DIV_DN_6   0
#define RAIL_VOL_DIV_UP_7   0
#define RAIL_VOL_DIV_DN_7   0
#define RAIL_VOL_DIV_UP_8   0
#define RAIL_VOL_DIV_DN_8   0

#endif      // _LIB_VOL_H_

