//*****************************************************************************
//
// lib_i2c.h - I2C routines for the IPMI application.
//
//*****************************************************************************

#ifndef __LIB_I2C_H__
#define __LIB_I2C_H__

#include "inc/hw_memmap.h"
#include "ucos_ii.h"
#include <inttypes.h>

#define IPMB_SLAVE_ADDR_DEF         0x07
#define I2C_CHIP_ADDR_UCD9081       0x00
#define I2C_CHIP_ADDR_MAX6635_1     0x00
#define I2C_CHIP_ADDR_MAX6635_2     0x00
#define I2C_CHIP_ADDR_PCF8563       0x00
#define I2C_CHIP_ADDR_AT24CXX       0x00

#define I2C_STAT_IDLE               0       // 空闲状态
#define I2C_STAT_ADDR               1       // 发送数据地址状态
#define I2C_STAT_DATA               2       // 接收或发送数据状态
#define I2C_STAT_FINISH             3       // 收发完成状态

#define I2C_CMD_SEND                0
#define I2C_CMD_RECV                1

typedef struct
{
    uint8_t slave_addr;                     // The Chip Slaver Address, 7 bit only
    uint32_t reg_addr;                      // Chip Register Address
    uint32_t addr_size;                     // Chip Register Address size
    uint8_t *data_buf;                      // Data Buffer
    uint32_t data_size;                     // Data Size
} I2C_DEVICE;

typedef struct
{
    OS_EVENT *sem;                          //
    I2C_DEVICE *dev;                        //
    uint32_t addr_index;                    //
    uint32_t data_index;                    //
    uint8_t reg_addr[4];                    //
    uint8_t flags;                          //
    uint8_t status;
} I2C_DRIVER;

void I2C_init(void);
int I2C_dev_read(unsigned long dev, unsigned char slave_addr, unsigned long data_addr,
        unsigned char addr_size, unsigned long data_size, char *buf);
int I2C_dev_write(unsigned long dev, unsigned char slave_addr, unsigned long data_addr,
        unsigned char addr_size, unsigned long data_size, char *buf);

void I2C_i2c0_ipmb_init(void);
int I2C_i2c0_ipmb_read(char *buf, unsigned long *size);
int I2C_i2c0_ipmb_write(unsigned char slave_addr, char *buf, unsigned long size);
int I2C_i2c0_ipmb_self_addr_set(unsigned char self_addr);
char I2C_i2c0_ipmb_self_addr_get(void);
int I2C_i2c0_read_write(I2C_DEVICE *dev, uint8_t flags);
void I2C_i2c0_slave_dev_init(I2C_DEVICE *dev, uint8_t slave_addr, uint32_t addr_size);
void I2C_i2c0_slave_dev_set(I2C_DEVICE *dev, uint32_t reg_addr, uint8_t *data_buf, uint32_t data_size);
#define I2C_i2c0_master_read(d)     I2C_i2c0_read_write(d, I2C_CMD_RECV)
#define I2C_i2c0_master_write(d)    I2C_i2c0_read_write(d, I2C_CMD_SEND)

void I2C_i2c1_pmb_init(void);
int I2C_i2c1_pmb_read(unsigned slave_addr, unsigned long data_addr, unsigned char addr_size, char *buf, unsigned long size);
int I2C_i2c1_pmb_write(unsigned slave_addr, unsigned long data_addr, unsigned char addr_size, char *buf, unsigned long size);
int I2C_i2c1_read_write(I2C_DEVICE *dev, uint8_t flags);
void I2C_i2c1_slave_dev_init(I2C_DEVICE *dev, uint8_t slave_addr, uint32_t addr_size);
void I2C_i2c1_slave_dev_set(I2C_DEVICE *dev, uint32_t reg_addr, uint8_t *data_buf, uint32_t data_size);
#define I2C_i2c1_master_read(d)     I2C_i2c1_read_write(d, I2C_CMD_RECV)
#define I2C_i2c1_master_write(d)    I2C_i2c1_read_write(d, I2C_CMD_SEND)

void I2C_i2c1_soft_pmb_init(void);
int I2C_i2c1_soft_pmb_read(unsigned slave_addr, char *buf, unsigned long *size);
int I2C_i2c1_soft_pmb_write(unsigned slave_addr, char *buf, unsigned long size);



#endif // __LIB_I2C_H__

