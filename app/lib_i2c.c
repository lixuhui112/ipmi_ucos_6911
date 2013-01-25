//*****************************************************************************
//
// lib_i2c.c - I2C routines for the IPMI application.
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************
#include <stdlib.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/i2c.h"
#include "driverlib/debug.h"
#include "driverlib/interrupt.h"
#include "ipmi_lib/ipmi_modules.h"
#include "ipmi_lib/ipmi_intf.h"
#include "app/lib_common.h"
#include "app/lib_gpio.h"
#include "app/lib_i2c.h"
#include "ucos_ii.h"

//*****************************************************************************
// Defines bus and device for the ipmi/i2c
//*****************************************************************************
#if 0
static ipmi_i2c_bus i2c0;
static ipmi_i2c_dev at24c;
static ipmi_i2c_dev adt7470_1;
static ipmi_i2c_dev adt7470_2;
static ipmi_i2c_dev max6635;
static ipmi_i2c_dev ina230;
static ipmi_i2c_ram_dev i2c_ram_dev;
#endif

//*****************************************************************************
// Read some data from slave device on the I2C bus.
//*****************************************************************************
uint32_t I2C_dev_read(unsigned long dev, unsigned char slave_addr, unsigned long data_addr,
        unsigned char addr_size, unsigned long data_size, char *buf)
{
    unsigned char data_addr_i2c[4];
    int addr_i2c_len = 0;
    int data_i2c_index = 0;
    int error;

    //
    // Check the arguments.
    //
    ASSERT(dev);
    ASSERT(data_addr);
    ASSERT(addr_size);
    ASSERT(data_size);
    ASSERT(buf);

    // 准备从设备寄存器地址
    switch (addr_size)
    {
        case 4:                         // 32位寄存器地址
            addr_i2c_len = 4;
            data_addr_i2c[0] = (char)(data_addr >> 24);
            data_addr_i2c[1] = (char)(data_addr >> 16);
            data_addr_i2c[2] = (char)(data_addr >> 8);
            data_addr_i2c[3] = (char)(data_addr);
            break;
        case 3:                         // 24位寄存器地址
            addr_i2c_len = 3;
            data_addr_i2c[0] = (char)(data_addr >> 16);
            data_addr_i2c[1] = (char)(data_addr >> 8);
            data_addr_i2c[2] = (char)(data_addr);
            break;
        case 2:                         // 16位寄存器地址
            addr_i2c_len = 2;
            data_addr_i2c[0] = (char)(data_addr >> 8);
            data_addr_i2c[1] = (char)(data_addr);
            break;
        case 1:                         // 8位寄存器地址
            addr_i2c_len = 1;
            data_addr_i2c[0] = (char)(data_addr);
            break;
        default:
            break;
    }

    // 如果没有地址或者数据长度为0，则直接返回
    if (addr_i2c_len == 0 || data_size == 0)
    {
        return I2C_MASTER_ERR_NONE;
    }

    // 设置从设备的地址
    I2CMasterSlaveAddrSet(dev, slave_addr, false);

    // 将从设备寄存器地址放到寄存器中
    I2CMasterDataPut(dev, data_addr_i2c[data_i2c_index]);

    // 防止多主机同时操作，等待总线空闲
    //while (I2CMasterBusBusy(dev->bus->i2c_hw_master_base));

    // 主机突发发送起始
    I2CMasterControl(dev, I2C_MASTER_CMD_BURST_SEND_START);

    // 循环将从设备寄存器地址发到总线上
    for (data_i2c_index = 1; data_i2c_index < addr_i2c_len; data_i2c_index++)
    {
        // 等待芯片空闲
        while (I2CMasterBusy(dev));

        // 检测可能的错误
        if ((error = I2CMasterErr(dev)) != I2C_MASTER_ERR_NONE)
        {
            if (error & I2C_MASTER_ERR_ARB_LOST)
            {
                I2CMasterControl(dev, I2C_MASTER_CMD_BURST_SEND_ERROR_STOP);
            }
            return error;
        }

        // 继续将从设备寄存器地址放到芯片寄存器中
        I2CMasterDataPut(dev, data_addr_i2c[data_i2c_index]);

        // 突发继续
        I2CMasterControl(dev, I2C_MASTER_CMD_BURST_SEND_CONT);
    }

    // 设置芯片为接收模式
    I2CMasterSlaveAddrSet(dev, slave_addr, true);

    // 等待芯片空闲
    while (I2CMasterBusy(dev));

    // 发送接收命令
    I2CMasterControl(dev, I2C_MASTER_CMD_BURST_RECEIVE_START);

    for (data_i2c_index = 0; data_i2c_index < data_size - 1; data_i2c_index++)
    {
        // 等待芯片空闲
        while (I2CMasterBusy(dev));

        // 检测可能的错误
        if ((error = I2CMasterErr(dev)) != I2C_MASTER_ERR_NONE)
        {
            if (error & I2C_MASTER_ERR_ARB_LOST)
            {
                I2CMasterControl(dev, I2C_MASTER_CMD_BURST_SEND_ERROR_STOP);
            }
            return error;
        }

        buf[data_i2c_index]= I2CMasterDataGet(dev);
    }

    // 传输结束
    I2CMasterControl(dev, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);

    // 等待芯片空闲
    while (I2CMasterBusy(dev));

    // 检测可能的错误
    if ((error = I2CMasterErr(dev)) != I2C_MASTER_ERR_NONE)
    {
        if (error & I2C_MASTER_ERR_ARB_LOST)
        {
            I2CMasterControl(dev, I2C_MASTER_CMD_BURST_SEND_ERROR_STOP);
        }
        return error;
    }

    buf[data_i2c_index]= I2CMasterDataGet(dev);

    // 返回可能的错误
    return I2CMasterErr(dev);
}

