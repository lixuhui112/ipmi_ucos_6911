//*****************************************************************************
//
// ipmi_pmb.c - IPMI Private Management Bus Interface
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#include "ipmi.h"
#if 0
#include "third_party/softi2c.h"

//*****************************************************************************
//
// The I2C slave address of the AT24C08A EEPROM device.  This address is based
// on the A2 pin of the AT24C08A being pulled high on the board.
//
//*****************************************************************************
#define SLAVE_ADDR              0x54

//*****************************************************************************
//
// The states in the interrupt handler state machine.
//
//*****************************************************************************
#define STATE_IDLE              0
#define STATE_WRITE_NEXT        1
#define STATE_WRITE_FINAL       2
#define STATE_WAIT_ACK          3
#define STATE_SEND_ACK          4
#define STATE_READ_ONE          5
#define STATE_READ_FIRST        6
#define STATE_READ_NEXT         7
#define STATE_READ_FINAL        8
#define STATE_READ_WAIT         9

static tSoftI2C g_sI2C;
static unsigned char *g_pucData = 0;
static unsigned long g_ulCount = 0;
static volatile unsigned long g_ulState = STATE_IDLE;



//*****************************************************************************
//
// The callback function for the SoftI2C module.
//
//*****************************************************************************
void
SoftI2CCallback(void)
{
    //
    // Clear the SoftI2C interrupt.
    //
    SoftI2CIntClear(&g_sI2C);

    //
    // Determine what to do based on the current state.
    //
    switch(g_ulState)
    {
        //
        // The idle state.
        //
        case STATE_IDLE:
        {
            //
            // There is nothing to be done.
            //
            break;
        }

        //
        // The state for the middle of a burst write.
        //
        case STATE_WRITE_NEXT:
        {
            //
            // Write the next data byte.
            //
            SoftI2CDataPut(&g_sI2C, *g_pucData++);
            g_ulCount--;

            //
            // Continue the burst write.
            //
            SoftI2CControl(&g_sI2C, SOFTI2C_CMD_BURST_SEND_CONT);

            //
            // If there is one byte left, set the next state to the final write
            // state.
            //
            if(g_ulCount == 1)
            {
                g_ulState = STATE_WRITE_FINAL;
            }

            //
            // This state is done.
            //
            break;
        }

        //
        // The state for the final write of a burst sequence.
        //
        case STATE_WRITE_FINAL:
        {
            //
            // Write the final data byte.
            //
            SoftI2CDataPut(&g_sI2C, *g_pucData++);
            g_ulCount--;

            //
            // Finish the burst write.
            //
            SoftI2CControl(&g_sI2C, SOFTI2C_CMD_BURST_SEND_FINISH);

            //
            // The next state is to wait for the burst write to complete.
            //
            g_ulState = STATE_SEND_ACK;

            //
            // This state is done.
            //
            break;
        }

        //
        // Wait for an ACK on the read after a write.
        //
        case STATE_WAIT_ACK:
        {
            //
            // See if there was an error on the previously issued read.
            //
            if(SoftI2CErr(&g_sI2C) == SOFTI2C_ERR_NONE)
            {
                //
                // Read the byte received.
                //
                SoftI2CDataGet(&g_sI2C);

                //
                // There was no error, so the state machine is now idle.
                //
                g_ulState = STATE_IDLE;

                //
                // This state is done.
                //
                break;
            }

            //
            // Fall through to STATE_SEND_ACK.
            //
        }

        //
        // Send a read request, looking for the ACK to indicate that the write
        // is done.
        //
        case STATE_SEND_ACK:
        {
            //
            // Put the I2C master into receive mode.
            //
            SoftI2CSlaveAddrSet(&g_sI2C, SLAVE_ADDR, true);

            //
            // Perform a single byte read.
            //
            SoftI2CControl(&g_sI2C, SOFTI2C_CMD_SINGLE_RECEIVE);

            //
            // The next state is the wait for the ack.
            //
            g_ulState = STATE_WAIT_ACK;

            //
            // This state is done.
            //
            break;
        }

        //
        // The state for a single byte read.
        //
        case STATE_READ_ONE:
        {
            //
            // Put the SoftI2C module into receive mode.
            //
            SoftI2CSlaveAddrSet(&g_sI2C, SLAVE_ADDR, true);

            //
            // Perform a single byte read.
            //
            SoftI2CControl(&g_sI2C, SOFTI2C_CMD_SINGLE_RECEIVE);

            //
            // The next state is the wait for final read state.
            //
            g_ulState = STATE_READ_WAIT;

            //
            // This state is done.
            //
            break;
        }

        //
        // The state for the start of a burst read.
        //
        case STATE_READ_FIRST:
        {
            //
            // Put the SoftI2C module into receive mode.
            //
            SoftI2CSlaveAddrSet(&g_sI2C, SLAVE_ADDR, true);

            //
            // Start the burst receive.
            //
            SoftI2CControl(&g_sI2C, SOFTI2C_CMD_BURST_RECEIVE_START);

            //
            // The next state is the middle of the burst read.
            //
            g_ulState = STATE_READ_NEXT;

            //
            // This state is done.
            //
            break;
        }

        //
        // The state for the middle of a burst read.
        //
        case STATE_READ_NEXT:
        {
            //
            // Read the received character.
            //
            *g_pucData++ = SoftI2CDataGet(&g_sI2C);
            g_ulCount--;

            //
            // Continue the burst read.
            //
            SoftI2CControl(&g_sI2C, SOFTI2C_CMD_BURST_RECEIVE_CONT);

            //
            // If there are two characters left to be read, make the next
            // state be the end of burst read state.
            //
            if(g_ulCount == 2)
            {
                g_ulState = STATE_READ_FINAL;
            }

            //
            // This state is done.
            //
            break;
        }

        //
        // The state for the end of a burst read.
        //
        case STATE_READ_FINAL:
        {
            //
            // Read the received character.
            //
            *g_pucData++ = SoftI2CDataGet(&g_sI2C);
            g_ulCount--;

            //
            // Finish the burst read.
            //
            SoftI2CControl(&g_sI2C, SOFTI2C_CMD_BURST_RECEIVE_FINISH);

            //
            // The next state is the wait for final read state.
            //
            g_ulState = STATE_READ_WAIT;

            //
            // This state is done.
            //
            break;
        }

        //
        // This state is for the final read of a single or burst read.
        //
        case STATE_READ_WAIT:
        {
            //
            // Read the received character.
            //
            *g_pucData++ = SoftI2CDataGet(&g_sI2C);
            g_ulCount--;

            //
            // The state machine is now idle.
            //
            g_ulState = STATE_IDLE;

            //
            // This state is done.
            //
            break;
        }
    }
}


