//*****************************************************************************
//
// ipmi.h
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#ifndef _IPMI_H_
#define _IPMI_H_

#include <inttypes.h>
#include "inc/hw_types.h"

/* NetFn
 * See IPMI specification table 5-1 Network Function Codes
 */
#define IPMI_NETFN_CHASSIS                          0x0
#define IPMI_NETFN_BRIDGE                           0x2
#define IPMI_NETFN_SE                               0x4
#define IPMI_NETFN_APP                              0x6
#define IPMI_NETFN_FIRMWARE                         0x8
#define IPMI_NETFN_STORAGE                          0xa
#define IPMI_NETFN_TRANSPORT                        0xc
#define IPMI_NETFN_PICMG                            0x2c
#define IPMI_NETFN_DCGRP                            0x2c
#define IPMI_NETFN_ISOL                             0x34
#define IPMI_NETFN_TSOL                             0x30

#define IPMI_BMC_SLAVE_ADDR                         0x20
#define IPMI_REMOTE_SWID                            0x81


/* CC
 * See IPMI specification table 5-2 Generic Completion Codes
 */
#define IPMI_CC_OK                                  0x00    // "Command completed normally"
#define IPMI_CC_NODE_BUSY                           0xc0    // "Node busy"
#define IPMI_CC_INV_CMD                             0xc1    // "Invalid command"
#define IPMI_CC_INV_CMD_FOR_LUN                     0xc2    // "Invalid command on LUN"
#define IPMI_CC_TIMEOUT                             0xc3    // "Timeout"
#define IPMI_CC_OUT_OF_SPACE                        0xc4    // "Out of space"
#define IPMI_CC_RES_CANCELED                        0xc5    // "Reservation cancelled or invalid"
#define IPMI_CC_REQ_DATA_TRUNC                      0xc6    // "Request data truncated"
#define IPMI_CC_REQ_DATA_INV_LENGTH                 0xc7    // "Request data length invalid"
#define IPMI_CC_REQ_DATA_FIELD_EXCEED               0xc8    // "Request data field length limit exceeded"
#define IPMI_CC_PARAM_OUT_OF_RANGE                  0xc9    // "Parameter out of range"
#define IPMI_CC_CANT_RET_NUM_REQ_BYTES              0xca    // "Cannot return number of requested data bytes"
#define IPMI_CC_REQ_DATA_NOT_PRESENT                0xcb    // "Requested sensor, data, or record not found"
#define IPMI_CC_INV_DATA_FIELD_IN_REQ               0xcc    // "Invalid data field in request"
#define IPMI_CC_ILL_SENSOR_OR_RECORD                0xcd    // "Command illegal for specified sensor or record type"
#define IPMI_CC_RESP_COULD_NOT_BE_PRV               0xce    // "Command response could not be provided"
#define IPMI_CC_CANT_RESP_DUPLI_REQ                 0xcf    // "Cannot execute duplicated request"
#define IPMI_CC_CANT_RESP_SDRR_UPDATE               0xd0    // "SDR Repository in update mode"
#define IPMI_CC_CANT_RESP_FIRM_UPDATE               0xd1    // "Device firmeware in update mode"
#define IPMI_CC_CANT_RESP_BMC_INIT                  0xd2    // "BMC initialization in progress"
#define IPMI_CC_DESTINATION_UNAVAILABLE             0xd3    // "Destination unavailable"
#define IPMI_CC_INSUFFICIENT_PRIVILEGES             0xd4    // "Insufficient privilege level"
#define IPMI_CC_NOT_SUPPORTED_PRESENT_STATE         0xd5    // "Command not supported in present state"
#define IPMI_CC_ILLEGAL_COMMAND_DISABLED            0xd6    // "Cannot execute command, command disabled"
#define IPMI_CC_UNSPECIFIED_ERROR                   0xff    // "Unspecified error"
#define IPMI_CC_RECORD_REJECTED_DUE_TO_MISMATCH     0x80    // "Record rejected due to mismatch between record length in header data and number of bytes written."
#define IPMI_CC_SEL_ERASE_PROGRESS                  0x81    // "Cannot execute command, SEL erase in progress"

