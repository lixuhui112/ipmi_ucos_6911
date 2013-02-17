//*****************************************************************************
//
// ipmi_cfg.h
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#ifndef __IPMI_CFG_H__
#define __IPMI_CFG_H__

//*****************************************************************************
// IPMI��λ��ַ��Χ
//*****************************************************************************
#define IPMI_SLOT_ADDR_POWER_1      0x01
#define IPMI_SLOT_ADDR_POWER_2      0x02
#define IPMI_SLOT_ADDR_SWITCH_1     0x03
#define IPMI_SLOT_ADDR_SWITCH_2     0x04
#define IPMI_SLOT_ADDR_FABRIC_1     0x05
#define IPMI_SLOT_ADDR_SWITCH10G    0x06
#define IPMI_SLOT_ADDR_FABRIC_2     0x07
#define IPMI_SLOT_ADDR_SWITCH_3     0x08
#define IPMI_SLOT_ADDR_SWITCH_4     0x09
#define IPMI_SLOT_ADDR_FAN_1        0x0a
#define IPMI_SLOT_ADDR_FAN_2        0x0b
#define IPMI_SLOT_ADDR_MIN          IPMI_SLOT_ADDR_POWER_1
#define IPMI_SLOT_ADDR_MAX          IPMI_SLOT_ADDR_FAN_2


//*****************************************************************************
// IPMI��������ַ
//*****************************************************************************
#define AT24CXX_SLAVE_ADDR      0x54        // EEPROMоƬ��ַ

#define ADT7470_SLAVE_ADDR_1    0x2c        // ����״̬���оƬ��ַ
#define ADT7470_SLAVE_ADDR_2    0x2f        //

#define MAX6635_SLAVE_ADDR_1    0x48        // �¶ȴ�����оƬ��ַ1
#define MAX6635_SLAVE_ADDR_2    0x4a        //0x49        //

#define INA230_SLAVE_ADDR_1     0x40        // ��Դ���ѹ�������оƬ��ַ
#define INA230_SLAVE_ADDR_2     0x41

#define UCD9081_SLAVE_ADDR      0x65        // �������ѹ���оƬ��ַ

#define PCF8563_SLAVE_ADDR      0x51        // ����RTCоƬ��ַ

//*****************************************************************************
// IPMIĬ������MAC��ַ��IP��ַ
//*****************************************************************************
#define DEFAULT_MACADDR0        0x00b61a00
#define DEFAULT_MACADDR1        0x00740200

#define DEFAULT_IPADDR0         192
#define DEFAULT_IPADDR1         168
#define DEFAULT_IPADDR2         0
#define DEFAULT_IPADDR3         63

#define DEFAULT_NETMASK0        255
#define DEFAULT_NETMASK1        255
#define DEFAULT_NETMASK2        255
#define DEFAULT_NETMASK3        0

#define USE_STATIC_IP

//
// IPMI ��ʱ������
//
#define IPMI_SENSOR_SCAN_PERIOD 1           // scan sensor per 1 second
#define IPMI_SENSOR_DEAD_LIMIT  120         // sensor(mc-locator) dead after 120 second with no response

#endif  // __IPMI_CFG_H__

