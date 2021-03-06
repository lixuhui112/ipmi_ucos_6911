//*****************************************************************************
//
// startup_ewarm.c - Startup code for use with IAR's Embedded Workbench,
//                   version 5.
//
// Copyright (c) 2005-2010 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
//
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
//
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
//
// This is part of revision 6075 of the EK-LM3S6965 Firmware Package.
//
//*****************************************************************************

#include <ipmi_lib/ipmi_modules.h>
//*****************************************************************************
//
// Enable the IAR extensions for this source file.
//
//*****************************************************************************
#pragma language=extended

//*****************************************************************************
//
// Forward declaration of the default fault handlers.
//
//*****************************************************************************
static void NmiSR(void);
static void FaultISR(void);
static void IntDefaultHandler(void);

//*****************************************************************************
//
// External declaration for the interrupt handler used by the application.
//
//*****************************************************************************
extern void SysTickIntHandler(void);
extern void OSPendSV(void);
#if (defined(IPMI_MODULES_I2C0_IPMB))
extern void I2C_i2c0_int_handler(void);         /* To IPMI-IPMB as Master and Slaver */
#endif
#if (defined(IPMI_MODULES_I2C1_HARD_PMB))
extern void I2C_i2c1_int_handler(void);         /* To Sensors as Master */
#endif
#if (defined(IPMI_MODULES_UART0_DEBUG))
extern void UART_uart0_int_handler(void);       /* To Debug */
#endif
#if (defined(IPMI_MODULES_UART1_ICMB))
extern void UART_uart1_int_handler(void);       /* To IPMI-ICMB */
#endif
#if (defined(IPMI_MODULES_UART2_SOL))
extern void UART_uart2_int_handler(void);       /* To IPMI-SOL */
#endif
#if (defined(IPMI_MODULES_SPI1_SSIF))
extern void SPI_spi1_int_handler(void);         /* To CPU as Slaver */
#endif
#if (defined(IPMI_MODULES_ETH_LAN))
extern void ETH_eth0_int_handler(void);         /* To IPMI-LAN */
#endif
extern void IO_gpio_a_int_handler(void);        /* To Interrupt */
extern void IO_gpio_b_int_handler(void);        /* To Interrupt */
extern void IO_gpio_c_int_handler(void);        /* To Interrupt */

//*****************************************************************************
//
// The entry point for the application startup code.
//
//*****************************************************************************
extern void __iar_program_start(void);

//*****************************************************************************
//
// Reserve space for the system stack.
//
//*****************************************************************************
static unsigned long pulStack[256] @ ".noinit";

//*****************************************************************************
//
// A union that describes the entries of the vector table.  The union is needed
// since the first entry is the stack pointer and the remainder are function
// pointers.
//
//*****************************************************************************
typedef union
{
    void (*pfnHandler)(void);
    unsigned long ulPtr;
}
uVectorEntry;

