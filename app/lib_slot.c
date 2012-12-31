//*****************************************************************************
//
// lib_slot.c - Slot routines for the IPMI application.
//
//*****************************************************************************

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "app/lib_gpio.h"
#include "app/lib_uart.h"
#include "app/lib_io.h"
#include "app/lib_slot.h"
#include "app/lib_common.h"

#define SITOK_SIGNAL_RETRY          10

//*****************************************************************************
//
// Defines Slot Number for the IPMI System
//
//*****************************************************************************
unsigned long g_slot_number;

//*****************************************************************************
//
// Defines Slot Type for the IPMI System
//
//*****************************************************************************
unsigned long g_slot_type;

#if 0
//*****************************************************************************
//
// Init and Check Slot Present signal is Done
//
//*****************************************************************************
int Slot_Present_Check(void)
{
    int present_signal;
    int retry = SITOK_SIGNAL_RETRY;

    while (1)
    {
        // read sit_signal from cpld
#if defined(BOARD_6911_FAN) || defined(BOARD_6911_POWER)
        present_signal = IO_present_check();
#else
        {
            unsigned char present = 0;
            unsigned char *regaddr = LOGIC_2_R_FPGA(LOGIC_SITOK_REG);

            IPMI_LOGIC_READ(regaddr, (unsigned char*)&present);
            present_signal = present;
        }
#endif

        if (present_signal)
            break;

        if (--retry == 0)
        {
            // loop forever and blink led signal
            init_fail();
        }
        else
        {
            // delay 2s
            delay(2);
        }
    }
    return 0;
}
#endif

//*****************************************************************************
//
// Init Slot Number to System/IPMB_addr
//
//*****************************************************************************
int Slot_Num_Init(void)
{
    // Enable GPIO bank to read of the slot number.
    SysCtlPeripheralEnable(GPIO_SLOT_ADDR0_PERIPH_BASE);
    SysCtlPeripheralEnable(GPIO_SLOT_ADDR1_PERIPH_BASE);
    SysCtlPeripheralEnable(GPIO_SLOT_ADDR2_PERIPH_BASE);
    SysCtlPeripheralEnable(GPIO_SLOT_ADDR3_PERIPH_BASE);
    SysCtlPeripheralEnable(GPIO_SLOT_ADDR4_PERIPH_BASE);

    GPIOPinTypeGPIOInput(GPIO_SLOT_ADDR0_PORT, GPIO_SLOT_ADDR0_PIN);
    GPIOPadConfigSet(GPIO_SLOT_ADDR0_PORT, GPIO_SLOT_ADDR0_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPinTypeGPIOInput(GPIO_SLOT_ADDR1_PORT, GPIO_SLOT_ADDR1_PIN);
    GPIOPadConfigSet(GPIO_SLOT_ADDR1_PORT, GPIO_SLOT_ADDR1_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPinTypeGPIOInput(GPIO_SLOT_ADDR2_PORT, GPIO_SLOT_ADDR2_PIN);
    GPIOPadConfigSet(GPIO_SLOT_ADDR2_PORT, GPIO_SLOT_ADDR2_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPinTypeGPIOInput(GPIO_SLOT_ADDR3_PORT, GPIO_SLOT_ADDR3_PIN);
    GPIOPadConfigSet(GPIO_SLOT_ADDR3_PORT, GPIO_SLOT_ADDR3_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPinTypeGPIOInput(GPIO_SLOT_ADDR4_PORT, GPIO_SLOT_ADDR4_PIN);
    GPIOPadConfigSet(GPIO_SLOT_ADDR4_PORT, GPIO_SLOT_ADDR4_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    g_slot_number = 0;
    g_slot_number |= GPIOPinRead(GPIO_SLOT_ADDR4_PORT, GPIO_SLOT_ADDR4_PIN) ? 0x1 << 4 : 0;
    g_slot_number |= GPIOPinRead(GPIO_SLOT_ADDR3_PORT, GPIO_SLOT_ADDR3_PIN) ? 0x1 << 3 : 0;
    g_slot_number |= GPIOPinRead(GPIO_SLOT_ADDR2_PORT, GPIO_SLOT_ADDR2_PIN) ? 0x1 << 2 : 0;
    g_slot_number |= GPIOPinRead(GPIO_SLOT_ADDR1_PORT, GPIO_SLOT_ADDR1_PIN) ? 0x1 << 1 : 0;
    g_slot_number |= GPIOPinRead(GPIO_SLOT_ADDR0_PORT, GPIO_SLOT_ADDR0_PIN) ? 0x1 << 0 : 0;

    return 0;
}

//*****************************************************************************
//
// Init Slot Type to System
//
//*****************************************************************************
int Slot_Type_Init(void)
{
    // Enable GPIO bank to read of the slot number.
    SysCtlPeripheralEnable(GPIO_SLOT_TYPE0_PERIPH_BASE);
    SysCtlPeripheralEnable(GPIO_SLOT_TYPE1_PERIPH_BASE);
    SysCtlPeripheralEnable(GPIO_SLOT_TYPE2_PERIPH_BASE);

    GPIOPinTypeGPIOInput(GPIO_SLOT_TYPE0_PORT, GPIO_SLOT_TYPE0_PIN);
    GPIOPadConfigSet(GPIO_SLOT_TYPE0_PORT, GPIO_SLOT_TYPE0_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPinTypeGPIOInput(GPIO_SLOT_TYPE1_PORT, GPIO_SLOT_TYPE1_PIN);
    GPIOPadConfigSet(GPIO_SLOT_TYPE1_PORT, GPIO_SLOT_TYPE1_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPinTypeGPIOInput(GPIO_SLOT_TYPE2_PORT, GPIO_SLOT_TYPE2_PIN);
    GPIOPadConfigSet(GPIO_SLOT_TYPE2_PORT, GPIO_SLOT_TYPE2_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    g_slot_type = 0;
    g_slot_type |= GPIOPinRead(GPIO_SLOT_TYPE2_PORT, GPIO_SLOT_TYPE2_PIN) ? 0x1 << 2 : 0;
    g_slot_type |= GPIOPinRead(GPIO_SLOT_TYPE1_PORT, GPIO_SLOT_TYPE1_PIN) ? 0x1 << 1 : 0;
    g_slot_type |= GPIOPinRead(GPIO_SLOT_TYPE0_PORT, GPIO_SLOT_TYPE0_PIN) ? 0x1 << 0 : 0;

    return 0;
}

//*****************************************************************************
//
// Open the Slot HotSwap chip
//
//*****************************************************************************
int Slot_HotSwap_Open(void)
{
    IO_i2c_hotswap_set(1);

    return 0;
}

//*****************************************************************************
//
// Open the Slot HotSwap chip
//
//*****************************************************************************
int Slot_Power_Open(void)
{
    return 0;
}

