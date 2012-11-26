//*****************************************************************************
//
// ipmi_se.c - IPMI Command Header File for Sensor/Event
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#ifndef __IPMI_SE_H__
#define __IPMI_SE_H__

#include <stdint.h>
#include <math.h>
#include "ipmi_lib/ipmi.h"


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


/* threshold specification bits for analog sensors for get sensor threshold command
 * and set sensor threshold command
 */
#define UPPER_NON_RECOV_SPECIFIED                   0x20
#define UPPER_CRIT_SPECIFIED                        0x10
#define UPPER_NON_CRIT_SPECIFIED                    0x08
#define LOWER_NON_RECOV_SPECIFIED                   0x04
#define LOWER_CRIT_SPECIFIED                        0x02
#define LOWER_NON_CRIT_SPECIFIED                    0x01

/* state assertion bits for discrete sensors for get sensor reading command */
#define STATE_0_ASSERTED                            0x01
#define STATE_1_ASSERTED                            0x02
#define STATE_2_ASSERTED                            0x04
#define STATE_3_ASSERTED                            0x08
#define STATE_4_ASSERTED                            0x10
#define STATE_5_ASSERTED                            0x20
#define STATE_6_ASSERTED                            0x40
#define STATE_7_ASSERTED                            0x80
#define STATE_8_ASSERTED                            0x01
#define STATE_9_ASSERTED                            0x02
#define STATE_10_ASSERTED                           0x04
#define STATE_11_ASSERTED                           0x08
#define STATE_12_ASSERTED                           0x10
#define STATE_13_ASSERTED                           0x20
#define STATE_14_ASSERTED                           0x40

/* SDR Record Common Sensor header macros */
#define IS_THRESHOLD_SENSOR(s)	                    ((s)->event_type_code == 1)
#define UNITS_ARE_DISCRETE(s)	                    ((s)->unit.analog_data_format == 3)

#define MAX_SDR_BYTES                               64

/* Sensor Reading Conversion Formula
 * Y = (M * VAL + (B * log(10,K1)) * log(10,K2))
 */
#define SENSOR_VAL2RAW(v,m,b,k1,k2)                 (((v / pow(10, (int)(k2))) - (b * pow(10, (int)(k1)))) / m)
#define SENSOR_VAL2RAW_NOK(v,m,b)                   ((v - b) / m)

typedef struct sensor_formula_st {
    char *sensor_id;
    uint8_t type;
    uint8_t unit;
    int32_t M;
    int32_t B;
    int32_t K1;
    int32_t K2;
    uint32_t vmin;
    uint32_t vmax;
} SENSOR_FORMULA;

typedef struct get_device_sdr_cmd {
    uint8_t reservation_id_lsb;                 /* Reservation ID. LS Byte. Only required
                                                   for partial reads with a non-zero
                                                   Offset into record field. Use 0000h
                                                   for reservation ID otherwise. */
    uint8_t reservation_id_msb;                 /* Reservation ID. MS Byte. */
    uint8_t record_id_lsb;                      /* Record ID of record to Get, LS Byte.
                                                   0000h returns the first record. */
    uint8_t record_id_msb;                      /* Record ID of record to Get, MS Byte */
    uint8_t offset;                             /* Offset into record */
    uint8_t bytes_to_read;                      /* Bytes to read. FFh means read entire record. */
} GET_DEVICE_SDR_CMD;

typedef struct get_device_sdr_resp {
    uint8_t rec_id_next_lsb;                    /* Record ID for next record, LS Byte */
    uint8_t rec_id_next_msb;                    /* Record ID for next record, MS Byte */
    uint8_t req_bytes[MAX_SDR_BYTES];           /* 4:3+N Requested bytes from record */
} GET_DEVICE_SDR_RESP;

struct sensor_set_thresh_rq {
    uint8_t sensor_num;                         /* sensor # */
    uint8_t set_mask;                           /* threshold setting mask */
    uint8_t lower_non_crit;                     /* new lower non critical threshold*/
    uint8_t lower_crit;                         /* new lower critical threshold*/
    uint8_t lower_non_recov;                    /* new lower non recoverable threshold*/
    uint8_t upper_non_crit;                     /* new upper non critical threshold*/
    uint8_t upper_crit;                         /* new upper critical threshold*/
    uint8_t upper_non_recov;                    /* new upper non recoverable threshold*/
};

struct sdr_repo_info_rs {
    uint8_t version;                            /* SDR version (51h) */
    uint16_t count;                             /* number of records */
    uint16_t free;                              /* free space in SDR */
    uint32_t add_stamp;                         /* last add timestamp */
    uint32_t erase_stamp;                       /* last del timestamp */
    uint8_t op_support;                         /* supported operations */
};

struct sdr_device_info_rs {
    uint8_t count;                              /* number of records */
#ifdef __LITTLE_ENDIAN__
    uint8_t device_luns:4,                      /* [3:0] - Device LUNs */
            resv1:3,                            /* [6:4] - reserved */
            flags:1;                            /* [7]   - static/dynamic sensor */
#else
    uint8_t flags:1,
            resv1:3,
            device_luns:4;
#endif
    uint8_t popChangeInd[4];                    /* Sensor Population Change Indicator. LS byte first. */
};

struct reserve_device_sdr_repository_resp {
    uint8_t reservation_id_lsb;                 /* Reservation ID, LS Byte 0000h reserved. */
    uint8_t reservation_id_msb;                 /* Reservation ID, MS Byte */
};

struct sdr_get_rs {
    uint16_t next;                              /* next record id */
    uint16_t id;                                /* record ID */
    uint8_t version;                            /* SDR version (51h) */
    uint8_t type;                               /* record type */
    uint8_t length;                             /* remaining record bytes */
};

typedef struct get_sensor_reading_cmd_req {
    uint8_t sensor_number;
} GET_SENSOR_READING_CMD_REQ;


#pragma pack(1)
typedef struct get_sensor_reading_resp {
    uint8_t sensor_reading;                     /* byte of reading. Ignore on read if sensor
                                                   does not return an numeric (analog) reading */
#ifdef __LITTLE_ENDIAN__
    uint8_t rev1:5,
            unavailable:1,
            sensor_scanning_enabled:1,
            event_messages_enabled:1;
#else
    uint8_t event_messages_enabled:1,           /* 0b = All Event Messages disabled from this sensor */
            sensor_scanning_enabled:1,          /* 0b = sensor scanning disabled */
            unavailable:1,                      /* 1b = reading/state unavailable */
            rev1:5;                             /* reserved */
#endif
#ifdef __LITTLE_ENDIAN__
    uint8_t lower_non_critical_threshold:1,
            lower_critical_threshold:1,
            lower_non_recoverable_threshold:1,
            upper_non_critical_threshold:1,
            upper_critical_threshold:1,
            upper_non_recoverable_threshold:1,
            rev2:2;
#else
    uint8_t rev2:2,
            upper_non_recoverable_threshold:1,
            upper_critical_threshold:1,
            upper_non_critical_threshold:1,
            lower_non_recoverable_threshold:1,
            lower_critical_threshold:1,
            lower_non_critical_threshold:1;
#endif
} GET_SENSOR_READING_RESP;
#pragma pack()


