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
#include "ipmi_lib/ipmi_modules.h"

//
// FPGA告警输出管脚
//
#if defined(IPMI_MODULES_GPIO_FPGA_INT_ALARM)                       // PE6,PE7
#define GPIO_FPGA_PERIPH_BASE               SYSCTL_PERIPH_GPIOE
#define GPIO_FPGA_INT_PORT                  GPIO_PORTE_BASE
#define GPIO_FPGA_INT_PIN                   GPIO_PIN_7
#define GPIO_FPGA_ALARM_PORT                GPIO_PORTE_BASE
#define GPIO_FPGA_ALARM_PIN                 GPIO_PIN_6
#endif
//*****************************************************************************


//
// CPU告警输出管脚
//
#if defined(IPMI_MODULES_GPIO_CPU_INT_ALARM)                        // PD0,PD1
#define GPIO_CPU_PERIPH_BASE                SYSCTL_PERIPH_GPIOD
#define GPIO_CPU_INT_PORT                   GPIO_PORTD_BASE
#define GPIO_CPU_INT_PIN                    GPIO_PIN_0
#define GPIO_CPU_ALARM_PORT                 GPIO_PORTD_BASE
#define GPIO_CPU_ALARM_PIN                  GPIO_PIN_1
#endif
//*****************************************************************************


//
// I2C热插拔芯片使能管脚
//
#if defined(IPMI_MODULES_GPIO_I2C_HOTSWAP_SEL)
#if (defined(BOARD_6911_SWITCH) || defined(BOARD_6911_FABRIC))
#define GPIO_I2C_HOTSWAP_PERIPH_BASE        SYSCTL_PERIPH_GPIOE     // PE5
#define GPIO_I2C_HOTSWAP_SEL_PORT           GPIO_PORTE_BASE
#define GPIO_I2C_HOTSWAP_SEL_PIN            GPIO_PIN_5
#endif
#if defined(BOARD_6911_FAN)
#define GPIO_I2C_HOTSWAP_PERIPH_BASE        SYSCTL_PERIPH_GPIOE     // PE4
#define GPIO_I2C_HOTSWAP_SEL_PORT           GPIO_PORTE_BASE
#define GPIO_I2C_HOTSWAP_SEL_PIN            GPIO_PIN_4
#endif
#endif  // IPMI_MODULES_GPIO_I2C_HOTSWAP_SEL
//*****************************************************************************


//
// SOL功能CPU串口重定向使能管脚
//
#if defined(IPMI_MODULES_GPIO_SOL_SEL)                              // PE4
#define GPIO_SOL_UART2_PERIPH_BASE          SYSCTL_PERIPH_GPIOE
#define GPIO_SOL_UART2_SEL_PORT             GPIO_PORTE_BASE
#define GPIO_SOL_UART2_SEL_PIN              GPIO_PIN_4
#endif  // IPMI_MODULES_GPIO_SOL_SEL
//*****************************************************************************


//
// 电压传感器UCD9081监控状态输入管脚
//
#if defined(IPMI_CHIP_UCD9081)                                      // PC6,PC7
#define GPIO_CHIP_UCD9081_GPIO1_PERIPH      SYSCTL_PERIPH_GPIOC
#define GPIO_CHIP_UCD9081_GPIO1_PORT        GPIO_PORTC_BASE
#define GPIO_CHIP_UCD9081_GPIO1_PIN         GPIO_PIN_6
#define GPIO_CHIP_UCD9081_GPIO1_INT         INT_GPIOC
#define GPIO_CHIP_UCD9081_GPIO2_PERIPH      SYSCTL_PERIPH_GPIOC
#define GPIO_CHIP_UCD9081_GPIO2_PORT        GPIO_PORTC_BASE
#define GPIO_CHIP_UCD9081_GPIO2_PIN         GPIO_PIN_7
#define GPIO_CHIP_UCD9081_GPIO2_INT         INT_GPIOC
#endif
//*****************************************************************************


