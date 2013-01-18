//*****************************************************************************
//
// ipmi_common.h - IPMI Command for Common
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************
#ifndef __LIB_COMMON_H__
#define __LIB_COMMON_H__

#include <stdarg.h>
#include <inttypes.h>
#include "third_party/uartstdio.h"

#define IPMI_DEBUG
#undef  IPMI_DEBUG
#ifdef  IPMI_DEBUG
#define DEBUG(msg...)   UARTprintf(##msg)
#else
#define DEBUG(msg...)
#endif

#define RET_OK          (0)
#define RET_ERR         (0xffffffff)

//*****************************************************************************
//
// Defines for setting up the system clock.
//
//*****************************************************************************
#define SYSTICKHZ               100
#define SYSTICKMS               (1000 / SYSTICKHZ)
#define SYSTICKUS               (1000000 / SYSTICKHZ)
#define SYSTICKNS               (1000000000 / SYSTICKHZ)

#define SEC2PERIOD(s)           ((s) ? (s * 10) : 1)

void delay(unsigned long ulSeconds);
void mdelay(int ms);
void init_fail(void);


//*****************************************************************************
//
// Defines for Type of BaseBoard.
//
//*****************************************************************************
#define BOARD_TYPE_FABRIC       0x01
#define BOARD_TYPE_SWITCH       0x02
#define BOARD_TYPE_FAN          0x03
#define BOARD_TYPE_POWER        0x04
#define BOARD_TYPE_SWITCH10G    0x05
#define BOARD_TYPE_NONE         0xff

extern uint8_t g_slot_addr;
extern uint8_t g_board_type;
extern uint8_t g_present_ok;

uint8_t board_type(void);


#endif  //__LIB_COMMON_H__

