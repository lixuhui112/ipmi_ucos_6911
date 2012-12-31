//*****************************************************************************
//
// lib_io.c - I/O routines for the IPMI application.
//
//*****************************************************************************

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_pwm.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "ipmi_lib/ipmi_modules.h"
#include "app/lib_gpio.h"
#include "app/lib_io.h"

static unsigned long gpio_a_int_mask;
static unsigned long gpio_b_int_mask;
static unsigned long gpio_c_int_mask;
static IO_INT *gpio_a_int_table[8];
static IO_INT *gpio_b_int_table[8];
static IO_INT *gpio_c_int_table[8];

#ifdef IPMI_MODULES_GPIO_CPU_LED
//*****************************************************************************
//
// Set the status LED0 on or off.
//
//*****************************************************************************
void IO_led0_set(tBoolean bOn)
{
    // Turn the LED on or off as requested.
    GPIOPinWrite(GPIO_LED0_PORT, GPIO_LED0_PIN, bOn ? GPIO_LED0_PIN : 0);
}

//*****************************************************************************
//
// LED0 Init
//
//*****************************************************************************
void IO_led0_init(void)
{
    // Enable GPIO bank C to allow control of the LED.
    SysCtlPeripheralEnable(GPIO_LED0_PERIPH_BASE);

    // Configure Port for as an output for the status LED.
    GPIOPinTypeGPIOOutput(GPIO_LED0_PORT, GPIO_LED0_PIN);

    // Initialize LED to ON (1)
    GPIOPinWrite(GPIO_LED0_PORT, GPIO_LED0_PIN, GPIO_LED0_PIN);
}

//*****************************************************************************
//
// Set the status LED1 on or off.
//
//*****************************************************************************
void IO_led1_set(tBoolean bOn)
{
    // Turn the LED on or off as requested.
    GPIOPinWrite(GPIO_LED1_PORT, GPIO_LED1_PIN, bOn ? GPIO_LED1_PIN : 0);
}

//*****************************************************************************
//
// LED1 Init
//
//*****************************************************************************
void IO_led1_init(void)
{
    // Enable GPIO bank C to allow control of the LED.
    SysCtlPeripheralEnable(GPIO_LED1_PERIPH_BASE);

    // Configure Port for as an output for the status LED.
    GPIOPinTypeGPIOOutput(GPIO_LED1_PORT, GPIO_LED1_PIN);

    // Initialize LED to ON (1)
    GPIOPinWrite(GPIO_LED1_PORT, GPIO_LED1_PIN, GPIO_LED1_PIN);
}
#endif


#ifdef IPMI_MODULES_GPIO_SOL_SEL
//*****************************************************************************
//
// Set the status SOL_SEL on or off.
//
//*****************************************************************************
void IO_sol_set(tBoolean bOn)
{
    // Turn the LED on or off as requested.
    GPIOPinWrite(GPIO_SOL_UART2_SEL_PORT, GPIO_SOL_UART2_SEL_PIN, bOn ? GPIO_SOL_UART2_SEL_PIN : 0);
}

//*****************************************************************************
//
// Get the status of SOL_SEL.
//
//*****************************************************************************
int IO_sol_get(void)
{
    //
    // Get the state of the SOL
    //
    if (GPIOPinRead(GPIO_SOL_UART2_SEL_PORT, GPIO_SOL_UART2_SEL_PIN))
    {
        return(1);
    }
    else
    {
        return(0);
    }
}

//*****************************************************************************
//
// SOL Init
//
//*****************************************************************************
void IO_sol_init(void)
{
    // Enable GPIO bank to allow control of the SOL.
    SysCtlPeripheralEnable(GPIO_SOL_UART2_PERIPH_BASE);

    // Configure Port for as an output for the status SOL.
    GPIOPinTypeGPIOOutput(GPIO_SOL_UART2_SEL_PORT, GPIO_SOL_UART2_SEL_PIN);

    // Initialize SOL to OFF (0)
    GPIOPinWrite(GPIO_SOL_UART2_SEL_PORT, GPIO_SOL_UART2_SEL_PIN, 0);
}
#endif


