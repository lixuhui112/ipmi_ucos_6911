//*****************************************************************************
//
// ipmi_i2c.h
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#ifndef _IPMI_I2C_H_
#define _IPMI_I2C_H_

#include "ucos_ii.h"
#include "third_party/list.h"

//*****************************************************************************
//
// Set the address for slave module. This is a 7-bit address sent in the
// following format:
//                      [A6:A5:A4:A3:A2:A1:A0:RS]
//
// A zero in the "RS" position of the first byte means that the master
// transmits (sends) data to the selected slave, and a one in this position
// means that the master receives data from the slave.
//
//*****************************************************************************


//
// IPMI_I2C错误码
//
#define IPMI_I2C_ERR_ADDR       0x00000001
#define IPMI_I2C_ERR_INTER      0x00000002

//
// I2C总线结构体
//
typedef struct {
    struct list_head list;              // 双向循环链表
    unsigned long sys_peripheral;       // sysctl外设
    unsigned long i2c_scl_periph;       // I2C_SCL外设
    unsigned long i2c_sda_periph;       // I2C_SDA外设
    unsigned long i2c_scl_gpio_port;    // I2C_SCL GPIO端口
    unsigned long i2c_sda_gpio_port;    // I2C_SDA GPIO端口
    unsigned long i2c_scl_gpio_pin;     // I2C_SCL GPIO管脚
    unsigned long i2c_sda_gpio_pin;     // I2C_SDA GPIO管脚
    unsigned long i2c_scl_gpio_mux;     // I2C_SCL混杂GPIO配置
    unsigned long i2c_sda_gpio_mux;     // I2C_SDA混杂GPIO配置
    unsigned long i2c_hw_master_base;   // I2C硬件主设备基址
    unsigned long i2c_hw_slave_base;    // I2C硬件从设备基址
    unsigned long i2c_int;              // I2C中断向量
} ipmi_i2c_bus;

typedef struct {
    struct list_head head;              // 双向循环链表头节点
    unsigned long count;                // 总线计数
} ipmi_i2c_bus_head;


//
// I2C总线函数
//
int ipmi_i2c_bus_init(ipmi_i2c_bus *bus);


//
// I2C设备结构体
//
typedef struct {
    struct list_head list;              // 双向循环链表
    ipmi_i2c_bus  *bus;                 // I2C总线
    unsigned char slave_addr;           // 从机地址，7位纯地址，不含读写位
    unsigned long data_addr;            // 数据地址，从机的寄存器地址
    unsigned long data_size;            // 收发数据长度
    char          *buf_addr;            // 收发数据缓冲区的指针
} ipmi_i2c_dev;

typedef struct {
    struct list_head head;              // 双向循环链表头节点
    unsigned long count;                // 总线计数
} ipmi_i2c_dev_head;


//
// I2C设备函数
//
int ipmi_i2c_dev_init(ipmi_i2c_dev *dev, ipmi_i2c_bus *bus, unsigned char slave_addr);
int ipmi_i2c_dev_read(ipmi_i2c_dev *dev, unsigned long data_addr, unsigned long data_size, char *buf);
int ipmi_i2c_dev_write(ipmi_i2c_dev *dev, unsigned long data_addr, unsigned long data_size, char *buf);


//
// I2C虚拟从设备结构体
//
typedef struct {
    struct list_head list;              // 双向循环链表
    OS_EVENT *read_lock;                // 读信号量
    OS_EVENT *write_lock;               // 写信号量
    ipmi_i2c_bus  *bus;                 // I2C总线
    unsigned char my_addr;              // 虚拟设备的I2C地址
    unsigned long ram_reg_size;         // 虚拟设备的内存寄存器大小
    unsigned long ram_reg_addr;         // 准备要操作的寄存器地址
    char          *ram_reg;             // 虚拟设备的内存指针
} ipmi_i2c_ram_dev;

typedef struct {
    struct list_head head;              // 双向循环链表头节点
    unsigned long count;                // 总线计数
} ipmi_i2c_ram_dev_head;


//
// I2C虚拟从设备函数
//
int ipmi_i2c_ram_dev_init(ipmi_i2c_ram_dev *dev, ipmi_i2c_bus *bus,
        unsigned char addr, unsigned long reg_size, char *reg_buf);
void ipmi_i2c_ram_dev_service(ipmi_i2c_ram_dev *dev, unsigned long status);


//
// I2C模块初始化函数
//
void ipmi_i2c_module_init(void);


#endif