//*****************************************************************************
// Write some data to slave device on the I2C bus.
//*****************************************************************************
uint32_t I2C_dev_write(unsigned long dev, unsigned char slave_addr, unsigned long data_addr,
        unsigned char addr_size, unsigned long data_size, char *buf)
{
    unsigned char data_addr_i2c[4];
    unsigned long error;
    int addr_i2c_len = 0;
    int data_i2c_index = 0;

    //
    // Check the arguments.
    //
    ASSERT(dev);
    ASSERT(addr_size);
    ASSERT(data_size);
    ASSERT(buf);

    // 准备从设备寄存器地址
    switch (addr_size)
    {
        case 4:                         // 32位寄存器地址
            addr_i2c_len = 4;
            data_addr_i2c[0] = (char)(data_addr >> 24);
            data_addr_i2c[1] = (char)(data_addr >> 16);
            data_addr_i2c[2] = (char)(data_addr >> 8);
            data_addr_i2c[3] = (char)(data_addr);
            break;
        case 3:                         // 24位寄存器地址
            addr_i2c_len = 3;
            data_addr_i2c[0] = (char)(data_addr >> 16);
            data_addr_i2c[1] = (char)(data_addr >> 8);
            data_addr_i2c[2] = (char)(data_addr);
            break;
        case 2:                         // 16位寄存器地址
            addr_i2c_len = 2;
            data_addr_i2c[0] = (char)(data_addr >> 8);
            data_addr_i2c[1] = (char)(data_addr);
            break;
        case 1:                         // 8位寄存器地址
            addr_i2c_len = 1;
            data_addr_i2c[0] = (char)(data_addr);
            break;
        default:
            break;
    }

    //DEBUG("data_addr=%d, data_size=%d\n", data_addr, data_size);

    // 如果没有地址或者数据长度为0，则直接返回
    if (addr_i2c_len == 0 || data_size == 0)
    {
        return I2C_MASTER_ERR_NONE;
    }

    I2CMasterInit(dev, false);

    // Enables the I2C Master interrupt.
    //I2CMasterIntEnable(I2C0_MASTER_BASE);

    // Enables the I2C Master block.
    I2CMasterEnable(dev);

    // 设置从设备的地址
    I2CMasterSlaveAddrSet(dev, slave_addr, false);

    // 将从设备寄存器地址放到寄存器中
    I2CMasterDataPut(dev, data_addr_i2c[data_i2c_index++]);

    // 防止多主机同时操作，等待总线空闲
    //while (I2CMasterBusBusy(dev));

    // 主机突发发送起始
    I2CMasterControl(dev, I2C_MASTER_CMD_BURST_SEND_START);

    // 循环将从设备寄存器地址发到总线上
    for (data_i2c_index = 0; data_i2c_index < (addr_i2c_len + data_size); data_i2c_index++)
    {
        // 等待芯片空闲
        while (I2CMasterBusy(dev));

        // 检测可能的错误
        if ((error = I2CMasterErr(dev)) != I2C_MASTER_ERR_NONE)
        {
            if (error & I2C_MASTER_ERR_ARB_LOST)
            {
                I2CMasterControl(dev, I2C_MASTER_CMD_BURST_SEND_ERROR_STOP);
            }

            I2CMasterDisable(dev);

            return error;
        }

        // 继续将数据放到芯片寄存器中
        if (data_i2c_index < addr_i2c_len)
        {
            I2CMasterDataPut(dev, data_addr_i2c[data_i2c_index]);
        }
        else
        {
            I2CMasterDataPut(dev, buf[data_i2c_index - addr_i2c_len]);
        }

        // 突发继续
        I2CMasterControl(dev, I2C_MASTER_CMD_BURST_SEND_CONT);
    }

    I2CMasterControl(dev, I2C_MASTER_CMD_BURST_SEND_FINISH);

    // 等待芯片空闲
    while (I2CMasterBusy(dev));

    // 检测可能的错误
    error = I2CMasterErr(dev);

    I2CMasterDisable(dev);

    return error;
}

