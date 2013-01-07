//*****************************************************************************
//
// lib_spi.c - SPI routines for the IPMI application.
//
//*****************************************************************************

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/ssi.h"
#include "driverlib/interrupt.h"
#include "ipmi_lib/ipmi_modules.h"
#include "ipmi_lib/ipmi_intf.h"
#include "app/lib_gpio.h"
#include "app/lib_spi.h"
#include "app/lib_io.h"
#include "app/lib_common.h"
#include "ucos_ii.h"
#include "third_party/ustdlib.h"

#ifdef IPMI_MODULES_SPI1_SSIF
#define SPI_BUF_SIZE            0x80
static unsigned char spi_rx_buf[SPI_BUF_SIZE];
static unsigned char spi_tx_buf[SPI_BUF_SIZE];
static unsigned char spi_rx_idx;
static unsigned char spi_tx_idx;
static unsigned char spi_rx_len;
static unsigned char spi_tx_len;
static unsigned long spi_rx_timestamp;

//*****************************************************************************
//
// The spi1 slaver interrupt handler.
//
// CPU WRITE:   [RX]0f f0 5a 01 20 01 02 03 04 ...
// CPU READ:    [RX]0f f0 5a 02 [TX] 20 01 02 03 04 ...
// CPU STATE:   [RX]0f f0 5a 03 [TX] 20
//
//*****************************************************************************
void SPI_spi1_int_handler(void)
{
    unsigned char i;
    unsigned long status;
    unsigned long data;
    static unsigned char spi1_frame = 0;
    static unsigned char spi1_op = 0;
    static unsigned char spi1_size = 0;
    //static unsigned char onoff = 0;
    char cntbuf[16];

    status = SSIIntStatus(SSI1_BASE, true);        // 获取中断状态
    if (!status)
    {
        return;
    }

    SSIIntClear(SSI1_BASE, status);

    // if recvie frame time out, reset the frame
    if (OSTimeGet() - spi_rx_timestamp > SYSTICKHZ * 2)
    {
        spi1_frame = 0;
    }

    // for test interrupt use led
    //onoff = ~onoff;
    //IO_led0_set(onoff);

    // save the recvie frame time stamp
    spi_rx_timestamp = OSTimeGet();

    // for debug interrupt status
    //usprintf(cntbuf, "[%02x] ", status);
    //DEBUG_UARTSend(cntbuf);

    switch (status)
    {
        case SSI_RXFF:
        case SSI_RXOR:
        case SSI_RXTO:
            while (1)
            {
                if (SSIDataGetNonBlocking(SSI1_BASE, &data))
                {
                    // for debug receive buffer
                    //usprintf(cntbuf, "%02x ", data);
                    //DEBUG_UARTSend(cntbuf);

                    // 检测前导码
                    if (spi1_frame < IPMI_FRAME_CHAR_SIZE)
                    {
                        if (IPMI_FRAME_CHAR[spi1_frame] == (unsigned char)data)
                        {
                            spi1_frame++;
                        }
                        else
                        {
                            spi1_frame = 0;
                        }
                        spi1_size = 0;
                    }

                    // SPI命令
                    else if (spi1_frame == IPMI_FRAME_CHAR_SIZE)
                    {
                        //DEBUG_UARTSend("#");

                        if (IPMI_FRAME_CMD_READ == (unsigned char)data)
                        {
                            spi1_op = IPMI_FRAME_CMD_READ;
                            SSIIntDisable(SSI1_BASE, SSI_RXOR|SSI_RXFF|SSI_RXTO);
                            SSIIntEnable(SSI1_BASE, SSI_TXFF);
                        }
                        else if (IPMI_FRAME_CMD_WRITE == (unsigned char)data)
                        {
                            spi1_op = IPMI_FRAME_CMD_WRITE;
                            spi1_frame++;
                        }
                        else if (IPMI_FRAME_CMD_STATE == (unsigned char)data)
                        {
                            spi1_op = IPMI_FRAME_CMD_STATE;
                            SSIIntDisable(SSI1_BASE, SSI_RXOR|SSI_RXFF|SSI_RXTO);
                            SSIIntEnable(SSI1_BASE, SSI_TXFF);
                        }
                        else
                        {
                            spi1_frame = 0;
                            spi1_op = 0;
                        }
                        spi1_size = 0;
                    }

                    // 正常数据
                    // 命令: 写入数据
                    else if (spi1_op == IPMI_FRAME_CMD_WRITE)
                    {
                        //DEBUG_UARTSend("$");

                        if (spi1_size == 0)
                        {
                            spi1_size = (unsigned char)data;
                            spi_rx_idx = 0;
                            spi_rx_len = 0;

                            if (spi1_size > SPI_BUF_SIZE)
                            {
                                spi1_frame = 0;
                                break;
                            }
                        }

                        spi_rx_buf[spi_rx_idx++] = (unsigned char)data;

                        if (spi_rx_idx == spi1_size)
                        {
                            //DEBUG_UARTSend("%");

                            spi_rx_len = spi_rx_idx;
                            ipmi_intf_recv_post(IPMI_INTF_SSIF);

                            spi1_op = 0;
                            spi1_frame = 0;
                        }
                    }
                    break;
                }
                else
                {
                    break;
                }
            }
            break;

        case SSI_TXFF:
            //DEBUG_UARTSend("^");

            // 命令: 获取状态
            if (spi1_op == IPMI_FRAME_CMD_STATE)
            {
                SSIDataPutNonBlocking(SSI1_BASE, spi_tx_len);

                spi1_op = 0;
                spi1_frame = 0;
                SSIIntDisable(SSI1_BASE, SSI_TXFF);
                SSIIntEnable(SSI1_BASE, SSI_RXFF|SSI_RXTO|SSI_RXOR);
            }

            // 命令: 读取数据
            else if (spi1_op == IPMI_FRAME_CMD_READ)
            {
                for (i = 0; i < 4; i++)
                {
                    if (spi_tx_idx < spi_tx_len)    // 正在发送数据
                    {
                        data = (unsigned long)spi_tx_buf[spi_tx_idx];
                        SSIDataPutNonBlocking(SSI1_BASE, data);
                        spi_tx_idx++;
                    }
                }

                if (spi_tx_idx == spi_tx_len)        // 发送完成
                {
                    spi_tx_idx = 0;
                    spi_tx_len = 0;

                    spi1_op = 0;
                    spi1_frame = 0;
                    SSIDataPutNonBlocking(SSI1_BASE, 0);
                    SSIIntDisable(SSI1_BASE, SSI_TXFF);
                    SSIIntEnable(SSI1_BASE, SSI_RXFF|SSI_RXTO|SSI_RXOR);
                }
            }

            else                                // 错误的发送状态
            {
                spi1_op = 0;
                spi1_frame = 0;
                SSIDataPutNonBlocking(SSI1_BASE, 0);
                SSIIntDisable(SSI1_BASE, SSI_TXFF);
                SSIIntEnable(SSI1_BASE, SSI_RXFF|SSI_RXTO|SSI_RXOR);
            }
            break;

        default:
            break;
    }
}

