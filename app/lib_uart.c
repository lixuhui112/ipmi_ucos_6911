//*****************************************************************************
//
// lib_uart.c - UART routines for the IPMI application.
//
//*****************************************************************************

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/interrupt.h"
#include "ipmi_lib/ipmi_modules.h"
#include "ipmi_lib/ipmi_intf.h"
#include "third_party/uartstdio.h"
#include "app/lib_common.h"
#include "app/lib_gpio.h"
#include "app/lib_uart.h"
#include "ucos_ii.h"

#if defined(IPMI_MODULES_UART0_DEBUG) || defined(IPMI_MODULES_UART1_ICMB)
//*****************************************************************************
//
// Define the ICMB(UART1) or DEBUG(UART0) TX/RX Buffer
//
//*****************************************************************************
#define UART1_BUF_SIZE  0x80

static unsigned char uart1_rx_buf[UART1_BUF_SIZE];
static unsigned char uart1_tx_buf[UART1_BUF_SIZE];
static unsigned char uart1_rx_idx;
static unsigned char uart1_tx_idx;
static unsigned char uart1_rx_len;
static unsigned char uart1_tx_len;
static unsigned long uart1_rx_timestamp;
#endif

#ifdef IPMI_MODULES_UART0_DEBUG
//*****************************************************************************
//
// The UART0 interrupt handler.
//
//*****************************************************************************
void UART_uart0_int_handler(void)
{
    static unsigned char uart0_frame = 0;
    static unsigned char uart0_size = 0;
    unsigned long ulStatus;
    unsigned char data;

    // Get the interrrupt status.
    ulStatus = UARTIntStatus(UART0_BASE, true);

    // Clear the asserted interrupts.
    UARTIntClear(UART0_BASE, ulStatus);

    // if recvie frame time out, reset the frame
    if (OSTimeGet() - uart1_rx_timestamp > SYSTICKHZ * 2)
    {
        uart0_frame = 0;
    }

    // save the recvie frame time stamp
    uart1_rx_timestamp = OSTimeGet();

    // Loop while there are characters in the receive FIFO.
    while (UARTCharsAvail(UART0_BASE))
    {
        // Read the character from the UART.
        data = (unsigned char)UARTCharGetNonBlocking(UART0_BASE);

        // 检测前导码
        if (uart0_frame < sizeof (IPMI_FRAME_CHAR))
        {
            if (IPMI_FRAME_CHAR[uart0_frame] == data)
            {
                uart0_frame++;
            }
            else
            {
                uart0_frame = 0;
            }
            uart0_size = 0;
        }

        // 正常数据
        else
        {
            if (uart0_size == 0)
            {
                uart0_size = data;
                uart1_rx_idx = 0;
                uart1_rx_len = 0;

                if (uart0_size > UART1_BUF_SIZE)
                {
                    uart0_frame = 0;
                    break;
                }
            }


            uart1_rx_buf[uart1_rx_idx++] = data;

            if (uart1_rx_idx == uart0_size)
            {
                uart0_frame = 0;
                uart1_rx_len = uart1_rx_idx;
                ipmi_intf_recv_post(IPMI_INTF_DEBUG);
                break;
            }
        }
    }
}
//*****************************************************************************
//
// Init UART0 hardware as DEBUG
//
//*****************************************************************************
void UART_uart0_init(void)
{
    // Enable GPIO port which is used for UART0 pins.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Configure the pin muxing for UART0 functions
    // on port A0/A1.
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);

    // Select the alternate (UART) function for these pins.
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Configure the UART0 for 115,200, 8-N-1 operation.
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_NONE));

    // Initialize the UART for console I/O. (DEBUG)
    //UARTStdioInit(0);

    // Enable the UART interrupt.
    UARTIntRegister(UART0_BASE, UART_uart0_int_handler);
    IntEnable(INT_UART0);
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
}

int UART_uart0_read(char *buf, unsigned long *size)
{
    unsigned long i;

    DEBUG("debug read:");
    for (i = 0; i < uart1_rx_len; i++)
    {
        DEBUG("%02x ", uart1_rx_buf[i]);
        buf[i] = uart1_rx_buf[i];
    }
    DEBUG("(%d)\r\n", uart1_rx_len);

    *size = uart1_rx_len;

    return 0;
}

