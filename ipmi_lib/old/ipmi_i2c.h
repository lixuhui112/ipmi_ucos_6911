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
// IPMI_I2C������
//
#define IPMI_I2C_ERR_ADDR       0x00000001
#define IPMI_I2C_ERR_INTER      0x00000002

//
// I2C���߽ṹ��
//
typedef struct {
    struct list_head list;              // ˫��ѭ������
    unsigned long sys_peripheral;       // sysctl����
    unsigned long i2c_scl_periph;       // I2C_SCL����
    unsigned long i2c_sda_periph;       // I2C_SDA����
    unsigned long i2c_scl_gpio_port;    // I2C_SCL GPIO�˿�
    unsigned long i2c_sda_gpio_port;    // I2C_SDA GPIO�˿�
    unsigned long i2c_scl_gpio_pin;     // I2C_SCL GPIO�ܽ�
    unsigned long i2c_sda_gpio_pin;     // I2C_SDA GPIO�ܽ�
    unsigned long i2c_scl_gpio_mux;     // I2C_SCL����GPIO����
    unsigned long i2c_sda_gpio_mux;     // I2C_SDA����GPIO����
    unsigned long i2c_hw_master_base;   // I2CӲ�����豸��ַ
    unsigned long i2c_hw_slave_base;    // I2CӲ�����豸��ַ
    unsigned long i2c_int;              // I2C�ж�����
} ipmi_i2c_bus;

typedef struct {
    struct list_head head;              // ˫��ѭ������ͷ�ڵ�
    unsigned long count;                // ���߼���
} ipmi_i2c_bus_head;


//
// I2C���ߺ���
//
int ipmi_i2c_bus_init(ipmi_i2c_bus *bus);


//
// I2C�豸�ṹ��
//
typedef struct {
    struct list_head list;              // ˫��ѭ������
    ipmi_i2c_bus  *bus;                 // I2C����
    unsigned char slave_addr;           // �ӻ���ַ��7λ����ַ��������дλ
    unsigned long data_addr;            // ���ݵ�ַ���ӻ��ļĴ�����ַ
    unsigned long data_size;            // �շ����ݳ���
    char          *buf_addr;            // �շ����ݻ�������ָ��
} ipmi_i2c_dev;

typedef struct {
    struct list_head head;              // ˫��ѭ������ͷ�ڵ�
    unsigned long count;                // ���߼���
} ipmi_i2c_dev_head;


//
// I2C�豸����
//
int ipmi_i2c_dev_init(ipmi_i2c_dev *dev, ipmi_i2c_bus *bus, unsigned char slave_addr);
int ipmi_i2c_dev_read(ipmi_i2c_dev *dev, unsigned long data_addr, unsigned long data_size, char *buf);
int ipmi_i2c_dev_write(ipmi_i2c_dev *dev, unsigned long data_addr, unsigned long data_size, char *buf);


//
// I2C������豸�ṹ��
//
typedef struct {
    struct list_head list;              // ˫��ѭ������
    OS_EVENT *read_lock;                // ���ź���
    OS_EVENT *write_lock;               // д�ź���
    ipmi_i2c_bus  *bus;                 // I2C����
    unsigned char my_addr;              // �����豸��I2C��ַ
    unsigned long ram_reg_size;         // �����豸���ڴ�Ĵ�����С
    unsigned long ram_reg_addr;         // ׼��Ҫ�����ļĴ�����ַ
    char          *ram_reg;             // �����豸���ڴ�ָ��
} ipmi_i2c_ram_dev;

typedef struct {
    struct list_head head;              // ˫��ѭ������ͷ�ڵ�
    unsigned long count;                // ���߼���
} ipmi_i2c_ram_dev_head;


//
// I2C������豸����
//
int ipmi_i2c_ram_dev_init(ipmi_i2c_ram_dev *dev, ipmi_i2c_bus *bus,
        unsigned char addr, unsigned long reg_size, char *reg_buf);
void ipmi_i2c_ram_dev_service(ipmi_i2c_ram_dev *dev, unsigned long status);


//
// I2Cģ���ʼ������
//
void ipmi_i2c_module_init(void);


#endif

