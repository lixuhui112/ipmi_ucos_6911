//*****************************************************************************
//
// sensor_key.c - The User Keyboard Sensor
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

/******************************************************************************
TODO:
    2012/9/26   温度阈值告警中断需要调试

History:
    2012/9/26   完成基本的传感器SE命令框架，支持的命令包括:
                sensor reading
                sensor get
                sensor thresh
                sensor list
******************************************************************************/


#include "ipmi_lib/ipmi.h"
#include "app/lib_common.h"
#include "app/lib_i2c.h"
#include <string.h>
#include <stdio.h>

#define MAX_KEY_SENSOR_COUNT    1

SDR_RECORD_FULL key_sr[MAX_KEY_SENSOR_COUNT];
sensor_data_t key_sd[MAX_KEY_SENSOR_COUNT];


void key_init_chip(void)
{
}

void key_sett_function(void *arg)
{
    sensor_data_t *sd = (sensor_data_t *)arg;
    uint8_t key_id;
    int16_t temp_val;

    key_id = sd->local_sensor_id;

}

void key_scan_function(void *arg)
{
    sensor_data_t *sd = (sensor_data_t *)arg;
    uint8_t key_id = sd->local_sensor_id;
    uint32_t error;
    int16_t temp_val = 0;

    /* TODO:
     * read the KEY from the physical chip device
     */
    sd->last_sensor_reading = 2;

    sd->lower_non_recoverable_threshold = 0;
    sd->lower_critical_threshold = 0;
    sd->lower_non_critical_threshold = 0;
    sd->upper_non_critical_threshold = 0;
    sd->upper_critical_threshold = 0;
    sd->upper_non_recoverable_threshold = 0;
}