#pragma pack(1)
typedef struct sensor_thresholds_st {
#ifdef __LITTLE_ENDIAN__
    uint8_t set_lower_non_critical_threshold:1,
            set_lower_critical_threshold:1,
            set_lower_non_recoverable_threshold:1,
            set_upper_non_critical_threshold:1,
            set_upper_critical_threshold:1,
            set_upper_non_recoverable_threshold:1,
            rev1:2;
#else
    uint8_t rev1:2,
            set_upper_non_recoverable_threshold:1,
            set_upper_critical_threshold:1,
            set_upper_non_critical_threshold:1,
            set_lower_non_recoverable_threshold:1,
            set_lower_critical_threshold:1,
            set_lower_non_critical_threshold:1;
#endif
    uint8_t lower_non_critical_threshold;
    uint8_t lower_critical_threshold;
    uint8_t lower_non_recoverable_threshold;
    uint8_t upper_non_critical_threshold;
    uint8_t upper_critical_threshold;
    uint8_t upper_non_recoverable_threshold;
} SENSOR_THRESHOLDS;
#pragma pack()


#pragma pack(1)
typedef struct sensor_event_enable_st {
#ifdef __LITTLE_ENDIAN__
    uint8_t reserved:6,
            sensor_scanning_enabled:1,
            event_messages_enabled:1;
#else
    uint8_t event_messages_enabled:1,
            sensor_scanning_enabled:1,
            reserved:6;
#endif
    union {
        struct {
            uint16_t assertion_event_msg;
            uint16_t dassertion_event_msg;
        } discrete;
        struct {
#ifdef __LITTLE_ENDIAN__
            uint8_t assert_lower_non_critical_low:1,
                    assert_lower_non_critical_high:1,
                    assert_lower_critical_low:1,
                    assert_lower_critical_high:1,
                    assert_lower_non_recoverable_low:1,
                    assert_lower_non_recoverable_high:1,
                    assert_upper_non_critical_low:1,
                    assert_upper_non_critical_high:1;
            uint8_t assert_upper_critical_low:1,
                    assert_upper_critical_high:1,
                    assert_upper_non_recoverable_low:1,
                    assert_upper_non_recoverable_high:1,
                    reserved_1:4;
            uint8_t dassert_lower_non_critical_low:1,
                    dassert_lower_non_critical_high:1,
                    dassert_lower_critical_low:1,
                    dassert_lower_critical_high:1,
                    dassert_lower_non_recoverable_low:1,
                    dassert_lower_non_recoverable_high:1,
                    dassert_upper_non_critical_low:1,
                    dassert_upper_non_critical_high:1;
            uint8_t dassert_upper_critical_low:1,
                    dassert_upper_critical_high:1,
                    dassert_upper_non_recoverable_low:1,
                    dassert_upper_non_recoverable_high:1,
                    reserved_2:4;
#else
            uint8_t assert_upper_non_critical_high:1,
                    assert_upper_non_critical_low:1,
                    assert_lower_non_recoverable_high:1,
                    assert_lower_non_recoverable_low:1,
                    assert_lower_critical_high:1,
                    assert_lower_critical_low:1,
                    assert_lower_non_critical_high:1,
                    assert_lower_non_critical_low:1;
            uint8_t reserved_1:4,
                    assert_upper_non_recoverable_high:1,
                    assert_upper_non_recoverable_low:1,
                    assert_upper_critical_high:1,
                    assert_upper_critical_low:1;
            uint8_t dassert_upper_non_critical_high:1,
                    dassert_upper_non_critical_low:1,
                    dassert_lower_non_recoverable_high:1,
                    dassert_lower_non_recoverable_low:1,
                    dassert_lower_critical_high:1,
                    dassert_lower_critical_low:1,
                    dassert_lower_non_critical_high:1,
                    dassert_lower_non_critical_low:1;
            uint8_t reserved_2:4,
                    dassert_upper_non_recoverable_high:1,
                    dassert_upper_non_recoverable_low:1,
                    dassert_upper_critical_high:1,
                    dassert_upper_critical_low:1;
#endif
        } threshold;
    } type;
} SENSOR_EVENT_ENABLE;
#pragma pack()


#pragma pack(1)
typedef struct sensor_event_status_st {
#ifdef __LITTLE_ENDIAN__
    uint8_t rev1:5,
            unavailable:1,
            sensor_scanning_enabled:1,
            event_messages_enabled:1;
#else
    uint8_t event_messages_enabled:1,           /* 0b = All Event Messages disabled from this sensor */
            sensor_scanning_enabled:1,          /* 0b = sensor scanning disabled */
            unavailable:1,                      /* 1b = reading/state unavailable */
            rev1:5;                             /* reserved */
#endif
    union {
        struct {
            uint16_t assertion_event_msg;
            uint16_t dassertion_event_msg;
        } discrete;
        struct {
#ifdef __LITTLE_ENDIAN__
            uint8_t assert_lower_non_critical_low:1,
                    assert_lower_non_critical_high:1,
                    assert_lower_critical_low:1,
                    assert_lower_critical_high:1,
                    assert_lower_non_recoverable_low:1,
                    assert_lower_non_recoverable_high:1,
                    assert_upper_non_critical_low:1,
                    assert_upper_non_critical_high:1;
            uint8_t assert_upper_critical_low:1,
                    assert_upper_critical_high:1,
                    assert_upper_non_recoverable_low:1,
                    assert_upper_non_recoverable_high:1,
                    reserved_1:4;
            uint8_t dassert_lower_non_critical_low:1,
                    dassert_lower_non_critical_high:1,
                    dassert_lower_critical_low:1,
                    dassert_lower_critical_high:1,
                    dassert_lower_non_recoverable_low:1,
                    dassert_lower_non_recoverable_high:1,
                    dassert_upper_non_critical_low:1,
                    dassert_upper_non_critical_high:1;
            uint8_t dassert_upper_critical_low:1,
                    dassert_upper_critical_high:1,
                    dassert_upper_non_recoverable_low:1,
                    dassert_upper_non_recoverable_high:1,
                    reserved_2:4;
#else
            uint8_t assert_upper_non_critical_high:1,
                    assert_upper_non_critical_low:1,
                    assert_lower_non_recoverable_high:1,
                    assert_lower_non_recoverable_low:1,
                    assert_lower_critical_high:1,
                    assert_lower_critical_low:1,
                    assert_lower_non_critical_high:1,
                    assert_lower_non_critical_low:1;
            uint8_t reserved_1:4,
                    assert_upper_non_recoverable_high:1,
                    assert_upper_non_recoverable_low:1,
                    assert_upper_critical_high:1,
                    assert_upper_critical_low:1;
            uint8_t dassert_upper_non_critical_high:1,
                    dassert_upper_non_critical_low:1,
                    dassert_lower_non_recoverable_high:1,
                    dassert_lower_non_recoverable_low:1,
                    dassert_lower_critical_high:1,
                    dassert_lower_critical_low:1,
                    dassert_lower_non_critical_high:1,
                    dassert_lower_non_critical_low:1;
            uint8_t reserved_2:4,
                    dassert_upper_non_recoverable_high:1,
                    dassert_upper_non_recoverable_low:1,
                    dassert_upper_critical_high:1,
                    dassert_upper_critical_low:1;
#endif
        } threshold;
    } type;
} SENSOR_EVENT_STATUS;
#pragma pack()