#if (defined(IPMI_MODULES_I2C0_IPMB))
//*****************************************************************************
//
// The function of I2C1 Interface
//
//*****************************************************************************
static I2C_DRIVER i2c0;                                     // I2C0设备驱动
static I2C_DEVICE i2c0_peer_dev;                            // I2C0主模式对端设备
static uint8_t i2c0_self_address;                           // I2C0从设备地址

#define I2C0_BUF_SIZE           0x80                        // IPMB帧最大长度
static unsigned char i2c0_rx_buf[I2C0_BUF_SIZE];
static unsigned char i2c0_tx_buf[I2C0_BUF_SIZE];
static unsigned char i2c0_rx_idx;
static unsigned char i2c0_rx_len;
static unsigned char i2c0_tx_idx;
static unsigned char i2c0_tx_len;
static unsigned long i2c0_rx_timestamp;

//*****************************************************************************
// The interrupt handler for the I2C0 interrupt.
//*****************************************************************************
void I2C_i2c0_int_handler(void)
{
    unsigned long status;

    // 获取I2C0主机中断状态
    status = I2CMasterIntStatus(I2C0_MASTER_BASE, true);
    if (status)
    {
        I2CMasterIntClear(I2C0_MASTER_BASE);

        // 若遇到错误
        if ((i2c0.error = I2CMasterErr(I2C0_MASTER_BASE)) != I2C_MASTER_ERR_NONE)
        {
            if (i2c0.error & I2C_MASTER_ERR_ARB_LOST)
            {
                I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_ERROR_STOP);
            }

            i2c0.status = I2C_STAT_IDLE;
            return;
        }

        switch (i2c0.status)
        {
            // 发送数据地址状态
            case I2C_STAT_ADDR:

                // 若数据地址未发送完毕
                if (i2c0.addr_index < i2c0.dev->addr_size)
                {
                    // 继续发送数据地址
                    I2CMasterDataPut(I2C0_MASTER_BASE, i2c0.reg_addr[i2c0.addr_index++]);

                    // 命令:主机突发发送继续
                    I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);

                    break;
                }
                else
                {
                    // 设置状态:收发数据
                    i2c0.status = I2C_STAT_DATA;

                    // 若是接收操作
                    if (i2c0.flags == I2C_CMD_RECV)
                    {
                        //  设置从机地址，读操作
                        I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, i2c0.dev->slave_addr, true);

                        // 若只准备接收1个字节
                        if (i2c0.dev->data_size == 1)
                        {
                            // 命令:主机单次接收
                            I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);

                            // 设置状态:接收结束
                            i2c0.status = I2C_STAT_FINISH;
                        }
                        else
                        {
                            // 命令:主机突发接收起始
                            I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
                        }
                        break;
                    }
                }
                // 直接进入下一条case语句

            // 收发数据状态
            case I2C_STAT_DATA:

                // 若是接收操作
                if (i2c0.flags == I2C_CMD_RECV)
                {
                    // 读取接收到的数据
                    i2c0.dev->data_buf[i2c0.data_index++] = I2CMasterDataGet(I2C0_MASTER_BASE);

                    // 若数据未接收完毕
                    if (i2c0.data_index + 1 < i2c0.dev->data_size)
                    {
                        // 命令:主机突发接收继续
                        I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
                    }
                    else
                    {
                        // 命令:主机突发接收完成
                        I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);

                        // 设置状态:接收完成
                        i2c0.status = I2C_STAT_FINISH;
                    }
                }
                // 若是发送操作
                else
                {
                    // 发送数据至寄存器
                    I2CMasterDataPut(I2C0_MASTER_BASE, i2c0.dev->data_buf[i2c0.data_index++]);

                    // 若数据未发送完毕
                    if (i2c0.data_index < i2c0.dev->data_size)
                    {
                        // 命令:主机突发发送继续
                        I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
                    }
                    else
                    {
                        // 命令:主机突发发送完成
                        I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);

                        // 设置状态:发送完成
                        i2c0.status = I2C_STAT_FINISH;
                    }
                }
                break;

            // 收发完成状态
            case I2C_STAT_FINISH:

                // 若是接收操作
                if (i2c0.flags == I2C_CMD_RECV)
                {
                    // 读取接最后收到的数据
                    i2c0.dev->data_buf[i2c0.data_index] = I2CMasterDataGet(I2C0_MASTER_BASE);
                }

                // 设置状态:空闲
                i2c0.status = I2C_STAT_IDLE;
                break;

            default:
                break;
        }
        return;
    }

    // 获取I2C0从机中断状态
    status = I2CSlaveIntStatus(I2C0_SLAVE_BASE, true);
    if (status)
    {
        unsigned long act;
        unsigned char data;
        static unsigned char i2c0_frame = 0;
        static unsigned char i2c0_size = 0;

        I2CSlaveIntClear(I2C0_SLAVE_BASE);

        // if recvie frame time out, reset the frame
        if (OSTimeGet() - i2c0_rx_timestamp > SYSTICKHZ * 2)
        {
            i2c0_frame = 0;
        }

        // save the recvie frame time stamp
        i2c0_rx_timestamp = OSTimeGet();

        act = I2CSlaveStatus(I2C0_SLAVE_BASE);
        switch (act)
        {
            // 主机没有请求任何动作
            case I2C_SLAVE_ACT_NONE:
                break;

            // 主机已发送从机地址和数据地址首字节
            case I2C_SLAVE_ACT_RREQ_FBR:
                data = (unsigned char)I2CSlaveDataGet(I2C0_SLAVE_BASE);

                DEBUG("\r\naddr=0x%x\r\n", data);

                // 读取的数据为寄存器地址，默认为0
                i2c0_frame = 0;
                i2c0_size = 0;
                break;

            // 主机已经发送数据到从机
            case I2C_SLAVE_ACT_RREQ:
                data = (unsigned char)I2CSlaveDataGet(I2C0_SLAVE_BASE);

                DEBUG(">data=0x%x\r\n", data);

                // 检测前导码
                if (i2c0_frame < IPMI_FRAME_CHAR_SIZE)
                {
                    if (IPMI_FRAME_CHAR[i2c0_frame] == data)
                    {
                        i2c0_frame++;
                    }
                    else
                    {
                        i2c0_frame = 0;
                    }
                    i2c0_size = 0;
                }

                // 正常数据
                else
                {
                    if (i2c0_size == 0)
                    {
                        i2c0_size = data;
                        i2c0_rx_idx = 0;
                        i2c0_rx_len = 0;

                        if (i2c0_size > I2C0_BUF_SIZE)
                        {
                            i2c0_frame = 0;
                            break;
                        }
                    }

                    i2c0_rx_buf[i2c0_rx_idx++] = data;

                    if (i2c0_rx_idx == i2c0_size)
                    {
                        i2c0_rx_len = i2c0_rx_idx;
                        ipmi_intf_recv_post(IPMI_INTF_IPMB);
                        i2c0_frame = 0;
                    }
                }
                break;

            case I2C_SLAVE_ACT_TREQ:                            // 主机请求从机发送数据
                I2CSlaveDataPut(I2C0_SLAVE_BASE, 0x5a);
                DEBUG("<get=0x%x\r\n", 0x5a);
#if 0
                /*
                 * IPMB为主写模式操作，不需要从设备写数据到主机
                 */
                if (i2c0_tx_idx == 0)
                {
                    if (i2c0_frame < sizeof (IPMI_FRAME_CHAR))
                }
                if (i2c0_tx_idx < i2c0_tx_len)
                {
                    data = (unsigned long)i2c0_tx_buf[i2c0_tx_idx++];
                    I2CSlaveDataPut(I2C0_SLAVE_BASE, data);
                    i2c0_tx_idx &= (I2C0_BUF_SIZE - 1);
                }
                else                                            // 发送结束
                {
                    i2c0_tx_idx = 0;
                    i2c0_tx_len = 0;
                    I2CSlaveDataPut(I2C0_SLAVE_BASE, 0);
                }
#endif
                break;

            default:
                break;
        }
    }
}

