//*****************************************************************************
//
// ipmi_chassis.h - IPMI Header File for Chassis
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#ifndef __IPMI_CHASSIS_H__
#define __IPMI_CHASSIS_H__

// �忨���ƣ�chassis control
#define IPMI_CHASSIS_CTL_POWER_DOWN	    0x0
#define IPMI_CHASSIS_CTL_POWER_UP	    0x1
#define IPMI_CHASSIS_CTL_POWER_CYCLE	0x2
#define IPMI_CHASSIS_CTL_HARD_RESET	    0x3
#define IPMI_CHASSIS_CTL_PULSE_DIAG	    0x4
#define IPMI_CHASSIS_CTL_ACPI_SOFT	    0x5

// ��Դ�ָ����ԣ�power restore policy
#define IPMI_CHASSIS_POLICY_NO_CHANGE	0x3
#define IPMI_CHASSIS_POLICY_ALWAYS_ON	0x2
#define IPMI_CHASSIS_POLICY_PREVIOUS	0x1
#define IPMI_CHASSIS_POLICY_ALWAYS_OFF	0x0
#define IPMI_CHASSIS_POLICY_DEFAULT     IPMI_CHASSIS_POLICY_ALWAYS_ON

// ��ǰ��Դ״̬��Current Power State
#define IPMI_CHASSIS_POW_STATE_POW_OFF  0x00
#define IPMI_CHASSIS_POW_STATE_POW_ON   0x01
#define IPMI_CHASSIS_POW_STATE_POW_OVER 0x02
#define IPMI_CHASSIS_POW_STATE_POW_LOCK 0x04
#define IPMI_CHASSIS_POW_STATE_POW_FALT 0x08
#define IPMI_CHASSIS_POW_STATE_POW_CFT  0x10
#define IPMI_CHASSIS_POW_STATE_DEFAULT  IPMI_CHASSIS_POW_STATE_POW_OFF

// ����Դ�¼���Last Power Event
#define IPMI_CHASSIS_EVN_ON_CMD         0x10
#define IPMI_CHASSIS_EVN_DOWN_FALT      0x08
#define IPMI_CHASSIS_EVN_DOWN_LOCK      0x04
#define IPMI_CHASSIS_EVN_DOWN_OVER      0x02
#define IPMI_CHASSIS_EVN_DOWN_AC        0x01

// �忨���ܣ�Capabilities Flags
#define IPMI_CHASSIS_PROV_POW_INTERLOCK 0x8
#define IPMI_CHASSIS_PROV_DIAG_INT      0x4
#define IPMI_CHASSIS_PROV_FRONT_P_LOCK  0x2
#define IPMI_CHASSIS_PROV_INTRUSION     0x1

// �忨״̬��Chassis State
#define IPMI_CHASSIS_STATE_FAN_FAULT    0x08        // ��⵽����״̬�쳣
#define IPMI_CHASSIS_STATE_DRIVE_FAULT  0x04        // �����쳣
#define IPMI_CHASSIS_STATE_FP_LOCK      0x02        // ǰ���������
#define IPMI_CHASSIS_STATE_INTRUSION    0x01        // �������ּ�⼤��

void chassis_power_policy_set(int policy);
void chassis_power_state_set(int state);
void chassis_last_power_event_set(int event);
void chassis_state_set(int state);

#endif  // __IPMI_CHASSIS_H__