#pragma pack(1)
typedef struct sensor_type_rsp {
    uint8_t sensor_type;
    uint8_t event_type_code;
} SENSOR_TYPE_RSP;
#pragma pack()

//*****************************************************************************
// Sensor Record Header
//*****************************************************************************
#pragma pack(1)
struct sensor_record_header {
    uint16_t record_id;             /* The Record ID is used by the Sensor Data Repository
                                       device for record organization and access.
                                       It is not related to the sensor ID. */
    uint8_t sdr_version;            /* Version of the Sensor Model specification that this
                                       record is compatible with. 51h for this specification.
                                       BCD encoded with bits 7:4 holding the Least Significant
                                       digit of the revision and bits 3:0 holding the Most
                                       Significant bits. */
    uint8_t record_type;            /* Record Type Number = 01h, Full Sensor Record */
    uint8_t record_len;             /* Number of remaining record bytes following. */
};
#pragma pack()

//*****************************************************************************
// Sensor Record Key
//*****************************************************************************
#pragma pack(1)
struct sensor_record_key {
        /* RECORD KEY BYTES */
#ifdef __LITTLE_ENDIAN__
    uint8_t id_type:1,              /* [0]
                                       - 0b = ID is IPMB Slave Address,
                                       - 1b = system software ID */
            owner_id:7;             /* [7:1]
                                       Sensor Owner ID 7-bit I2C Slave
                                       Address, or 7-bit system software ID */
#else
    uint8_t owner_id:7,
            id_type:1;
#endif

#ifdef __LITTLE_ENDIAN__
    uint8_t sensor_owner_lun:2,
            rsv1:2,
            channel_num:4;
#else
    uint8_t channel_num:4,          /* 7 Sensor Owner LUN -
                                       [7:4] - Channel Number. The Channel Number
                                       can be, used to specify access to sensors that
                                       are located on management controllers that
                                       are connected to the BMC via channels other
                                       than the primary IPMB. (Note: In IPMI v1.5
                                       the ordering of bits 7:2 of this byte have
                                       changed to support the 4-bit channel number.) */
            rsv1:2,                 /* [3:2] - reserved */
            sensor_owner_lun:2;     /* [1:0] - Sensor Owner LUN. LUN in the Sensor
                                       Owner that is used to send/receive IPMB
                                       messages to access the sensor. 00b if system
                                       software is Sensor Owner. */
#endif
    uint8_t sensor_number;          /* 8 Sensor Number - Unique number identifying
                                       the sensor behind a given slave address and
                                       LUN. Code FFh reserved. */
};
#pragma pack()

//*****************************************************************************
// Sensor Entity ID
//*****************************************************************************
#pragma pack(1)
struct sensor_entity_id {
    uint8_t id;                     /* 9 Entity ID - Indicates the physical entity
                                       that the sensor is monitoring or is otherwise
                                       associated with the sensor. See Table 43-13,
                                       Entity ID Codes. */
#ifdef __LITTLE_ENDIAN__
    uint8_t entity_instance_num:7,
            entity_type:1;
#else
    uint8_t entity_type:1,          /* 10 Entity Instance -
                                       [7] - 0b = treat entity as a physical entity
                                       per Entity ID table (ENTITY_TYPE_PHYSICAL)
                                       1b = treat entity as a logical container entity.
                                       For example, if this bit is set, and the Entity
                                       ID is Processor’, the container entity would
                                       be considered to represent a logical
                                       Processor Group’ rather than a physical
                                       processor. This bit is typically used in
                                       conjunction with an Entity Association record.
                                       (ENTITY_TYPE_LOGICAL) */
            entity_instance_num:7;  /* [6:0] - Instance number for entity.
                                       (See section 39.1, System- and Device-relative
                                       Entity Instance Values for more information)
                                       00h-5Fh system-relative Entity Instance.
                                       The Entity Instance number must be unique
                                       for each different entity of the same type
                                       Entity ID in the system.
                                       60h-7Fh device-relative Entity Instance.
                                       The Entity Instance number must only be
                                       unique relative to the management controller
                                       providing access to the Entity. */
#endif
};
#pragma pack()

//*****************************************************************************
// Sensor Initialization
//*****************************************************************************
#pragma pack(1)
struct sensor_initialization {          /* 11 Sensor Initialization */
#ifdef __LITTLE_ENDIAN__
    uint8_t powerup_sensor_scanning:1,
            powerup_evt_generation:1,
            init_sensor_type:1,
            init_hysteresis:1,
            init_thresholds:1,
            init_events:1,
            init_scanning:1,
            rsv2:1;
#else
    uint8_t rsv2:1,                     /* [7] - reserved. Write as 0b. */
            init_scanning:1,            /* [6] - Init Scanning 1b = enable scanning
                                           (this bit=1 implies that the sensor accepts
                                           the enable/disable scanning bit in the
                                           Set Sensor Event Enable command). */
            init_events:1,              /* [5] - Init Events 1b = enable events (per
                                           Sensor Event Message Control Support bits
                                           in Sensor Capabilities field, and per the
                                           Event Mask fields, below). */
            init_thresholds:1,          /* [4] - Init Thresholds 1b = initialize sensor
                                           thresholds (per settable threshold mask below). */
            init_hysteresis:1,          /* [3] - Init Hysteresis 1b = initialize sensor
                                           hysteresis (per Sensor Hysteresis Support
                                           bits in the Sensor Capabilities field, below). */
            init_sensor_type:1,         /* [2] - Init Sensor Type 1b = initialize
                                           Sensor Type and Event / Reading Type code. */
            powerup_evt_generation:1,   /* Sensor Default (power up) State
                                           -------------------------------
                                           Reports how this sensor comes up on device power
                                           up and hardware/cold reset.
                                           The Initialization Agent does not use this
                                           bit. This bit solely reports to software
                                           how the sensor comes prior to being
                                           initialized by the Initialization Agent.
                                           [1] - 0b = event generation disabled,
                                           1b = event generation enabled */
            powerup_sensor_scanning:1;  /* [0] - 0b = sensor scanning disabled,
                                           1b = sensor scanning enabled */
#endif
};
#pragma pack()