//
// 温度传感器状态输入管脚
//
#if (defined(IPMI_CHIP_MAX6635))
#if (defined(BOARD_6911_SWITCH) || defined(BOARD_6911_FABRIC))      // PC4,PC5,PB0,PB1
#define GPIO_CHIP_MAX6635_0_PERIPH_BASE     SYSCTL_PERIPH_GPIOC
#define GPIO_CHIP_MAX6635_0_OVER_PORT       GPIO_PORTC_BASE
#define GPIO_CHIP_MAX6635_0_OVER_PIN        GPIO_PIN_4
#define GPIO_CHIP_MAX6635_0_OVER_INT        INT_GPIOC
#define GPIO_CHIP_MAX6635_0_ALERT_PORT      GPIO_PORTC_BASE
#define GPIO_CHIP_MAX6635_0_ALERT_PIN       GPIO_PIN_5
#define GPIO_CHIP_MAX6635_0_ALERT_INT       INT_GPIOC
#define GPIO_CHIP_MAX6635_1_PERIPH_BASE     SYSCTL_PERIPH_GPIOB
#define GPIO_CHIP_MAX6635_1_OVER_PORT       GPIO_PORTB_BASE
#define GPIO_CHIP_MAX6635_1_OVER_PIN        GPIO_PIN_0
#define GPIO_CHIP_MAX6635_1_OVER_INT        INT_GPIOB
#define GPIO_CHIP_MAX6635_1_ALERT_PORT      GPIO_PORTB_BASE
#define GPIO_CHIP_MAX6635_1_ALERT_PIN       GPIO_PIN_1
#define GPIO_CHIP_MAX6635_1_ALERT_INT       INT_GPIOB
#endif
#if (defined(BOARD_6911_POWER))                                     // PC6,PC7,PB5,PB6
#define GPIO_CHIP_MAX6635_0_PERIPH_BASE     SYSCTL_PERIPH_GPIOC
#define GPIO_CHIP_MAX6635_0_OVER_PORT       GPIO_PORTC_BASE
#define GPIO_CHIP_MAX6635_0_OVER_PIN        GPIO_PIN_6
#define GPIO_CHIP_MAX6635_0_OVER_INT        INT_GPIOC
#define GPIO_CHIP_MAX6635_0_ALERT_PORT      GPIO_PORTC_BASE
#define GPIO_CHIP_MAX6635_0_ALERT_PIN       GPIO_PIN_7
#define GPIO_CHIP_MAX6635_0_ALERT_INT       INT_GPIOC
#define GPIO_CHIP_MAX6635_1_PERIPH_BASE     SYSCTL_PERIPH_GPIOB
#define GPIO_CHIP_MAX6635_1_OVER_PORT       GPIO_PORTB_BASE
#define GPIO_CHIP_MAX6635_1_OVER_PIN        GPIO_PIN_5
#define GPIO_CHIP_MAX6635_1_OVER_INT        INT_GPIOB
#define GPIO_CHIP_MAX6635_1_ALERT_PORT      GPIO_PORTB_BASE
#define GPIO_CHIP_MAX6635_1_ALERT_PIN       GPIO_PIN_6
#define GPIO_CHIP_MAX6635_1_ALERT_INT       INT_GPIOB
#endif
#endif
//*****************************************************************************


//
// 板卡槽位信号读取管脚
//
#if (defined(IPMI_MODULES_GPIO_SLOT_ADDR))
#if (defined(BOARD_6911_SWITCH) || defined(BOARD_6911_FABRIC))      // PD4,PD5,PD6,PD7,PB4
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
#endif
#if (defined(BOARD_6911_FAN))                                       // PD0,PD1
#define GPIO_SLOT_ADDR0_PERIPH_BASE         SYSCTL_PERIPH_GPIOD
#define GPIO_SLOT_ADDR0_PORT                GPIO_PORTD_BASE
#define GPIO_SLOT_ADDR0_PIN                 GPIO_PIN_0
#define GPIO_SLOT_ADDR1_PERIPH_BASE         SYSCTL_PERIPH_GPIOD
#define GPIO_SLOT_ADDR1_PORT                GPIO_PORTD_BASE
#define GPIO_SLOT_ADDR1_PIN                 GPIO_PIN_1
#endif
#if (defined(BOARD_6911_POWER))                                     // PA2,PA3,PA4
#define GPIO_SLOT_ADDR0_PERIPH_BASE         SYSCTL_PERIPH_GPIOA
#define GPIO_SLOT_ADDR0_PORT                GPIO_PORTA_BASE
#define GPIO_SLOT_ADDR0_PIN                 GPIO_PIN_2
#define GPIO_SLOT_ADDR1_PERIPH_BASE         SYSCTL_PERIPH_GPIOA
#define GPIO_SLOT_ADDR1_PORT                GPIO_PORTA_BASE
#define GPIO_SLOT_ADDR1_PIN                 GPIO_PIN_3
#define GPIO_SLOT_ADDR2_PERIPH_BASE         SYSCTL_PERIPH_GPIOA
#define GPIO_SLOT_ADDR2_PORT                GPIO_PORTA_BASE
#define GPIO_SLOT_ADDR2_PIN                 GPIO_PIN_4
#endif
#endif  //IPMI_MODULES_GPIO_SLOT_ADDR
//*****************************************************************************


