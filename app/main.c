//*****************************************************************************
//
// main.c - IPMI system on LM3S6911
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

//*****************************************************************************
// 该LM3S6911微控制器包括下列的产品特性
//   32位RISC性能
//   采用为小封装应用方案而优化的 32位ARM. Cortex.-M3 v7M架构。
//   提供系统时钟、包括一个简单的24位写清零、递减、自装载计数器，同时具有灵活的控制机制
//   仅采用与Thumb.兼容的Thumb-2指令集以获取更高的代码密度
//   工作频率为50-MHz
//   硬件除法和单周期乘法
//   集成嵌套向量中断控制器（NVIC），使中断的处理更为简捷
//   30 中断具有8个优先等级
//   带存储器保护单元（MPU），提供特权模式来保护操作系统的功能
//   非对齐式数据访问，使数据能够更为有效的安置到存储器中
//   精确的位操作（bit-banding），不仅最大限度的利用了存储器空间而且还改良了对外设的控制
//  256 KB单周期Flash
//  64 KB单周期访问的SRAM
//  4个通用定时器模块(GPTM)，每个提供2个16-位定时器
//  兼容ARM FiRM的看门狗定时器
//  10/100以太网控制器
//  两个同步串行接口（SSI）
//  3个完全可编程的16C550-type UART，支持IrDA
//  2个独立集成的模拟比较器
//  两个I2C模块,主机带有仲裁和时钟同步功能、支持多个主机、以及7位寻址模式
//  高达10-46个GPIO，具体数目取决于配置
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

