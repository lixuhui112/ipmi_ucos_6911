//*****************************************************************************
//
// sensor_adt7470.c - The ADT7470 Sensor
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

/******************************************************************************
TODO:
    2012/9/26   风扇转速阈值告警中断需要调试
                风扇PWM值需要设置

History:
    2012/9/29   完成基本的传感器SE命令框架，支持的命令包括:
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

#ifdef IPMI_CHIP_ADT7470

#define MAX_ADT7470_COUNT           8       /* 传感器数量       */
#define MAX_ADT7470_DEV             2       /* 设备数量         */
#define MAX_ADT7470_SE_PER_DEV      4       /* 每设备传感器数量 */

#define ADT7470_REG_TACH1           0x2a    /* Fan Tach Reading Registers (Power-On Default = 0x00). */
#define ADT7470_REG_TACH2           0x2c
#define ADT7470_REG_TACH3           0x2e
#define ADT7470_REG_TACH4           0x30

#define ADT7470_REG_CDC_PWM1        0x32    /* Current PWM Duty Cycle Registers (Power-On Default = 0xFF). */
#define ADT7470_REG_CDC_PWM2        0x33
#define ADT7470_REG_CDC_PWM3        0x34
#define ADT7470_REG_CDC_PWM4        0x35

#define ADT7470_REG_MAXDC_PWM1      0x38    /* PWM Max Duty Cycle Registers (Power-On Default = 0xFF). */
#define ADT7470_REG_MAXDC_PWM2      0x39
#define ADT7470_REG_MAXDC_PWM3      0x3a
#define ADT7470_REG_MAXDC_PWM4      0x3b

#define ADT7470_REG_DEV_ID          0x3d    /* Device ID Register (Power-On Default = 0x70). */
#define ADT7470_REG_COM_ID          0x3e    /* Company ID Register (Power-On Default = 0x41). */
#define ADT7470_REG_REV_ID          0x3f    /* Revision Register (Power-On Default = 0x02). */

#define ADT7470_REG_CFG1            0x40    /* Configuration Register 1 (Power-On Default = 0x01). */
#define ADT7470_CFG1_STRT           0x01    /* Bit[0]
                                               Logic 1 enables monitoring and PWM control outputs
                                                based on the limit settings programmed.
                                               Logic 0 disables monitoring and PWM control based on
                                                the default power-up limit settings.
                                               The limit values programmed are preserved even if
                                                a Logic 0 is written to this bit and the default
                                                settings are enabled. */
#define ADT7470_CFG1_FST_TCH        0x20    /* BIT[5]
                                               Enable Fast Tach measurement.
                                               0 = Tach measurement rate is 1 measurement per second
                                               1 = Tach measurement rate is 1 measurement every 250ms */
#define ADT7470_CFG1_HFLF           0x40    /* Bit[6]
                                               This bit switches between high frequency and low frequency fan drive.
                                               0 (default) = high frequency fan drive (1.4 kHz or 22.5 kHz.
                                                See Configuration Register 2, Register 0x74, Bits [6:4]) in Table 44.
                                               1 = low frequency fandrive (frequency determined by Configuration Register 2,
                                                Register 0x74, Bits[6:4]) in Table 44. */

#define ADT7470_REG_INT_STATUS1     0x41    /* Interrupt Status Register 1 (Power-On Default = 0x00). */
#define ADT7470_INT_OOL             0x80    /* Bit[7]
                                               A 1 indicates that an out-of-limit event has been latched in Status Register 2.
                                               This bit is a logical OR of all status bits in Status Register 2.
                                               Software can test this bit in isolation to determine whether any of
                                               the temperature or fan speed readings represented by Status Register 2
                                               are out of limit. This saves the need to read Status Register 2 every
                                               interrupt or polling cycle. */
