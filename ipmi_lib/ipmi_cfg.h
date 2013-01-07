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

//
// IPMI��������ַ
//
#define AT24CXX_SLAVE_ADDR      0x54        // EEPROMоƬ��ַ

#define ADT7470_SLAVE_ADDR_1    0x2c        // ����״̬���оƬ��ַ
#define ADT7470_SLAVE_ADDR_2    0x2f        //

#define MAX6635_SLAVE_ADDR_1    0x48        // �¶ȴ�����оƬ��ַ1
#define MAX6635_SLAVE_ADDR_2    0x49        //

#define INA230_SLAVE_ADDR_1     0x66        // ��Դ���ѹ�������оƬ��ַ
#define INA230_SLAVE_ADDR_2     0x67

#define UCD9081_SLAVE_ADDR      0x65        // �������ѹ���оƬ��ַ

#define PCF8563_SLAVE_ADDR      0x51        // ����RTCоƬ��ַ

//
// IPMIĬ������MAC��ַ��IP��ַ
//
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

#endif  // __IPMI_CFG_H__

