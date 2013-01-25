//*****************************************************************************
//
// ipmi_cmd.h - IPMI Command Header File
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#ifndef __IPMI_CMD_H__
#define __IPMI_CMD_H__


//*****************************************************************************
//
// Defines IPMI Command ID for IPM Device Global
//
//*****************************************************************************
#define BMC_GET_DEVICE_ID	            0x01
#define BMC_COLD_RESET		            0x02
#define BMC_WARM_RESET		            0x03
#define BMC_GET_SELF_TEST	            0x04
#define BMC_MANUFACTURING_TEST_ON       0x05
#define SET_ACPI_POWER_STATE            0x06
#define GET_ACPI_POWER_STATE            0x07
#define GET_DEVICE_GUID                 0x08
#define GET_NETFN_SUPPORT               0x09
#define GET_COMMAND_SUPPORT             0x0A
#define GET_COMMAND_SUB_FUN_SUPPORT     0x0B
#define GET_CONFIG_COMMANDS             0x0C
#define GET_CONFIG_COMMAND_SUB_FUNC     0x0D
#define SET_COMMAND_ENABLES             0x60
#define GET_COMMAND_ENABLES             0x61
#define SET_COMMAND_SUB_FUNC_ENABLES    0x62
#define GET_COMMAND_SUB_FUNC_ENABLES    0x63
#define GET_OEM_NETFN_IANA_SUPPORT      0x64


//*****************************************************************************
//
// Defines IPMI Command ID for BMC Watchdog Timer
//
//*****************************************************************************
#define BMC_RESET_WATCHDOG_TIMER	    0x22
#define BMC_SET_WATCHDOG_TIMER	        0x24
#define BMC_GET_WATCHDOG_TIMER	        0x25


//*****************************************************************************
//
// Defines IPMI Command ID for BMC Device and Messaging
//
//*****************************************************************************
#define BMC_SET_GLOBAL_ENABLES	        0x2E
#define BMC_GET_GLOBAL_ENABLES	        0x2F
#define CLEAR_MESSAGE_FLAGS             0x30
#define GET_MESSAGE_FLAGS               0x31
#define ENABLE_MESSAGE_CHANNEL_RECEIVE  0x32
#define GET_MESSAGE                     0x33
#define SEND_MESSAGE                    0x34
#define READ_EVENT_MESSAGE_BUFFER       0x35
#define GET_BT_INTERFACE_CAPABILITIES   0x36
#define GET_SYSTEM_GUID                 0x37
#define SET_SYSTEM_INFO_PARAMETERS      0x58
#define GET_SYSTEM_INFO_PARAMETERS      0x59
#define GET_CHANNEL_AUTH_CAPABILITIES   0x38
#define GET_SESSION_CHALLENGE           0x39
#define ACTIVATE_SESSION                0x3A
#define SET_SESSION_PRIVILEGE_LEVEL     0x3B
#define CLOSE_SESSION                   0x3C
#define GET_SESSION_INFO                0x3D
#define GET_AUTHCODE                    0x3F
#define SET_CHANNEL_ACCESS              0x40
#define GET_CHANNEL_ACCESS              0x41
#define GET_CHANNEL_INFO_COMMAND        0x42
#define SET_USER_ACCESS_COMMAND         0x43
#define GET_USER_ACCESS_COMMAND         0x44
#define SET_USER_NAME                   0x45
#define GET_USER_NAME_COMMAND           0x46
#define SET_USER_PASSWORD_COMMAND       0x47
#define ACTIVATE_PAYLOAD                0x48
#define DEACTIVATE_PAYLOAD              0x49
#define GET_PAYLOAD_ACTIVATION_STATUS   0x4A
#define GET_PAYLOAD_INSTANCE_INFO       0x4B
#define SET_USER_PAYLOAD_ACCESS         0x4C
#define GET_USER_PAYLOAD_ACCESS         0x4D
#define GET_CHANNEL_PAYLOAD_SUPPORT     0x4E
#define GET_CHANNEL_PAYLOAD_VERSION     0x4F
#define GET_CHANNEL_OEM_PAYLOAD_INFO    0x50
#define MASTER_WRITE_READ               0x52
#define GET_CHANNEL_CIPHER_SUITES       0x54
#define SR_PAYLOAD_ENCRYPTION           0x55
#define SET_CHANNEL_SECURITY_KEYS       0x56
#define GET_SYSTEM_INTF_CAPABILITIES    0x57


//*****************************************************************************
//
// Defines IPMI Command ID for Chassis Device
//
//*****************************************************************************
#define CHASSIS_GET_CAPABILITIES        0x00
#define CHASSIS_GET_STATUS              0x01
#define CHASSIS_CONTROL                 0x02
#define CHASSIS_RESET                   0x03
#define CHASSIS_IDENTIFY                0x04
#define CHASSIS_SET_CAPABILITIES        0x05
#define CHASSIS_SET_POW_RESTORE_POLICY  0x06
#define CHASSIS_GET_RESTART_CAUSE       0x07
#define CHASSIS_SET_SYS_BOOT_OPT        0x08
#define CHASSIS_GET_SYS_BOOT_OPT        0x09
#define CHASSIS_SET_FP_BUTTON_ENABLE    0x0a
#define CHASSIS_SET_POW_CYCLE_INTERVAL  0x0b
#define CHASSIS_GET_POH_COUNTER         0x0f