/* CHANNEL NUMBERS */
/* Each interface has a channel number that is used when configuring the channel
 * and for routing messages between channels. Only the channel number assignments
 * for the primary IPMB and the System Interface are fixed, the assignment of
 * other channel numbers can vary on a per-platform basis. Software uses a Get
 * Channel Info command to determine what types of channels are available and
 * what channel number assignments are used on a given platform.
 */
/* CH_NUM
 * See IPMI specification Table 6-1, Channel Number Assignments
 */
#define IPMI_CH_NUM_PRIMARY_IPMB                    0x0
#define IPMI_CH_NUM_CONSOLE                         0x1
#define IPMI_CH_NUM_ICMB                            0x2
#define IPMI_CH_NUM_PMB                             0x3
#define IPMI_CH_NUM_LAN                             0x7
#define IPMI_CH_NUM_PRESENT_INTERFACE               0xE
#define IPMI_CH_NUM_SYS_INTERFACE                   0xF

/* CHANNEL PROTOCOL TYPE */
/* The protocol used for transferring IPMI messages on a given channel is
 * identified using a channel protocol type number.
 */
/* CH_PROTOCOL
 * See IPMI specification Table 6-2, Channel Protocol Type Numbers
 */
#define IPMI_CH_PROTOCOL_IPMB                       0x1     /* used for IPMB, serial/modem Basic Mode, and LAN */
#define IPMI_CH_PROTOCOL_ICMB                       0x2     /* ICMB v1.0 */
#define IPMI_CH_PROTOCOL_SMB                        0x4     /* IPMI on SMSBus */
#define IPMI_CH_PROTOCOL_KCS                        0x5     /* KCS System Interface Format */
#define IPMI_CH_PROTOCOL_SMIC                       0x6     /* SMIC System Interface Format */
#define IPMI_CH_PROTOCOL_BT10                       0x7     /* BT System Interface Format, IPMI v1.0 */
#define IPMI_CH_PROTOCOL_BT15                       0x8     /* BT System Interface Format, IPMI v1.5 */
#define IPMI_CH_PROTOCOL_TMODE                      0x9     /* Terminal Mode */
#define IPMI_CH_PROTOCOL_NONE                       0xf     /* Invalid protocol type */

/* CHANNEL MEDIUM TYPE */
/* The Channel Medium Type number is a seven-bit value that identifies the
 * general class of medium that is being used for the channel.
 */
/* CH_MEDIUM
 * See IPMI specification Table 6-3, Channel Medium Type Numbers
 */
#define IPMI_CH_MEDIUM_IPMB                         0x1     /* IPMB (I2C)                        */
#define IPMI_CH_MEDIUM_ICMB10                       0x2     /* ICMB v1.0                         */
#define IPMI_CH_MEDIUM_ICMB09                       0x3     /* ICMB v0.9                         */
#define IPMI_CH_MEDIUM_LAN                          0x4     /* 802.3 LAN                         */
#define IPMI_CH_MEDIUM_SERIAL                       0x5     /* Asynch. Serial/Modem (RS-232)     */
#define IPMI_CH_MEDIUM_LAN_AUX                      0x6     /* Other LAN                         */
#define IPMI_CH_MEDIUM_PCI_SMB                      0x7     /* PCI SMBus                         */
#define IPMI_CH_MEDIUM_SMB_1x                       0x8     /* SMBus v1.0/1.1                    */
#define IPMI_CH_MEDIUM_SMB_20                       0x9     /* SMBus v2.0                        */
#define IPMI_CH_MEDIUM_USB_1x                       0xA     /* reserved for USB 1.x              */
#define IPMI_CH_MEDIUM_USB_20                       0xB     /* reserved for USB 2.x              */
#define IPMI_CH_MEDIUM_SYS                          0xC     /* System Interface (KCS, SMIC, or BT)   */