void key_init_sensor_record(uint8_t i)
{
    char id_string[16];

	/* SENSOR RECORD HEADER */
	key_sr[i].header.record_id         = 1;
	key_sr[i].header.sdr_version       = 0x51;
	key_sr[i].header.record_type       = SDR_RECORD_TYPE_FULL_SENSOR;
	key_sr[i].header.record_len        = sizeof(key_sr[i]);

	/* RECORD KEY BYTES */
	key_sr[i].key.owner_id             = 0;	                    /* 7-bit system software ID */
	key_sr[i].key.id_type              = 1;	                    /* System software type */
	key_sr[i].key.channel_num          = 0;
	key_sr[i].key.sensor_owner_lun     = 0;
	key_sr[i].key.sensor_number        = 0;	                    /* -CHANGE- this will get replaced by the actual
	                                                               sensor number when we register the SDR */

    /* RECORD BODY BYTES */
	key_sr[i].entity_id.id = ENTITY_ID_SYSTEM_BOARD;            /* physical entity the sensor is monitoring */
	key_sr[i].entity_id.entity_type = 0;	                    /* treat entity as a physical entity */
	key_sr[i].entity_id.entity_instance_num  = 0;

	/* Sensor Default (power up) State */
	key_sr[i].initialization.powerup_sensor_scanning = 1;       /* sensor scanning enabled */
    key_sr[i].initialization.powerup_evt_generation = 0;        /* event generation disabled */
    key_sr[i].initialization.init_sensor_type = 0;
    key_sr[i].initialization.init_hysteresis = 0;
    key_sr[i].initialization.init_thresholds = 0;
    key_sr[i].initialization.init_events = 0;
    key_sr[i].initialization.init_scanning = 1;                 /* accepts scanning bit in the SetSensorEventEnable command */

	/* Sensor Capabilities */
	key_sr[i].capabilities.ignore_sensor = 0;			        /* Ignore sensor if entity is not present or disabled. */
	key_sr[i].capabilities.sensor_rearm_support = 1;		    /* automatically rearms itself when the event clears */
	key_sr[i].capabilities.sensor_hysteresis_support = 2; 	    /* hysteresis is readable and settable */
	key_sr[i].capabilities.sensor_threshold_access = 2;	        /* thresholds are readable and settable
                                                                   per Reading Mask and Settable Threshold Mask, respectively */
	key_sr[i].capabilities.event_msg_control = 0;			    /* per threshold/discrete-state event
                                                                   enable/disable control (implies that entire
                                                                   sensor and global disable are also supported) */
	key_sr[i].sensor_type = SENSOR_TYPE_BUTTON_SWITCH;		    /* From Table 42-3, Sensor Type Codes */
	key_sr[i].event_type_code = 0x03;		                        /* From Table 42-1, discrete */

	/* Event Mask */
	key_sr[i].mask.type.discrete.assert_event = 0;
	key_sr[i].mask.type.discrete.deassert_event = 0;
	key_sr[i].mask.type.discrete.read = 0;

    /* Unit 1, 2, 3 */
    key_sr[i].unit.analog_data_format = 3;		                /* 2's complement (signed) */
	key_sr[i].unit.rate_unit = 0;			                    /* none */
	key_sr[i].unit.modifier_unit = 0;			                /* 00b = none */
	key_sr[i].unit.percentage = 0;                              /* not a percentage value */
	key_sr[i].unit.unit_2_base = SENSOR_UNIT_HIT;               /* Base Unit */
	key_sr[i].unit.unit_3_modifier = 0;                         /* no modifier unit */
	key_sr[i].linearization = 0;		                        /* Linear */

	key_sr[i].mtol.m_ls = 0;                                    /* M = 0 */
	key_sr[i].mtol.m_ms = 0;
	key_sr[i].mtol.tolerance = 0;                               /* Tolerance in +/- 1/2 raw counts */

	key_sr[i].bacc.b_ls = 0;
	key_sr[i].bacc.b_ms = 0;
	key_sr[i].bacc.accuracy_ls = 0;
	key_sr[i].bacc.accuracy_ms = 0;
	key_sr[i].bacc.accuracy_exp = 0;
	key_sr[i].bacc.sensor_dir = 0;
	key_sr[i].bacc.b_exponent = 0;
	key_sr[i].bacc.r_exponent = 0;

	key_sr[i].analog_flags.nominal_reading_specified = 0;
	key_sr[i].analog_flags.normal_max_specified = 0;
	key_sr[i].analog_flags.normal_min_specified = 0;

	key_sr[i].sensor_maximum_reading = 0;
	key_sr[i].sensor_minimum_reading = 0;

	key_sr[i].upper_non_recoverable_threshold = 0;
	key_sr[i].upper_critical_threshold = 0;
	key_sr[i].upper_non_critical_threshold = 0;
	key_sr[i].lower_non_recoverable_threshold = 0;
	key_sr[i].lower_critical_threshold = 0;
	key_sr[i].lower_non_critical_threshold = 0;

	key_sr[i].positive_going_threshold_hysteresis_value = 0;    /* KEY_VAL_POSI_HYST(KEY_TEMP_UCR) */
	key_sr[i].negative_going_threshold_hysteresis_value = 0;    /* KEY_VAL_NEGA_HYST(KEY_TEMP_LCR) */

	key_sr[i].reserved2 = 0;
	key_sr[i].reserved3 = 0;
	key_sr[i].oem = 0;
	key_sr[i].id_type_length.type = 3;	                        /* 11 = 8-bit ASCII + Latin 1. */
	snprintf(id_string, 16, "key-%d", i);
	key_sr[i].id_type_length.length = strlen(id_string);        /* length of following data, in characters */
	memcpy(key_sr[i].id_string_bytes, id_string, strlen(id_string));

    key_sd[i].sensor_id = 0;
    key_sd[i].last_sensor_reading = 0;
    key_sd[i].scan_period = 0;
    key_sd[i].scan_function = key_scan_function;
    key_sd[i].sett_function = key_sett_function;
    key_sd[i].event_messages_enabled = 1;
    key_sd[i].sensor_scanning_enabled = 1;
    key_sd[i].unavailable = 0;
    key_sd[i].local_sensor_id = i;
    key_sd[i].recv1 = 0;
    key_sd[i].recv2 = 0;
    key_sd[i].recv3 = 0;

	sensor_add(&key_sd[i], SDR_RECORD_TYPE_FULL_SENSOR, &key_sr[i]);
}

void key_init(void)
{
    uint8_t i;

    key_init_chip();

    for (i = 0; i < MAX_KEY_SENSOR_COUNT; i++)
    {
        key_init_sensor_record(i);
    }
}