//*****************************************************************************
//
// The vector table.  Note that the proper constructs must be placed on this to
// ensure that it ends up at physical address 0x0000.0000.
//
//*****************************************************************************
__root const uVectorEntry __vector_table[] @ ".intvec" =
{
    { .ulPtr = (unsigned long)pulStack + sizeof(pulStack) },
                                            // The initial stack pointer
    __iar_program_start,                    // The reset handler
    NmiSR,                                  // The NMI handler
    FaultISR,                               // The hard fault handler
    IntDefaultHandler,                      // The MPU fault handler
    IntDefaultHandler,                      // The bus fault handler
    IntDefaultHandler,                      // The usage fault handler
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    IntDefaultHandler,                      // SVCall handler
    IntDefaultHandler,                      // Debug monitor handler
    0,                                      // Reserved
    OSPendSV,                               // The PendSV handler
    SysTickIntHandler,                      // The SysTick handler
    IO_gpio_a_int_handler,                  // GPIO Port A
    IO_gpio_b_int_handler,                  // GPIO Port B
    IO_gpio_c_int_handler,                  // GPIO Port C
    IntDefaultHandler,                      // GPIO Port D
    IntDefaultHandler,                      // GPIO Port E
#if (defined(IPMI_MODULES_UART0_DEBUG))
    UART_uart0_int_handler,                 // UART0 Rx and Tx
#else
    IntDefaultHandler,
#endif
#if (defined(IPMI_MODULES_UART1_ICMB))
    UART_uart1_int_handler,                 // UART1 Rx and Tx
#else
    IntDefaultHandler,
#endif
    IntDefaultHandler,                      // SSI0 Rx and Tx
#if (defined(IPMI_MODULES_I2C0_IPMB))
    I2C_i2c0_int_handler,                   // I2C0 Master and Slave
#else
    IntDefaultHandler,
#endif
    IntDefaultHandler,                      // PWM Fault
    IntDefaultHandler,                      // PWM Generator 0
    IntDefaultHandler,                      // PWM Generator 1
    IntDefaultHandler,                      // PWM Generator 2
    IntDefaultHandler,                      // Quadrature Encoder 0
    IntDefaultHandler,                      // ADC Sequence 0
    IntDefaultHandler,                      // ADC Sequence 1
    IntDefaultHandler,                      // ADC Sequence 2
    IntDefaultHandler,                      // ADC Sequence 3
    IntDefaultHandler,                      // Watchdog timer
    IntDefaultHandler,                      // Timer 0 subtimer A
    IntDefaultHandler,                      // Timer 0 subtimer B
    IntDefaultHandler,                      // Timer 1 subtimer A
    IntDefaultHandler,                      // Timer 1 subtimer B
    IntDefaultHandler,                      // Timer 2 subtimer A
    IntDefaultHandler,                      // Timer 2 subtimer B
    IntDefaultHandler,                      // Analog Comparator 0
    IntDefaultHandler,                      // Analog Comparator 1
    IntDefaultHandler,                      // Analog Comparator 2
    IntDefaultHandler,                      // System Control (PLL, OSC, BO)
    IntDefaultHandler,                      // FLASH Control
    IntDefaultHandler,                      // GPIO Port F
    IntDefaultHandler,                      // GPIO Port G
    IntDefaultHandler,                      // GPIO Port H
#if (defined(IPMI_MODULES_UART2_SOL))
    UART_uart2_int_handler,                 // UART2 Rx and Tx
#else
    IntDefaultHandler,
#endif
#if (defined(IPMI_MODULES_SPI1_SSIF))
    SPI_spi1_int_handler,                   // SSI1 Rx and Tx
#else
    IntDefaultHandler,
#endif
    IntDefaultHandler,                      // Timer 3 subtimer A
    IntDefaultHandler,                      // Timer 3 subtimer B
#if (defined(IPMI_MODULES_I2C1_HARD_PMB))
    I2C_i2c1_int_handler,                   // I2C1 Master and Slave
#else
    IntDefaultHandler,
#endif
    IntDefaultHandler,                      // Quadrature Encoder 1
    IntDefaultHandler,                      // CAN0
    IntDefaultHandler,                      // CAN1
    IntDefaultHandler,                      // CAN2
#if (defined(IPMI_MODULES_ETH_LAN))
    ETH_eth0_int_handler,                   // Ethernet
#else
    IntDefaultHandler,
#endif
    IntDefaultHandler                       // Hibernate
};

//*****************************************************************************
//
// This is the code that gets called when the processor receives a NMI.  This
// simply enters an infinite loop, preserving the system state for examination
// by a debugger.
//
//*****************************************************************************
static void
NmiSR(void)
{
    //
    // Enter an infinite loop.
    //
    while(1)
    {
    }
}

//*****************************************************************************
//
// This is the code that gets called when the processor receives a fault
// interrupt.  This simply enters an infinite loop, preserving the system state
// for examination by a debugger.
//
//*****************************************************************************
static void
FaultISR(void)
{
    //
    // Enter an infinite loop.
    //
    while(1)
    {
    }
}

//*****************************************************************************
//
// This is the code that gets called when the processor receives an unexpected
// interrupt.  This simply enters an infinite loop, preserving the system state
// for examination by a debugger.
//
//*****************************************************************************
static void
IntDefaultHandler(void)
{
    //
    // Go into an infinite loop.
    //
    while(1)
    {
    }
}