#pragma pack(1)
struct sensor_capabilities {
#ifdef __LITTLE_ENDIAN__
    uint8_t event_msg_control:2,
            sensor_threshold_access:2,
            sensor_hysteresis_support:2,
            sensor_rearm_support:1,
            ignore_sensor:1;
#else
    uint8_t ignore_sensor:1,            /* 12 Sensor Capabilities - [7]
                                           1b = Ignore sensor if Entity is not present
                                           or disabled.
                                           0b = don't ignore sensor */

            sensor_rearm_support:1,    /* Sensor Auto Re-arm Support
                                           Indicates whether the sensor requires manual rearming, or
                                           automatically rearms itself when the event clears. manual
                                           implies that the get sensor event status and rearm sensor
                                           events commands are supported
                                           [6] - 0b = no (manual), 1b = yes (auto) */

            sensor_hysteresis_support:2,/* Sensor Hysteresis Support
                                           [5:4]
                                           00b = No hysteresis, or hysteresis built-in
                                           but not specified.
                                           01b = hysteresis is readable.
                                           10b = hysteresis is readable and settable.
                                           11b = Fixed, unreadable, hysteresis.
                                           Hysteresis fields values implemented
                                           in the sensor. */


            sensor_threshold_access:2,  /* Sensor Threshold Access Support
                                           [3:2]
                                           00b no thresholds.
                                           01b thresholds are readable, per Reading
                                               Mask, below.
                                           10b thresholds are readable and settable
                                               per Reading Mask and Settable Threshold
                                               Mask, respectively.
                                           11b = Fixed, unreadable, thresholds. Which
                                               thresholds are supported is
                                               reflected by the Reading Mask. The
                                               threshold value fields report the
                                               values that are hard-coded in the
                                               sensor. */


            event_msg_control:2;        /* Sensor Event Message Control Support
                                           Indicates whether this sensor generates Event Messages, and
                                           if so, what type of Event Message control is offered.
                                           [1:0]
                                           00b per threshold/discrete-state event
                                               enable/disable control (implies that entire
                                               sensor and global disable are also supported)
                                           01b entire sensor only (implies that global
                                               disable is also supported)
                                           10b global disable only
                                           11b no events from sensor */
#endif
};
#pragma pack()

//*****************************************************************************
// Sensor Record Mask Struct
//*****************************************************************************
#pragma pack(1)
struct sensor_record_mask {
    union {
        struct {
            uint16_t assert_event;              /* assertion event mask */
            uint16_t deassert_event;            /* de-assertion event mask */
            uint16_t read;                      /* discrete reading mask */
        } discrete;

        struct {
#ifdef __LITTLE_ENDIAN__
            /* 15 & 16 Assertion Event Mask / Lower Threshold Reading Mask */
            uint16_t assert_lower_non_critical_low:1,
                     assert_lower_non_critical_high:1,
                     assert_lower_critical_low:1,
                     assert_lower_critical_high:1,
                     assert_lower_non_recoverable_low:1,
                     assert_lower_non_recoverable_high:1,
                     assert_upper_non_critical_low:1,
                     assert_upper_non_critical_high:1,
                     assert_upper_critical_low:1,
                     assert_upper_critical_high:1,
                     assert_upper_non_recoverable_low:1,
                     assert_upper_non_recoverable_high:1,
                     status_lower_non_critical:1,
                     status_lower_critical:1,
                     status_lower_non_recoverable:1,
                     reserved:1;
#else
            uint16_t reserved:1,
                     status_lower_non_recoverable:1,
                     status_lower_critical:1,
                     status_lower_non_critical:1,
                     assert_upper_non_recoverable_high:1,
                     assert_upper_non_recoverable_low:1,
                     assert_upper_critical_high:1,
                     assert_upper_critical_low:1,
                     assert_upper_non_critical_high:1,
                     assert_upper_non_critical_low:1,
                     assert_lower_non_recoverable_high:1,
                     assert_lower_non_recoverable_low:1,
                     assert_lower_critical_high:1,
                     assert_lower_critical_low:1,
                     assert_lower_non_critical_high:1,
                     assert_lower_non_critical_low:1;
#endif
#ifdef __LITTLE_ENDIAN__
            /* 17 & 18 - Deassertion Event Mask / Upper Threshold Reading Mask */
            uint16_t dassert_lower_non_critical_low:1,
                     dassert_lower_non_critical_high:1,
                     dassert_lower_critical_low:1,
                     dassert_lower_critical_high:1,
                     dassert_lower_non_recoverable_low:1,
                     dassert_lower_non_recoverable_high:1,
                     dassert_upper_non_critical_low:1,
                     dassert_upper_non_critical_high:1,
                     dassert_upper_critical_low:1,
                     dassert_upper_critical_high:1,
                     dassert_upper_non_recoverable_low:1,
                     dassert_upper_non_recoverable_high:1,
                     status_upper_non_critical:1,
                     status_upper_critical:1,
                     status_upper_non_recoverable:1,
                     reserved_2:1;
#else
            uint16_t reserved_2:1,
                     status_upper_non_recoverable:1,
                     status_upper_critical:1,
                     status_upper_non_critical:1,
                     dassert_upper_non_recoverable_high:1,
                     dassert_upper_non_recoverable_low:1,
                     dassert_upper_critical_high:1,
                     dassert_upper_critical_low:1,
                     dassert_upper_non_critical_high:1,
                     dassert_upper_non_critical_low:1,
                     dassert_lower_non_recoverable_high:1,
                     dassert_lower_non_recoverable_low:1,
                     dassert_lower_critical_high:1,
                     dassert_lower_critical_low:1,
                     dassert_lower_non_critical_high:1,
                     dassert_lower_non_critical_low:1;
#endif
#ifdef __LITTLE_ENDIAN__
            /* 19 & 20 - Settable Threshold Mask, Readable Threshold Mask */
            uint16_t lower_non_critical_readable:1,
                     lower_critical_readable:1,
                     lower_non_recoverable_readable:1,
                     upper_non_critical_readable:1,
                     upper_critical_readable:1,
                     upper_non_recoverable_readable:1,
                     reserved_3:2,
                     lower_non_critical_settable:1,
                     lower_critical_settable:1,
                     lower_non_recoverable_settable:1,
                     upper_non_critical_settable:1,
                     upper_critical_settable:1,
                     upper_non_recoverable_settable:1,
                     reserved_4:2;
#else
            uint16_t reserved_4:2,
                     upper_non_recoverable_settable:1,
                     upper_critical_settable:1,
                     upper_non_critical_settable:1,
                     lower_non_recoverable_settable:1,
                     lower_critical_settable:1,
                     lower_non_critical_settable:1,
                     reserved_3:2,
                     upper_non_recoverable_readable:1,
                     upper_critical_readable:1,
                     upper_non_critical_readable:1,
                     lower_non_recoverable_readable:1,
                     lower_critical_readable:1,
                     lower_non_critical_readable:1;
#endif
        } threshold;
    } type;
};
#pragma pack()