//
// 板卡类型信号读取管脚
//
#if (defined(IPMI_MODULES_GPIO_BOARD_TYPE))
#if (defined(BOARD_6911_FAN))                                       // PB4,PB5,PB6
#define GPIO_BOARD_TYPE0_PERIPH_BASE        SYSCTL_PERIPH_GPIOB
#define GPIO_BOARD_TYPE0_PORT               GPIO_PORTB_BASE
#define GPIO_BOARD_TYPE0_PIN                GPIO_PIN_4
#define GPIO_BOARD_TYPE1_PERIPH_BASE        SYSCTL_PERIPH_GPIOB
#define GPIO_BOARD_TYPE1_PORT               GPIO_PORTB_BASE
#define GPIO_BOARD_TYPE1_PIN                GPIO_PIN_5
#define GPIO_BOARD_TYPE2_PERIPH_BASE        SYSCTL_PERIPH_GPIOB
#define GPIO_BOARD_TYPE2_PORT               GPIO_PORTB_BASE
#define GPIO_BOARD_TYPE2_PIN                GPIO_PIN_6
#endif
#if (defined(BOARD_6911_POWER))                                     // PB0,PB1
#define GPIO_BOARD_TYPE0_PERIPH_BASE        SYSCTL_PERIPH_GPIOB
#define GPIO_BOARD_TYPE0_PORT               GPIO_PORTB_BASE
#define GPIO_BOARD_TYPE0_PIN                GPIO_PIN_0
#define GPIO_BOARD_TYPE1_PERIPH_BASE        SYSCTL_PERIPH_GPIOB
#define GPIO_BOARD_TYPE1_PORT               GPIO_PORTB_BASE
#define GPIO_BOARD_TYPE1_PIN                GPIO_PIN_1
#endif
#endif  //IPMI_MODULES_GPIO_BOARD_TYPE
//*****************************************************************************


//
// 在位信号读取管脚
//
#if (defined(IPMI_MODULES_GPIO_PRESENT))
#if (defined(BOARD_6911_FAN))                                       // PC4
#define GPIO_SLOT_SITCHK0_PERIPH_BASE       SYSCTL_PERIPH_GPIOC
#define GPIO_SLOT_SITCHK0_PORT              GPIO_PORTC_BASE
#define GPIO_SLOT_SITCHK0_PIN               GPIO_PIN_4
#define GPIO_SLOT_SITCHK_MASK               0x01
#endif
#if (defined(BOARD_6911_POWER))                                     // PE0,PE1
#define GPIO_SLOT_SITCHK0_PERIPH_BASE       SYSCTL_PERIPH_GPIOE
#define GPIO_SLOT_SITCHK0_PORT              GPIO_PORTE_BASE
#define GPIO_SLOT_SITCHK0_PIN               GPIO_PIN_0
#define GPIO_SLOT_SITCHK1_PERIPH_BASE       SYSCTL_PERIPH_GPIOE
#define GPIO_SLOT_SITCHK1_PORT              GPIO_PORTE_BASE
#define GPIO_SLOT_SITCHK1_PIN               GPIO_PIN_1
#define GPIO_SLOT_SITCHK_MASK               0x02
#endif
//
// 在位信号通知管脚
//
#if (defined(BOARD_6911_FAN))                                       // PB0
#define GPIO_SLOT_SITOK0_PERIPH_BASE        SYSCTL_PERIPH_GPIOB
#define GPIO_SLOT_SITOK0_PORT               GPIO_PORTB_BASE
#define GPIO_SLOT_SITOK0_PIN                GPIO_PIN_0
#endif
#if (defined(BOARD_6911_POWER))                                     // PE6,PE7
#define GPIO_SLOT_SITOK0_PERIPH_BASE        SYSCTL_PERIPH_GPIOE
#define GPIO_SLOT_SITOK0_PORT               GPIO_PORTE_BASE
#define GPIO_SLOT_SITOK0_PIN                GPIO_PIN_6
#define GPIO_SLOT_SITOK1_PERIPH_BASE        SYSCTL_PERIPH_GPIOE
#define GPIO_SLOT_SITOK1_PORT               GPIO_PORTE_BASE
#define GPIO_SLOT_SITOK1_PIN                GPIO_PIN_7
#endif
#endif  //IPMI_MODULES_GPIO_PRESENT
//*****************************************************************************