/* IPMI Sensor Type
 * See IPMI specification Table 42-3, Sensor Type Codes
 */
#define SENSOR_TYPE_TEMPERATURE                     0x01    // Temperature
#define SENSOR_TYPE_VOLTAGE                         0x02    // Voltage
#define SENSOR_TYPE_CURRENT                         0x03    // Current
#define SENSOR_TYPE_FAN                             0x04    // Fan
#define SENSOR_TYPE_PS                              0x05    // Physical Security (Chassis Intrusion)
#define SENSOR_TYPE_PSVA                            0x06    // Platform Security Violation Attempt
#define SENSOR_TYPE_PROCESSOR                       0x07    // Processor
#define SENSOR_TYPE_POW_SU                          0x08    // Power Supply
#define SENSOR_TYPE_BUTTON_SWITCH                   0x14    // Button / Switch

/* Entity ID Codes
 * See IPMI specification Table 43-13, Entity ID Codes
 */
#define ENTITY_ID_UNSPECIFIED                       0x00    /* unspecified */
#define ENTITY_ID_OTHER                             0x01    /* other */
#define ENTITY_ID_UNKNOWN                           0x02    /* unknown (unspecified) */
#define ENTITY_ID_PROCESSOR                         0x03    /* processor */
#define ENTITY_ID_DISK                              0x04    /* disk or disk bay */
#define ENTITY_ID_PERIPHERAL_BAY                    0x05    /* peripheral bay */
#define ENTITY_ID_SYS_MGMT_MODULE                   0x06    /* system management module */
#define ENTITY_ID_SYSTEM_BOARD                      0x07    /* system board (main system board, may
                                                               also be a processor board and/or internal
                                                               expansion board) */
#define ENTITY_ID_MEM_MODULE                        0x08    /* memory module (board holding memory devices) */
#define ENTITY_ID_PROCESSOR_MODULE                  0x09    /* processor module (holds processors,
                                                               use this designation when processors
                                                               are not mounted on system board) */
#define ENTITY_ID_POWER_SUPPLY                      0x0A    /* power supply (DMI refers to this
                                                               as a power unit, but its used
                                                               to represent a power supply).
                                                               Use this value for the main power
                                                               supply (supplies) for the system. */
#define ENTITY_ID_ADD_IN_CARD                       0x0B    /* add-in card */
#define ENTITY_ID_FRONT_PANEL_BOARD                 0x0C    /* front panel board (control panel) */
#define ENTITY_ID_BACK_PANEL_BOARD                  0x0D    /* back panel board */
#define ENTITY_ID_POWER_SYSTEM_BOARD                0x0E    /* power system board */
#define ENTITY_ID_DRIVE_BACKPLANE                   0x0F    /* drive backplane */
#define ENTITY_ID_INT_EXPANSION_BOARD               0x10    /* system internal expansion board
                                                               (contains expansion slots). */
#define ENTITY_ID_OTHER_SYS_BOARD                   0x11    /* Other system board (part of board set) */
#define ENTITY_ID_PROCESSOR_BOARD                   0x12    /* processor board (holds 1 or more
                                                               processors - includes boards that
                                                               hold SECC modules) */
#define ENTITY_ID_POWER_UNIT                        0x13    /* power unit / power domain - This
                                                               Entity ID is typically used as a
                                                               pre-defined logical entity for
                                                               grouping power supplies. */
#define ENTITY_ID_POWER_MODULE                      0x14    /* power module / DC-to-DC converter
                                                               - Use this value for internal converters.
                                                               Note: You should use Entity ID 10
                                                               (power supply) for the main power
                                                               supply even if the main supply is
                                                               a DC-to-DC converter, e.g. gets
                                                               external power from a -48 DC source. */