//*****************************************************************************
// The I2C0 IPMB slave device read.
//*****************************************************************************
uint32_t I2C_i2c0_ipmb_read(char *buf, unsigned long *size)
{
    for (i2c0_rx_idx = 0; i2c0_rx_idx < i2c0_rx_len; i2c0_rx_idx++)
    {
        buf[i2c0_rx_idx] = i2c0_rx_buf[i2c0_rx_idx];
    }

    *size = i2c0_rx_len;

    return 0;
}

//*****************************************************************************
// The I2C0 IPMB master device write.
//*****************************************************************************
uint32_t I2C_i2c0_ipmb_write(unsigned char slave_addr, char *buf, unsigned long size)
{
    unsigned char j;
    uint8_t error;

    // 先写前导码
    for (i2c0_tx_idx = 0; i2c0_tx_idx < sizeof(IPMI_FRAME_CHAR); i2c0_tx_idx++)
    {
        i2c0_tx_buf[i2c0_tx_idx] = IPMI_FRAME_CHAR[i2c0_tx_idx];
    }

    // 写数据
    for (j = 0; j < size && i2c0_tx_idx < I2C0_BUF_SIZE; j++, i2c0_tx_idx++)
    {
        i2c0_tx_buf[i2c0_tx_idx] = buf[j];
    }

    i2c0_tx_len = i2c0_tx_idx;
    i2c0_tx_idx = 0;

#if 1
    // 信号量保护，防止函数重入
    OSSemPend(i2c0.sem, 0, &error);
    error = I2C_dev_write(I2C0_MASTER_BASE, slave_addr, 0, 1, i2c0_tx_len, (char*)&i2c0_tx_buf[0]);
    OSSemPost(i2c0.sem);
    return error;
#else
    I2C_i2c0_slave_dev_init(&i2c0_peer_dev, slave_addr, 1);
    I2C_i2c0_slave_dev_set(&i2c0_peer_dev, 0, (uint8_t*)&i2c0_tx_buf[0], i2c0_tx_len);
    error = I2C_i2c0_master_write(&i2c0_peer_dev);
    return error;
#endif
}