//*****************************************************************************
//
// Write to the Atmel device.
//
//*****************************************************************************
void
AtmelWrite(unsigned char *pucData, unsigned long ulOffset,
           unsigned long ulCount)
{
    //
    // Save the data buffer to be written.
    //
    g_pucData = pucData;
    g_ulCount = ulCount;

    //
    // Set the next state of the callback state machine based on the number of
    // bytes to write.
    //
    if(ulCount != 1)
    {
        g_ulState = STATE_WRITE_NEXT;
    }
    else
    {
        g_ulState = STATE_WRITE_FINAL;
    }

    //
    // Set the slave address and setup for a transmit operation.
    //
    SoftI2CSlaveAddrSet(&g_sI2C, SLAVE_ADDR | (ulOffset >> 8), false);

    //
    // Write the address to be written as the first data byte.
    //
    SoftI2CDataPut(&g_sI2C, ulOffset);

    //
    // Start the burst cycle, writing the address as the first byte.
    //
    SoftI2CControl(&g_sI2C, SOFTI2C_CMD_BURST_SEND_START);

    //
    // Wait until the SoftI2C callback state machine is idle.
    //
    while(g_ulState != STATE_IDLE)
    {
    }
}



//*****************************************************************************
//
// Read from the Atmel device.
//
//*****************************************************************************
void
AtmelRead(unsigned char *pucData, unsigned long ulOffset,
          unsigned long ulCount)
{
    //
    // Save the data buffer to be read.
    //
    g_pucData = pucData;
    g_ulCount = ulCount;

    //
    // Set the next state of the callback state machine based on the number of
    // bytes to read.
    //
    if(ulCount == 1)
    {
        g_ulState = STATE_READ_ONE;
    }
    else
    {
        g_ulState = STATE_READ_FIRST;
    }

    //
    // Start with a dummy write to get the address set in the EEPROM.
    //
    SoftI2CSlaveAddrSet(&g_sI2C, SLAVE_ADDR | (ulOffset >> 8), false);

    //
    // Write the address to be written as the first data byte.
    //
    SoftI2CDataPut(&g_sI2C, ulOffset);

    //
    // Perform a single send, writing the address as the only byte.
    //
    SoftI2CControl(&g_sI2C, SOFTI2C_CMD_SINGLE_SEND);

    //
    // Wait until the SoftI2C callback state machine is idle.
    //
    while(g_ulState != STATE_IDLE)
    {
    }
}