//*****************************************************************************
//
// Defines IPMI Command ID for Event
//
//*****************************************************************************
#define SET_EVENT_RECEIVER              0x00
#define GET_EVENT_RECEIVER              0x01
#define PLATFORM_EVENT                  0x02


//*****************************************************************************
//
// Defines IPMI Command ID for PEF and Alerting
//
//*****************************************************************************
#define GET_PEF_CAPABILITIES            0x10
#define ARM_PEF_POSTPONE_TIMER          0x11
#define SET_PEF_CONFIG_PARAM            0x12
#define GET_PEF_CONFIG_PARAM            0x13
#define SET_LAST_PROCESSED_EVENT_ID     0x14
#define GET_LAST_PROCESSED_EVENT_ID     0x15
#define ALERT_IMMEDIATE                 0x16
#define PET_ACKNOWLEDGE                 0x17


//*****************************************************************************
//
// Defines IPMI Command ID for Sensor Device
//
//*****************************************************************************
#define GET_DEVICE_SDR_INFO             0x20
#define GET_DEVICE_SDR                  0x21
#define GET_SDR_RESERVE_REPO            0x22
#define GET_SENSOR_FACTORS              0x23
#define SET_SENSOR_HYSTERESIS	        0x24
#define GET_SENSOR_HYSTERESIS	        0x25
#define SET_SENSOR_THRESHOLDS           0x26
#define GET_SENSOR_THRESHOLDS           0x27
#define SET_SENSOR_EVENT_ENABLE	        0x28
#define GET_SENSOR_EVENT_ENABLE         0x29
#define RE_ARM_SENSOR_EVENTS            0x2a
#define GET_SENSOR_EVENT_STATUS	        0x2b
#define GET_SENSOR_READING	            0x2d
#define SET_SENSOR_TYPE                 0x2e
#define GET_SENSOR_TYPE		            0x2f
#define SET_SENSOR_READING_STATUS       0x30


//*****************************************************************************
//
// Defines IPMI Command ID for FRU Device
//
//*****************************************************************************
#define GET_FRU_INVENTORY_AREA_INFO     0x10
#define READ_FRU_DATA                   0x11
#define WRITE_FRU_DATA                  0x12


//*****************************************************************************
//
// Defines IPMI Command ID for SDR Device
//
//*****************************************************************************
#define GET_SDR_REPOS_INFO              0x20
#define GET_SDR_REPOS_ALLOC_INFO        0x21
#define RESERVE_SDR_REPOSITORY          0x22
#define GET_SDR                         0x23
#define ADD_SDR                         0x24
#define PARTIAL_ADD_SDR                 0x25
#define DELETE_SDR                      0x26
#define CLEAR_SDR_REPOSITORY            0x27
#define GET_SDR_REPOSITORY_TIME         0x28
#define SET_SDR_REPOSITORY_TIME         0x29
#define ENTER_SDR_REPOS_UPDATE_MODE     0x2A
#define EXIT_SDR_REPOS_UPDATE_MODE      0x2B
#define RUN_INITIALIZATION_AGENT        0x2C


//*****************************************************************************
//
// Defines IPMI Command ID for SEL Device
//
//*****************************************************************************
#define GET_SEL_INFO                    0x40
#define GET_SEL_ALLOCATION_INFO         0x41
#define RESERVE_SEL                     0x42
#define GET_SEL_ENTRY                   0x43
#define ADD_SEL_ENTRY                   0x44
#define PARTIAL_ADD_SEL_ENTRY           0x45
#define DELETE_SEL_ENTRY                0x46
#define CLEAR_SEL                       0x47
#define GET_SEL_TIME                    0x48
#define SET_SEL_TIME                    0x49
#define GET_AUXILIARY_LOG_STATUS        0x5A
#define SET_AUXILIARY_LOG_STATUS        0x5B
#define GET_SEL_TIME_UTC_OFFSET         0x5C
#define SET_SEL_TIME_UTC_OFFSET         0x5D


//*****************************************************************************
//
// Defines IPMI Command ID for Transport
//
//*****************************************************************************
#define SET_LAN_CONFIG_PARAM            0x01
#define GET_LAN_CONFIG_PARAM            0x02
#define SUSPEND_BMC_ARPS                0x03
#define GET_IP_UDP_RMCP_STATISTICS      0x04
#define SET_SERIAL_MODEM_CONFIG         0x10
#define GET_SERIAL_MODEM_CONFIG         0x11
#define SET_SERIAL_MODEM_MUX            0x12
#define GET_TAP_RESPONSE_CODES          0x13
#define SET_PPP_UDP_PROXY_TRANSMIT_DATA 0x14
#define GET_PPP_UDP_PROXY_TRANSMIT_DATA 0x15
#define SEND_PPP_UDP_PROXY_PACKET       0x16
#define GET_PPP_UDP_PROXY_RECEIVE_DATA  0x17
#define SERIAL_MODEM_CONNECTION_ACTIVE  0x18
#define CALLBACK                        0x19
#define SET_USER_CALLBACK_OPTIONS       0x1A
#define GET_USER_CALLBACK_OPTIONS       0x1B
#define SET_SERIAL_ROUTING_MUX          0x1C
#define SOL_ACTIVATING                  0x20
#define SET_SOL_CONFIG_PARAM            0x21
#define GET_SOL_CONFIG_PARAM            0x22
#define FORWARDED_COMMAND               0x30
#define SET_FORWARDED_COMMANDS          0x31
#define GET_FORWARDED_COMMANDS          0x32
#define ENABLE_FORWARDED_COMMANDS       0x33
#define NOTIFY_RMC_INFO                 0x40

