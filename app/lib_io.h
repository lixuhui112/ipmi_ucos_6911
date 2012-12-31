//*****************************************************************************
//
// lib_io.h - I/O routines for the IPMI application.
//
//*****************************************************************************

#ifndef __LIB_IO_H__
#define __LIB_IO_H__

#include "inc/hw_types.h"

typedef void (*handler_t)(void *);

typedef struct IO_int_st {
    unsigned long peripheral;
    unsigned long port;
    unsigned char pins;
    unsigned long interrupt;
    handler_t handler;
    void *param;
} IO_INT;

void IO_init(void);
void IO_cpu_led_set(tBoolean bOn);
void IO_sol_set(tBoolean bOn);
int  IO_sol_get(void);
void IO_i2c_hotswap_set(tBoolean bOn);
int  IO_i2c_hotswap_get(void);
void IO_gpio_int_register(IO_INT *io_int);
int  IO_present_check(void);

#endif // __LIB_IO_H__