void pmb_test(void *args)
{
    unsigned char pucData[16];
    unsigned long ulIdx;
    //int ret;
    //uint8_t netfn;

    //ipmi_i2c_dev_init(&at24c, &i2c0, 0x54);

    //HWREG(I2C0_MASTER_BASE + I2C_O_MCR) |= 0x01;

    while (1)
    {
        //
        // Display the example setup on the console.
        //
        UARTprintf("SoftI2C Atmel AT24C08A example\n");

        //
        // Write a data=address pattern into the first 16 bytes of the Atmel
        // device.
        //
        UARTprintf("Write:");
        for(ulIdx = 0; ulIdx < 16; ulIdx++)
        {
            pucData[ulIdx] = ulIdx;
            UARTprintf(" %02x", pucData[ulIdx]);
        }
        UARTprintf("\n");
        AtmelWrite(pucData, 3, 16);

        //
        // Clear the memroy
        //
        for(ulIdx = 0; ulIdx < 16; ulIdx++)
        {
            pucData[ulIdx] = 0;
        }

        //
        // Read back the first 16 bytes of the Atmel device.
        //
        UARTprintf("Read :");
        AtmelRead(pucData, 3, 16);
        for(ulIdx = 0; ulIdx < 16; ulIdx++)
        {
            UARTprintf(" %02x", pucData[ulIdx]);
        }
        UARTprintf("\n");

        //
        // Tell the user that the test is done.
        //
        UARTprintf("Done.\n\n");
#if 0
        //
        // Display the example setup on the console.
        //
        UARTprintf("I2C Atmel AT24C08A example\n");

        //
        // Write a data=address pattern into the first 16 bytes of the Atmel
        // device.
        //
        UARTprintf("Write:");
        for(ulIdx = 0; ulIdx < 16; ulIdx++)
        {
            pucData[ulIdx] = ulIdx;
            UARTprintf(" %02x", pucData[ulIdx]);
        }
        UARTprintf("\n");
        ret = ipmi_i2c_dev_write(&at24c, 0, 16, pucData);
        UARTprintf("i2c write ret = %d\n", ret);

        //
        // Clear the memroy
        //
        for(ulIdx = 0; ulIdx < 16; ulIdx++)
        {
            pucData[ulIdx] = 0;
        }

        //
        // Read back the first 16 bytes of the Atmel device.
        //
        ret = ipmi_i2c_dev_read(&at24c, 0, 16, pucData);
        UARTprintf("Read :");
        for(ulIdx = 0; ulIdx < 16; ulIdx++)
        {
            UARTprintf(" %02x", pucData[ulIdx]);
        }
        UARTprintf("\n");
        UARTprintf("i2c read ret = %d\n", ret);

        //
        // Tell the user that the test is done.
        //
        UARTprintf("Done.\n\n");
#endif
        OSTimeDlyHMSM(0, 0, 10, 0);
    }
}

#endif

