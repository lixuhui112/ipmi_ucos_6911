//*****************************************************************************
//
// lib_uart.h - UART routines for the IPMI application.
//
//*****************************************************************************

#ifndef __LIB_UART_H__
#define __LIB_UART_H__

#ifdef __cplusplus
extern "C"
{
#endif

void UART_init(void);

int UART_uart0_read(char *buf, unsigned long *size);
int UART_uart0_write(char *buf, unsigned long size);

int UART_uart1_read(char *buf, unsigned long *size);
int UART_uart1_write(char *buf, unsigned long size);

int UART_uart2_read(char *buf, unsigned long *size);
int UART_uart2_write(char *buf, unsigned long size);

void DEBUG_UARTSend(const unsigned char *pucBuffer);

#ifdef __cplusplus
}
#endif

#endif  // _LIB_UART_H_

