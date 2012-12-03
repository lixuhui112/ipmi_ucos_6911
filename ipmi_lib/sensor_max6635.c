//*****************************************************************************
//
// sensor_max6635.c - The MAX6635 Sensor
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
#include "app/lib_gpio.h"
#include "app/lib_io.h"
#include <string.h>
#include <stdio.h>

#define MAX_MAX6635_SENSOR_COUNT    2
#define MAX6635_TEMP_MIN            -55
#define MAX6635_TEMP_MAX            150
#define MAX6635_TEMP_UNR            80
#define MAX6635_TEMP_UCR            64
#define MAX6635_TEMP_UNC            0
#define MAX6635_TEMP_LNR            -15
#define MAX6635_TEMP_LCR            0
#define MAX6635_TEMP_LNC            0
#define MAX6635_TEMP_HYST           2


#define MAX6635_REG_TEMP            0x00    /* 16bit Temperature register (READ only)*/
#define MAX6635_REG_CFG             0x01    /*  8bit Configuration-Byte register Power-on default = 0h. */
#define MAX6635_REG_THYST           0x02    /* 16bit THYST register Power-On Default: THYST = 2°C   (0100h) */
#define MAX6635_REG_TMAX            0x03    /* 16bit TMAX register  Power-On Default: TMAX  = +80°C (2008h) */
#define MAX6635_REG_TLOW            0x04    /* 16bit TLOW register  Power-On Default: TLOW  = +10°C (0500h) */
#define MAX6635_REG_THIGH           0x05    /* 16bit THIGH register Power-On Default: THIGH = +64°C (2000h) */

#define MAX6635_ALERT_TLOW          0x1
#define MAX6635_ALERT_THIGH         0x2
#define MAX6635_ALERT_TMAX          0x4

#define MAX6635_CFG_SHUTDOWN        0x01
#define MAX6635_CFG_CMPINT          0x02
#define MAX6635_CFG_OVERT_PLT       0x04
#define MAX6635_CFG_ALERT_PLT       0x08
#define MAX6635_CFG_FLT_QUE         0x10
#define MAX6635_CFG_SMB             0x20

#define MAX6635_VAL_M               1
#define MAX6635_VAL_B               64
#define MAX6635_VAL_MIN             ((uint8_t)(MAX6635_TEMP_MIN - MAX6635_VAL_B))
#define MAX6635_VAL_MAX             ((uint8_t)(MAX6635_TEMP_MAX - MAX6635_VAL_B))
#define MAX6635_VAL_K1              0
#define MAX6635_VAL_K2              0
#define MAX6635_VAL_UNR(v)          ((uint8_t)(v - MAX6635_VAL_B))  /* upper_non_recoverable_threshold      80 degress C    */
#define MAX6635_VAL_UCR(v)          ((uint8_t)(v - MAX6635_VAL_B))  /* upper_critical_threshold             64 degress C    */
#define MAX6635_VAL_UNC(v)          0                               /* upper_non_critical_threshold                         */
#define MAX6635_VAL_LNR(v)          ((uint8_t)(v - MAX6635_VAL_B))  /* lower_non_recoverable_threshold      -15 degress C   */
#define MAX6635_VAL_LCR(v)          ((uint8_t)(v - MAX6635_VAL_B))  /* lower_critical_threshold             0 degress C     */
#define MAX6635_VAL_LNC(v)          0                               /* lower_non_critical_threshold                         */
#define MAX6635_VAL_TOLERANCE       2
#define MAX6635_VAL_POSI_HYST(v)    ((uint8_t)(v - MAX6635_TEMP_HYST))
#define MAX6635_VAL_NEGA_HYST(v)    ((uint8_t)(v + MAX6635_TEMP_HYST))

#define THRESHOLD_VAL_2_REG(v)      ((uint16_t)((v + MAX6635_VAL_B) << 7))
#define THRESHOLD_REG_2_VAL(v)      ((uint8_t)((v >> 7) - MAX6635_VAL_B))
#define TEMPERATURE_REG_2_VAL(v)    ((uint8_t)((v >> 7) - MAX6635_VAL_B))
#define TEMPERATURE_REG_2_TRD(v)    ((uint8_t)(v & 0x7))

void max6635_scan_function(void *arg);
void max6635_sett_function(void *arg);
void max6635_over_handler(void *param);
void max6635_alert_handler(void *param);

