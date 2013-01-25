//*****************************************************************************
//
// sensor_ina230.c - The INA230 Sensor
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

/******************************************************************************
TODO:
    2012/09/26  从硬件获取纹波电阻值，并计算Calibration寄存器配置值
                配置纹波过压、纹波欠压、总线过压、总线欠压、功率超标配置值
    2012/09/28  需要对INA230的传感器精度进行设置

History:
    2012/09/28  完成INA230读取功能
    2012/09/28  INA230拆分为3个传感器，分别是电压、电流、功率

******************************************************************************/


#include "ipmi_lib/ipmi.h"
#include "ipmi_lib/ipmi_cfg.h"
#include <string.h>
#include <stdio.h>

#ifdef IPMI_CHIP_INA230

#define MAX_INA230_DEV          2                       /* 2个芯片，每个拆分成3个传感器，电压、电流、功率 */
#define MAX_INA230_READER       3
#define MAX_INA230_COUNT        (MAX_INA230_DEV*MAX_INA230_READER)
#define INA230_ID_VOL           0
#define INA230_ID_CUR           1
#define INA230_ID_POW           2

#define INA230_REG_SIZE         0x02                    /* 2字节，16bit寄存器 */

#define INA230_REG_CFG          0x00                    /* RW, 配置寄存器地址 */
#define INA230_REG_SHUNT_VOL    0x01                    /* R,  纹波电压 */
#define INA230_REG_BUS_VOL      0x02                    /* R,  总线电压 */
#define INA230_REG_POWER        0x03                    /* R,  功率 */
#define INA230_REG_CURRENT      0x04                    /* R,  电流 */
#define INA230_REG_CALIBRATION  0x05                    /* RW, 校准 */
#define INA230_REG_MASK         0x06                    /* RW, 功能掩码 */
#define INA230_REG_ALERT        0x07                    /* RW, 告警门限 */
#define INA230_REG_DIEID        0xFF                    /* R,  身份识别号 */

#define INA230_CFG_RST          0x8000                  /* 配置寄存器配置值 */
#define INA230_CFG_AVG_1        0x0000
#define INA230_CFG_AVG_4        0x0200
#define INA230_CFG_AVG_16       0x0400
#define INA230_CFG_AVG_64       0x0600
#define INA230_CFG_AVG_128      0x0800
#define INA230_CFG_AVG_256      0x0a00
#define INA230_CFG_AVG_512      0x0c00
#define INA230_CFG_AVG_1024     0x0e00
#define INA230_CFG_VBUS_140US   0x0000
#define INA230_CFG_VBUS_204US   0x0040
#define INA230_CFG_VBUS_332US   0x0080
#define INA230_CFG_VBUS_588US   0x00c0
#define INA230_CFG_VBUS_1MS     0x0100
#define INA230_CFG_VBUS_2MS     0x0140
#define INA230_CFG_VBUS_4MS     0x0180
#define INA230_CFG_VBUS_8MS     0x01c0
#define INA230_CFG_VSH_140US    0x0000
#define INA230_CFG_VSH_204US    0x0008
#define INA230_CFG_VSH_332US    0x0010
#define INA230_CFG_VSH_588US    0x0018
#define INA230_CFG_VSH_1MS      0x0020
#define INA230_CFG_VSH_2MS      0x0028
#define INA230_CFG_VSH_4MS      0x0030
#define INA230_CFG_VSH_8MS      0x0038
#define INA230_CFG_MOD_PWD      0x0000
#define INA230_CFG_MOD_VSH_T    0x0001
#define INA230_CFG_MOD_VBUS_T   0x0002
#define INA230_CFG_MOD_VSHBUS_T 0x0003
#define INA230_CFG_MOD_PWD2     0x0004
#define INA230_CFG_MOD_VSH_C    0x0005
#define INA230_CFG_MOD_VBUS_C   0x0006
#define INA230_CFG_MOD_VSHBUS_C 0x0007

