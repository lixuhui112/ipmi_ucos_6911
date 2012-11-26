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
#define AT24C_SLAVE_ADDR        0x54        // EEPROMоƬ��ַ

#define ADT7470_SLAVE_ADDR_1    0x2c        // ����״̬���оƬ��ַ
#define ADT7470_SLAVE_ADDR_2    0x2f        //

#define MAX6635_SLAVE_ADDR_1    0x48        // �¶ȴ�����оƬ��ַ1
#define MAX6635_SLAVE_ADDR_2    0x49        //

#define INA230_SLAVE_ADDR       0x00        // ��Դ���ѹ�������оƬ��ַ

#define UCD9081_SLAVE_ADDR      0x65        // �������ѹ���оƬ��ַ

#define PCF8563_SLAVE_ADDR      0x51        // ����RTCоƬ��ַ



#endif  // __IPMI_CFG_H__

