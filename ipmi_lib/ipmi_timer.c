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
extern uint32_t g_sel_sdr_time;
extern void ipmi_sensor_scan_period(void);
void ipmi_timer_task(void *ptmr, void *param)
{
    // add storage time per second
    g_sel_sdr_time++;

    // scan sensor chip who use period function per second
    if (g_sel_sdr_time % 5) {
        ipmi_sensor_scan_period();
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