int UART_uart0_write(char *buf, unsigned long size)
{
    unsigned long i, j;

    // 先写前导码
    for (i = 0; i < sizeof(IPMI_FRAME_CHAR); i++)
    {
        uart1_tx_buf[i] = IPMI_FRAME_CHAR[i];
    }

    // 写数据
    for (j = 0; j < size && i < UART1_BUF_SIZE; j++, i++)
    {
        uart1_tx_buf[i] = buf[j];
    }

    uart1_tx_len = i;

    DEBUG("debug write(%d):", uart1_tx_len);
    for (uart1_tx_idx = 0; uart1_tx_idx < uart1_tx_len; uart1_tx_idx++)
    {
        DEBUG("%02x ", uart1_tx_buf[uart1_tx_idx]);
        UARTCharPut(UART0_BASE, uart1_tx_buf[uart1_tx_idx]);
    }
    DEBUG("\r\n");

    uart1_tx_len = 0;
    uart1_tx_idx = 0;

    return 0;
}

#endif



#ifdef IPMI_MODULES_UART1_ICMB
//*****************************************************************************
//
// The UART1 interrupt handler.
//
//*****************************************************************************
void UART_uart1_int_handler(void)
{
    static unsigned char uart1_frame = 0;
    static unsigned char uart1_size = 0;
    unsigned long ulStatus;
    unsigned char data;

    // Get the interrrupt status.
    ulStatus = UARTIntStatus(UART1_BASE, true);

    // Clear the asserted interrupts.
    UARTIntClear(UART1_BASE, ulStatus);

    // if recvie frame time out, reset the frame
    if (OSTimeGet() - uart1_rx_timestamp > SYSTICKHZ * 2)
    {
        uart1_frame = 0;
    }

    // save the recvie frame time stamp
    uart1_rx_timestamp = OSTimeGet();

    // Loop while there are characters in the receive FIFO.
    while (UARTCharsAvail(UART1_BASE))
    {
        // Read the character from the UART.
        data = (unsigned char)UARTCharGetNonBlocking(UART1_BASE);

        // 检测前导码
        if (uart1_frame < sizeof (IPMI_FRAME_CHAR))
        {
            if (IPMI_FRAME_CHAR[uart1_frame] == data)
            {
                uart1_frame++;
            }
            else
            {
                uart1_frame = 0;
            }
            uart1_size = 0;
        }

        // 正常数据
        else
        {
            if (uart1_size == 0)
            {
                uart1_size = data;
                uart1_rx_idx = 0;
                uart1_rx_len = 0;
            }

            uart1_rx_buf[uart1_rx_idx++] = data;

            //UARTCharPutNonBlocking(UART1_BASE, data); /*test*/

            if (uart1_rx_idx == uart1_size)
            {
                uart1_frame = 0;
                uart1_rx_len = uart1_rx_idx;

                //UARTCharPutNonBlocking(UART1_BASE, uart1_rx_len); /*test*/

                ipmi_intf_recv_post(IPMI_INTF_ICMB);
                break;
            }
        }
    }
}

int UART_uart1_read(char *buf, unsigned long *size)
{
    unsigned long i;

    for (i = 0; i < uart1_rx_len; i++)
    {
        buf[i] = uart1_rx_buf[i];
    }

    *size = uart1_rx_len;

    return 0;
}

int UART_uart1_write(char *buf, unsigned long size)
{
    unsigned long i, j;

    // 先写前导码
    for (i = 0; i < sizeof(IPMI_FRAME_CHAR); i++)
    {
        uart1_tx_buf[i] = IPMI_FRAME_CHAR[i];
    }

    // 写数据
    for (j = 0; j < size && i < UART1_BUF_SIZE; j++, i++)
    {
        uart1_tx_buf[i] = buf[j];
    }

    uart1_tx_len = i;

    DEBUG("uart1 write(%d):", uart1_tx_len);
    for (uart1_tx_idx = 0; uart1_tx_idx < uart1_tx_len; uart1_tx_idx++)
    {
        DEBUG("%02x ", uart1_tx_buf[uart1_tx_idx]);
        UARTCharPut(UART1_BASE, uart1_tx_buf[uart1_tx_idx]);
    }
    DEBUG("\r\n");

    uart1_tx_len = 0;
    uart1_tx_idx = 0;

    return 0;
}