#define ENTITY_ID_POWER_MGMT_BOARD                  0x15    /* power management / power distribution board */
#define ENTITY_ID_CHASSIS_BACKPLANE                 0x16    /* chassis back panel board */
#define ENTITY_ID_SYSTEM_CHASSIS                    0x17    /* system chassis */
#define ENTITY_ID_SUB_CHASSIS                       0x18    /* sub-chassis */
#define ENTITY_ID_OTHER_CHASSIS_BOARD               0x19    /* Other chassis board */
#define ENTITY_ID_DISK_DRIVE_BAY                    0x1A    /* Disk Drive Bay */
#define ENTITY_ID_PERIPHERAL_BAY2                   0x1B    /* Peripheral Bay */
#define ENTITY_ID_DEVICE_BAY                        0x1C    /* Device Bay */
#define ENTITY_ID_FAN                               0x1D    /* fan / cooling device */
#define ENTITY_ID_COOLING_UNIT                      0x1E    /* cooling unit - This Entity ID can
                                                               be used as a pre-defined logical entity
                                                               for grouping fans or other cooling devices. */
#define ENTITY_ID_CABLE_INTERCONNECT                0x1F    /* cable / interconnect */
#define ENTITY_ID_MEMORY_DEVICE                     0x20    /* memory device -This Entity ID
                                                               should be used for replaceable
                                                               memory devices, e.g. DIMM/SIMM.
                                                               It is recommended that Entity IDs
                                                               not be used for individual non-replaceable
                                                               memory devices. Rather, monitoring and
                                                               error reporting should be associated
                                                               with the FRU [e.g. memory card] holding
                                                               the memory. */
#define ENTITY_ID_SYS_MGMT_SOFTWARE                 0x21    /* System Management Software */
#define ENTITY_ID_BIOS                              0x22    /* BIOS */
#define ENTITY_ID_OPERATING_SYSTEM                  0x23    /* Operating System */
#define ENTITY_ID_SYSTEM_BUS                        0x24    /* system bus */
#define ENTITY_ID_GROUP                             0x25    /* Group - This is a logical entity
                                                               for use with Entity Association records.
                                                               It is provided to allow an Entity association
                                                               record to define a grouping of entities when
                                                               there is no appropriate pre-defined entity
                                                               for the container entity. This Entity should
                                                               not be used as a physical entity. */
#define ENTITY_ID_REMOTE_MGMT_COMM_DEV              0x26    /* Remote (Out of Band) Management Communication Device */
#define ENTITY_ID_EXT_ENVIRONMENT                   0x27    /* External Environment - This Entity
                                                               ID can be used to identify the environment
                                                               outside the system chassis. For example, a
                                                               system may have a temperature sensor that
                                                               monitors the temperature outside the box.
                                                               Such a temperature sensor can be associated
                                                               with an External Environment entity.
                                                               This value will typically be used as a single
                                                               instance physical entity. However, the Entity
                                                               Instance value can be used to denote a difference
                                                               in regions of the external environment. For
                                                               example, the region around the front of a
                                                               chassis may be considered to be different from
                                                               the region around the back, in which case it would
                                                               be reasonable to have two different instances of
                                                               the External Environment entity. */
#define ENTITY_ID_BATTERY                           0x28    /* battery */
#define ENTITY_ID_PROCESSING_BLADE                  0x29    /* Processing blade (a blade module that
                                                               contains processor, memory, and I/O
                                                               connections that enable it to operate as
                                                               a processing entity) */
#define ENTITY_ID_CONNECTIVITY_SWITCH               0x2A    /* Connectivity switch (a blade module that
                                                               provides the fabric or network connection
                                                               for one or more processing blades or modules) */
#define ENTITY_ID_PROCESSOR_MEM_MODULE              0x2B    /* Processor/memory module (processor and
                                                               memory together on a module) */
#define ENTITY_ID_IO_MODULE                         0x2C    /* I/O module (a module that contains the
                                                               main elements of an I/O interface) */
#define ENTITY_ID_PROCESSOR_IO_MODULE               0x2D    /* Processor/ IO module (a module that
                                                               contains the main elements of an I/O interface) */
#define ENTITY_ID_MGMT_COTROLLER_FW                 0x2E    /* Management Controller Firmware (Represents
                                                               firmware or software running on a management
                                                               controller) */