//*****************************************************************************
// The I2C0 IPMB slave device init
//*****************************************************************************
void I2C_i2c0_slave_dev_init(I2C_DEVICE *dev, uint8_t slave_addr, uint32_t addr_size)
{
    dev->slave_addr = slave_addr;
    dev->addr_size = addr_size;
}

//*****************************************************************************
// The I2C0 IPMB slave device setting
//*****************************************************************************
void I2C_i2c0_slave_dev_set(I2C_DEVICE *dev, uint32_t reg_addr, uint8_t *data_buf, uint32_t data_size)
{
    dev->reg_addr = reg_addr;
    dev->data_buf = data_buf;
    dev->data_size = data_size;
}

//*****************************************************************************
// The I2C0 IPMB slave device address set
//*****************************************************************************
void I2C_i2c0_ipmb_self_addr_set(unsigned char self_addr)
{
    i2c0_self_address = self_addr;

    // Set the slave address to SLAVE_ADDRESS.
    I2CSlaveInit(I2C0_SLAVE_BASE, i2c0_self_address);

    // Enables the I2C Slave interrupt.
    I2CSlaveIntEnable(I2C0_SLAVE_BASE);

    // Enables the I2C Slave block.
    I2CSlaveEnable(I2C0_SLAVE_BASE);

    //DEBUG("set ipmb_self_addr=0x%x\r\n", i2c0_self_address);
}

//*****************************************************************************
// The I2C0 IPMB slave device address get
//*****************************************************************************
uint8_t I2C_i2c0_ipmb_self_addr_get(void)
{
    return i2c0_self_address;
}

//*****************************************************************************
// The I2C0 hardware Initailize for IPMB.
//*****************************************************************************
void I2C_i2c0_hardware_init(void)
{
    // The I2C0 peripheral must be enabled before use.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);

    // For this example I2C0 is used with PortB[3:2].
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    // Configure the pin muxing for I2C0 functions on port B2 and B3.
    //GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    //GPIOPinConfigure(GPIO_PB3_I2C0SDA);

    // Select the I2C function for these pins.
    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_2 | GPIO_PIN_3);

    // Enables the I2C0 interrupt.
    IntEnable(INT_I2C0);
}

void I2C_i2c0_hardware_reset(void)
{
    IntDisable(INT_I2C0);
    SysCtlPeripheralDisable(SYSCTL_PERIPH_I2C0);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_2);
    GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_3);
    GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}

//*****************************************************************************
// Enable and initialize the I2C1 master module. Use a data rate of 100kbps.
//*****************************************************************************
void I2C_i2c0_mode_master(void)
{
    //I2CSlaveDisable(I2C0_SLAVE_BASE);

    // Initializes the I2C Master block.
    I2CMasterInit(I2C0_MASTER_BASE, false);

    // Enables the I2C Master interrupt.
    I2CMasterIntEnable(I2C0_MASTER_BASE);

    // Enables the I2C Master block.
    I2CMasterEnable(I2C0_MASTER_BASE);
}

//*****************************************************************************
// Enable and initialize the I2C0 slaver module and set the self address .
//*****************************************************************************
void I2C_i2c0_mode_slaver(void)
{
    I2CMasterIntDisable(I2C0_MASTER_BASE);
    I2CMasterDisable(I2C0_MASTER_BASE);

    I2C_i2c0_hardware_reset();
    I2C_i2c0_hardware_init();

    I2C_i2c0_ipmb_self_addr_set(IPMB_SLAVE_ADDR_BASE + 1);
}

