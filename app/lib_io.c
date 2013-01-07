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


static uint32_t gpio_a_int_mask;
static uint32_t gpio_b_int_mask;
static uint32_t gpio_c_int_mask;
static IO_INT *gpio_a_int_table[8];
static IO_INT *gpio_b_int_table[8];
static IO_INT *gpio_c_int_table[8];


#ifdef IPMI_MODULES_GPIO_CPU_LED
//*****************************************************************************
// Set the status LED0 on or off.
//*****************************************************************************
void IO_led0_set(tBoolean bOn)
{
    // Turn the LED on or off as requested.
    GPIOPinWrite(GPIO_LED0_PORT, GPIO_LED0_PIN, bOn ? GPIO_LED0_PIN : 0);
}

//*****************************************************************************
// LED0 Init
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
// Set the status LED1 on or off.
//*****************************************************************************
void IO_led1_set(tBoolean bOn)
{
    // Turn the LED on or off as requested.
    GPIOPinWrite(GPIO_LED1_PORT, GPIO_LED1_PIN, bOn ? GPIO_LED1_PIN : 0);
}

//*****************************************************************************
// LED1 Init
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
// Set the status SOL_SEL on or off.
//*****************************************************************************
void IO_sol_set(tBoolean bOn)
{
    // Turn the LED on or off as requested.
    GPIOPinWrite(GPIO_SOL_UART2_SEL_PORT, GPIO_SOL_UART2_SEL_PIN, bOn ? GPIO_SOL_UART2_SEL_PIN : 0);
}

//*****************************************************************************
// Get the status of SOL_SEL.
//*****************************************************************************
uint8_t IO_sol_get(void)
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
// SOL Init
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
// Set the status I2C_HOTSWAP on or off.
//*****************************************************************************
void IO_i2c_hotswap_set(tBoolean bOn)
{
    // Turn the LED on or off as requested.
    GPIOPinWrite(GPIO_I2C_HOTSWAP_SEL_PORT, GPIO_I2C_HOTSWAP_SEL_PIN, bOn ? GPIO_I2C_HOTSWAP_SEL_PIN : 0);
}


//*****************************************************************************
// Get the status of I2C_HOTSWAP.
//*****************************************************************************
uint8_t IO_i2c_hotswap_get(void)
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
// I2C_HOTSWAP Init
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
// Present OK to Fabric
//*****************************************************************************
void IO_present_ok_fab(tBoolean bOn)
{
    // Enable GPIO bank to allow control of the SITOK.
    SysCtlPeripheralEnable(GPIO_SLOT_SITOK0_PERIPH_BASE);

    // Configure Port for as an output for the status SITOK.
    GPIOPinTypeGPIOOutput(GPIO_SLOT_SITOK0_PORT, GPIO_SLOT_SITOK0_PIN);

    // write SITOK
    GPIOPinWrite(GPIO_SLOT_SITOK0_PORT, GPIO_SLOT_SITOK0_PIN, bOn ? GPIO_SLOT_SITOK0_PIN : 0);
}

//*****************************************************************************
// Present Check
//*****************************************************************************
uint8_t IO_present_check(void)
{
    unsigned char present_ok;

    present_ok = 0;
    present_ok |= GPIOPinRead(GPIO_SLOT_SITCHK3_PORT, GPIO_SLOT_SITCHK3_PIN) ? 0x1 << 3 : 0;
    present_ok |= GPIOPinRead(GPIO_SLOT_SITCHK2_PORT, GPIO_SLOT_SITCHK2_PIN) ? 0x1 << 2 : 0;
    present_ok |= GPIOPinRead(GPIO_SLOT_SITCHK1_PORT, GPIO_SLOT_SITCHK1_PIN) ? 0x1 << 1 : 0;
    present_ok |= GPIOPinRead(GPIO_SLOT_SITCHK0_PORT, GPIO_SLOT_SITCHK0_PIN) ? 0x1 << 0 : 0;

    if ((present_ok & GPIO_SLOT_SITCHK_MASK) == GPIO_SLOT_SITCHK_MASK)
    {
        return 1;
    }

    return 0;
}