#define ENTITY_ID_IPMI_CHANNEL                      0x2F    /* IPMI Channel - This Entity ID enables
                                                               associating sensors with the IPMI
                                                               communication channels - for example a
                                                               Redundancy sensor could be used to report
                                                               redundancy status for a channel that is
                                                               composed of multiple physical links. By
                                                               convention, the Entity Instance corresponds
                                                               to the channel number. */
#define ENTITY_ID_PCI_BUS                           0x30    /* PCI Bus */
#define ENTITY_ID_PCI_EXPRESS_BUS                   0x31    /* PCI Express Bus */
#define ENTITY_ID_SCSI_BUS                          0x32    /* SCSI Bus (parallel) */
#define ENTITY_ID_SATA_SAS_BUS                      0x33    /* SATA / SAS bus */
#define ENTITY_ID_PROCESSOR_FSB                     0x34    /* Processor / front-side bus */
/* - 90h-AFh Chassis-specific Entities. These IDs are system specific and can be
 *   assigned by the chassis provider.
 * - B0h-CFh Board-set specific Entities. These IDs are system specific and can be
 *   assigned by the Board-set provider.
 * - D0h-FFh OEM System Integrator defined. These IDs are system specific and can be
 *   assigned by the system integrator, or OEM.
 * - all other values reserved
 */

/* Sensor Unit Type Codes (abridged)
 * See IPMI specification Table 43-15, Sensor Unit Type Codes
 */
#define SENSOR_UNIT_UNSPECIFIED                     0
#define SENSOR_UNIT_DEGREES_CELSIUS                 1
#define SENSOR_UNIT_DEGREES_FAHRENHEIT              2
#define SENSOR_UNIT_DEGREES_KELVIN                  3
#define SENSOR_UNIT_VOLTS                           4
#define SENSOR_UNIT_AMPS                            5
#define SENSOR_UNIT_WATTS                           6
#define SENSOR_UNIT_JOULES                          7
#define SENSOR_UNIT_COULOMBS                        8
#define SENSOR_UNIT_VA                              9
#define SENSOR_UNIT_NITS                            10
#define SENSOR_UNIT_LUMEN                           11
#define SENSOR_UNIT_LUX                             12
#define SENSOR_UNIT_CANDELA                         13
#define SENSOR_UNIT_KPA                             14
#define SENSOR_UNIT_PSI                             15
#define SENSOR_UNIT_NEWTON                          16
#define SENSOR_UNIT_CFM                             17
#define SENSOR_UNIT_RPM                             18
#define SENSOR_UNIT_HZ                              19
#define SENSOR_UNIT_MICROSECOND                     20
#define SENSOR_UNIT_MILLISECOND                     21
#define SENSOR_UNIT_SECOND                          22
#define SENSOR_UNIT_MINUTE                          23
#define SENSOR_UNIT_HOUR                            24
#define SENSOR_UNIT_DAY                             25
#define SENSOR_UNIT_WEEK                            26
#define SENSOR_UNIT_HIT                             78

#define SDR_RECORD_TYPE_FULL_SENSOR                 0x01
#define SDR_RECORD_TYPE_COMPACT_SENSOR              0x02
#define SDR_RECORD_TYPE_EVENTONLY_SENSOR            0x03
#define SDR_RECORD_TYPE_ENTITY_ASSOC                0x08
#define SDR_RECORD_TYPE_DEVICE_ENTITY_ASSOC         0x09
#define SDR_RECORD_TYPE_GENERIC_DEVICE_LOCATOR      0x10
#define SDR_RECORD_TYPE_FRU_DEVICE_LOCATOR          0x11
#define SDR_RECORD_TYPE_MC_DEVICE_LOCATOR           0x12
#define SDR_RECORD_TYPE_MC_CONFIRMATION             0x13
#define SDR_RECORD_TYPE_BMC_MSG_CHANNEL_INFO        0x14
#define SDR_RECORD_TYPE_OEM                         0xc0