//*****************************************************************************
// Sensor Unit
//*****************************************************************************
#pragma pack(1)
struct sensor_unit {
#ifdef __LITTLE_ENDIAN__
    uint8_t percentage:1,
            modifier_unit:2,
            rate_unit:3,
            analog_data_format:2;
#else
    uint8_t analog_data_format:2,       /* 21 Sensor Units
                                           [7:6] - Analog (numeric) Data Format.
                                           Specifies threshold and analog reading,
                                           if analog reading provided. If neither
                                           thresholds nor analog reading are
                                           provided, this field should be written
                                           as 00h.
                                               00b = unsigned
                                               01b = 1s complement (signed)
                                               10b = 2s complement (signed)
                                               11b = Does not return analog (numeric) reading */
            rate_unit:3,                /* [5:3] - Rate unit
                                               000b = none
                                               001b = per S
                                               010b = per ms
                                               011b = per s
                                               100b = per minute
                                               101b = per hour
                                               110b = per day
                                               111b = reserved */
            modifier_unit:2,            /* [2:1] - Modifier unit
                                               00b = none
                                               01b = Basic Unit / Modifier Unit
                                               10b = Basic Unit * Modifier Unit
                                               11b = reserved */
            percentage:1;               /* [0] - Percentage
                                               0b = no,
                                               1b = yes */

#endif
    uint8_t unit_2_base;                /* Sensor Units 2 - Base Unit
                                           [7:0] - Units Type code:
                                               See Table 43-15, Sensor Unit Type Codes. */
    uint8_t unit_3_modifier;            /* Sensor Units 3 - Modifier Unit
                                           [7:0] - Units Type code, 00h if unused. */
};
#pragma pack()

#pragma pack(1)
struct sensor_record_mtol {
#ifdef __LITTLE_ENDIAN__
    uint8_t m_ls;
    uint8_t tolerance:6,
            m_ms:2;
#else
    uint8_t m_ls;                       /* 25 M
                                           [7:0] - M: LS 8 bits [2's complement, signed, 10 bit 'M' value.] */

    uint8_t m_ms:2,                     /* 26 M, Tolerance
                                           [7:6] - M: MS 2 bits */
            tolerance:6;                /* [5:0] - Tolerance: 6 bits, unsigned
                                                (Tolerance in +/- 1/2 raw counts) */
#endif
};
#pragma pack()


#define FORMULA_M_LS(v)     ((uint8_t)((uint16_t)v & 0xff))
#define FORMULA_M_MS(v)     ((uint8_t)((uint16_t)v >> 8))
#define FORMULA_B_LS(v)     ((uint8_t)((uint16_t)v & 0xff))
#define FORMULA_B_MS(v)     ((uint8_t)((uint16_t)v >> 8))

/* Sensor Reading Conversion Formula
 * Y = (M * VAL + (B * log(10,K1)) * log(10,K2))
 */
#pragma pack(1)
struct sensor_record_bacc {
#ifdef __LITTLE_ENDIAN__
    uint8_t b_ls;                       /* 27 M, [7:0] - B: LS 8 bits [2's complement, signed, 10-bit 'B' value.] */
    uint8_t accuracy_ls:6,              /* 28 M, Accuracy exponent:
                                                 [5:0] - Accuracy: LS 6 bits */
            b_ms:2;                     /*       [7:6] - B: MS 2 bits Unsigned, 10-bit Basic Sensor Accuracy
                                                 in 1/100 percent scaled up by unsigned */
    uint8_t sensor_dir:2,               /* 29 M, [1:0] - Sensor Direction. Indicates whether the sensor
                                                 is monitoring an input or output relative to the given Entity.
                                                 E.g. if the sensor is monitoring a current, this can be used
                                                 to specify whether it is an input voltage or an output voltage.
                                                    00b = unspecified / not applicable
                                                    01b = input
                                                    10b = output
                                                    11b = reserved */
            accuracy_exp:2,             /*       [3:2] - Accuracy exp: 2 bits, unsigned */
            accuracy_ms:4;              /*       [7:4] - Accuracy: MS 4 bits */
    uint8_t b_exponent:4,               /* 30 M, [3:0] - [k1]B exponent 4 bits, 2's complement, signed */
            r_exponent:4;               /*       [7:4] - [k2]R (result) exponent 4 bits, 2's complement, signed */
#else
    uint8_t b_ls;
    uint8_t b_ms:2,
            accuracy_ls:6;
    uint8_t accuracy_ms:4,
            accuracy_exp:2,
            sensor_dir:2;
    uint8_t r_exponent:4,
            b_exponent:4;
#endif
};
#pragma pack()


#pragma pack(1)
struct analog_characteristic_flags {    /* 31 Analog characteristic flags */
#ifdef __LITTLE_ENDIAN__
    uint8_t nominal_reading_specified:1,
            normal_max_specified:1,
            normal_min_specified:1,
            reserved:5;
#else
    uint8_t reserved:5,
            normal_min_specified:1,
            normal_max_specified:1,
            nominal_reading_specified:1;
#endif
};
#pragma pack()


#pragma pack(1)
struct sdr_id_string_type_length {      /* 48 ID String Type/Length Code */
#ifdef __LITTLE_ENDIAN__
    uint8_t length:5,                   /* length of following data, in characters.
                                           00000b indicates 'none following'
                                           11111b = reserved */
            reserved:1,
            type:2;                     /* Sensor ID String Type/Length Code, per Section 43.15,
                                           Type/Length Byte Format. (copied here)
                                           [7:6]
                                               00 = Unicode
                                               01 = BCD plus (see below)
                                               10 = 6-bit ASCII, packed
                                               11 = 8-bit ASCII + Latin 1.
                                           At least two bytes of data must be present
                                           when this type is used. Therefore, the
                                           length (number of data bytes) will be >1 if
                                           data is present, 0 if data is not present.
                                           A length of 1 is reserved. */
#else
    uint8_t type:2,
            reserved:1,
            length:5;
#endif
};
#pragma pack()