//
// 主控信号通知管脚
//
#if (defined(IPMI_MODULES_GPIO_GOOD_FAB))
#if (defined(BOARD_6911_POWER))                                     // PE4,PE5
#define GPIO_GOOD_FAB0_PERIPH_BASE          SYSCTL_PERIPH_GPIOE
#define GPIO_GOOD_FAB0_PORT                 GPIO_PORTE_BASE
#define GPIO_GOOD_FAB0_PIN                  GPIO_PIN_4
#define GPIO_GOOD_FAB1_PERIPH_BASE          SYSCTL_PERIPH_GPIOE
#define GPIO_GOOD_FAB1_PORT                 GPIO_PORTE_BASE
#define GPIO_GOOD_FAB1_PIN                  GPIO_PIN_5
#endif
#endif  // IPMI_MODULES_GPIO_GOOD_FAB
//*****************************************************************************


//
// 看门狗喂狗管脚                           PF1
//
#if (defined(IPMI_MODULES_GPIO_WATCHDOG))
#if (defined(BOARD_6911_SWITCH) || defined(BOARD_6911_FABRIC))      // PF1
#define GPIO_WATCHDOG_PERIPH_BASE           SYSCTL_PERIPH_GPIOF
#define GPIO_WATCHDOG_PORT                  GPIO_PORTF_BASE
#define GPIO_WATCHDOG_PIN                   GPIO_PIN_1
#endif
#if (defined(BOARD_6911_FAN) || defined(BOARD_6911_POWER))          // PA5
#define GPIO_WATCHDOG_PERIPH_BASE           SYSCTL_PERIPH_GPIOA
#define GPIO_WATCHDOG_PORT                  GPIO_PORTA_BASE
#define GPIO_WATCHDOG_PIN                   GPIO_PIN_5
#endif
#endif  // IPMI_MODULES_GPIO_WATCHDOG
//*****************************************************************************


//
// LED管脚                                  PD1
//
#if (defined(IPMI_MODULES_GPIO_CPU_LED))
#if (defined(BOARD_6911_SWITCH) || defined(BOARD_6911_FABRIC))      // PD1,PF3
#define GPIO_LED0_PERIPH_BASE               SYSCTL_PERIPH_GPIOD
#define GPIO_LED0_PORT                      GPIO_PORTD_BASE
#define GPIO_LED0_PIN                       GPIO_PIN_1
#define GPIO_LED1_PERIPH_BASE               SYSCTL_PERIPH_GPIOF
#define GPIO_LED1_PORT                      GPIO_PORTF_BASE
#define GPIO_LED1_PIN                       GPIO_PIN_3
#endif
#if (defined(BOARD_6911_FAN) || defined(BOARD_6911_POWER))          // PF0,PF1
#define GPIO_LED0_PERIPH_BASE               SYSCTL_PERIPH_GPIOF
#define GPIO_LED0_PORT                      GPIO_PORTF_BASE
#define GPIO_LED0_PIN                       GPIO_PIN_0
#define GPIO_LED1_PERIPH_BASE               SYSCTL_PERIPH_GPIOF
#define GPIO_LED1_PORT                      GPIO_PORTF_BASE
#define GPIO_LED1_PIN                       GPIO_PIN_1
#endif
#endif  // IPMI_MODULES_GPIO_CPU_LED
//*****************************************************************************


//
// 风扇FULLSPEED管脚
//
#if (defined(IPMI_MODULES_GPIO_FULL_SPEED))
#if (defined(BOARD_6911_FAN))                                       // PE0,PE1
#define GPIO_FULL_SPEED0_PERIPH_BASE        SYSCTL_PERIPH_GPIOE
#define GPIO_FULL_SPEED0_PORT               GPIO_PORTE_BASE
#define GPIO_FULL_SPEED0_PIN                GPIO_PIN_0
#define GPIO_FULL_SPEED1_PERIPH_BASE        SYSCTL_PERIPH_GPIOE
#define GPIO_FULL_SPEED1_PORT               GPIO_PORTE_BASE
#define GPIO_FULL_SPEED1_PIN                GPIO_PIN_1
#endif
#endif  // IPMI_MODULES_GPIO_FULL_SPEED
//*****************************************************************************

#endif