#define IPMI_BUF_SIZE                               0x80


//#if HAVE_PRAGMA_PACK
//#define ATTRIBUTE_PACKING
//#else
//#define ATTRIBUTE_PACKING __attribute__ ((packed))
//#endif


/* From table 13.16 of the IPMI v2 specification */
#define IPMI_PAYLOAD_TYPE_IPMI                      0x00
#define IPMI_PAYLOAD_TYPE_SOL                       0x01
#define IPMI_PAYLOAD_TYPE_OEM                       0x02
#define IPMI_PAYLOAD_TYPE_RMCP_OPEN_REQUEST         0x10
#define IPMI_PAYLOAD_TYPE_RMCP_OPEN_RESPONSE        0x11
#define IPMI_PAYLOAD_TYPE_RAKP_1                    0x12
#define IPMI_PAYLOAD_TYPE_RAKP_2                    0x13
#define IPMI_PAYLOAD_TYPE_RAKP_3                    0x14
#define IPMI_PAYLOAD_TYPE_RAKP_4                    0x15

//extern int verbose;
//extern int csv_output;
struct _ipmi_req_cmd {
    uint8_t data_len;               // 数据总长度
    uint8_t rs_sa;                  // 响应从地址
#ifdef __LITTLE_ENDIAN__
    uint8_t rs_lun:2,               // 响应逻辑单元号
            netfn:6;                // 网络功能码
#else
    uint8_t netfn:6,
            rs_lun:2;
#endif
    uint8_t checksum1;              // 校验码1
    uint8_t rq_sa;                  // 请求从地址
#ifdef __LITTLE_ENDIAN__
    uint8_t rq_lun:2,               // 请求逻辑单元号
            rq_seq:6;               // 请求序列号
#else
    uint8_t rq_seq:6,
            rq_lun:2;
#endif
    uint8_t cmd;                    // 命令号
};

struct ipmi_req {
    struct _ipmi_req_cmd msg;
    uint8_t data[IPMI_BUF_SIZE - sizeof(struct _ipmi_req_cmd)];
};


struct ipmi_req_entry {
    struct ipmi_req req;
    //struct ipmi_intf *intf;
    uint8_t rq_seq;
    //uint8_t *msg_data;
    //int msg_len;
    //int bridging_level;
    struct ipmi_req_entry *next;
};

struct ipmi_rsp {
    struct _ipmi_rsp_cmd {
        uint8_t data_len;
        uint8_t ccode;
    } msg;
    uint8_t data[IPMI_BUF_SIZE - sizeof(struct _ipmi_rsp_cmd)];

    /*
     * Looks like this is the length of the entire packet, including the RMCP
     * stuff, then modified to be the length of the extra IPMI message data
     */

#if 0   /* open it with v2,v3 version */
    struct {
        uint8_t netfn;
        uint8_t cmd;
        uint8_t seq;
        uint8_t lun;
    } msg;

    struct {
        uint8_t authtype;
        uint32_t seq;
        uint32_t id;
        uint8_t bEncrypted;    /* IPMI v2 only */
        uint8_t bAuthenticated;    /* IPMI v2 only */
        uint8_t payloadtype;    /* IPMI v2 only */
        /* This is the total length of the payload or
           IPMI message.  IPMI v2.0 requires this to
           be 2 bytes.  Not really used for much. */
        uint16_t msglen;
    } session;

