//*****************************************************************************
//
// ipmi_timer.c - IPMI Timer Function for Common
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************
#include <stdlib.h>
#include "ipmi.h"
#include "ucos_ii.h"
#include "app/lib_common.h"


//*****************************************************************************
//
// Defines timer for the ipmi task
//
//*****************************************************************************
OS_TMR *ipmi_timer;


//*****************************************************************************
//
// Defines IPMI Generic Timer Task, shot per second
//
//*****************************************************************************
extern void ipmi_sensor_scan_period(void);
extern void ipmi_sensor_dead_limit(void);
void ipmi_timer_task(void *ptmr, void *param)
{
    // add storage time per second
    ipmi_global.timestamp++;

    // scan sensor chip who use period function per second
    if (ipmi_global.timestamp % IPMI_SENSOR_SCAN_PERIOD == 0) {
        ipmi_sensor_scan_period();
        //ipmi_sensor_dead_limit();     // no use
    }
}

void ipmi_timer_init(void)
{
    INT8U err;

    // LED定时器
    led_start();

    // IPMI秒定时器
    ipmi_timer = OSTmrCreate(0, IPMI_TIMER_PERIOD, OS_TMR_OPT_PERIODIC, ipmi_timer_task, NULL, "ipmi_timer", &err);
    if (err == OS_ERR_NONE)
    {
        OSTmrStart(ipmi_timer, &err);
    }

}




