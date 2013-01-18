//*****************************************************************************
//
// lib_slot.c - Slot routines for the IPMI application.
//
//*****************************************************************************

#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "app/lib_common.h"

//*****************************************************************************
//
// Global variable for device
//
//*****************************************************************************
uint8_t g_slot_addr;
uint8_t g_board_type;
uint8_t g_present_ok;

//*****************************************************************************
//
// Delay for the specified number of seconds.  Depending upon the current
// SysTick value, the delay will be between N-1 and N seconds (i.e. N-1 full
// seconds are guaranteed, along with the remainder of the current second).
//
//*****************************************************************************
void delay(unsigned long ulSeconds)
{
    //
    // Loop while there are more seconds to wait.
    //
    while(ulSeconds--)
    {
        //
        // Wait until the SysTick value is less than 1000.
        //
        while(SysTickValueGet() > 1000)
        {
        }

        //
        // Wait until the SysTick value is greater than 1000.
        //
        while(SysTickValueGet() < 1000)
        {
        }
    }
}

void mdelay(int ms)
{
    SysCtlDelay(ms * (SysCtlClockGet() / 1000));
}

void init_fail(void)
{
    while (1)
    {
    }
}

uint8_t board_type(void)
{
    if (g_slot_addr == 0x01 || g_slot_addr == 0x02) {
        return BOARD_TYPE_POWER;
    } else if (g_slot_addr == 0x03 || g_slot_addr == 0x04 || g_slot_addr == 0x08 || g_slot_addr == 0x09) {
        return BOARD_TYPE_SWITCH;
    } else if (g_slot_addr == 0x05 || g_slot_addr == 0x07) {
        return BOARD_TYPE_FABRIC;
    } else if (g_slot_addr == 0x06) {
        return BOARD_TYPE_SWITCH10G;
    } else if (g_slot_addr == 0x1d || g_slot_addr == 0x1e) {
        return BOARD_TYPE_FAN;
    }
    return BOARD_TYPE_NONE;
}