//*****************************************************************************
// The I2C0 IPMB master device read and write
//*****************************************************************************
uint32_t I2C_i2c0_read_write(I2C_DEVICE *dev, tBoolean flags)
{
    INT8U err;

    if ((dev->addr_size <= 0) || (dev->data_size <= 0))
    {
        return(I2C_MASTER_ERR_ADDR_ACK);
    }

    // 信号量保护，防止函数重入
    OSSemPend(i2c0.sem, 0, &err);

    i2c0.dev = dev;
    i2c0.flags = flags;
    i2c0.addr_index = 0;
    i2c0.data_index = 0;

    switch (dev->addr_size)                                     // 将数据地址分解成数组
    {
        case 1:                                                 // 1字节数据地址
            i2c0.reg_addr[0] = (uint8_t)(dev->reg_addr);
            break;

        case 2:                                                 // 2字节数据地址
            i2c0.reg_addr[0] = (uint8_t)(dev->reg_addr >> 8);
            i2c0.reg_addr[1] = (uint8_t)(dev->reg_addr);
            break;

        case 3:                                                 // 3字节数据地址
            i2c0.reg_addr[0] = (uint8_t)(dev->reg_addr >> 16);
            i2c0.reg_addr[1] = (uint8_t)(dev->reg_addr >> 8);
            i2c0.reg_addr[2] = (uint8_t)(dev->reg_addr);
            break;

        case 4:                                                 // 4字节数据地址
            i2c0.reg_addr[0] = (uint8_t)(dev->reg_addr >> 24);
            i2c0.reg_addr[1] = (uint8_t)(dev->reg_addr >> 16);
            i2c0.reg_addr[2] = (uint8_t)(dev->reg_addr >> 8);
            i2c0.reg_addr[3] = (uint8_t)(dev->reg_addr);
            break;

        default:
            break;
    }

    // 如果是多主机通信，则需要首先等待总线空闲
    // while (I2CMasterBusBusy(I2CM_BASE));

    // 设置为主模式
    I2C_i2c0_mode_master();

    // 设置从机地址，写操作
    I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, i2c0.dev->slave_addr, false);

    // 开始发送数据地址
    I2CMasterDataPut(I2C0_MASTER_BASE, i2c0.reg_addr[i2c0.addr_index++]);

    // 设置状态：发送数据地址
    i2c0.status = I2C_STAT_ADDR;

    // 命令：主机突发发送起始
    I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_START);

    // 等待总线操作完毕
    while (i2c0.status != I2C_STAT_IDLE);

    // 等待主机工作完成
    while (I2CMasterBusy(I2C0_MASTER_BASE));

    // 设置为从模式
    I2C_i2c0_mode_slaver();

    // 释放信号量
    OSSemPost(i2c0.sem);

    // 返回可能的错误状态
    return(i2c0.error);
}

void I2C_i2c0_ipmb_init(void)
{
    i2c0.sem = OSSemCreate(1);
    i2c0.dev = NULL;
    i2c0.addr_index = 0;
    i2c0.data_index = 0;
    i2c0.flags = 0;
    i2c0.status = I2C_STAT_IDLE;

    I2C_i2c0_slave_dev_init(&i2c0_peer_dev, IPMB_SLAVE_ADDR_BASE, 1);

    I2C_i2c0_hardware_init();

    // Default is use Slaver mode on I2C0.
    //I2C_i2c0_mode_slaver();
    I2C_i2c0_ipmb_self_addr_set(IPMB_SLAVE_ADDR_BASE + 1);
}
#endif


#if (defined(IPMI_MODULES_I2C1_HARD_PMB))
//*****************************************************************************
//
// The function of I2C1 Interface
//
//*****************************************************************************

static I2C_DRIVER i2c1;

#define I2C1_BUF_SIZE  0x80                        // IPMB帧最大长度
//static unsigned char i2c1_rx_buf[I2C1_BUF_SIZE];
//static unsigned char i2c1_tx_buf[I2C1_BUF_SIZE];
//static unsigned char i2c1_rx_idx;
//static unsigned char i2c1_tx_idx;
//static unsigned char i2c1_rx_len;
//static unsigned char i2c1_tx_len;
//static unsigned char i2c1_ipmb_addr = IPMB_SLAVE_ADDR_DEF;
//static unsigned long i2c1_rx_timestamp;

