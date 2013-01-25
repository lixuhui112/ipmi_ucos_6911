//*****************************************************************************
//
// lib_io.h - I/O routines for the IPMI application.
//
//*****************************************************************************

#ifndef __LIB_IO_H__
#define __LIB_IO_H__

#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include <inttypes.h>

typedef void (*handler_t)(void *);

typedef struct IO_int_st {
    unsigned long peripheral;
    unsigned long port;
    unsigned char pins;
    unsigned long interrupt;
    handler_t handler;
    void *param;
} IO_INT;

void    IO_init(void);

void    IO_led0_init(void);
void    IO_led1_init(void);
void    IO_led0_set(tBoolean bOn);
void    IO_led1_set(tBoolean bOn);

void    IO_sol_set(tBoolean bOn);
uint8_t IO_sol_get(void);
void    IO_sol_init(void);

void    IO_i2c_hotswap_set(tBoolean bOn);
uint8_t IO_i2c_hotswap_get(void);
void    IO_i2c_hotswap_init(void);

void    IO_present_ok_fab(tBoolean bOn);
uint8_t IO_present_check(void);
void    IO_present_init(void);

uint8_t IO_slot_addr_get(void);
void    IO_slot_addr_init(void);

void    IO_board_type_init(void);
uint8_t IO_board_type_get(void);

void    IO_good_fab_init(void);
void    IO_good_fab_set(tBoolean bOn);

void    IO_watchdog_init(void);
void    IO_watchdog_set(tBoolean bOn);

void    IO_full_speed_init(void);
void    IO_full_speed_set(tBoolean bOn);

void    IO_gpio_int_register(IO_INT *io_int);

#endif // __LIB_IO_H__