#define INA230_ME_SOL           0x8000                  /* [15] Shunt Voltage Over-Voltage */
#define INA230_ME_SUL           0x4000                  /* [14] Shunt Voltage Under-Voltage */
#define INA230_ME_BOL           0x2000                  /* [13] Bus Voltage Over-Voltage */
#define INA230_ME_BUL           0x1000                  /* [12] Bus Voltage Under-Voltage */
#define INA230_ME_POL           0x0800                  /* [11] Over-Limit Power */
#define INA230_ME_CNVR          0x0400                  /* [10] Conversion Ready */
#define INA230_ME_AFF           0x0010                  /* [05] Alert Function Flag */
#define INA230_ME_CVRF          0x0008                  /* [04] Conversion Ready Flag */
#define INA230_ME_OVF           0x0004                  /* [03] Math Overflow Flag */
#define INA230_ME_APOL          0x0002                  /* [02] Alert Polarity */
#define INA230_ME_LEN           0x0001                  /* [01] Alert Latch Enable */

/* 预设的公式原始参数 */
#define INA230_1_VOL_MIN        (0)                     /* 最小电压读数 */
#define INA230_1_VOL_MAX        (25)                    /* 最大电压读数 */
#define INA230_1_VOL_M          (1)                     /* 电压M倍数 */
#define INA230_1_VOL_B          (0)                     /* 电压B偏移 */
#define INA230_1_VOL_K1         (0)                     /* 电压B偏移指数 */
#define INA230_1_VOL_K2         (-1)                    /* 电压最终指数 */

#define INA230_1_CUR_MIN        (0)
#define INA230_1_CUR_MAX        (50)
#define INA230_1_CUR_M          (1)
#define INA230_1_CUR_B          (0)
#define INA230_1_CUR_K1         (0)
#define INA230_1_CUR_K2         (0)

#define INA230_1_POW_MIN        (0)
#define INA230_1_POW_MAX        (600)
#define INA230_1_POW_M          (1)
#define INA230_1_POW_B          (0)
#define INA230_1_POW_K1         (0)
#define INA230_1_POW_K2         (0)

#define INA230_2_VOL_MIN        (0)                     /* 最小电压读数 */
#define INA230_2_VOL_MAX        (5)                     /* 最大电压读数 */
#define INA230_2_VOL_M          (1)                     /* 电压M倍数 */
#define INA230_2_VOL_B          (0)                     /* 电压B偏移 */
#define INA230_2_VOL_K1         (0)                     /* 电压B偏移指数 */
#define INA230_2_VOL_K2         (-1)                    /* 电压最终指数 */

#define INA230_2_CUR_MIN        (0)
#define INA230_2_CUR_MAX        (8)
#define INA230_2_CUR_M          (1)
#define INA230_2_CUR_B          (0)
#define INA230_2_CUR_K1         (0)
#define INA230_2_CUR_K2         (-1)

#define INA230_2_POW_MIN        (0)
#define INA230_2_POW_MAX        (40)
#define INA230_2_POW_M          (1)
#define INA230_2_POW_B          (0)
#define INA230_2_POW_K1         (0)
#define INA230_2_POW_K2         (-1)

#define R_SHUNT                 ((float)0.007 / 4)
#define CUR_LSB(maxcur)         ((float)(maxcur) / pow(2, 15))
#define POW_LSB(curlsb)         ((float)(curlsb) * 25)
#define CAL(curlsb, rshunt)     ((float)0.00512 / (curlsb * rshunt))

#define RAW_2_VSHT_VAL(v)       ((float)(v * 0.0025))                       /* 纹波电压转换, 单位 mV */
#define RAW_2_VBUS_VAL(v)       ((float)(v * 0.00125))                      /* 总线电压转换, 单位 V  */
#define RAW_2_CUR_VAL(c,maxcur) ((float)(c * CUR_LSB(maxcur)))              /* 电流转换, 单位 A */
#define RAW_2_POW_VAL(p,maxcur) ((float)(p * POW_LSB(CUR_LSB(maxcur))))     /* 功率转换, 单位 W */