//*****************************************************************************
//
// Defines IPMI Command ID for Bridge
//
//*****************************************************************************
#define GET_BRIDGE_STATE                0x00
#define SET_BRIDGE_STATE                0x01
#define GET_ICMB_ADDRESS                0x02
#define SET_ICMB_ADDRESS                0x03
#define SET_BRIDGE_PROXYADDRESS         0x04
#define GET_BRIDGE_STATISTICS           0x05
#define GET_ICMB_CAPABILITIES           0x06
#define CLEAR_BRIDGE_STATISTICS         0x08
#define GET_BRIDGE_PROXY_ADDRESS        0x09
#define GET_ICMB_CONNECTOR_INFO         0x0A
#define GET_ICMB_CONNECTION_ID          0x0B
#define SEND_ICMB_CONNECTION_ID         0x0C
#define PREPAREFORDISCOVERY             0x10
#define GETADDRESSES                    0x11
#define SETDISCOVERED                   0x12
#define GETCHASSISDEVICEID              0x13
#define SETCHASSISDEVICEID              0x14
#define BRIDGEREQUEST                   0x20
#define BRIDGEMESSAGE                   0x21
#define GETEVENTCOUNT                   0x30
#define SETEVENTDESTINATION             0x31
#define SETEVENTRECEPTIONSTATE          0x32
#define SENDICMBEVENTMESSAGE            0x33
#define GETEVENTDESTINATION             0x34
#define GETEVENTRECEPTIONSTATE          0x35
#define ERROR_REPORT                    0xFF


//*****************************************************************************
//
// Defines IPMI Command ID for PICMG commands
//
//*****************************************************************************
#define PICMG_GET_PICMG_PROPERTIES_CMD             0x00
#define PICMG_GET_ADDRESS_INFO_CMD                 0x01
#define PICMG_GET_SHELF_ADDRESS_INFO_CMD           0x02
#define PICMG_SET_SHELF_ADDRESS_INFO_CMD           0x03
#define PICMG_FRU_CONTROL_CMD                      0x04
#define PICMG_GET_FRU_LED_PROPERTIES_CMD           0x05
#define PICMG_GET_LED_COLOR_CAPABILITIES_CMD       0x06
#define PICMG_SET_FRU_LED_STATE_CMD                0x07
#define PICMG_GET_FRU_LED_STATE_CMD                0x08
#define PICMG_SET_IPMB_CMD                         0x09
#define PICMG_SET_FRU_POLICY_CMD                   0x0A
#define PICMG_GET_FRU_POLICY_CMD                   0x0B
#define PICMG_FRU_ACTIVATION_CMD                   0x0C
#define PICMG_GET_DEVICE_LOCATOR_RECORD_CMD        0x0D
#define PICMG_SET_PORT_STATE_CMD                   0x0E
#define PICMG_GET_PORT_STATE_CMD                   0x0F
#define PICMG_COMPUTE_POWER_PROPERTIES_CMD         0x10
#define PICMG_SET_POWER_LEVEL_CMD                  0x11
#define PICMG_GET_POWER_LEVEL_CMD                  0x12
#define PICMG_RENEGOTIATE_POWER_CMD                0x13
#define PICMG_GET_FAN_SPEED_PROPERTIES_CMD         0x14
#define PICMG_SET_FAN_LEVEL_CMD                    0x15
#define PICMG_GET_FAN_LEVEL_CMD                    0x16
#define PICMG_BUSED_RESOURCE_CMD                   0x17


//*****************************************************************************
//
// Defines IPMI Command handle
//
//*****************************************************************************
int ipmi_cmd_chassis(struct ipmi_ctx *ctx_cmd);
int ipmi_cmd_bridge(struct ipmi_ctx *ctx_cmd);
int ipmi_cmd_se(struct ipmi_ctx *ctx_cmd);
int ipmi_cmd_app(struct ipmi_ctx *ctx_cmd);
int ipmi_cmd_firmware(struct ipmi_ctx *ctx_cmd);
int ipmi_cmd_storage(struct ipmi_ctx *ctx_cmd);
int ipmi_cmd_transport(struct ipmi_ctx *ctx_cmd);
int ipmi_cmd_picmg(struct ipmi_ctx *ctx_cmd);

#endif  // __IPMI_CMD_H__