#ifdef IPMI_MODULES_GPIO_I2C_HOTSWAP_SEL
//*****************************************************************************
//
// Set the status I2C_HOTSWAP on or off.
//
//*****************************************************************************
void IO_i2c_hotswap_set(tBoolean bOn)
{
    // Turn the LED on or off as requested.
    GPIOPinWrite(GPIO_I2C_HOTSWAP_SEL_PORT, GPIO_I2C_HOTSWAP_SEL_PIN, bOn ? GPIO_I2C_HOTSWAP_SEL_PIN : 0);
}


//*****************************************************************************
//
// Get the status of I2C_HOTSWAP.
//
//*****************************************************************************
int IO_i2c_hotswap_get(void)
{
    // Get the state of the SOL
    if (GPIOPinRead(GPIO_I2C_HOTSWAP_SEL_PORT, GPIO_I2C_HOTSWAP_SEL_PIN))
    {
        return(1);
    }
    else
    {
        return(0);
    }
}

//*****************************************************************************
//
// I2C_HOTSWAP Init
//
//*****************************************************************************
void IO_i2c_hotswap_init(void)
{
    // Enable GPIO bank to allow control of the I2C_HOTSWAP.
    SysCtlPeripheralEnable(GPIO_I2C_HOTSWAP_PERIPH_BASE);

    // Configure Port for as an output for the status I2C_HOTSWAP.
    GPIOPinTypeGPIOOutput(GPIO_I2C_HOTSWAP_SEL_PORT, GPIO_I2C_HOTSWAP_SEL_PIN);

    // Initialize I2C_HOTSWAP to OFF (0)
    GPIOPinWrite(GPIO_I2C_HOTSWAP_SEL_PORT, GPIO_I2C_HOTSWAP_SEL_PIN, 0);
}
#endif


#ifdef IPMI_MODULES_GPIO_PRESENT
//*****************************************************************************
//
// Present Check
//
//*****************************************************************************
int IO_present_check(void)
{
    unsigned char present_ok;

    present_ok = 0;
    present_ok |= GPIOPinRead(GPIO_SLOT_SITOK3_PORT, GPIO_SLOT_SITOK3_PIN) ? 0x1 << 3 : 0;
    present_ok |= GPIOPinRead(GPIO_SLOT_SITOK2_PORT, GPIO_SLOT_SITOK2_PIN) ? 0x1 << 2 : 0;
    present_ok |= GPIOPinRead(GPIO_SLOT_SITOK1_PORT, GPIO_SLOT_SITOK1_PIN) ? 0x1 << 1 : 0;
    present_ok |= GPIOPinRead(GPIO_SLOT_SITOK0_PORT, GPIO_SLOT_SITOK0_PIN) ? 0x1 << 0 : 0;

    if ((present_ok & GPIO_SLOT_SITOK_MASK) == GPIO_SLOT_SITOK_MASK)
    {
        return 1;
    }

    return 0;
}