//*****************************************************************************
// Sensor Record Header and Key struct
//*****************************************************************************
#pragma pack(1)
typedef struct sdr_record_header_key {
    struct sensor_record_header header;
    struct sensor_record_key key;
} SDR_RECORD_HEADER_KEY;
#pragma pack()

//*****************************************************************************
// Full Sensor Record
//*****************************************************************************
#pragma pack(1)
typedef struct sdr_record_full {
    struct sensor_record_header header;
    struct sensor_record_key key;
    struct sensor_entity_id entity_id;
    struct sensor_initialization initialization;
    struct sensor_capabilities capabilities;
    uint8_t sensor_type;                /* 13 Sensor Type - Code representing the sensor type.
                                           From Table 42-3, Sensor Type Codes.
                                           E.g. Temperature, Voltage, Processor, etc. */
    uint8_t event_type_code;            /* 14 Event/Reading Type Code - Event/Reading
                                           Type Code. From Table 42-1, Event/Reading
                                           Type Code Ranges. */
    struct sensor_record_mask mask;
    struct sensor_unit unit;
    uint8_t linearization;              /* 24 Linearization -
                                           [7] - reserved
                                           [6:0] - enum (linear, ln, log10, log2, e,
                                           exp10, exp2, 1/x, sqr(x), cube(x), sqrt(x),
                                           cuberoot(x) ) -
                                               70h = non-linear.
                                               71h-7Fh = non-linear, OEM defined. */
    struct sensor_record_mtol mtol;
    struct sensor_record_bacc bacc;
    struct analog_characteristic_flags analog_flags;
    uint8_t nominal_reading;            /* 32 Nominal Reading - Given as a raw value.
                                           Must be converted to units-based value using
                                           the y=Mx+B formula. 1s or 2s complement
                                           signed or unsigned per flag bits in Sensor
                                           Units 1. */
    uint8_t normal_maximum;             /* 33 Normal Maximum - Given as a raw value.
                                           Must be converted to units-based value using
                                           the y=Mx+B formula. 1s or 2s complement
                                           signed or unsigned per signed bit in Sensor
                                           Units 1. */
    uint8_t normal_minimum;             /* 34 Normal Minimum - Given as a raw value.
                                           Must be converted to units-based value using
                                           the y=Mx+B formula. Signed or unsigned per
                                           signed bit in Sensor Units 1. */
    uint8_t sensor_maximum_reading;     /* 35 Sensor Maximum Reading - Given as
                                           a raw value. Must be converted to units-based
                                           value based using the y=Mx+B formula.
                                           Signed or unsigned per signed bit in sensor
                                           flags. Normally FFh for an 8-bit unsigned
                                           sensor, but can be a lesser value if the sensor
                                           has a restricted range. If max. reading cannot
                                           be pre-specified this value should be set
                                           to max value, based on data format, (e.g. FFh
                                           for an unsigned sensor, 7Fh for 2s complement,
                                           etc.) */
    uint8_t sensor_minimum_reading;     /* 36 Sensor Minimum Reading -
                                           Given as a raw value. Must be converted to
                                           units-based value using the y=Mx+B formula.
                                           Signed or unsigned per signed bit in sensor
                                           flags. If min. reading cannot be pre-specified
                                           this value should be set to min value, based
                                           on data format, (e.g. 00h for an unsigned
                                           sensor, 80h for 2s complement, etc.) */
    uint8_t upper_non_recoverable_threshold;
                                        /* 37 Upper non-recoverable Threshold - Use of
                                           this field is based on Settable Threshold
                                           Mask. If the corresponding bit is set in the
                                           mask byte and the Init Sensor Thresholds
                                           bit is also set, then this value will be
                                           used for initializing the sensor threshold.
                                           Otherwise, this value should be ignored.
                                           The thresholds are given as raw values that
                                           must be converted to units-based values using
                                           the y=Mx+B formula. */
    uint8_t upper_critical_threshold;
                                        /* 38 Upper critical Threshold - Use of this
                                           field is based on Settable Threshold Mask,
                                           above */
    uint8_t upper_non_critical_threshold;
                                        /* 39 Upper non-critical Threshold - Use of
                                           this field is based on Settable Threshold Mask,
                                           above */
    uint8_t lower_non_recoverable_threshold;
                                        /* 40 Lower non-recoverable Threshold - Use of
                                           this field is based on Settable Threshold
                                           Mask, above */
    uint8_t lower_critical_threshold;
                                        /* 41 Lower critical Threshold - Use of this
                                           field is based on Settable Threshold Mask,
                                           above */
    uint8_t lower_non_critical_threshold;
                                        /* 42 Lower non-critical Threshold - Use of
                                           this field is based on Settable Threshold
                                           Mask, above */
    uint8_t positive_going_threshold_hysteresis_value;
                                        /* 43 Positive-going Threshold Hysteresis value
                                           Positive hysteresis is defined as the
                                           unsigned number of counts that are subtracted
                                           from the raw threshold values to create the
                                           re-arm’ point for all positive-going
                                           thresholds on the sensor. 0 indicates that
                                           there is no hysteresis on positive-going
                                           thresholds for this sensor. Hysteresis values
                                           are given as raw counts. That is, to find the
                                           degree of hysteresis in units, the value must
                                           be converted using the y=Mx+B formula. */
    uint8_t negative_going_threshold_hysteresis_value;
                                        /* 44 Negative-going Threshold Hysteresis value
                                           Negative hysteresis is defined as the
                                           unsigned number of counts that are added
                                           to the raw threshold value to create the
                                           re-arm point for all negative-going
                                           thresholds on the sensor. 0 indicates that
                                           there is no hysteresis on negative-going
                                           thresholds for this sensor. */
    uint8_t reserved2;                  /* 45 reserved. Write as 00h. */
    uint8_t reserved3;                  /* 46 reserved. Write as 00h. */
    uint8_t oem;                        /* 47 OEM - Reserved for OEM use. */
    struct sdr_id_string_type_length id_type_length;
    uint8_t id_string_bytes[16];        /* 49:+N ID String Bytes - Sensor ID String bytes.
                                           Only present if non-zero length in Type/Length
                                           field. 16 bytes, maximum. Note: the SDR can
                                           be implemented as a fixed length record.
                                           Bytes beyond the ID string bytes are
                                           unspecified and should be ignored. */
} SDR_RECORD_FULL;
#pragma pack()