//*****************************************************************************
//
// Init SPI1 hardware to CPU(SLAVER) as SSIF
//
//*****************************************************************************
void SPI_spi1_slaver_init(void)
{
    unsigned long tmpbuf;

    // The SSI0 peripheral must be enabled for use.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI1);

    // For this example SSI1 is used with PortE[3:0]. GPIO port E needs to be
    // enabled so these pins can be used.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    // Configure the pin muxing for SSI1 functions on port E0, E1, E2, and E3.
    // This step is not necessary if your part does not support pin muxing.
    GPIOPinConfigure(GPIO_PE0_SSI1CLK);
    GPIOPinConfigure(GPIO_PE1_SSI1FSS);
    GPIOPinConfigure(GPIO_PE2_SSI1RX);
    GPIOPinConfigure(GPIO_PE3_SSI1TX);

    // Configure the GPIO settings for the SSI pins.
    // The pins are assigned as follows:
    //      PE3 - SSI1Tx
    //      PE2 - SSI1Rx
    //      PE1 - SSI1Fss
    //      PE0 - SSI1CLK
    GPIOPinTypeSSI(GPIO_PORTE_BASE, GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1 |
                   GPIO_PIN_0);

    // Configure and enable the SSI port for SPI master mode.  Use SSI1,
    // system clock supply, idle clock level low and active low clock in
    // freescale SPI mode, master mode, 1MHz SSI frequency, and 8-bit data.
    SSIConfigSetExpClk(SSI1_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_2,
                       SSI_MODE_SLAVE, 1000000, 8);

    // Enable the SSI1 module.
    SSIEnable(SSI1_BASE);

    // Emputy the SSI rx FIFO
    while (SSIDataGetNonBlocking(SSI1_BASE, &tmpbuf)) {}

    // Register SPI Int Handler
    //SSIIntRegister(SSI1_BASE, SPI_spi1_int_handler);

    // Enable the SSI interrupt.
    IntEnable(INT_SSI1);
    SSIIntEnable(SSI1_BASE, /*SSI_TXFF|*/SSI_RXFF|SSI_RXTO|SSI_RXOR);
}

int SPI_spi1_read(char *buf, unsigned long *size)
{
    unsigned long i;

    for (i = 0; i < spi_rx_len; i++)
    {
        buf[i] = spi_rx_buf[i];
    }

    *size = spi_rx_len;

    return 0;
}