//*****************************************************************************
//
// Present Init
//
//*****************************************************************************
void IO_present_init(void)
{
    // Enable GPIO bank to read of the slot number.
    SysCtlPeripheralEnable(GPIO_SLOT_SITOK0_PERIPH_BASE);
    SysCtlPeripheralEnable(GPIO_SLOT_SITOK1_PERIPH_BASE);
    SysCtlPeripheralEnable(GPIO_SLOT_SITOK2_PERIPH_BASE);
    SysCtlPeripheralEnable(GPIO_SLOT_SITOK3_PERIPH_BASE);

    // Configure Port for as an input for the present check.
    GPIOPinTypeGPIOInput(GPIO_SLOT_SITOK0_PORT, GPIO_SLOT_SITOK0_PIN);
    GPIOPadConfigSet(GPIO_SLOT_SITOK0_PORT, GPIO_SLOT_SITOK0_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPinTypeGPIOInput(GPIO_SLOT_SITOK1_PORT, GPIO_SLOT_SITOK1_PIN);
    GPIOPadConfigSet(GPIO_SLOT_SITOK1_PORT, GPIO_SLOT_SITOK1_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPinTypeGPIOInput(GPIO_SLOT_SITOK2_PORT, GPIO_SLOT_SITOK2_PIN);
    GPIOPadConfigSet(GPIO_SLOT_SITOK2_PORT, GPIO_SLOT_SITOK2_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPinTypeGPIOInput(GPIO_SLOT_SITOK3_PORT, GPIO_SLOT_SITOK3_PIN);
    GPIOPadConfigSet(GPIO_SLOT_SITOK3_PORT, GPIO_SLOT_SITOK3_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}
#endif

int gpio_pin_number(unsigned long pins)
{
    int index = -1;

    while (pins != 0) {
        index++;
        if (pins & 0x1) {
            return index;
        }
        pins >>= 1;
    }
    return index;
}

void IO_gpio_int_register(IO_INT *io_int)
{
    if (io_int->pins <= 0 || io_int->pins > GPIO_PIN_7)
        return;

    SysCtlPeripheralEnable(io_int->peripheral);                     // 使能INT所在的GPIO端口
    GPIOPinTypeGPIOInput(io_int->port, io_int->pins);               // 设置INT所在管脚为输入
    GPIOIntTypeSet(io_int->port, io_int->pins, GPIO_FALLING_EDGE);  // 设置INT管脚的中断类型

    switch (io_int->port)
    {
        case GPIO_PORTA_BASE:
            gpio_a_int_mask |= io_int->pins;
            gpio_a_int_table[gpio_pin_number(io_int->pins)] = io_int;
            break;
        case GPIO_PORTB_BASE:
            gpio_b_int_mask |= io_int->pins;
            gpio_b_int_table[gpio_pin_number(io_int->pins)] = io_int;
            break;
        case GPIO_PORTC_BASE:
            gpio_c_int_mask |= io_int->pins;
            gpio_c_int_table[gpio_pin_number(io_int->pins)] = io_int;
            break;
    }

    GPIOPinIntEnable(io_int->port, io_int->pins);                   // 使能INT所在管脚的中断
    IntEnable(io_int->interrupt);                                   // 使能INT端口中断
    IntMasterEnable();                                              // 使能处理器中断
}

void IO_gpio_a_int_handler(void)
{
    long status;
    IO_INT *io_int;

    status = GPIOPinIntStatus(GPIO_PORTA_BASE, true);               // 读取中断状态
    GPIOPinIntClear(GPIO_PORTA_BASE, status);                       // 清除中断状态，重要
    if (status & gpio_a_int_mask)                                   // 如果中断状态有效
    {
        io_int = gpio_a_int_table[gpio_pin_number(status)];         // 获取中断向量表
        io_int->handler(io_int->param);                             // 调用中断函数及参数
    }
}

void IO_gpio_b_int_handler(void)
{
    long status;
    IO_INT *io_int;

    status = GPIOPinIntStatus(GPIO_PORTB_BASE, true);               // 读取中断状态
    GPIOPinIntClear(GPIO_PORTB_BASE, status);                       // 清除中断状态，重要
    if (status & gpio_b_int_mask)                                   // 如果中断状态有效
    {
        io_int = gpio_b_int_table[gpio_pin_number(status)];         // 获取中断向量表
        io_int->handler(io_int->param);                             // 调用中断函数及参数
    }
}

void IO_gpio_c_int_handler(void)
{
    long status;
    IO_INT *io_int;

    status = GPIOPinIntStatus(GPIO_PORTC_BASE, true);               // 读取中断状态
    GPIOPinIntClear(GPIO_PORTC_BASE, status);                       // 清除中断状态，重要
    if (status & gpio_c_int_mask)                                   // 如果中断状态有效
    {
        io_int = gpio_c_int_table[gpio_pin_number(status)];         // 获取中断向量表
        io_int->handler(io_int->param);                             // 调用中断函数及参数
    }
}


//*****************************************************************************
//
// Init IO hardware for LED/SOL_SEL/I2C_HOTSWAP
//
//*****************************************************************************
void IO_init(void)
{
#ifdef IPMI_MODULES_GPIO_CPU_LED
    IO_led0_init();
    IO_led1_init();
#endif


#ifdef IPMI_MODULES_GPIO_PRESENT
    IO_present_init();
#endif

#ifdef IPMI_MODULES_GPIO_SOL_SEL
    IO_sol_init();
#endif

#ifdef IPMI_MODULES_GPIO_I2C_HOTSWAP_SEL
    IO_i2c_hotswap_init();
#endif
}

