//*****************************************************************************
//
// ipmi_gpio.h
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#ifndef _IPMI_GPIO_H_
#define _IPMI_GPIO_H_

#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"

//
// FPGA告警输出管脚                         PE6,PE7
//
#define GPIO_FPGA_PERIPH_BASE               SYSCTL_PERIPH_GPIOE
#define GPIO_FPGA_INT_PORT                  GPIO_PORTE_BASE
#define GPIO_FPGA_INT_PIN                   GPIO_PIN_7
#define GPIO_FPGA_ALARM_PORT                GPIO_PORTE_BASE
#define GPIO_FPGA_ALARM_PIN                 GPIO_PIN_6

//
// CPU告警输出管脚                          PD0,PD1
//
#define GPIO_CPU_PERIPH_BASE                SYSCTL_PERIPH_GPIOD
#define GPIO_CPU_INT_PORT                   GPIO_PORTD_BASE
#define GPIO_CPU_INT_PIN                    GPIO_PIN_0
#define GPIO_CPU_ALARM_PORT                 GPIO_PORTD_BASE
#define GPIO_CPU_ALARM_PIN                  GPIO_PIN_1

//
// I2C热插拔芯片使能管脚                    PE5
//
#define GPIO_I2C_HOTSWAP_PERIPH_BASE        SYSCTL_PERIPH_GPIOE
#define GPIO_I2C_HOTSWAP_SEL_PORT           GPIO_PORTE_BASE
#define GPIO_I2C_HOTSWAP_SEL_PIN            GPIO_PIN_5

//
// SOL功能CPU串口重定向使能管脚             PE4
//
#define GPIO_SOL_UART2_PERIPH_BASE          SYSCTL_PERIPH_GPIOE
#define GPIO_SOL_UART2_SEL_PORT             GPIO_PORTE_BASE
#define GPIO_SOL_UART2_SEL_PIN              GPIO_PIN_4

//
// 电压传感器监控状态输入管脚               PC6,PC7
//
#define GPIO_CHIP_UCD9081_GPIO1_PERIPH      SYSCTL_PERIPH_GPIOC
#define GPIO_CHIP_UCD9081_GPIO1_PORT        GPIO_PORTC_BASE
#define GPIO_CHIP_UCD9081_GPIO1_PIN         GPIO_PIN_6
#define GPIO_CHIP_UCD9081_GPIO1_INT         INT_GPIOC
#define GPIO_CHIP_UCD9081_GPIO2_PERIPH      SYSCTL_PERIPH_GPIOC
#define GPIO_CHIP_UCD9081_GPIO2_PORT        GPIO_PORTC_BASE
#define GPIO_CHIP_UCD9081_GPIO2_PIN         GPIO_PIN_7
#define GPIO_CHIP_UCD9081_GPIO2_INT         INT_GPIOC

//
// 温度传感器0状态输入管脚                  PC4,PC5
//
#define GPIO_CHIP_MAX6635_PERIPH0_BASE      SYSCTL_PERIPH_GPIOC
#define GPIO_CHIP_MAX6635_OVER0_PORT        GPIO_PORTC_BASE
#define GPIO_CHIP_MAX6635_OVER0_PIN         GPIO_PIN_4
#define GPIO_CHIP_MAX6635_OVER0_INT         INT_GPIOC
#define GPIO_CHIP_MAX6635_ALERT0_PORT       GPIO_PORTC_BASE
#define GPIO_CHIP_MAX6635_ALERT0_PIN        GPIO_PIN_5
#define GPIO_CHIP_MAX6635_ALERT0_INT        INT_GPIOC

//
// 温度传感器1状态输入管脚                  PB5,PB6
//
#define GPIO_CHIP_MAX6635_PERIPH1_BASE      SYSCTL_PERIPH_GPIOB
#define GPIO_CHIP_MAX6635_OVER1_PORT        GPIO_PORTB_BASE
#define GPIO_CHIP_MAX6635_OVER1_PIN         GPIO_PIN_6
#define GPIO_CHIP_MAX6635_OVER1_INT         INT_GPIOB
#define GPIO_CHIP_MAX6635_ALERT1_PORT       GPIO_PORTB_BASE
#define GPIO_CHIP_MAX6635_ALERT1_PIN        GPIO_PIN_5
#define GPIO_CHIP_MAX6635_ALERT1_INT        INT_GPIOB