    /*
     * A union of the different possible payload meta-data
     */
    union {
        struct {
            uint8_t rq_addr;
            uint8_t netfn;
            uint8_t rq_lun;
            uint8_t rs_addr;
            uint8_t rq_seq;
            uint8_t rs_lun;
            uint8_t cmd;
        } ipmi_response;
        struct {
            uint8_t message_tag;
            uint8_t rakp_return_code;
            uint8_t max_priv_level;
            uint32_t console_id;
            uint32_t bmc_id;
            uint8_t auth_alg;
            uint8_t integrity_alg;
            uint8_t crypt_alg;
        } open_session_response;
        struct {
            uint8_t message_tag;
            uint8_t rakp_return_code;
            uint32_t console_id;
            uint8_t bmc_rand[16];    /* Random number generated by the BMC */
            uint8_t bmc_guid[16];
            uint8_t key_exchange_auth_code[20];
        } rakp2_message;
        struct {
            uint8_t message_tag;
            uint8_t rakp_return_code;
            uint32_t console_id;
            uint8_t integrity_check_value[20];
        } rakp4_message;
        struct {
            uint8_t packet_sequence_number;
            uint8_t acked_packet_number;
            uint8_t accepted_character_count;
            uint8_t is_nack;    /* bool */
            uint8_t transfer_unavailable;    /* bool */
            uint8_t sol_inactive;    /* bool */
            uint8_t transmit_overrun;    /* bool */
            uint8_t break_detected;    /* bool */
        } sol_packet;

    } payload;
#endif
};

#if 0
/*
 * This is what the sendrcv_v2() function would take as an argument. The common case
 * is for payload_type to be IPMI_PAYLOAD_TYPE_IPMI.
 */
struct ipmi_v2_payload {
    uint16_t payload_length;
    uint8_t payload_type;

    union {

        struct {
            uint8_t rq_seq;
            struct ipmi_rq *request;
        } ipmi_request;

        struct {
            uint8_t rs_seq;
            struct ipmi_rs *response;
        } ipmi_response;

        /* Only used internally by the lanplus interface */
        struct {
            uint8_t *request;
        } open_session_request;

        /* Only used internally by the lanplus interface */
        struct {
            uint8_t *message;
        } rakp_1_message;

        /* Only used internally by the lanplus interface */
        struct {
            uint8_t *message;
        } rakp_2_message;

        /* Only used internally by the lanplus interface */
        struct {
            uint8_t *message;
        } rakp_3_message;

        /* Only used internally by the lanplus interface */
        struct {
            uint8_t *message;
        } rakp_4_message;

        struct {
            uint8_t data[IPMI_BUF_SIZE];
            uint16_t character_count;
            uint8_t packet_sequence_number;
            uint8_t acked_packet_number;
            uint8_t accepted_character_count;
            uint8_t is_nack;    /* bool */
            uint8_t assert_ring_wor;    /* bool */
            uint8_t generate_break;    /* bool */
            uint8_t deassert_cts;    /* bool */
            uint8_t deassert_dcd_dsr;    /* bool */
            uint8_t flush_inbound;    /* bool */
            uint8_t flush_outbound;    /* bool */
        } sol_packet;

    } payload;
};
#endif

#define IPMI_CTX_ENABLE             0x01
#define IPMI_CTX_BRIDGE             0x02
#define IPMI_CTX_NO_RESPONSE        0x80

struct ipmi_ctx {
    struct ipmi_req req;
    struct ipmi_rsp rsp;
    uint8_t channel;
    uint8_t rq_seq;
    uint8_t flags;
    uint16_t req_len;
    uint16_t rsp_len;
};

extern unsigned long device_available;

#include "ipmi_lib/ipmi_modules.h"
#include "ipmi_lib/ipmi_version.h"
#include "ipmi_lib/ipmi_intf.h"
#include "ipmi_lib/ipmi_common.h"
#include "ipmi_lib/ipmi_timer.h"
#include "ipmi_lib/ipmi_logic.h"
#include "ipmi_lib/ipmi_cmd.h"
#include "ipmi_lib/ipmi_debug.h"
#include "ipmi_lib/ipmi_cfg.h"
#include "ipmi_lib/ipmi_chassis.h"
#include "ipmi_lib/ipmi_se.h"
#include "ipmi_lib/ipmi_storage.h"
#include "ipmi_lib/ipmi_transport.h"
#include "ipmi_lib/ipmi_sel.h"
#include "ipmi_lib/ipmi_sdr.h"
#include "ipmi_lib/ipmi_app.h"

#endif  /* _IPMI_H_ */


