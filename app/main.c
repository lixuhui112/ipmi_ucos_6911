//*****************************************************************************
//
// main.c - IPMI system on LM3S6911
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************


#include <inttypes.h>
#include <string.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_i2c.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "driverlib/i2c.h"
#include "third_party/ustdlib.h"
#include "third_party/uartstdio.h"
#include "ipmi_lib/ipmi_intf.h"
#include "app/lib_common.h"
#include "ucos_ii.h"

extern void ipmi_main_start(void);

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
    UARTprintf("Assert! file:%s line%ld\r\n", pcFilename, ulLine);

    while(1)
    {
    }
}
#endif

//*****************************************************************************
//
// The interrupt handler for the SysTick interrupt.
//
//*****************************************************************************
void
SysTickIntHandler(void)
{
    OS_ENTER_CRITICAL();
    OSIntNesting++;
    OS_EXIT_CRITICAL();

    OSTimeTick();
    OSIntExit();

    //
    // Call the SoftI2C tick function.
    //
    //SoftI2CTimerTick(&g_sI2C);
}

//*****************************************************************************
//
// JTAG 防锁死等待
//
//*****************************************************************************
void JTAG_Wait(void)
{
    unsigned long n = 10000;

    while (n > 0)
        n--;
}


//*********************************************************
// Init Ethernet hardware for IPMI-LAN
//*********************************************************
//void ETH_Init(void)
//{
//}


//*****************************************************************************
//
// 板级开发包初始化
//
//*****************************************************************************
void BSP_init(void)
{
    // Set the clocking to run directly from the crystal.
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_8MHZ);

    // Enable processor interrupts.
    IntMasterEnable();

    // Configure SysTick for a periodic interrupt.
    SysTickPeriodSet(SysCtlClockGet() / SYSTICKHZ);
    SysTickEnable();
    SysTickIntEnable();

    // Initialize the DEBUG UART.           (UART0)
    DEBUG_UART_init();

    // Initialize the IO Hardware.          (LED/SOL/I2C_HOTSWAP)
    IO_init();

    // Initialize the UART Hardware.        (ICMB/SOL)
    UART_init();

    // Initialize the SPI Hardware.         (SSIF)
    SPI_init();

    // Initialize the I2C Hardware.         (IPMB/PMB)
    I2C_init();

    // Initialize the Ethernet Hardware.    (LAN)
    ETH_init();
}

int main(void)
{
    // JTAG防写死，如果使用JTAG引脚作为GPIO，需要开启功能以便烧写调试
    //JTAG_Wait();

    // 芯片外设初始化
    BSP_init();

    // IPMI主任务开始运行
    ipmi_main_start();

    while(1)
    {
    }
}

