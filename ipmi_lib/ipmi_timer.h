//*****************************************************************************
//
// ipmi_timer.h - IPMI Timer Header File for Common
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#ifndef __IPMI_TIMER_H__
#define __IPMI_TIMER_H__
#include <stdlib.h>
#include "ipmi.h"
#include "ucos_ii.h"
#include "app/lib_common.h"
#include "ipmi_lib/ipmi_common.h"


//*****************************************************************************
//
// Defines timer for the ipmi task
//
//*****************************************************************************
#define IPMI_TIMER_PERIOD       10


void ipmi_timer_init(void);

#endif  // __IPMI_TIMER_H__