//*****************************************************************************
// Present Init
//*****************************************************************************
void IO_present_init(void)
{
    // Enable GPIO bank to read of the slot number.
    SysCtlPeripheralEnable(GPIO_SLOT_SITCHK0_PERIPH_BASE);
    SysCtlPeripheralEnable(GPIO_SLOT_SITCHK1_PERIPH_BASE);
    SysCtlPeripheralEnable(GPIO_SLOT_SITCHK2_PERIPH_BASE);
    SysCtlPeripheralEnable(GPIO_SLOT_SITCHK3_PERIPH_BASE);

    // Configure Port for as an input for the present check.
    GPIOPinTypeGPIOInput(GPIO_SLOT_SITCHK0_PORT, GPIO_SLOT_SITCHK0_PIN);
    GPIOPadConfigSet(GPIO_SLOT_SITCHK0_PORT, GPIO_SLOT_SITCHK0_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPinTypeGPIOInput(GPIO_SLOT_SITCHK1_PORT, GPIO_SLOT_SITCHK1_PIN);
    GPIOPadConfigSet(GPIO_SLOT_SITCHK1_PORT, GPIO_SLOT_SITCHK1_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPinTypeGPIOInput(GPIO_SLOT_SITCHK2_PORT, GPIO_SLOT_SITCHK2_PIN);
    GPIOPadConfigSet(GPIO_SLOT_SITCHK2_PORT, GPIO_SLOT_SITCHK2_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPinTypeGPIOInput(GPIO_SLOT_SITCHK3_PORT, GPIO_SLOT_SITCHK3_PIN);
    GPIOPadConfigSet(GPIO_SLOT_SITCHK3_PORT, GPIO_SLOT_SITCHK3_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}
#endif

#ifdef IPMI_MODULES_GPIO_SLOT_ADDR
//*****************************************************************************
// Slot Address Get
//*****************************************************************************
uint8_t IO_slot_addr_get(void)
{
    uint8_t board_slot_addr = 0;

    board_slot_addr |= GPIOPinRead(GPIO_SLOT_ADDR4_PORT, GPIO_SLOT_ADDR4_PIN) ? 0x1 << 4 : 0;
    board_slot_addr |= GPIOPinRead(GPIO_SLOT_ADDR3_PORT, GPIO_SLOT_ADDR3_PIN) ? 0x1 << 3 : 0;
    board_slot_addr |= GPIOPinRead(GPIO_SLOT_ADDR2_PORT, GPIO_SLOT_ADDR2_PIN) ? 0x1 << 2 : 0;
    board_slot_addr |= GPIOPinRead(GPIO_SLOT_ADDR1_PORT, GPIO_SLOT_ADDR1_PIN) ? 0x1 << 1 : 0;
    board_slot_addr |= GPIOPinRead(GPIO_SLOT_ADDR0_PORT, GPIO_SLOT_ADDR0_PIN) ? 0x1 << 0 : 0;

    return board_slot_addr;
}

//*****************************************************************************
// Slot Address Init
//*****************************************************************************
void IO_slot_addr_init(void)
{
    // Enable GPIO bank to read of the slot number.
    SysCtlPeripheralEnable(GPIO_SLOT_ADDR0_PERIPH_BASE);
    SysCtlPeripheralEnable(GPIO_SLOT_ADDR1_PERIPH_BASE);
    SysCtlPeripheralEnable(GPIO_SLOT_ADDR2_PERIPH_BASE);
    SysCtlPeripheralEnable(GPIO_SLOT_ADDR3_PERIPH_BASE);
    SysCtlPeripheralEnable(GPIO_SLOT_ADDR4_PERIPH_BASE);

    // Configure Port for as an input for the present check.
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
}
#endif

#ifdef IPMI_MODULES_GPIO_BOARD_TYPE
//*****************************************************************************
// Board Type Get
//*****************************************************************************
uint8_t IO_board_type_get(void)
{
    uint8_t board_type = 0;

    board_type |= GPIOPinRead(GPIO_BOARD_TYPE2_PORT, GPIO_BOARD_TYPE2_PIN) ? 0x1 << 2 : 0;
    board_type |= GPIOPinRead(GPIO_BOARD_TYPE1_PORT, GPIO_BOARD_TYPE1_PIN) ? 0x1 << 1 : 0;
    board_type |= GPIOPinRead(GPIO_BOARD_TYPE0_PORT, GPIO_BOARD_TYPE0_PIN) ? 0x1 << 0 : 0;

    return board_type;
}

//*****************************************************************************
// Board Type Init
//*****************************************************************************
void IO_board_type_init(void)
{
    // Enable GPIO bank to read of the slot number.
    SysCtlPeripheralEnable(GPIO_BOARD_TYPE0_PERIPH_BASE);
    SysCtlPeripheralEnable(GPIO_BOARD_TYPE1_PERIPH_BASE);
    SysCtlPeripheralEnable(GPIO_BOARD_TYPE2_PERIPH_BASE);

    // Configure Port for as an input for the present check.
    GPIOPinTypeGPIOInput(GPIO_BOARD_TYPE0_PORT, GPIO_BOARD_TYPE0_PIN);
    GPIOPadConfigSet(GPIO_BOARD_TYPE0_PORT, GPIO_BOARD_TYPE0_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPinTypeGPIOInput(GPIO_BOARD_TYPE1_PORT, GPIO_BOARD_TYPE1_PIN);
    GPIOPadConfigSet(GPIO_BOARD_TYPE1_PORT, GPIO_BOARD_TYPE1_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPinTypeGPIOInput(GPIO_BOARD_TYPE2_PORT, GPIO_BOARD_TYPE2_PIN);
    GPIOPadConfigSet(GPIO_BOARD_TYPE2_PORT, GPIO_BOARD_TYPE2_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}
#endif

#ifdef IPMI_MODULES_GPIO_GOOD_FAB
//*****************************************************************************
// Set the status GOOD_FAB on or off.
//*****************************************************************************
void IO_good_fab_set(tBoolean bOn)
{
    // Turn the LED on or off as requested.
    GPIOPinWrite(GPIO_GOOD_FAB0_PORT, GPIO_GOOD_FAB0_PIN, bOn ? GPIO_GOOD_FAB0_PIN : 0);
    GPIOPinWrite(GPIO_GOOD_FAB1_PORT, GPIO_GOOD_FAB1_PIN, bOn ? GPIO_GOOD_FAB1_PIN : 0);
}

//*****************************************************************************
// Good Fabric Init
//*****************************************************************************
void IO_good_fab_init(void)
{
    // Enable GPIO bank to allow control of the GOOD_FA.
    SysCtlPeripheralEnable(GPIO_GOOD_FAB0_PERIPH_BASE);
    SysCtlPeripheralEnable(GPIO_GOOD_FAB1_PERIPH_BASE);

    // Configure Port for as an output for the status GOOD_FA.
    GPIOPinTypeGPIOOutput(GPIO_GOOD_FAB0_PORT, GPIO_GOOD_FAB0_PIN);
    GPIOPinTypeGPIOOutput(GPIO_GOOD_FAB1_PORT, GPIO_GOOD_FAB1_PIN);

    // Initialize GOOD_FA to Off (0)
    GPIOPinWrite(GPIO_GOOD_FAB0_PORT, GPIO_GOOD_FAB0_PIN, 0);
    GPIOPinWrite(GPIO_GOOD_FAB1_PORT, GPIO_GOOD_FAB1_PIN, 0);
}
#endif


#ifdef GPIO_WATCHDOG_PERIPH_BASE
//*****************************************************************************
// Set the status WATCHDOG
//*****************************************************************************
void IO_watchdog_set(tBoolean bOn)
{
    // Turn the WATCHDOG WDI on or off as requested.
    GPIOPinWrite(GPIO_WATCHDOG_PORT, GPIO_WATCHDOG_PIN, bOn ? GPIO_WATCHDOG_PIN : 0);
}

//*****************************************************************************
// WatchDog Init
//*****************************************************************************
void IO_watchdog_init(void)
{
    // Enable GPIO bank to allow control of the WATCHDOG.
    SysCtlPeripheralEnable(GPIO_WATCHDOG_PERIPH_BASE);

    // Configure Port for as an output for the status WATCHDOG.
    GPIOPinTypeGPIOOutput(GPIO_WATCHDOG_PORT, GPIO_WATCHDOG_PIN);

    // Initialize WATCHDOG to Off (0)
    GPIOPinWrite(GPIO_WATCHDOG_PORT, GPIO_WATCHDOG_PIN, 0);
}
#endif


#ifdef IPMI_MODULES_GPIO_FULL_SPEED
//*****************************************************************************
// Set the status FULL_SPEED
//*****************************************************************************
void IO_full_speed_set(tBoolean bOn)
{
    // Turn the FULL_SPEED on or off as requested.
    GPIOPinWrite(GPIO_FULL_SPEED0_PORT, GPIO_FULL_SPEED0_PIN, bOn ? GPIO_FULL_SPEED0_PIN : 0);
    GPIOPinWrite(GPIO_FULL_SPEED1_PORT, GPIO_FULL_SPEED1_PIN, bOn ? GPIO_FULL_SPEED1_PIN : 0);
}

//*****************************************************************************
// FULL_SPEED Init
//*****************************************************************************
void IO_full_speed_init(void)
{
    // Enable GPIO bank to allow control of the FULL_SPEED.
    SysCtlPeripheralEnable(GPIO_FULL_SPEED0_PERIPH_BASE);
    SysCtlPeripheralEnable(GPIO_FULL_SPEED1_PERIPH_BASE);

    // Configure Port for as an output for the status FULL_SPEED.
    GPIOPinTypeGPIOOutput(GPIO_FULL_SPEED0_PORT, GPIO_FULL_SPEED0_PIN);
    GPIOPinTypeGPIOOutput(GPIO_FULL_SPEED1_PORT, GPIO_FULL_SPEED1_PIN);

    // Initialize FULL_SPEED to ON (0)
    GPIOPinWrite(GPIO_FULL_SPEED0_PORT, GPIO_FULL_SPEED0_PIN, 0);
    GPIOPinWrite(GPIO_FULL_SPEED1_PORT, GPIO_FULL_SPEED1_PIN, 0);
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
#ifdef IPMI_MODULES_GPIO_WATCHDOG                                   // 看门狗初始化
    IO_watchdog_init();
#endif

#ifdef IPMI_MODULES_GPIO_CPU_LED                                    // LED灯初始化
    IO_led0_init();
    IO_led1_init();
#endif

#ifdef IPMI_MODULES_GPIO_PRESENT                                    // 在位信号初始化并通知主控
    IO_present_init();
    if (IO_present_check())
    {
        IO_present_ok_fab(1);
    }
    else
    {
        IO_present_ok_fab(0);
    }
#endif

#ifdef IPMI_MODULES_GPIO_SLOT_ADDR                                  // 槽位地址初始化
    IO_slot_addr_init();
#endif

#ifdef IPMI_MODULES_GPIO_BOARD_TYPE                                 // 板卡类型初始化
    IO_board_type_init();
#endif

#ifdef IPMI_MODULES_GPIO_SOL_SEL                                    // SOL硬件初始化
    IO_sol_init();
#endif

#ifdef IPMI_MODULES_GPIO_I2C_HOTSWAP_SEL                            // I2C热插拔硬件初始化
    IO_i2c_hotswap_init();
#endif

#ifdef IPMI_MODULES_GPIO_GOOD_FAB                                   // 主控信号通知初始化
    IO_good_fab_init();
#endif

#ifdef IPMI_MODULES_GPIO_FULL_SPEED                                 // 风扇板初始化
    IO_full_speed_init();
#endif
}