#define ADT7470_REG_INT_STATUS2     0x42    /* Interrupt Status Register 2 (Power-On Default = 0x00). */
#define ADT7470_INT_FAN1            0x10    /* A 1 indicates that Fan 1 has gone above max speed or dropped below min speed. */
#define ADT7470_INT_FAN2            0x20    /* A 1 indicates that Fan 2 has gone above max speed or dropped below min speed. */
#define ADT7470_INT_FAN3            0x40    /* A 1 indicates that Fan 3 has gone above max speed or dropped below min speed. */
#define ADT7470_INT_FAN4            0x80    /* A 1 indicates that Fan 4 has gone above max speed or dropped below min speed. */

#define ADT7470_REG_FPR             0x43    /* Fan Pulses per Revolution Register (Power-On Default = 0x55).
                                                [1:0] Fan 1
                                                [3:2] Fan 2
                                                [5:4] Fan 3
                                                [7:6] Fan 4
                                               Sets the number of pulses to be counted when measuring Fan 1 speed.
                                               Can be used to determine fan’s pulses per revolution number for unknown fan type.
                                               Pulses Counted
                                                00 = 1
                                                01 = 2 (default)
                                                10 = 3
                                                11 = 4 */
#define ADT7470_FPR_1               0x0
#define ADT7470_FPR_2               0x1
#define ADT7470_FPR_3               0x2
#define ADT7470_FPR_4               0x3

#define ADT7470_REG_TACH1_MIN       0x58    /* Exceeding any of the tach min limit registers shown in Table 36      */
#define ADT7470_REG_TACH2_MIN       0x5a    /*  by 1 indicates that the fan is running too slowly or has stalled.   */
#define ADT7470_REG_TACH3_MIN       0x5c    /*  The appropriate status bit is set in Interrupt Status Register 2    */
#define ADT7470_REG_TACH4_MIN       0x5e    /*  to indicate the fan failure.                                        */
#define ADT7470_REG_TACH1_MAX       0x60    /* Exceeding any of the tach max limit registers by 1 indicates that    */
#define ADT7470_REG_TACH2_MAX       0x62    /*  the fan is too fast. The appropriate status bit is set in           */
#define ADT7470_REG_TACH3_MAX       0x64    /*  Interrupt Status Register 2 to indicate the fan failure.            */
#define ADT7470_REG_TACH4_MAX       0x66

#define ADT7470_REG_CFG_PWM12       0x68    /* PWM1/PWM2 Configuration Register (Power-On Default = 0x00). */
#define ADT7470_CFG_PWM_INV2        0x10    /* Setting this bit to 1 inverts the PWM2 output. Default = 0. */
#define ADT7470_CFG_PWM_INV1        0x20    /* Setting this bit to 1 inverts the PWM1 output. Default = 0. */
#define ADT7470_CFG_PWM_BHVR2       0x40    /* This bit assigns fan behavior for PWM2 output.
                                                0 = manual fan control mode (PWM duty cycle controlled in software).
                                                1 = automatic fan control mode. */
#define ADT7470_CFG_PWM_BHVR1       0x80    /* This bit assigns fan behavior for PWM1 output.
                                                0 = manual fan control mode (PWM duty cycle controlled in software).
                                                1 = automatic fan control mode. */