SDR_RECORD_FULL ina230_sr[MAX_INA230_COUNT];
sensor_data_t ina230_sd[MAX_INA230_COUNT];
I2C_DEVICE ina230_dev[2];

const SENSOR_FORMULA ina230_formula[MAX_INA230_COUNT] =
{
    {
        .sensor_id  = "ina230-1-vol",
        .type       = SENSOR_TYPE_VOLTAGE,
        .unit       = SENSOR_UNIT_VOLTS,
        .vmin       = INA230_1_VOL_MIN,
        .vmax       = INA230_1_VOL_MAX,
        .M          = INA230_1_VOL_M,
        .B          = INA230_1_VOL_B,
        .K1         = INA230_1_VOL_K1,
        .K2         = INA230_1_VOL_K2,
    },
    {
        .sensor_id  = "ina230-1-cur",
        .type       = SENSOR_TYPE_CURRENT,
        .unit       = SENSOR_UNIT_AMPS,
        .vmin       = INA230_1_CUR_MIN,
        .vmax       = INA230_1_CUR_MAX,
        .M          = INA230_1_CUR_M,
        .B          = INA230_1_CUR_B,
        .K1         = INA230_1_CUR_K1,
        .K2         = INA230_1_CUR_K2,
    },
    {
        .sensor_id  = "ina230-1-pow",
        .type       = SENSOR_TYPE_CURRENT,
        .unit       = SENSOR_UNIT_WATTS,
        .vmin       = INA230_1_POW_MIN,
        .vmax       = INA230_1_POW_MAX,
        .M          = INA230_1_POW_M,
        .B          = INA230_1_POW_B,
        .K1         = INA230_1_POW_K1,
        .K2         = INA230_1_POW_K2,
    },
    {
        .sensor_id  = "ina230-2-vol",
        .type       = SENSOR_TYPE_VOLTAGE,
        .unit       = SENSOR_UNIT_VOLTS,
        .vmin       = INA230_2_VOL_MIN,
        .vmax       = INA230_2_VOL_MAX,
        .M          = INA230_2_VOL_M,
        .B          = INA230_2_VOL_B,
        .K1         = INA230_2_VOL_K1,
        .K2         = INA230_2_VOL_K2,
    },
    {
        .sensor_id  = "ina230-2-cur",
        .type       = SENSOR_TYPE_CURRENT,
        .unit       = SENSOR_UNIT_AMPS,
        .vmin       = INA230_2_CUR_MIN,
        .vmax       = INA230_2_CUR_MAX,
        .M          = INA230_2_CUR_M,
        .B          = INA230_2_CUR_B,
        .K1         = INA230_2_CUR_K1,
        .K2         = INA230_2_CUR_K2,
    },
    {
        .sensor_id  = "ina230-2-pow",
        .type       = SENSOR_TYPE_CURRENT,
        .unit       = SENSOR_UNIT_WATTS,
        .vmin       = INA230_2_POW_MIN,
        .vmax       = INA230_2_POW_MAX,
        .M          = INA230_2_POW_M,
        .B          = INA230_2_POW_B,
        .K1         = INA230_2_POW_K1,
        .K2         = INA230_2_POW_K2,
    },
};