#pragma pack(1)
struct sensor_record_sharing_dir {      /* 24 Sensor Record Sharing, Sensor Direction
                                           25 Entity Instance Sharing */
#ifdef __LITTLE_ENAIDAN__
    uint16_t id_str_mod_offset:7,
             entity_inst_same:1,
             share_count:4,
             id_str_mod_type:2,
             sensor_direction:2;
#else
    uint16_t sensor_direction:2,        /* [7:6] - Sensor Direction. Indicates
                                               whether the sensor is monitoring an input or output
                                               relative to the given Entity. E.g. if the sensor is
                                               monitoring a current, this can be used to specify whether
                                               it is an input voltage or an output voltage.
                                           00b = unspecified / not applicable
                                           01b = input
                                           10b = output
                                           11b = reserved */
             id_str_mod_type:2,         /* [5:4] - ID String Instance Modifier Type (The instance modifier
                                               is a character(s) that software can append to the end of
                                               the ID String. This field selects whether the appended
                                               character(s) will be numeric or alpha.
                                               The Instance Modified Offset field, below, selects
                                               the starting value for the character.)
                                           00b = numeric
                                           01b = alpha */
             share_count:4,             /* [3:0] - Share count (number of sensors sharing
                                               this record). Sensor numbers sharing this record are
                                               sequential starting with the sensor number specified by
                                               the Sensor Number field for this record. E.g. if the
                                               starting sensor number was 10, and the share count was 3,
                                               then sensors 10, 11, and 12 would share this record. */
             entity_inst_same:1,        /* [7] -
                                           0b = Entity Instance same for all shared records
                                           1b = Entity Instance increments for each shared record */
             id_str_mod_offset:7;       /* [6:0] - ID String Instance Modifier Offset
                                               Multiple Discrete sensors can share the same sensor data
                                               record. The ID String Instance Modifier and Modifier
                                               Offset are used to modify the Sensor ID String as follows:
                                               Suppose sensor ID is Temp for Temperature Sensor,
                                               share count = 3, ID string instance modifier = numeric,
                                               instance modifier offset = 5 - then the sensors could be
                                               identified as: Temp 5, Temp 6, Temp 7
                                               If the modifier = alpha, offset=0 corresponds to A,
                                               offset=25 corresponds to Z, and offset = 26 corresponds
                                               to AA, thus, for offset=26 the sensors could be identified
                                               as: Temp AA, Temp AB, Temp AC (alpha characters are
                                               considered to be base 26 for ASCII) */
#endif
};
#pragma pack()


// Table 43-2, Compact Sensor Record - SDR Type 02h
#pragma pack(1)
typedef struct sdr_record_compact {
    struct sensor_record_header header;
    struct sensor_record_key key;
    struct sensor_entity_id entity_id;
    struct sensor_initialization initialization;
    struct sensor_capabilities capabilities;
    uint8_t sensor_type;                /* 13 Sensor Type - Code representing the sensor type.
                                           From Table 42-3, Sensor Type Codes.
                                           E.g. Temperature, Voltage, Processor, etc. */
    uint8_t event_type_code;            /* 14 Event/Reading Type Code - Event/Reading
                                           Type Code. From Table 42-1, Event/Reading
                                           Type Code Ranges. */
    struct sensor_record_mask mask;
    struct sensor_unit unit;
    struct sensor_record_sharing_dir sharing_dir;

    // RECORD BODY BYTES
    uint8_t positive_hysteresis;        /* 26 Positive-going Threshold Hysteresis value
                                           Positive hysteresis is defined as the unsigned number of
                                           counts that are subtracted from the raw threshold values
                                           to create the re-arm point for all positive-going thresholds
                                           on the sensor. 0 indicates that there is no hysteresis on
                                           positive-going thresholds for this sensor. Hysteresis values
                                           are given as raw counts. That is, to find the degree of
                                           hysteresis in units, the value must be converted using the
                                           y=Mx+B formula.
                                           Note: Cannot use shared record if sensors
                                           require individual hysteresis settings. */
    uint8_t negative_hysteresis;        /* 27 Negative-going Threshold Hysteresis value
                                           Negative hysteresis is defined as the unsigned number of
                                           counts that are added to the raw threshold value to create
                                           the re-arm point for all negative-going thresholds on the
                                           sensor. 0 indicates that there is no hysteresis on negative-going
                                           thresholds for this sensor.
                                           Note: Cannot use shared record
                                           if sensors require individual hysteresis settings. */
    uint8_t rsv4;                       /* 28 reserved Write as 00h. */
    uint8_t rsv5;                       /* 29 reserved. Write as 00h. */
    uint8_t rsv6;                       /* 30 reserved. Write as 00h. */
    uint8_t oem;                        /* 31 Reserved for OEM use. */
    struct sdr_id_string_type_length id_type_length;
    uint8_t id_string_bytes[16];        /* 33:+N Sensor ID String bytes. Only present
                                           if non-zero length in Type/Length field. 16 bytes, maximum. */
} SDR_RECORD_COMPACT;
#pragma pack()


#pragma pack(1)
struct sdr_power_state_notification {
#ifdef __LITTLE_ENDIAN__
    uint8_t ctrl_init:2,
            log_init_agent_errs:1,
            ctrl_logs_init_errs:1,
            rsv2:1,
            rsv1:1,
            acpi_dev_pwr_st_notify_req:1,
            acpi_sys_pwr_st_notify_req:1;
#else
    uint8_t acpi_sys_pwr_st_notify_req:1,   /* [7] - 1b = ACPI System Power State notification required (by system s/w)
                                                0b = no ACPI System Power State notification required */
            acpi_dev_pwr_st_notify_req:1,   /* [6] - 1b = ACPI Device Power State notification required (by system s/w)
                                                0b = no ACPI Device Power State notification required */
            rsv1:1,                         /* [5] - For backward compatibility, this bit does not apply to the BMC,
                                                and should be written as 0b.
                                                0b = Dynamic controller - controller may or may not
                                                be present. Software should not generate error status
                                                if this controller is not present.
                                                1b = Static controller - this controller is expected
                                                to be present in the system at all times. Software may
                                                generate an error status if controller is not detected. */
            rsv2:1,                         /* [4] - reserved */
            // Global Initialization
            ctrl_logs_init_errs:1,          /* [3] - 1b = Controller logs Initialization
                                                Agent errors (only applicable to Management
                                                Controller that implements the initialization
                                                agent function. Set to 0b otherwise.) */
            log_init_agent_errs:1,          /* [2] - 1b = Log Initialization Agent errors
                                                accessing this controller (this directs the
                                                initialization agent to log any failures
                                                setting the Event Receiver) */
            ctrl_init:2;                    /* [1:0]
                                                00b = Enable event message generation from controller
                                                (Init agent will set Event Receiver address into controller)
                                                01b = Disable event message generation from controller
                                                (Init agent will set Event Receiver to FFh). This provides
                                                a temporary fix for a broken controller that floods the
                                                system with events. It can also be used for development
                                                / debug purposes.
                                                10b = Do not initialize controller. This selection is for
                                                development / debug support.
                                                11b = reserved. */
#endif
};
#pragma pack()