//*****************************************************************************
// The interrupt handler for the I2C1 interrupt.
//*****************************************************************************
void I2C_i2c1_int_handler(void)
{
    unsigned long status;
    //unsigned long act;
    //unsigned char data;
    //static unsigned char i2c0_frame = 0;
    //static unsigned char i2c0_size = 0;

    // 获取I2C1主机中断状态
    status = I2CMasterIntStatus(I2C1_MASTER_BASE, true);
    if (status)
    {
        I2CMasterIntClear(I2C1_MASTER_BASE);

        // 若遇到错误
        if (I2CMasterErr(I2C1_MASTER_BASE) != I2C_MASTER_ERR_NONE)
        {
            i2c1.status = I2C_STAT_IDLE;
            return;
        }

        switch (i2c1.status)
        {
            // 发送数据地址状态
            case I2C_STAT_ADDR:

                // 若数据地址未发送完毕
                if (i2c1.addr_index < i2c1.dev->addr_size)
                {
                    // 继续发送数据地址
                    I2CMasterDataPut(I2C1_MASTER_BASE, i2c1.reg_addr[i2c1.addr_index++]);

                    // 命令:主机突发发送继续
                    I2CMasterControl(I2C1_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);

                    break;
                }
                else
                {
                    // 设置状态:收发数据
                    i2c1.status = I2C_STAT_DATA;

                    // 若是接收操作
                    if (i2c1.flags == I2C_CMD_RECV)
                    {
                        //  设置从机地址，读操作
                        I2CMasterSlaveAddrSet(I2C1_MASTER_BASE, i2c1.dev->slave_addr, true);

                        // 若只准备接收1个字节
                        if (i2c1.dev->data_size == 1)
                        {
                            // 命令:主机单次接收
                            I2CMasterControl(I2C1_MASTER_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);

                            // 设置状态:接收结束
                            i2c1.status = I2C_STAT_FINISH;
                        }
                        else
                        {
                            // 命令:主机突发接收起始
                            I2CMasterControl(I2C1_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
                        }
                        break;
                    }
                }
                // 直接进入下一条case语句

            // 收发数据状态
            case I2C_STAT_DATA:

                // 若是接收操作
                if (i2c1.flags == I2C_CMD_RECV)
                {
                    // 读取接收到的数据
                    i2c1.dev->data_buf[i2c1.data_index++] = I2CMasterDataGet(I2C1_MASTER_BASE);

                    // 若数据未接收完毕
                    if (i2c1.data_index + 1 < i2c1.dev->data_size)
                    {
                        // 命令:主机突发接收继续
                        I2CMasterControl(I2C1_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
                    }
                    else
                    {
                        // 命令:主机突发接收完成
                        I2CMasterControl(I2C1_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);

                        // 设置状态:接收完成
                        i2c1.status = I2C_STAT_FINISH;
                    }
                }
                // 若是发送操作
                else
                {
                    // 发送数据至寄存器
                    I2CMasterDataPut(I2C1_MASTER_BASE, i2c1.dev->data_buf[i2c1.data_index++]);

                    // 若数据未发送完毕
                    if (i2c1.data_index < i2c1.dev->data_size)
                    {
                        // 命令:主机突发发送继续
                        I2CMasterControl(I2C1_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
                    }
                    else
                    {
                        // 命令:主机突发发送完成
                        I2CMasterControl(I2C1_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);

                        // 设置状态:发送完成
                        i2c1.status = I2C_STAT_FINISH;
                    }
                }
                break;

            // 收发完成状态
            case I2C_STAT_FINISH:

                // 若是接收操作
                if (i2c1.flags == I2C_CMD_RECV)
                {
                    // 读取接最后收到的数据
                    i2c1.dev->data_buf[i2c1.data_index] = I2CMasterDataGet(I2C1_MASTER_BASE);
                }

                // 设置状态:空闲
                i2c1.status = I2C_STAT_IDLE;
                break;

            default:
                break;
        }
        return;
    }
}

void I2C_i2c1_slave_dev_init(I2C_DEVICE *dev, uint8_t slave_addr, uint32_t addr_size)
{
    dev->slave_addr = slave_addr;
    dev->addr_size = addr_size;
}

void I2C_i2c1_slave_dev_set(I2C_DEVICE *dev, uint32_t reg_addr, uint8_t *data_buf, uint32_t data_size)
{
    dev->reg_addr = reg_addr;
    dev->data_buf = data_buf;
    dev->data_size = data_size;
}

uint32_t I2C_i2c1_read_write(I2C_DEVICE *dev, tBoolean flags)
{
    INT8U err;

    if ((dev->addr_size <= 0) || (dev->data_size <= 0))
    {
        return(I2C_MASTER_ERR_ADDR_ACK);
    }

    OSSemPend(i2c1.sem, 0, &err);

    i2c1.dev = dev;
    i2c1.flags = flags;
    i2c1.addr_index = 0;
    i2c1.data_index = 0;

    switch (dev->addr_size)                                     // 将数据地址分解成数组
    {
        case 1:                                                 // 1字节数据地址
            i2c1.reg_addr[0] = (uint8_t)(dev->reg_addr);
            break;

        case 2:                                                 // 2字节数据地址
            i2c1.reg_addr[0] = (uint8_t)(dev->reg_addr >> 8);
            i2c1.reg_addr[1] = (uint8_t)(dev->reg_addr);
            break;

        case 3:                                                 // 3字节数据地址
            i2c1.reg_addr[0] = (uint8_t)(dev->reg_addr >> 16);
            i2c1.reg_addr[1] = (uint8_t)(dev->reg_addr >> 8);
            i2c1.reg_addr[2] = (uint8_t)(dev->reg_addr);
            break;

        case 4:                                                 // 4字节数据地址
            i2c1.reg_addr[0] = (uint8_t)(dev->reg_addr >> 24);
            i2c1.reg_addr[1] = (uint8_t)(dev->reg_addr >> 16);
            i2c1.reg_addr[2] = (uint8_t)(dev->reg_addr >> 8);
            i2c1.reg_addr[3] = (uint8_t)(dev->reg_addr);
            break;

        default:
            break;
    }

    // 如果是多主机通信，则需要首先等待总线空闲
    // while (I2CMasterBusBusy(I2CM_BASE));

    // 设置从机地址，写操作
    I2CMasterSlaveAddrSet(I2C1_MASTER_BASE, i2c1.dev->slave_addr, false);

    // 开始发送数据地址
    I2CMasterDataPut(I2C1_MASTER_BASE, i2c1.reg_addr[i2c1.addr_index++]);

    // 设置状态：发送数据地址
    i2c1.status = I2C_STAT_ADDR;

    // 命令：主机突发发送起始
    I2CMasterControl(I2C1_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_START);

    // 等待总线操作完毕
    while (i2c1.status != I2C_STAT_IDLE);

    OSSemPost(i2c1.sem);

    // 返回可能的错误状态
    return(I2CMasterErr(I2C1_MASTER_BASE));
}

//*****************************************************************************
// The I2C1 hardware Initailize for PMB.
//*****************************************************************************
void I2C_i2c1_pmb_init(void)
{
    // The I2C0 peripheral must be enabled before use.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);

    // For this example I2C0 is used with PortA[7:6].
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Configure the pin muxing for I2C0 functions on port B2 and B3.
    GPIOPinConfigure(GPIO_PA6_I2C1SCL);
    GPIOPinConfigure(GPIO_PA7_I2C1SDA);

    // Select the I2C function for these pins.
    GPIOPinTypeI2C(GPIO_PORTA_BASE, GPIO_PIN_6 | GPIO_PIN_7);

    IntEnable(INT_I2C1);

    // Enable and initialize the I2C1 master module. Use a data rate of 100kbps.
    I2CMasterInit(I2C1_MASTER_BASE, false);

    I2CMasterIntEnable(I2C1_MASTER_BASE);

    I2CMasterEnable(I2C1_MASTER_BASE);
}
#endif


//*****************************************************************************
//
// The function of SoftWare I2C Interface
//
//*****************************************************************************
#if (defined(IPMI_MODULES_SOFT_PMB))
void I2C_i2c1_soft_pmb_init(void)
{
#if 0
    //
    // 初始化模拟I2C
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    GPIOPinTypeI2C(GPIO_PORTD_BASE, GPIO_PIN_2 | GPIO_PIN_3);
    memset(&g_sI2C, 0, sizeof(g_sI2C));
    SoftI2CCallbackSet(&g_sI2C, SoftI2CCallback);
    SoftI2CSCLGPIOSet(&g_sI2C, GPIO_PORTD_BASE, GPIO_PIN_2);
    SoftI2CSDAGPIOSet(&g_sI2C, GPIO_PORTD_BASE, GPIO_PIN_3);
    SoftI2CInit(&g_sI2C);
#endif
}

uint32_t I2C_i2c1_soft_pmb_read(unsigned slave_addr, char *buf, unsigned long *size)
{
    return 0;
}

uint32_t I2C_i2c1_soft_pmb_write(unsigned slave_addr, char *buf, unsigned long size)
{
    return 0;
}

#endif

//*****************************************************************************
//
// Init I2C hardware for IPMI-IPMB/IPMI-PMB
//
//*****************************************************************************
void I2C_init(void)
{
#if (defined(IPMI_MODULES_I2C0_IPMB))
    I2C_i2c0_ipmb_init();
#endif
#if (defined(IPMI_MODULES_I2C1_HARD_PMB))
    I2C_i2c1_pmb_init();
#endif
#if (defined(IPMI_MODULES_SOFT_PMB))
    I2C_i2c1_soft_pmb_init();
#endif
}