void ina230_init_chip(void)
{
    uint8_t rvalue[2];
    uint16_t cal;

    I2C_i2c1_slave_dev_init(&ina230_dev[0], INA230_SLAVE_ADDR_1, 1);
    I2C_i2c1_slave_dev_init(&ina230_dev[1], INA230_SLAVE_ADDR_2, 1);

    // set the ina230-1 calibration register
    //CAL = 0.00512 / ((50.0 / (2 ^ 15)) * (0.007 / 4))
    cal = (uint16_t)(CAL(CUR_LSB(INA230_1_CUR_MAX), R_SHUNT));
    rvalue[0] = (cal >> 8) & 0xff;
    rvalue[1] = cal & 0xff;
    I2C_i2c1_slave_dev_set(&ina230_dev[0], INA230_REG_CALIBRATION, (uint8_t*)&rvalue, 2);
    I2C_i2c1_master_write(&ina230_dev[0]);

    // set the ina230-2 calibration register
    //CAL = 0.00512 / ((8.0 / (2 ^ 15)) * (0.007 / 4))
    cal = (uint16_t)(CAL(CUR_LSB(INA230_2_CUR_MAX), R_SHUNT));
    rvalue[0] = (cal >> 8) & 0xff;
    rvalue[1] = cal & 0xff;
    I2C_i2c1_slave_dev_set(&ina230_dev[1], INA230_REG_CALIBRATION, (uint8_t*)&rvalue, 2);
    I2C_i2c1_master_write(&ina230_dev[1]);
}

void ina230_scan_function(void *arg)
{
    sensor_data_t *sd = (sensor_data_t *)arg;
    uint8_t dev, id;
    uint32_t error = 0;
    uint16_t rvalue;
    float value;

    if (sd->local_sensor_id >= MAX_INA230_COUNT) {
        sd->unavailable = 1;
        return;
    }

    dev = sd->local_sensor_id / MAX_INA230_READER;
    id = sd->local_sensor_id % MAX_INA230_READER;


    /* read voltage, current, power from ina230 */
    switch (id)
    {
        case INA230_ID_VOL:
            I2C_i2c1_slave_dev_set(&ina230_dev[dev], INA230_REG_BUS_VOL, (uint8_t*)&rvalue, 2);
            error = I2C_i2c1_master_read(&ina230_dev[dev]);
            value = RAW_2_VBUS_VAL(BSWAP_16(rvalue));
            break;

        case INA230_ID_CUR:
            I2C_i2c1_slave_dev_set(&ina230_dev[dev], INA230_REG_CURRENT, (uint8_t*)&rvalue, 2);
            error = I2C_i2c1_master_read(&ina230_dev[dev]);
            value = RAW_2_CUR_VAL(BSWAP_16(rvalue), ina230_formula[sd->local_sensor_id].vmax);
            break;

        case INA230_ID_POW:
            I2C_i2c1_slave_dev_set(&ina230_dev[dev], INA230_REG_POWER, (uint8_t*)&rvalue, 2);
            error = I2C_i2c1_master_read(&ina230_dev[dev]);
            value = RAW_2_POW_VAL(BSWAP_16(rvalue), ina230_formula[sd->local_sensor_id].vmax);
            break;

        default:
            break;
    }
    if (error != 0) {
        //sd->recv2 = error & 0x1f;
        sd->unavailable = 1;
        return;
    } else {
        sd->unavailable = 0;
    }

    sd->last_sensor_reading = (uint8_t)(lrint(SENSOR_VAL2RAW(value,
                                        ina230_formula[sd->local_sensor_id].M,
                                        ina230_formula[sd->local_sensor_id].B,
                                        ina230_formula[sd->local_sensor_id].K1,
                                        ina230_formula[sd->local_sensor_id].K2)));

    sd->lower_non_critical_threshold = 0;
    sd->lower_critical_threshold = 0;
    sd->lower_non_recoverable_threshold = 0;
    sd->upper_non_critical_threshold = 0;
    sd->upper_critical_threshold = 0;
    sd->upper_non_recoverable_threshold = 0;

#if 0
    /* read status form ina230 */
    I2C_i2c0_slave_dev_set(&ina230_dev, INA230_REG_MASK, (uint8_t*)&value, 2);
    I2C_i2c0_master_read(&ina230_dev);

    if (value & INA230_ME_OVF)
    {
    }
#endif
}