int SPI_spi1_write(char *buf, unsigned long size)
{
    unsigned long i, j;

    // 先写前导码
    for (i = 0; i < IPMI_FRAME_CHAR_SIZE; i++)
    {
        spi_tx_buf[i] = IPMI_FRAME_CHAR[i];
    }

    // 写数据
    for (j = 0; j < size && i < SPI_BUF_SIZE; j++, i++)
    {
        spi_tx_buf[i] = buf[j];
    }

    spi_tx_idx = 0;
    spi_tx_len = i;

    return 0;
}
#endif


#ifdef IPMI_MODULES_SPI0_CPLD
//*****************************************************************************
//
// Init SPI0 hardware to FPGA(MASTER) as Normal
//
//*****************************************************************************

#if 0
//
// this dirver is use the interrupt to driven the ssi hardware
//
static SPI_DRIVER spi0_driver;

//*****************************************************************************
//
// The interrupt handler for the SSI0 interrupt.
//
//*****************************************************************************
void SPI_spi0_int_handler(void)
{
    unsigned long ulStatus, ulCount, ulData;

    //
    // Get the reason for the interrupt.
    //
    ulStatus = SSIIntStatus(SSI0_BASE, true);

    //
    // See if the receive interrupt is being asserted.
    //
    if(ulStatus & (SSI_RXFF | SSI_RXTO))
    {
        //
        // Loop as many times as required to empty the FIFO.
        //
        while(1)
        {
            //
            // Read a byte from the FIFO if possible.  Break out of the loop if
            // the FIFO is empty.
            //
            if(SSIDataGetNonBlocking(SSI0_BASE, &ulData) == 0)
            {
                break;
            }

            //
            // See if this byte needs to be saved.
            //
            if(spi0_driver.in_count)
            {
                //
                // Save this byte.
                //
                *spi0_driver.in_data++ = ulData;

                //
                // Decrement the count of bytes to save.
                //
                spi0_driver.in_count--;
            }
            else
            {
                //
                // Decrement the count of extra bytes to read.
                //
                spi0_driver.in_extra--;
            }
        }

        //
        // See if all data has been transmitted and received.
        //
        if((spi0_driver.in_count + spi0_driver.in_extra + spi0_driver.out_count + spi0_driver.out_extra) == 0)
        {
            //
            // All data has been transmitted and received, so disable the
            // receive interrupt.
            //
            SSIIntDisable(SSI0_BASE, SSI_RXFF | SSI_RXTO);

            //
            // Deassert the SSI chip select.
            //
            GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, GPIO_PIN_3);
        }
    }

    //
    // See if the transmit interrupt is being asserted.
    //
    if(ulStatus & SSI_TXFF)
    {
        //
        // Write up to four bytes into the FIFO.
        //
        for(ulCount = 0; ulCount < 4; ulCount++)
        {
            //
            // See if there is more data to be transmitted.
            //
            if(spi0_driver.out_count)
            {
                //
                // Transmit this byte if possible.  Break out of the loop if
                // the FIFO is full.
                //
                if(SSIDataPutNonBlocking(SSI0_BASE, *spi0_driver.out_data) == 0)
                {
                    break;
                }

                //
                // Decrement the count of bytes to be transmitted.
                //
                spi0_driver.out_data++;
                spi0_driver.out_count--;
            }

            //
            // See if there are more extra bytes to be transmitted.
            //
            else if(spi0_driver.out_extra)
            {
                //
                // Transmit this extra byte if possible.  Break out of the loop
                // if the FIFO is full.
                //
                if(SSIDataPutNonBlocking(SSI0_BASE, 0) == 0)
                {
                    break;
                }

                //
                // Decrement the count of extra bytes to be transmitted.
                //
                spi0_driver.out_extra--;
            }

            //
            // Otherwise, stop transmitting data.
            //
            else
            {
                //
                // Disable the transmit interrupt since all data to be
                // transmitted has been written into the FIFO.
                //
                SSIIntDisable(SSI0_BASE, SSI_TXFF);

                //
                // Break out of the loop since there is no more data to
                // transmit.
                //
                break;
            }
        }
    }
}