const unsigned char max6635_i2c_addr[MAX_MAX6635_SENSOR_COUNT] =
        {MAX6635_SLAVE_ADDR_1, MAX6635_SLAVE_ADDR_2};      /* 8bit address */

SDR_RECORD_FULL max6635_sr[MAX_MAX6635_SENSOR_COUNT];
sensor_data_t max6635_sd[MAX_MAX6635_SENSOR_COUNT];
I2C_DEVICE max6635_dev[MAX_MAX6635_SENSOR_COUNT];
const IO_INT max6635_over_int[MAX_MAX6635_SENSOR_COUNT] =
{
    {
        .peripheral     = GPIO_CHIP_MAX6635_PERIPH0_BASE,
        .port           = GPIO_CHIP_MAX6635_OVER0_PORT,
        .pins           = GPIO_CHIP_MAX6635_OVER0_PIN,
        .interrupt      = GPIO_CHIP_MAX6635_OVER0_INT,
        .handler        = max6635_over_handler,
        .param          = &max6635_sd[0],
    },
    {
        .peripheral     = GPIO_CHIP_MAX6635_PERIPH1_BASE,
        .port           = GPIO_CHIP_MAX6635_OVER1_PORT,
        .pins           = GPIO_CHIP_MAX6635_OVER1_PIN,
        .interrupt      = GPIO_CHIP_MAX6635_OVER1_INT,
        .handler        = max6635_over_handler,
        .param          = &max6635_sd[1],
    },
};
const IO_INT max6635_alert_int[MAX_MAX6635_SENSOR_COUNT] =
{
    {
        .peripheral     = GPIO_CHIP_MAX6635_PERIPH0_BASE,
        .port           = GPIO_CHIP_MAX6635_ALERT0_PORT,
        .pins           = GPIO_CHIP_MAX6635_ALERT0_PIN,
        .interrupt      = GPIO_CHIP_MAX6635_ALERT0_INT,
        .handler        = max6635_alert_handler,
        .param          = &max6635_sd[0],
    },
    {
        .peripheral     = GPIO_CHIP_MAX6635_PERIPH1_BASE,
        .port           = GPIO_CHIP_MAX6635_ALERT1_PORT,
        .pins           = GPIO_CHIP_MAX6635_ALERT1_PIN,
        .interrupt      = GPIO_CHIP_MAX6635_ALERT1_INT,
        .handler        = max6635_alert_handler,
        .param          = &max6635_sd[1],
    },
};


void max6635_over_handler(void *param)
{
    sensor_data_t *sd = (sensor_data_t *)param;

    max6635_scan_function(param);

    if (sd->lower_critical_threshold ||
        sd->upper_critical_threshold ||
        sd->upper_non_recoverable_threshold)
    {
        /* TODO:
         * bring a over event message to system
         * and save it to SEL
         */
    }
}

void max6635_alert_handler(void *param)
{
    sensor_data_t *sd = (sensor_data_t *)param;

    max6635_scan_function(param);

    if (sd->lower_critical_threshold ||
        sd->upper_critical_threshold ||
        sd->upper_non_recoverable_threshold)
    {
        /* TODO:
         * bring a alert event message to system
         * and save it to SEL
         */
    }
}

void max6635_init_chip(void)
{
    I2C_i2c1_slave_dev_init(&max6635_dev[0], max6635_i2c_addr[0], 1);
    //IO_gpio_int_register(&max6635_over_int[0]);
    //IO_gpio_int_register(&max6635_alert_int[0]);

    I2C_i2c1_slave_dev_init(&max6635_dev[1], max6635_i2c_addr[1], 1);
    //IO_gpio_int_register(&max6635_over_int[1]);
    //IO_gpio_int_register(&max6635_alert_int[1]);
}