#define ADT7470_REG_CFG_PWM34       0x69    /* PWM3/PWM4 Configuration Register (Power-On Default = 0x00). */
#define ADT7470_CFG_PWM_INV4        0x10    /* Setting this bit to 1 inverts the PWM4 output. Default = 0 */
#define ADT7470_CFG_PWM_INV3        0x20    /* Setting this bit to 1 inverts the PWM3 output. Default = 0 */
#define ADT7470_CFG_PWM_BHVR4       0x40    /* This bit assigns fan behavior for PWM4 output.
                                                0 = manual fan control mode (PWM duty cycle controlled in software).
                                                1 = (automatic fan control mode. */
#define ADT7470_CFG_PWM_BHVR3       0x80    /* This bit assigns fan behavior for PWM3 output.
                                                0 = manual fan control mode (PWM duty cycle controlled in software).
                                                1 = automatic fan control mode. */

#define ADT7470_REG_MINDC_PWM1      0x6a    /* PWM MIN Duty Cycle Registers (Power-On Default = 0x80 (50% Duty Cycle). */
#define ADT7470_REG_MINDC_PWM2      0x6b    /* These bits define the PWNMIN duty cycle for PWMx (x = 1 to 4).   */
#define ADT7470_REG_MINDC_PWM3      0x6c    /*  0x00 = 0% duty cycle (fan off).     */
#define ADT7470_REG_MINDC_PWM4      0x6d    /*  0x40 = 25% duty cycle.
                                                0x80 = 50% duty cycle.
                                                0xFF = 100% PMW max duty cycle (full fan speed).    */

#define ADT7470_REG_INT_MASK1       0x72    /* Interrupt Mask Register 1 (Power-On Default = 0x00). */
#define ADT7470_REG_INT_MASK2       0x73    /* Interrupt Mask Register 2 (Power-On Default = 0x00). */
#define ADT7470_INT_MASK_FAN1       0x10    /* A 1 masks the Fan 1 value from generating an interrupt on the SMBALERT output. */
#define ADT7470_INT_MASK_FAN2       0x20    /* The Fan x bit is set as normal in the status register for out-of-limit conditions. */
#define ADT7470_INT_MASK_FAN3       0x40    /*  */
#define ADT7470_INT_MASK_FAN4       0x80    /*  */

#define ADT7470_REG_CFG2            0x74    /* Configuration Register 2 (Power-On Default = 0x00). */
#define ADT7470_SHDN                0x80    /* Shutdown/low current mode. */
#define ADT7470_FREQ_MASK           0x70    /* These bits control PWM1–PWM4 frequency when the fan drive
                                                is configured as a low frequency drive. */
#define ADT7470_T4_DIS              0x08    /* Writing a 1 disables Tach 4 measurements. */
#define ADT7470_T3_DIS              0x04    /* Writing a 1 disables Tach 3 measurements. */
#define ADT7470_T2_DIS              0x02    /* Writing a 1 disables Tach 2 measurements. */
#define ADT7470_T1_DIS              0x01    /* Writing a 1 disables Tach 1 measurements. */


////////////////////////////////////////////////////////////////////////////////////////////////

#define ADT7470_SPEED_MIN           0
#define ADT7470_SPEED_MAX           2650

#define ADT7470_VAL_M               (21)
#define ADT7470_VAL_B               (0)
#define ADT7470_VAL_MIN             (ADT7470_SPEED_MIN)
#define ADT7470_VAL_MAX             (ADT7470_SPEED_MAX)
#define ADT7470_VAL_K1              (0)
#define ADT7470_VAL_K2              (0)

#define TACH_RAW_2_VAL(v,fpr)       ((90000 * 60 * fpr) / v)
#define PWM_RAW_2_VAL(pct)          ((float)pct * 2.55)


const unsigned char adt7470_i2c_addr[MAX_ADT7470_DEV] = {ADT7470_SLAVE_ADDR_1, ADT7470_SLAVE_ADDR_2};

I2C_DEVICE adt7470_dev[MAX_ADT7470_DEV];
SDR_RECORD_FULL adt7470_sr[MAX_ADT7470_COUNT];
sensor_data_t adt7470_sd[MAX_ADT7470_COUNT];


void adt7470_init_chip(void)
{
    I2C_i2c1_slave_dev_init(&adt7470_dev[0], adt7470_i2c_addr[0], 1);
    I2C_i2c1_slave_dev_init(&adt7470_dev[1], adt7470_i2c_addr[1], 1);
    //IO_gpio_int_register(&adt7470_int);
}

void adt7470_shutdown(uint8_t local_id)
{
    uint8_t dev, id;
    uint8_t val = 0x00;

    dev = local_id / MAX_ADT7470_SE_PER_DEV;
    id = local_id % MAX_ADT7470_SE_PER_DEV;

#if 0
    /* shutdown all pwm in this chip
     */
    I2C_i2c0_slave_dev_set(&adt7470_dev[dev], ADT7470_REG_CFG1, (uint8_t*)&val, 1);
    I2C_i2c0_master_write(&adt7470_dev[dev]);
#endif

    /* shutdown specific pwm in this chip
     */
    I2C_i2c1_slave_dev_set(&adt7470_dev[dev], ADT7470_REG_CDC_PWM1 + id, (uint8_t*)&val, 1);
    I2C_i2c1_master_write(&adt7470_dev[dev]);
}

void adt7470_set_speed(uint8_t local_id, uint8_t level)
{
    uint8_t dev, id;
    uint8_t val;

    dev = local_id / MAX_ADT7470_SE_PER_DEV;
    id = local_id % MAX_ADT7470_SE_PER_DEV;
    val = (uint8_t)PWM_RAW_2_VAL(level);

    I2C_i2c1_slave_dev_set(&adt7470_dev[dev], ADT7470_REG_CDC_PWM1 + id, (uint8_t*)&val, 1);
    I2C_i2c1_master_write(&adt7470_dev[dev]);
}

void adt7470_init_function(void *arg)
{
    uint32_t error;
    sensor_data_t *sd = (sensor_data_t *)arg;
    uint8_t dev;
    uint8_t val;

    dev = sd->local_sensor_id / MAX_ADT7470_SE_PER_DEV;

    val = ADT7470_CFG1_STRT|ADT7470_CFG1_FST_TCH;

    I2C_i2c1_slave_dev_set(&adt7470_dev[dev], ADT7470_REG_CFG1, (uint8_t*)&val, 1);
    error = I2C_i2c1_master_write(&adt7470_dev[dev]);

    if (error != 0) {
        sd->unavailable = 1;
        return;
    } else {
        sd->unavailable = 0;
    }
}

void adt7470_sett_function(void *arg)
{
    sensor_data_t *sd = (sensor_data_t *)arg;
    uint8_t adt7470_id;

    adt7470_id = sd->local_sensor_id;
    (void)adt7470_id;
}

void adt7470_scan_function(void *arg)
{
    sensor_data_t *sd = (sensor_data_t *)arg;
    uint32_t error;
    uint16_t fan_rpm = 0;
    uint8_t fan_tach[2] = {0};
    uint8_t dev, id = sd->local_sensor_id;

    /* TODO:
     * read the tach from the physical chip device
     */
    dev = id / MAX_ADT7470_SE_PER_DEV;
    id = id % MAX_ADT7470_SE_PER_DEV;

    if (dev >= MAX_ADT7470_DEV || id >= MAX_ADT7470_SE_PER_DEV) {
        sd->unavailable = 1;
        return;
    }

    I2C_i2c1_slave_dev_set(&adt7470_dev[dev], ADT7470_REG_TACH1 + (id * 2), (uint8_t*)&fan_tach[0], 2);
    error = I2C_i2c1_master_read(&adt7470_dev[dev]);
    if (error != 0) {
        sd->unavailable = 1;
        return;
    } else {
        sd->unavailable = 0;
    }

    //temp_val = 0xff17;      // 1758RPM (just for test)
    fan_rpm = ((fan_tach[0]<<8)|fan_tach[1]);

    sd->last_sensor_reading =
        (uint8_t)SENSOR_VAL2RAW(TACH_RAW_2_VAL(fan_rpm, 2),
                ADT7470_VAL_M,
                ADT7470_VAL_B,
                ADT7470_VAL_K1,
                ADT7470_VAL_K2);

    sd->lower_non_recoverable_threshold = 0;
    sd->lower_critical_threshold = 0;
    sd->lower_non_critical_threshold = 0;
    sd->upper_non_critical_threshold = 0;
    sd->upper_critical_threshold = 0;
    sd->upper_non_recoverable_threshold = 0;
}

void adt7470_init_sensor_record(uint8_t i)
{
    char id_string[16];

	/* SENSOR RECORD HEADER */
	adt7470_sr[i].header.record_id         = 1;
	adt7470_sr[i].header.sdr_version       = 0x51;
	adt7470_sr[i].header.record_type       = SDR_RECORD_TYPE_FULL_SENSOR;
	adt7470_sr[i].header.record_len        = sizeof(adt7470_sr[i]);

	/* RECORD KEY BYTES */
	adt7470_sr[i].key.owner_id             = 0;	                    /* 7-bit system software ID */
	adt7470_sr[i].key.id_type              = 1;	                    /* System software type */
	adt7470_sr[i].key.channel_num          = 0;
	adt7470_sr[i].key.sensor_owner_lun     = 0;
	adt7470_sr[i].key.sensor_number        = 0;	                    /* -CHANGE- this will get replaced by the actual
	                                                                    sensor number when we register the SDR */

    /* RECORD BODY BYTES */
	adt7470_sr[i].entity_id.id = ENTITY_ID_SYSTEM_BOARD;            /* physical entity the sensor is monitoring */
	adt7470_sr[i].entity_id.entity_type = 0;	                    /* treat entity as a physical entity */
	adt7470_sr[i].entity_id.entity_instance_num  = 0;

	/* Sensor Default (power up) State */
	adt7470_sr[i].initialization.powerup_sensor_scanning = 1;       /* sensor scanning enabled */
    adt7470_sr[i].initialization.powerup_evt_generation = 0;        /* event generation disabled */
    adt7470_sr[i].initialization.init_sensor_type = 0;
    adt7470_sr[i].initialization.init_hysteresis = 0;
    adt7470_sr[i].initialization.init_thresholds = 0;
    adt7470_sr[i].initialization.init_events = 0;
    adt7470_sr[i].initialization.init_scanning = 1;                 /* accepts scanning bit in the SetSensorEventEnable command */

	/* Sensor Capabilities */
	adt7470_sr[i].capabilities.ignore_sensor = 0;			        /* Ignore sensor if entity is not present or disabled. */
	adt7470_sr[i].capabilities.sensor_rearm_support = 1;		    /* automatically rearms itself when the event clears */
	adt7470_sr[i].capabilities.sensor_hysteresis_support = 2; 	    /* hysteresis is readable and settable */
	adt7470_sr[i].capabilities.sensor_threshold_access = 2;	        /* thresholds are readable and settable
                                                                        per Reading Mask and Settable Threshold Mask, respectively */
	adt7470_sr[i].capabilities.event_msg_control = 0;			    /* per threshold/discrete-state event
                                                                        enable/disable control (implies that entire
                                                                        sensor and global disable are also supported) */
	adt7470_sr[i].sensor_type = SENSOR_TYPE_FAN;		            /* From Table 42-3, Sensor Type Codes */
	adt7470_sr[i].event_type_code = 1;		                        /* From Table 42-1, Threshold */

	/* Event Mask */
	adt7470_sr[i].mask.type.threshold.assert_lower_non_critical_low = 0;

    /* Assertion Event Mask */
    adt7470_sr[i].mask.type.threshold.assert_lower_non_critical_low = 0;
    adt7470_sr[i].mask.type.threshold.assert_lower_non_critical_high = 0;
    adt7470_sr[i].mask.type.threshold.assert_lower_critical_low = 0;
    adt7470_sr[i].mask.type.threshold.assert_lower_critical_high = 1;
    adt7470_sr[i].mask.type.threshold.assert_lower_non_recoverable_low = 0;
    adt7470_sr[i].mask.type.threshold.assert_lower_non_recoverable_high = 1;
    adt7470_sr[i].mask.type.threshold.assert_upper_non_critical_low = 0;
    adt7470_sr[i].mask.type.threshold.assert_upper_non_critical_high = 0;
    adt7470_sr[i].mask.type.threshold.assert_upper_critical_low = 0;
    adt7470_sr[i].mask.type.threshold.assert_upper_critical_high = 1;
    adt7470_sr[i].mask.type.threshold.assert_upper_non_recoverable_low = 0;
    adt7470_sr[i].mask.type.threshold.assert_upper_non_recoverable_high = 1;

    /* Lower/Upper Threshold Reading Mask
     * for Command 'Get Sensor Reading'
     */
    adt7470_sr[i].mask.type.threshold.status_lower_non_critical = 0;
    adt7470_sr[i].mask.type.threshold.status_lower_critical = 1;                /* TLOW */
    adt7470_sr[i].mask.type.threshold.status_lower_non_recoverable = 0;
    adt7470_sr[i].mask.type.threshold.status_upper_non_critical = 0;
    adt7470_sr[i].mask.type.threshold.status_upper_critical = 1;                /* THIGH */
    adt7470_sr[i].mask.type.threshold.status_upper_non_recoverable = 1;         /* TMAX */

    /* Deassertion Event Mask */
    adt7470_sr[i].mask.type.threshold.dassert_lower_non_critical_low = 0;
    adt7470_sr[i].mask.type.threshold.dassert_lower_non_critical_high = 0;
    adt7470_sr[i].mask.type.threshold.dassert_lower_critical_low = 0;
    adt7470_sr[i].mask.type.threshold.dassert_lower_critical_high = 1;
    adt7470_sr[i].mask.type.threshold.dassert_lower_non_recoverable_low = 0;
    adt7470_sr[i].mask.type.threshold.dassert_lower_non_recoverable_high = 1;
    adt7470_sr[i].mask.type.threshold.dassert_upper_non_critical_low = 0;
    adt7470_sr[i].mask.type.threshold.dassert_upper_non_critical_high = 0;
    adt7470_sr[i].mask.type.threshold.dassert_upper_critical_low = 0;
    adt7470_sr[i].mask.type.threshold.dassert_upper_critical_high = 1;
    adt7470_sr[i].mask.type.threshold.dassert_upper_non_recoverable_low = 0;
    adt7470_sr[i].mask.type.threshold.dassert_upper_non_recoverable_high = 1;

    /* Settable Threshold Mask, Readable Threshold Mask
     * for Command 'Set Sensor Thresholds' and 'Get Sensor Thresholds'
     */
    adt7470_sr[i].mask.type.threshold.lower_non_critical_readable = 0;
    adt7470_sr[i].mask.type.threshold.lower_critical_readable = 1;          /* TLOW */
    adt7470_sr[i].mask.type.threshold.lower_non_recoverable_readable = 0;
    adt7470_sr[i].mask.type.threshold.upper_non_critical_readable = 0;
    adt7470_sr[i].mask.type.threshold.upper_critical_readable = 1;          /* THIGH */
    adt7470_sr[i].mask.type.threshold.upper_non_recoverable_readable = 1;   /* TMAX */

    adt7470_sr[i].mask.type.threshold.lower_non_critical_settable = 0;
    adt7470_sr[i].mask.type.threshold.lower_critical_settable = 1;
    adt7470_sr[i].mask.type.threshold.lower_non_recoverable_settable = 0;
    adt7470_sr[i].mask.type.threshold.upper_non_critical_settable = 0;
    adt7470_sr[i].mask.type.threshold.upper_critical_settable = 1;
    adt7470_sr[i].mask.type.threshold.upper_non_recoverable_settable = 1;


    /* Unit 1, 2, 3 */
    adt7470_sr[i].unit.analog_data_format = 2;		                /* 2's complement (signed) */
	adt7470_sr[i].unit.rate_unit = 0;			                    /* none */
	adt7470_sr[i].unit.modifier_unit = 0;			                /* 00b = none */
	adt7470_sr[i].unit.percentage = 0;                              /* not a percentage value */
	adt7470_sr[i].unit.unit_2_base = SENSOR_UNIT_RPM;               /* Base Unit */
	adt7470_sr[i].unit.unit_3_modifier = 0;                         /* no modifier unit */
	adt7470_sr[i].linearization = 0;		                        /* Linear */

	adt7470_sr[i].mtol.m_ls = FORMULA_M_LS(ADT7470_VAL_M);          /* M = 1 */
	adt7470_sr[i].mtol.m_ms = FORMULA_M_MS(ADT7470_VAL_M);
	adt7470_sr[i].mtol.tolerance = 2;                               /* Tolerance in +/- 1/2 raw counts */

	adt7470_sr[i].bacc.b_ls = FORMULA_B_LS(ADT7470_VAL_B);
	adt7470_sr[i].bacc.b_ms = FORMULA_B_MS(ADT7470_VAL_B);
	adt7470_sr[i].bacc.accuracy_ls = 0;
	adt7470_sr[i].bacc.accuracy_ms = 0;
	adt7470_sr[i].bacc.accuracy_exp = 0;
	adt7470_sr[i].bacc.sensor_dir = 0;
	adt7470_sr[i].bacc.b_exponent = 0;
	adt7470_sr[i].bacc.r_exponent = 0;

	adt7470_sr[i].analog_flags.nominal_reading_specified = 0;
	adt7470_sr[i].analog_flags.normal_max_specified = 1;
	adt7470_sr[i].analog_flags.normal_min_specified = 1;

	adt7470_sr[i].sensor_maximum_reading =
	    (uint8_t)SENSOR_VAL2RAW(ADT7470_VAL_MAX, ADT7470_VAL_M, ADT7470_VAL_B, ADT7470_VAL_K1, ADT7470_VAL_K2);
	adt7470_sr[i].sensor_minimum_reading =
	    (uint8_t)SENSOR_VAL2RAW(ADT7470_VAL_MIN, ADT7470_VAL_M, ADT7470_VAL_B, ADT7470_VAL_K1, ADT7470_VAL_K2);

	adt7470_sr[i].upper_non_recoverable_threshold = 0;
	adt7470_sr[i].upper_critical_threshold = 0;
	adt7470_sr[i].upper_non_critical_threshold = 0;
	adt7470_sr[i].lower_non_recoverable_threshold = 0;
	adt7470_sr[i].lower_critical_threshold = 0;
	adt7470_sr[i].lower_non_critical_threshold = 0;

	adt7470_sr[i].positive_going_threshold_hysteresis_value = 0;
	adt7470_sr[i].negative_going_threshold_hysteresis_value = 0;

	adt7470_sr[i].reserved2 = 0;
	adt7470_sr[i].reserved3 = 0;
	adt7470_sr[i].oem = 0;
	adt7470_sr[i].id_type_length.type = 3;	                        /* 11 = 8-bit ASCII + Latin 1. */
	snprintf(id_string, 16, "adt7470-%d", i);
	adt7470_sr[i].id_type_length.length = strlen(id_string);        /* length of following data, in characters */
	memcpy(adt7470_sr[i].id_string_bytes, id_string, strlen(id_string));

    adt7470_sd[i].sensor_id = 0;
    adt7470_sd[i].last_sensor_reading = 0;
    adt7470_sd[i].scan_period = 0;
    adt7470_sd[i].init_function = adt7470_init_function;
    adt7470_sd[i].scan_function = adt7470_scan_function;
    adt7470_sd[i].sett_function = adt7470_sett_function;
    adt7470_sd[i].event_messages_enabled = 1;
    adt7470_sd[i].sensor_scanning_enabled = 1;
    adt7470_sd[i].unavailable = 0;
    adt7470_sd[i].local_sensor_id = i;
    adt7470_sd[i].recv1 = 0;
    adt7470_sd[i].recv2 = 0;
    adt7470_sd[i].recv3 = 0;

	sensor_add(&adt7470_sd[i], SDR_RECORD_TYPE_FULL_SENSOR, &adt7470_sr[i]);
}

void adt7470_init(void)
{
    uint8_t i;

    adt7470_init_chip();

    for (i = 0; i < MAX_ADT7470_COUNT; i++)
    {
        adt7470_init_sensor_record(i);
        adt7470_init_function((void*)&adt7470_sd[i]);
    }
}

#endif  // IPMI_CHIP_ADT7470