//*****************************************************************************
//
// This will start an interrupt driven transfer to the SSI peripheral.
//
//*****************************************************************************
void SPI_spi0_xfer(unsigned char *pucDataOut, unsigned long ulOutCount,
            unsigned char *pucDataIn, unsigned long ulInCount)
{
    uint8_t err;

    OSSemPend(spi0_driver.sem, 0, &err);

    //
    // Save the output data buffer.
    //
    spi0_driver.out_data = pucDataOut;
    spi0_driver.out_count = ulOutCount;

    //
    // Save the input data buffer.
    //
    spi0_driver.in_data = pucDataIn;
    spi0_driver.in_count = ulInCount;

    //
    // Compute the number of extra bytes to send or receive.  These counts make
    // the number of bytes transmitted equal to the number of bytes received;
    // a requirement of the SSI peripheral.
    //
    if(ulInCount > ulOutCount)
    {
        spi0_driver.out_extra = ulInCount - ulOutCount;
        spi0_driver.in_extra = 0;
    }
    else
    {
        spi0_driver.out_extra = 0;
        spi0_driver.in_extra = ulOutCount - ulInCount;
    }

    //
    // Assert the SSI chip select.
    //
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, 0);

    //
    // Enable the transmit and receive interrupts.  This will start the actual
    // transfer.
    //
    SSIIntEnable(SSI0_BASE, SSI_TXFF | SSI_RXFF | SSI_RXTO);

    //
    // Wait until the SSI chip select deasserts, indicating the end of the
    // transfer.
    //
    while(!(GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_3) & GPIO_PIN_3))
    {
    }

    OSSemPost(spi0_driver.sem);
}
#endif

//*****************************************************************************
//
// This will use library function driven transfer to the SSI peripheral.
//
//*****************************************************************************
void SPI_spi0_xfer(unsigned char *pucDataOut, unsigned long ulOutCount,
            unsigned char *pucDataIn, unsigned long ulInCount)
{
    unsigned long ulDataTx;
    unsigned long ulDataRx;
    unsigned long ulindex;

    while(SSIDataGetNonBlocking(SSI0_BASE, &ulDataRx))
    {
    }

    for(ulindex = 0; ulindex < ulOutCount; ulindex++)
    {
        //
        // Send the data using the "blocking" put function.  This function
        // will wait until there is room in the send FIFO before returning.
        // This allows you to assure that all the data you send makes it into
        // the send FIFO.
        //
        ulDataTx = *pucDataOut++;
        SSIDataPut(SSI0_BASE, ulDataTx);
    }

    //
    // Wait until SSI0 is done transferring all the data in the transmit FIFO.
    //
    while(!SSIBusy(SSI0_BASE))
    {
    }

    for(ulindex = 0; ulindex < ulInCount; ulindex++)
    {
        //
        // Give a clock to slaver and recive data from fifo_rx
        //
        SSIDataPut(SSI0_BASE, 0);

        //
        // Receive the data using the "blocking" Get function. This function
        // will wait until there is data in the receive FIFO before returning.
        //
        SSIDataGet(SSI0_BASE, &ulDataRx);

        //
        // Since we are using 8-bit data, mask off the MSB.
        //
        ulDataRx &= 0x00FF;

        //
        // return the data that SSI0 received.
        //
        *pucDataIn++ = (unsigned char)ulDataRx;
    }
}

void SPI_spi0_master_init(void)
{
    //spi0_driver.sem = OSSemCreate(1);

    // The SSI0 peripheral must be enabled for use.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);

    // For this example SSI0 is used with PortA[5:2]. GPIO port A needs to be
    // enabled so these pins can be used.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Configure the pin muxing for SSI0 functions on port A2, A3, A4, and A5.
    // This step is not necessary if your part does not support pin muxing.
    GPIOPinConfigure(GPIO_PA2_SSI0CLK);
    GPIOPinConfigure(GPIO_PA3_SSI0FSS);
    GPIOPinConfigure(GPIO_PA4_SSI0RX);
    GPIOPinConfigure(GPIO_PA5_SSI0TX);

    // Configure the GPIO settings for the SSI pins.
    // The pins are assigned as follows:
    //      PA5 - SSI0Tx
    //      PA4 - SSI0Rx
    //      PA3 - SSI0Fss
    //      PA2 - SSI0CLK
    GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_3 | GPIO_PIN_2);

    // Use GPIO function to SPIFss Pin
    //GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_3);
    //GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, GPIO_PIN_3);

    // Configure and enable the SSI port for SPI master mode.  Use SSI0,
    // system clock supply, idle clock level low and active low clock in
    // freescale SPI mode, master mode, 1MHz SSI frequency, and 8-bit data.
    SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,
                       SSI_MODE_MASTER, 1000000, 8);

    // Enable the SSI0 module.
    SSIEnable(SSI0_BASE);
}

#endif



//*****************************************************************************
//
// Init the SPI Interface
//
//*****************************************************************************
void SPI_init(void)
{
#ifdef IPMI_MODULES_SPI1_SSIF
    SPI_spi1_slaver_init();
#endif

#ifdef IPMI_MODULES_SPI0_CPLD
    SPI_spi0_master_init();
#endif
}