//
// 板卡槽位信号读取管脚                     PD4,PD5,PD6,PD7,PB4
//
#define GPIO_SLOT_ADDR0_PERIPH_BASE         SYSCTL_PERIPH_GPIOD
#define GPIO_SLOT_ADDR0_PORT                GPIO_PORTD_BASE
#define GPIO_SLOT_ADDR0_PIN                 GPIO_PIN_4
#define GPIO_SLOT_ADDR1_PERIPH_BASE         SYSCTL_PERIPH_GPIOD
#define GPIO_SLOT_ADDR1_PORT                GPIO_PORTD_BASE
#define GPIO_SLOT_ADDR1_PIN                 GPIO_PIN_5
#define GPIO_SLOT_ADDR2_PERIPH_BASE         SYSCTL_PERIPH_GPIOD
#define GPIO_SLOT_ADDR2_PORT                GPIO_PORTD_BASE
#define GPIO_SLOT_ADDR2_PIN                 GPIO_PIN_6
#define GPIO_SLOT_ADDR3_PERIPH_BASE         SYSCTL_PERIPH_GPIOD
#define GPIO_SLOT_ADDR3_PORT                GPIO_PORTD_BASE
#define GPIO_SLOT_ADDR3_PIN                 GPIO_PIN_7
#define GPIO_SLOT_ADDR4_PERIPH_BASE         SYSCTL_PERIPH_GPIOB
#define GPIO_SLOT_ADDR4_PORT                GPIO_PORTB_BASE
#define GPIO_SLOT_ADDR4_PIN                 GPIO_PIN_4

//
// 板卡类型信号读取管脚                     PD4,PD5,PD6
//
#define GPIO_SLOT_TYPE0_PERIPH_BASE         SYSCTL_PERIPH_GPIOD
#define GPIO_SLOT_TYPE0_PORT                GPIO_PORTD_BASE
#define GPIO_SLOT_TYPE0_PIN                 GPIO_PIN_4
#define GPIO_SLOT_TYPE1_PERIPH_BASE         SYSCTL_PERIPH_GPIOD
#define GPIO_SLOT_TYPE1_PORT                GPIO_PORTD_BASE
#define GPIO_SLOT_TYPE1_PIN                 GPIO_PIN_5
#define GPIO_SLOT_TYPE2_PERIPH_BASE         SYSCTL_PERIPH_GPIOD
#define GPIO_SLOT_TYPE2_PORT                GPIO_PORTD_BASE
#define GPIO_SLOT_TYPE2_PIN                 GPIO_PIN_6

//
// 在位信号读取管脚                         PD4,PD5,PD6,PD7
//
#define GPIO_SLOT_SITOK0_PERIPH_BASE        SYSCTL_PERIPH_GPIOD
#define GPIO_SLOT_SITOK0_PORT               GPIO_PORTD_BASE
#define GPIO_SLOT_SITOK0_PIN                GPIO_PIN_4
#define GPIO_SLOT_SITOK1_PERIPH_BASE        SYSCTL_PERIPH_GPIOD
#define GPIO_SLOT_SITOK1_PORT               GPIO_PORTD_BASE
#define GPIO_SLOT_SITOK1_PIN                GPIO_PIN_5
#define GPIO_SLOT_SITOK2_PERIPH_BASE        SYSCTL_PERIPH_GPIOD
#define GPIO_SLOT_SITOK2_PORT               GPIO_PORTD_BASE
#define GPIO_SLOT_SITOK2_PIN                GPIO_PIN_6
#define GPIO_SLOT_SITOK3_PERIPH_BASE        SYSCTL_PERIPH_GPIOD
#define GPIO_SLOT_SITOK3_PORT               GPIO_PORTD_BASE
#define GPIO_SLOT_SITOK3_PIN                GPIO_PIN_7
#define GPIO_SLOT_SITOK_MASK                0x0F

//
// LED管脚                                  PD1
//
#define GPIO_LED_PERIPH_BASE                SYSCTL_PERIPH_GPIOD
#define GPIO_LED_PORT                       GPIO_PORTD_BASE
#define GPIO_LED_PIN                        GPIO_PIN_1
//                                          PD0
#define GPIO_LED1_PERIPH_BASE               SYSCTL_PERIPH_GPIOD
#define GPIO_LED1_PORT                      GPIO_PORTD_BASE
#define GPIO_LED1_PIN                       GPIO_PIN_0


#endif