//*****************************************************************************
//
// Init UART1 hardware as ICMB
//
//*****************************************************************************
void UART_uart1_init(void)
{
    // Enable GPIO port which is used for UART1 pins.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

    // Configure the pin muxing for UART1 functions
    // on port D2/D3.
    GPIOPinConfigure(GPIO_PD2_U1RX);
    GPIOPinConfigure(GPIO_PD3_U1TX);

    // Select the alternate (UART) function for these pins.
    GPIOPinTypeUART(GPIO_PORTD_BASE, GPIO_PIN_2 | GPIO_PIN_3);

    // Configure the UART1 for 9,600, 8-N-1 operation.
    UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 115200,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_NONE));
    UARTStdioInit(1);

    // Enable the UART interrupt.
    //UARTIntRegister(UART1_BASE, UART_uart1_int_handler);
    IntEnable(INT_UART1);
    UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT);
}
#endif



#ifdef IPMI_MODULES_UART2_SOL
#define UART2_BUF_SIZE  0x80
static unsigned char uart2_rx_buf[UART2_BUF_SIZE];
static unsigned char uart2_tx_buf[UART2_BUF_SIZE];
static unsigned char uart2_rx_idx;
static unsigned char uart2_tx_idx;
static unsigned char uart2_rx_len;
static unsigned char uart2_tx_len;
//*****************************************************************************
//
// The UART2 interrupt handler.
//
//*****************************************************************************
void UART_uart2_int_handler(void)
{
    unsigned long ulStatus;
    long ch;

    // Get the interrrupt status.
    ulStatus = UARTIntStatus(UART2_BASE, true);

    // Clear the asserted interrupts.
    UARTIntClear(UART2_BASE, ulStatus);

    // Loop while there are characters in the receive FIFO.
    while (UARTCharsAvail(UART2_BASE))
    {
        // Read the next character from the UART2 and write it back to the UART1.
        ch = UARTCharGetNonBlocking(UART2_BASE);
        uart2_rx_buf[uart2_rx_idx++] = (unsigned char)ch;
        uart2_rx_idx = uart2_rx_idx % UART2_BUF_SIZE;
        // TODO
        // read UART2 uart2_rx_buf to SOL(Ethernet)
        // and uart2_tx_buf from SOL to UART2

        // just for loop test [[ CPU -> UART2 -> UART1 ]]
        while (UARTCharPutNonBlocking(UART1_BASE, ch) == false);
    }
}

//*****************************************************************************
//
// Init UART1 hardware as SOL UART Input Channel
//
//*****************************************************************************
void UART_uart2_init(void)
{
    // Enable GPIO port which is used for UART2 pins.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART2);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);

    // Configure the pin muxing for UART2 functions
    // on port G0/G1.
    GPIOPinConfigure(GPIO_PG0_U2RX);
    GPIOPinConfigure(GPIO_PG1_U2TX);

    // Select the alternate (UART) function for these pins.
    GPIOPinTypeUART(GPIO_PORTG_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Configure the UART for 9,600, 8-N-1 operation.
    UARTConfigSetExpClk(UART2_BASE, SysCtlClockGet(), 9600,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_NONE));

    // Enable the UART interrupt.
    UARTIntRegister(UART2_BASE, UART_uart2_int_handler);
    IntEnable(INT_UART2);
    UARTIntEnable(UART2_BASE, UART_INT_RX | UART_INT_RT);
}

int UART_uart2_read(char *buf, unsigned long *size)
{
    return 0;
}

int UART_uart2_write(char *buf, unsigned long size)
{
    return 0;
}
#endif


#define DEBUG_UART_PORT UART1_BASE
//*****************************************************************************
//
// Send a string to the UART.
//
//*****************************************************************************
void DEBUG_UARTSend(const unsigned char *pucBuffer)
{
    //
    // Loop while there are more characters to send.
    //
    while (*pucBuffer)
    {
        //
        // Write the next character to the UART.
        //
        UARTCharPutNonBlocking(DEBUG_UART_PORT, *pucBuffer++);
    }
}


//*****************************************************************************
//
// Init UART0/UART1/UART2 hardware for DEBUG/ICMB/SOL UART
//
//*****************************************************************************
void UART_init(void)
{
#ifdef IPMI_MODULES_UART0_DEBUG
    UART_uart0_init();
#endif

#ifdef IPMI_MODULES_UART1_ICMB
    UART_uart1_init();
#endif

#ifdef IPMI_MODULES_UART2_SOL
    UART_uart2_init();
#endif
}