void max6635_sett_function(void *arg)
{
    sensor_data_t *sd = (sensor_data_t *)arg;
    uint8_t max6635_id;
    int16_t temp_val;

    max6635_id = sd->local_sensor_id;

    if (max6635_sr[max6635_id].mask.type.threshold.lower_non_recoverable_settable &&
        max6635_sd[max6635_id].set_lower_non_recoverable_threshold)
    {
        // MAX6635 not have lower non recoverable alert register
    }

    if (max6635_sr[max6635_id].mask.type.threshold.lower_critical_settable &&
        max6635_sd[max6635_id].set_lower_critical_threshold)
    {
        temp_val = THRESHOLD_VAL_2_REG(max6635_sr[max6635_id].lower_critical_threshold);
        temp_val = BSWAP_16(temp_val);

        I2C_i2c1_slave_dev_set(&max6635_dev[max6635_id], MAX6635_REG_TLOW, (uint8_t*)&temp_val, 2);
        I2C_i2c1_master_write(&max6635_dev[max6635_id]);
    }

    if (max6635_sr[max6635_id].mask.type.threshold.lower_non_critical_settable &&
        max6635_sd[max6635_id].set_lower_non_critical_threshold)
    {
        // MAX6635 not have lower non critical alert register
    }

    if (max6635_sr[max6635_id].mask.type.threshold.upper_non_critical_settable &&
        max6635_sd[max6635_id].set_upper_non_critical_threshold)
    {
        // MAX6635 not have upper non critical alert register
    }

    if (max6635_sr[max6635_id].mask.type.threshold.upper_critical_settable &&
        max6635_sd[max6635_id].set_upper_critical_threshold)
    {
        temp_val = THRESHOLD_VAL_2_REG(max6635_sr[max6635_id].upper_critical_threshold);
        temp_val = BSWAP_16(temp_val);

        I2C_i2c1_slave_dev_set(&max6635_dev[max6635_id], MAX6635_REG_THIGH, (uint8_t*)&temp_val, 2);
        I2C_i2c1_master_write(&max6635_dev[max6635_id]);
    }

    if (max6635_sr[max6635_id].mask.type.threshold.upper_non_recoverable_settable &&
        max6635_sd[max6635_id].set_upper_non_recoverable_threshold)
    {
        temp_val = THRESHOLD_VAL_2_REG(max6635_sr[max6635_id].upper_non_recoverable_threshold);
        temp_val = BSWAP_16(temp_val);

        I2C_i2c1_slave_dev_set(&max6635_dev[max6635_id], MAX6635_REG_TMAX, (uint8_t*)&temp_val, 2);
        I2C_i2c1_master_write(&max6635_dev[max6635_id]);
    }
}

void max6635_scan_function(void *arg)
{
    sensor_data_t *sd = (sensor_data_t *)arg;
    uint32_t error;
    int16_t temp_val = 0;
    uint8_t temp_trd;
    uint8_t max6635_id = sd->local_sensor_id;

    if (max6635_id >= MAX_MAX6635_SENSOR_COUNT) {
        sd->unavailable = 1;
        return;
    }

    /*
     * read the temperature from the physical chip device
     */
    I2C_i2c1_slave_dev_set(&max6635_dev[max6635_id], MAX6635_REG_TEMP, (uint8_t*)&temp_val, 2);
    error = I2C_i2c1_master_read(&max6635_dev[max6635_id]);
    if (error != 0) {
        //sd->recv2 = error & 0x1f;
        sd->unavailable = 1;
        return;
    } else {
        sd->unavailable = 0;
    }

    temp_val = BSWAP_16(temp_val);

    sd->last_sensor_reading = TEMPERATURE_REG_2_VAL(temp_val);
    temp_trd = TEMPERATURE_REG_2_TRD(temp_val);

    sd->lower_non_recoverable_threshold = 0;
    sd->lower_critical_threshold = 0;
    sd->lower_non_critical_threshold = 0;
    sd->upper_non_critical_threshold = 0;
    sd->upper_critical_threshold = 0;
    sd->upper_non_recoverable_threshold = 0;

    if (temp_trd & MAX6635_ALERT_TLOW)
        sd->lower_critical_threshold = 1;

    if (temp_trd & MAX6635_ALERT_THIGH)
        sd->upper_critical_threshold = 1;

    if (temp_trd & MAX6635_ALERT_TMAX)
        sd->upper_non_recoverable_threshold = 1;

    /* TODO:
     * read Thyst Tmax Tlow Thigh register from chip to SDR
     */
    I2C_i2c1_slave_dev_set(&max6635_dev[max6635_id], MAX6635_REG_TMAX, (uint8_t*)&temp_val, 2);
    I2C_i2c1_master_read(&max6635_dev[max6635_id]);
    temp_val = BSWAP_16(temp_val);
	max6635_sr[max6635_id].upper_non_recoverable_threshold = THRESHOLD_REG_2_VAL(temp_val);

    I2C_i2c1_slave_dev_set(&max6635_dev[max6635_id], MAX6635_REG_THIGH, (uint8_t*)&temp_val, 2);
    I2C_i2c1_master_read(&max6635_dev[max6635_id]);
    temp_val = BSWAP_16(temp_val);
	max6635_sr[max6635_id].upper_critical_threshold = THRESHOLD_REG_2_VAL(temp_val);

    I2C_i2c1_slave_dev_set(&max6635_dev[max6635_id], MAX6635_REG_TLOW, (uint8_t*)&temp_val, 2);
    I2C_i2c1_master_read(&max6635_dev[max6635_id]);
    temp_val = BSWAP_16(temp_val);
	max6635_sr[max6635_id].lower_critical_threshold = THRESHOLD_REG_2_VAL(temp_val);

	//max6635_sr[max6635_id].upper_non_critical_threshold = THRESHOLD_REG_2_VAL(MAX6635_TEMP_UNC);
	//max6635_sr[max6635_id].lower_non_recoverable_threshold = THRESHOLD_REG_2_VAL(MAX6635_TEMP_LNR);
	//max6635_sr[max6635_id].lower_non_critical_threshold = THRESHOLD_REG_2_VAL(MAX6635_TEMP_LNC);
}