void ina230_sett_function(void *arg)
{
    sensor_data_t *sd = (sensor_data_t *)arg;
    uint8_t sensor_id = sd->sensor_id;

    (void)sensor_id;

    /* TODO: 2012/9/28
     * Set the INA230 register:
     * - INA230_REG_CFG
     * - INA230_REG_CALIBRATION
     * - INA230_REG_MASK
     * - INA230_REG_ALERT
     */
}

void ina230_init_sensor_record(uint8_t i)
{
    /* SENSOR RECORD HEADER */
    ina230_sr[i].header.record_id = 0;                          /* -WILL BE CHANGED- */
    ina230_sr[i].header.sdr_version = 0x51;
    ina230_sr[i].header.record_type = SDR_RECORD_TYPE_FULL_SENSOR;
    ina230_sr[i].header.record_len = sizeof(ina230_sr[i]);

    /* RECORD KEY BYTES */
    ina230_sr[i].key.owner_id = 0;                              /* 7-bit system software ID */
    ina230_sr[i].key.id_type = 1;                               /* System software type */
    ina230_sr[i].key.channel_num = 0;
    ina230_sr[i].key.sensor_owner_lun = 0;
    ina230_sr[i].key.sensor_number = 0;                         /* -CHANGE- this will get replaced by the actual
                                                                    sensor number when we register the SDR */

    /* RECORD BODY BYTES */
    ina230_sr[i].entity_id.id = ENTITY_ID_SYSTEM_BOARD;         /* physical entity the sensor is monitoring */
    ina230_sr[i].entity_id.entity_type = 0;                     /* treat entity as a physical entity */
    ina230_sr[i].entity_id.entity_instance_num = 0;

    /* Sensor Default (power up) State */
    ina230_sr[i].initialization.powerup_sensor_scanning = 1;    /* sensor scanning enabled */
    ina230_sr[i].initialization.powerup_evt_generation = 0;     /* event generation disabled */
    ina230_sr[i].initialization.init_sensor_type = 0;
    ina230_sr[i].initialization.init_hysteresis = 0;
    ina230_sr[i].initialization.init_thresholds = 0;
    ina230_sr[i].initialization.init_events = 0;
    ina230_sr[i].initialization.init_scanning = 1;              /* accepts scanning bit in the SetSensorEventEnable command */

    /* Sensor Capabilities */
    ina230_sr[i].capabilities.ignore_sensor = 0;                /* Ignore sensor if entity is not present or disabled. */
    ina230_sr[i].capabilities.sensor_rearm_support = 1;         /* automatically rearms itself when the event clears */
    ina230_sr[i].capabilities.sensor_hysteresis_support = 2;    /* hysteresis is readable and settable */
    ina230_sr[i].capabilities.sensor_threshold_access = 2;      /* thresholds are readable and settable
                                                                   per Reading Mask and Settable Threshold Mask, respectively */
    ina230_sr[i].capabilities.event_msg_control = 0;            /* per threshold/discrete-state event
                                                                   enable/disable control (implies that entire
                                                                   sensor and global disable are also supported) */
    ina230_sr[i].sensor_type = ina230_formula[i].type;          /* From Table 42-3, Sensor Type Codes */
    ina230_sr[i].event_type_code = 1;                           /* From Table 42-1, Threshold */

    /* Assertion Event Mask */
    ina230_sr[i].mask.type.threshold.assert_lower_non_critical_low = 0;
    ina230_sr[i].mask.type.threshold.assert_lower_non_critical_high = 0;
    ina230_sr[i].mask.type.threshold.assert_lower_critical_low = 0;
    ina230_sr[i].mask.type.threshold.assert_lower_critical_high = 1;
    ina230_sr[i].mask.type.threshold.assert_lower_non_recoverable_low = 0;
    ina230_sr[i].mask.type.threshold.assert_lower_non_recoverable_high = 0;
    ina230_sr[i].mask.type.threshold.assert_upper_non_critical_low = 0;
    ina230_sr[i].mask.type.threshold.assert_upper_non_critical_high = 0;
    ina230_sr[i].mask.type.threshold.assert_upper_critical_low = 0;
    ina230_sr[i].mask.type.threshold.assert_upper_critical_high = 1;
    ina230_sr[i].mask.type.threshold.assert_upper_non_recoverable_low = 0;
    ina230_sr[i].mask.type.threshold.assert_upper_non_recoverable_high = 0;

    /* Lower/Upper Threshold Reading Mask
     * for Command 'Get Sensor Reading'
     */
    ina230_sr[i].mask.type.threshold.status_lower_non_critical = 0;
    ina230_sr[i].mask.type.threshold.status_lower_critical = 1;
    ina230_sr[i].mask.type.threshold.status_lower_non_recoverable = 0;
    ina230_sr[i].mask.type.threshold.status_upper_non_critical = 0;
    ina230_sr[i].mask.type.threshold.status_upper_critical = 1;
    ina230_sr[i].mask.type.threshold.status_upper_non_recoverable = 0;

    /* Deassertion Event Mask */
    ina230_sr[i].mask.type.threshold.dassert_lower_non_critical_low = 0;
    ina230_sr[i].mask.type.threshold.dassert_lower_non_critical_high = 0;
    ina230_sr[i].mask.type.threshold.dassert_lower_critical_low = 0;
    ina230_sr[i].mask.type.threshold.dassert_lower_critical_high = 1;
    ina230_sr[i].mask.type.threshold.dassert_lower_non_recoverable_low = 0;
    ina230_sr[i].mask.type.threshold.dassert_lower_non_recoverable_high = 0;
    ina230_sr[i].mask.type.threshold.dassert_upper_non_critical_low = 0;
    ina230_sr[i].mask.type.threshold.dassert_upper_non_critical_high = 0;
    ina230_sr[i].mask.type.threshold.dassert_upper_critical_low = 0;
    ina230_sr[i].mask.type.threshold.dassert_upper_critical_high = 1;
    ina230_sr[i].mask.type.threshold.dassert_upper_non_recoverable_low = 0;
    ina230_sr[i].mask.type.threshold.dassert_upper_non_recoverable_high = 0;

    /* Settable Threshold Mask, Readable Threshold Mask
     * for Command 'Set Sensor Thresholds' and 'Get Sensor Thresholds'
     */
    ina230_sr[i].mask.type.threshold.lower_non_critical_readable = 0;
    ina230_sr[i].mask.type.threshold.lower_critical_readable = 1;
    ina230_sr[i].mask.type.threshold.lower_non_recoverable_readable = 0;
    ina230_sr[i].mask.type.threshold.upper_non_critical_readable = 0;
    ina230_sr[i].mask.type.threshold.upper_critical_readable = 1;
    ina230_sr[i].mask.type.threshold.upper_non_recoverable_readable = 0;

    ina230_sr[i].mask.type.threshold.lower_non_critical_settable = 0;
    ina230_sr[i].mask.type.threshold.lower_critical_settable = 0;
    ina230_sr[i].mask.type.threshold.lower_non_recoverable_settable = 0;
    ina230_sr[i].mask.type.threshold.upper_non_critical_settable = 0;
    ina230_sr[i].mask.type.threshold.upper_critical_settable = 0;
    ina230_sr[i].mask.type.threshold.upper_non_recoverable_settable = 0;

    /* Unit 1, 2, 3 */
    ina230_sr[i].unit.analog_data_format = 2;                  /* 2's complement (signed) */
    ina230_sr[i].unit.rate_unit = 0;                           /* none */
    ina230_sr[i].unit.modifier_unit = 0;                       /* 00b = none */
    ina230_sr[i].unit.percentage = 0;                          /* not a percentage value */
    ina230_sr[i].unit.unit_2_base = ina230_formula[i].unit;    /* Base Unit */
    ina230_sr[i].unit.unit_3_modifier = 0;                     /* no modifier unit */
    ina230_sr[i].linearization = 0;                            /* Linear */

    ina230_sr[i].mtol.m_ls = FORMULA_M_LS(ina230_formula[i].M);
    ina230_sr[i].mtol.m_ms = FORMULA_M_MS(ina230_formula[i].M);
    ina230_sr[i].mtol.tolerance = 2;

	ina230_sr[i].bacc.b_ls = FORMULA_B_LS(ina230_formula[i].B);
	ina230_sr[i].bacc.b_ms = FORMULA_B_MS(ina230_formula[i].B);
	ina230_sr[i].bacc.accuracy_ls = 0;
	ina230_sr[i].bacc.accuracy_ms = 0;
    ina230_sr[i].bacc.accuracy_exp = 0;
    ina230_sr[i].bacc.sensor_dir = 0;
    ina230_sr[i].bacc.b_exponent = ina230_formula[i].K1;
    ina230_sr[i].bacc.r_exponent = ina230_formula[i].K2;

    ina230_sr[i].analog_flags.nominal_reading_specified = 0;
    ina230_sr[i].analog_flags.normal_max_specified = 1;
    ina230_sr[i].analog_flags.normal_min_specified = 1;

    ina230_sr[i].sensor_maximum_reading = (uint8_t)lrint(SENSOR_VAL2RAW(ina230_formula[i].vmax,
        ina230_formula[i].M, ina230_formula[i].B, ina230_formula[i].K1, ina230_formula[i].K2));
    ina230_sr[i].sensor_minimum_reading = (uint8_t)lrint(SENSOR_VAL2RAW(ina230_formula[i].vmin,
        ina230_formula[i].M, ina230_formula[i].B, ina230_formula[i].K1, ina230_formula[i].K2));

    ina230_sr[i].upper_non_recoverable_threshold = 0;
    ina230_sr[i].upper_critical_threshold = 0;
    ina230_sr[i].upper_non_critical_threshold = 0;
    ina230_sr[i].lower_non_recoverable_threshold = 0;
    ina230_sr[i].lower_critical_threshold = 0;
    ina230_sr[i].lower_non_critical_threshold = 0;
    ina230_sr[i].positive_going_threshold_hysteresis_value = 0;
    ina230_sr[i].negative_going_threshold_hysteresis_value = 0;
    ina230_sr[i].reserved2 = 0;
    ina230_sr[i].reserved3 = 0;
    ina230_sr[i].oem = 0;
    ina230_sr[i].id_type_length.type = 3;                                           /* 11 = 8-bit ASCII + Latin 1. */
    ina230_sr[i].id_type_length.length = strlen(ina230_formula[i].sensor_id);       /* length of following data, in characters */
    memcpy(ina230_sr[i].id_string_bytes, ina230_formula[i].sensor_id, ina230_sr[i].id_type_length.length);

    ina230_sd[i].sensor_id = 0;
    ina230_sd[i].last_sensor_reading = 0;
    ina230_sd[i].scan_period = 0;
    ina230_sd[i].scan_function = ina230_scan_function;
    ina230_sd[i].sett_function = ina230_sett_function;
    ina230_sd[i].event_messages_enabled = 1;
    ina230_sd[i].sensor_scanning_enabled = 1;
    ina230_sd[i].unavailable = 0;
    ina230_sd[i].local_sensor_id = i;
    ina230_sd[i].recv1 = 0;
    ina230_sd[i].recv2 = 0;
    ina230_sd[i].recv3 = 0;

	sensor_add(&ina230_sd[i], SDR_RECORD_TYPE_FULL_SENSOR, &ina230_sr[i]);
}

void ina230_init(void)
{
    uint8_t i;

    ina230_init_chip();

    for (i = 0; i < MAX_INA230_COUNT; i++)
    {
        ina230_init_sensor_record(i);
    }
}

#endif  // IPMI_CHIP_INA230

