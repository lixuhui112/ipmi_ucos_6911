//*****************************************************************************
//
// ipmi_smic.h
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#ifndef _IPMI_SMIC_H_
#define _IPMI_SMIC_H_

#include <inttypes.h>



#define SMIC_READ_REG       0x000           /* read register  0x000 0       */
#define SMIC_WRITE_REG      0x100           /* write register 0x100 256     */

#define SMIC_REG_SIZE       0x100           /* register size  0x100 256     */

#define SMIC_SLAVE_ADDR     0x1F            /* 虚拟的SMIC设备在I2C总线中的地址
                                             * 此地址仅为CPU使用，不在IPMB总线中使用
                                             * 因此不会造成冲突问题 */

typedef struct ipmi_smic_intf
{
    unsigned char smic_intf_slave_addr;     /* SMIC接口从地址 */
    unsigned char reg_buf[SMIC_REG_SIZE*2]; /* 寄存器缓冲 */
    char *req_buf;                          /* 请求寄存器指针 */
    char *rsp_buf;                          /* 响应寄存器指针 */
} ipmi_smic_intf_t;

typedef struct ipmi_smic_req                /* SMIC请求结构体 */
{
    uint8_t netfn:6;
    uint8_t lun:2;
    uint8_t cmd;
    uint8_t datalength;
    uint8_t data[1];
} ipmi_smic_req_t;

typedef struct ipmi_smic_rsp                /* SMIC响应结构体 */
{
    uint8_t netfn:6;
    uint8_t lun:2;
    uint8_t cmd;
    uint8_t ccode;
    uint8_t datalength;
    uint8_t data[1];
} ipmi_smic_rsp_t;


int ipmi_smic_intf_init(void);


#endif