void max6635_init_sensor_record(uint8_t i)
{
    char id_string[16];

	/* SENSOR RECORD HEADER */
	max6635_sr[i].header.record_id         = 1;
	max6635_sr[i].header.sdr_version       = 0x51;
	max6635_sr[i].header.record_type       = SDR_RECORD_TYPE_FULL_SENSOR;
	max6635_sr[i].header.record_len        = sizeof(max6635_sr[i]);

	/* RECORD KEY BYTES */
	max6635_sr[i].key.owner_id             = 0;	                    /* 7-bit system software ID */
	max6635_sr[i].key.id_type              = 1;	                    /* System software type */
	max6635_sr[i].key.channel_num          = 0;
	max6635_sr[i].key.sensor_owner_lun     = 0;
	max6635_sr[i].key.sensor_number        = 0;	                    /* -CHANGE- this will get replaced by the actual
	                                                                    sensor number when we register the SDR */

    /* RECORD BODY BYTES */
	max6635_sr[i].entity_id.id = ENTITY_ID_SYSTEM_BOARD;            /* physical entity the sensor is monitoring */
	max6635_sr[i].entity_id.entity_type = 0;	                    /* treat entity as a physical entity */
	max6635_sr[i].entity_id.entity_instance_num  = 0;

	/* Sensor Default (power up) State */
	max6635_sr[i].initialization.powerup_sensor_scanning = 1;       /* sensor scanning enabled */
    max6635_sr[i].initialization.powerup_evt_generation = 0;        /* event generation disabled */
    max6635_sr[i].initialization.init_sensor_type = 0;
    max6635_sr[i].initialization.init_hysteresis = 0;
    max6635_sr[i].initialization.init_thresholds = 0;
    max6635_sr[i].initialization.init_events = 0;
    max6635_sr[i].initialization.init_scanning = 1;                 /* accepts scanning bit in the SetSensorEventEnable command */

	/* Sensor Capabilities */
	max6635_sr[i].capabilities.ignore_sensor = 0;			        /* Ignore sensor if entity is not present or disabled. */
	max6635_sr[i].capabilities.sensor_rearm_support = 1;		    /* automatically rearms itself when the event clears */
	max6635_sr[i].capabilities.sensor_hysteresis_support = 2; 	    /* hysteresis is readable and settable */
	max6635_sr[i].capabilities.sensor_threshold_access = 2;	        /* thresholds are readable and settable
                                                                        per Reading Mask and Settable Threshold Mask, respectively */
	max6635_sr[i].capabilities.event_msg_control = 0;			    /* per threshold/discrete-state event
                                                                        enable/disable control (implies that entire
                                                                        sensor and global disable are also supported) */
	max6635_sr[i].sensor_type = SENSOR_TYPE_TEMPERATURE;		    /* From Table 42-3, Sensor Type Codes */
	max6635_sr[i].event_type_code = 1;		                        /* From Table 42-1, Threshold */

	/* Event Mask */
	max6635_sr[i].mask.type.threshold.assert_lower_non_critical_low = 0;

    /* Assertion Event Mask */
    max6635_sr[i].mask.type.threshold.assert_lower_non_critical_low = 0;
    max6635_sr[i].mask.type.threshold.assert_lower_non_critical_high = 0;
    max6635_sr[i].mask.type.threshold.assert_lower_critical_low = 0;
    max6635_sr[i].mask.type.threshold.assert_lower_critical_high = 1;
    max6635_sr[i].mask.type.threshold.assert_lower_non_recoverable_low = 0;
    max6635_sr[i].mask.type.threshold.assert_lower_non_recoverable_high = 1;
    max6635_sr[i].mask.type.threshold.assert_upper_non_critical_low = 0;
    max6635_sr[i].mask.type.threshold.assert_upper_non_critical_high = 0;
    max6635_sr[i].mask.type.threshold.assert_upper_critical_low = 0;
    max6635_sr[i].mask.type.threshold.assert_upper_critical_high = 1;
    max6635_sr[i].mask.type.threshold.assert_upper_non_recoverable_low = 0;
    max6635_sr[i].mask.type.threshold.assert_upper_non_recoverable_high = 1;

    /* Lower/Upper Threshold Reading Mask
     * for Command 'Get Sensor Reading'
     */
    max6635_sr[i].mask.type.threshold.status_lower_non_critical = 0;
    max6635_sr[i].mask.type.threshold.status_lower_critical = 1;                /* TLOW */
    max6635_sr[i].mask.type.threshold.status_lower_non_recoverable = 0;
    max6635_sr[i].mask.type.threshold.status_upper_non_critical = 0;
    max6635_sr[i].mask.type.threshold.status_upper_critical = 1;                /* THIGH */
    max6635_sr[i].mask.type.threshold.status_upper_non_recoverable = 1;         /* TMAX */

    /* Deassertion Event Mask */
    max6635_sr[i].mask.type.threshold.dassert_lower_non_critical_low = 0;
    max6635_sr[i].mask.type.threshold.dassert_lower_non_critical_high = 0;
    max6635_sr[i].mask.type.threshold.dassert_lower_critical_low = 0;
    max6635_sr[i].mask.type.threshold.dassert_lower_critical_high = 1;
    max6635_sr[i].mask.type.threshold.dassert_lower_non_recoverable_low = 0;
    max6635_sr[i].mask.type.threshold.dassert_lower_non_recoverable_high = 1;
    max6635_sr[i].mask.type.threshold.dassert_upper_non_critical_low = 0;
    max6635_sr[i].mask.type.threshold.dassert_upper_non_critical_high = 0;
    max6635_sr[i].mask.type.threshold.dassert_upper_critical_low = 0;
    max6635_sr[i].mask.type.threshold.dassert_upper_critical_high = 1;
    max6635_sr[i].mask.type.threshold.dassert_upper_non_recoverable_low = 0;
    max6635_sr[i].mask.type.threshold.dassert_upper_non_recoverable_high = 1;

    /* Settable Threshold Mask, Readable Threshold Mask
     * for Command 'Set Sensor Thresholds' and 'Get Sensor Thresholds'
     */
    max6635_sr[i].mask.type.threshold.lower_non_critical_readable = 0;
    max6635_sr[i].mask.type.threshold.lower_critical_readable = 1;          /* TLOW */
    max6635_sr[i].mask.type.threshold.lower_non_recoverable_readable = 0;
    max6635_sr[i].mask.type.threshold.upper_non_critical_readable = 0;
    max6635_sr[i].mask.type.threshold.upper_critical_readable = 1;          /* THIGH */
    max6635_sr[i].mask.type.threshold.upper_non_recoverable_readable = 1;   /* TMAX */

    max6635_sr[i].mask.type.threshold.lower_non_critical_settable = 0;
    max6635_sr[i].mask.type.threshold.lower_critical_settable = 1;
    max6635_sr[i].mask.type.threshold.lower_non_recoverable_settable = 0;
    max6635_sr[i].mask.type.threshold.upper_non_critical_settable = 0;
    max6635_sr[i].mask.type.threshold.upper_critical_settable = 1;
    max6635_sr[i].mask.type.threshold.upper_non_recoverable_settable = 1;


    /* Unit 1, 2, 3 */
    max6635_sr[i].unit.analog_data_format = 2;		                /* 2's complement (signed) */
	max6635_sr[i].unit.rate_unit = 0;			                    /* none */
	max6635_sr[i].unit.modifier_unit = 0;			                /* 00b = none */
	max6635_sr[i].unit.percentage = 0;                              /* not a percentage value */
	max6635_sr[i].unit.unit_2_base = SENSOR_UNIT_DEGREES_CELSIUS;   /* Base Unit */
	max6635_sr[i].unit.unit_3_modifier = 0;                         /* no modifier unit */
	max6635_sr[i].linearization = 0;		                        /* Linear */

	max6635_sr[i].mtol.m_ls = FORMULA_M_LS(MAX6635_VAL_M);          /* M = 1 */
	max6635_sr[i].mtol.m_ms = FORMULA_M_MS(MAX6635_VAL_M);
	max6635_sr[i].mtol.tolerance = MAX6635_VAL_TOLERANCE;           /* Tolerance in +/- 1/2 raw counts */

	max6635_sr[i].bacc.b_ls = FORMULA_B_LS(MAX6635_VAL_B);
	max6635_sr[i].bacc.b_ms = FORMULA_B_MS(MAX6635_VAL_B);
	max6635_sr[i].bacc.accuracy_ls = 0;
	max6635_sr[i].bacc.accuracy_ms = 0;
	max6635_sr[i].bacc.accuracy_exp = 0;
	max6635_sr[i].bacc.sensor_dir = 0;
	max6635_sr[i].bacc.b_exponent = 0;
	max6635_sr[i].bacc.r_exponent = 0;

	max6635_sr[i].analog_flags.nominal_reading_specified = 0;
	max6635_sr[i].analog_flags.normal_max_specified = 1;
	max6635_sr[i].analog_flags.normal_min_specified = 1;

	max6635_sr[i].sensor_maximum_reading = MAX6635_VAL_MAX;
	max6635_sr[i].sensor_minimum_reading = MAX6635_VAL_MIN;

	max6635_sr[i].upper_non_recoverable_threshold = MAX6635_VAL_UNR(MAX6635_TEMP_UNR);
	max6635_sr[i].upper_critical_threshold = MAX6635_VAL_UCR(MAX6635_TEMP_UCR);
	max6635_sr[i].upper_non_critical_threshold = MAX6635_VAL_UNC(MAX6635_TEMP_UNC);
	max6635_sr[i].lower_non_recoverable_threshold = MAX6635_VAL_LNR(MAX6635_TEMP_LNR);
	max6635_sr[i].lower_critical_threshold = MAX6635_VAL_LCR(MAX6635_TEMP_LCR);
	max6635_sr[i].lower_non_critical_threshold = MAX6635_VAL_LNC(MAX6635_TEMP_LNC);

	max6635_sr[i].positive_going_threshold_hysteresis_value = 0;    /* MAX6635_VAL_POSI_HYST(MAX6635_TEMP_UCR) */
	max6635_sr[i].negative_going_threshold_hysteresis_value = 0;    /* MAX6635_VAL_NEGA_HYST(MAX6635_TEMP_LCR) */

	max6635_sr[i].reserved2 = 0;
	max6635_sr[i].reserved3 = 0;
	max6635_sr[i].oem = 0;
	max6635_sr[i].id_type_length.type = 3;	                        /* 11 = 8-bit ASCII + Latin 1. */
	snprintf(id_string, 16, "max6635-%d", i);
	max6635_sr[i].id_type_length.length = strlen(id_string);        /* length of following data, in characters */
	memcpy(max6635_sr[i].id_string_bytes, id_string, strlen(id_string));

    max6635_sd[i].sensor_id = 0;
    max6635_sd[i].last_sensor_reading = 0;
    max6635_sd[i].scan_period = 0;
    max6635_sd[i].scan_function = max6635_scan_function;
    max6635_sd[i].sett_function = max6635_sett_function;
    max6635_sd[i].event_messages_enabled = 1;
    max6635_sd[i].sensor_scanning_enabled = 1;
    max6635_sd[i].unavailable = 0;
    max6635_sd[i].local_sensor_id = i;
    max6635_sd[i].recv1 = 0;
    max6635_sd[i].recv2 = 0;
    max6635_sd[i].recv3 = 0;

	sensor_add(&max6635_sd[i], SDR_RECORD_TYPE_FULL_SENSOR, &max6635_sr[i]);
}

void max6635_init(void)
{
    uint8_t i;

    max6635_init_chip();

    for (i = 0; i < MAX_MAX6635_SENSOR_COUNT; i++)
    {
        max6635_init_sensor_record(i);
    }
}