#pragma pack(1)
struct sdr_device_capabilities {
#ifdef __LITTLE_ENDIAN__
    uint8_t dev_sup_sensor:1,
            dev_sup_sdr_rep:1,
            dev_sup_sel:1,
            dev_sup_fru_inv:1,
            dev_sup_ipmb_evt_rcv:1,
            dev_sup_ipmb_evt_gen:1,
            dev_sup_bridge:1,
            dev_sup_chassis:1;
#else
    uint8_t dev_sup_chassis:1,              /* [7] - 1b = Chassis Device. (device
                                                functions as chassis device, per ICMB spec) */
            dev_sup_bridge:1,               /* [6] - 1b = Bridge (Controller responds
                                                to Bridge NetFn commands) */
            dev_sup_ipmb_evt_gen:1,         /* [5] - 1b = IPMB Event Generator (device
                                                generates event messages on IPMB) */
            dev_sup_ipmb_evt_rcv:1,         /* [4] - 1b = IPMB Event Receiver (device
                                                accepts event messages from IPMB) */
            dev_sup_fru_inv:1,              /* [3] - 1b = FRU Inventory Device
                                                (accepts FRU commands to FRU Device #0
                                                at LUN 00b) */
            dev_sup_sel:1,                  /* [2] - 1b = SEL Device (provides interface
                                                to SEL) */
            dev_sup_sdr_rep:1,              /* [1] - 1b = SDR Repository Device (For
                                               BMC, indicates BMC provides interface
                                               to 1b = SDR Repository. For other
                                               controller, indicates controller accepts
                                               Device SDR commands) */
            dev_sup_sensor:1;               /* [0] - 1b = Sensor Device (device accepts
                                               sensor commands) See Table 37-11,
                                               IPMB/I2C Device Type Codes */
#endif
};
#pragma pack()

#pragma pack(1)
struct mc_locator_record_key {
    /* RECORD KEY BYTES */
#ifdef __LITTLE_ENDIAN__
    uint8_t rsv1:1,                 /* [0]
                                       - 0b = ID is IPMB Slave Address,
                                       - 1b = system software ID */
            dev_slave_addr:7;       /* [7:1]
                                       7-bit I2C Slave Address of device on channel. */
#else
    uint8_t dev_slave_addr:7,
            rsv1:1;
#endif

#ifdef __LITTLE_ENDIAN__
    uint8_t rsv2:4,
            channel_num:4;
#else
    uint8_t channel_num:4,          /* 7 Sensor Owner LUN -
                                       [3:0] - Channel Number. The Channel Number
                                       can be, used to specify access to sensors that
                                       are located on management controllers that
                                       are connected to the BMC via channels other
                                       than the primary IPMB. (Note: In IPMI v1.5
                                       the ordering of bits 7:2 of this byte have
                                       changed to support the 4-bit channel number.) */
            rsv2:4;                 /* [7:4] - reserved */
#endif
};
#pragma pack()

/*
 * SDR Type 12h - Management Controller Device Locator Record
 */
#pragma pack(1)
typedef struct sdr_record_mc_locator {
    struct sensor_record_header header;
    struct mc_locator_record_key key;
    struct sdr_power_state_notification pow_state_noti;
    struct sdr_device_capabilities dev_cap;
    uint8_t rsv[3];
    struct sensor_entity_id entity_id;
    uint8_t oem;                            /* Reserved for OEM use. */
    struct sdr_id_string_type_length id_type_length;
    uint8_t id_string_bytes[16];            /* 17:+N Device ID String N Short ID
                                               string for the device. 16 bytes, maximum. */
} SDR_RECORD_MC_LOCATOR;
#pragma pack()


#pragma pack(1)
typedef struct sdr_record_entry {
    uint16_t record_id;
    uint8_t record_type;
    uint8_t record_len;
    void *record_ptr;
} sdr_record_entry_t;
#pragma pack()

#pragma pack(1)
typedef struct sensor_data {
    uint8_t sensor_id;
    uint8_t local_sensor_id;
    sdr_record_entry_t *sdr_record;
    uint8_t last_sensor_reading;
    uint8_t scan_period;                        /* time between each sensor scan in seconds, 0 = no scan */
    void (*scan_function)(void *);              /* the routine that does the sensor scan */
    void (*sett_function)(void *);              /* the routine that does set the register of chip */
#ifdef __LITTLE_ENDIAN__
    uint8_t set_lower_non_critical_threshold:1,
            set_lower_critical_threshold:1,
            set_lower_non_recoverable_threshold:1,
            set_upper_non_critical_threshold:1,
            set_upper_critical_threshold:1,
            set_upper_non_recoverable_threshold:1,
            recv1:2;
#else
    uint8_t recv1:2,
            set_upper_non_recoverable_threshold:1,
            set_upper_critical_threshold:1,
            set_upper_non_critical_threshold:1,
            set_lower_non_recoverable_threshold:1,
            set_lower_critical_threshold:1,
            set_lower_non_critical_threshold:1;
#endif
#ifdef __LITTLE_ENDIAN__
    uint8_t recv2:5,
            unavailable:1,
            sensor_scanning_enabled:1,
            event_messages_enabled:1;
#else
    uint8_t event_messages_enabled:1,           /* 0b = All Event Messages disabled from this sensor */
            sensor_scanning_enabled:1,          /* 0b = sensor scanning disabled */
            unavailable:1,                      /* 1b = reading/state unavailable */
            recv2:5;
#endif
#ifdef __LITTLE_ENDIAN__
    uint8_t lower_non_critical_threshold:1,
            lower_critical_threshold:1,
            lower_non_recoverable_threshold:1,
            upper_non_critical_threshold:1,
            upper_critical_threshold:1,
            upper_non_recoverable_threshold:1,
            recv3:2;
#else
    uint8_t recv3:2,
            upper_non_recoverable_threshold:1,
            upper_critical_threshold:1,
            upper_non_critical_threshold:1,
            lower_non_recoverable_threshold:1,
            lower_critical_threshold:1,
            lower_non_critical_threshold:1;
#endif
} sensor_data_t;
#pragma pack()

uint8_t sdr_record_len(uint8_t sdr_type);
int sensor_add(sensor_data_t *sensor_data, uint8_t sdr_type, void *sdr);
int ipmi_sdr_get(struct sdr_device_info_rs *sdr_info, uint8_t lun);
void ipmi_reserve_device_sdr_repository(struct ipmi_ctx *ctx_cmd);
void ipmi_get_device_sdr(struct ipmi_ctx *ctx_cmd);

#endif  // __IPMI_SE_H__

