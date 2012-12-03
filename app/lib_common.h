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
#include "third_party/uartstdio.h"

#define IPMI_DEBUG
#undef  IPMI_DEBUG
#ifdef  IPMI_DEBUG
#define DEBUG(msg...)   UARTprintf(##msg)
#else
#define DEBUG(msg...)
#endif


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
void init_fail(void);

#endif  //__LIB_COMMON_H__
