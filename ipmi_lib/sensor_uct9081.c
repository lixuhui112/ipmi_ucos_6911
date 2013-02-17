//*****************************************************************************
//
// sensor_ucd9081.c - The UCD9081 Sensor
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
#include "app/lib_i2c.h"
#include "app/lib_gpio.h"
#include "app/lib_io.h"
#include <string.h>
#include <stdio.h>

#ifdef IPMI_CHIP_UCD9081

#define MAX_UCD9081_SENSOR_COUNT    8

#if (defined(BOARD_6911_SWITCH))
#define RAIL_VOL_DIV_UP_1       0
#define RAIL_VOL_DIV_DN_1       0
#define RAIL_VOL_DIV_UP_2       0
#define RAIL_VOL_DIV_DN_2       0
#define RAIL_VOL_DIV_UP_3       0
#define RAIL_VOL_DIV_DN_3       0
#define RAIL_VOL_DIV_UP_4       0
#define RAIL_VOL_DIV_DN_4       0
#define RAIL_VOL_DIV_UP_5       1.96
#define RAIL_VOL_DIV_DN_5       1.96
#define RAIL_VOL_DIV_UP_6       1.96
#define RAIL_VOL_DIV_DN_6       1.96
#define RAIL_VOL_DIV_UP_7       0
#define RAIL_VOL_DIV_DN_7       0
#define RAIL_VOL_DIV_UP_8       0
#define RAIL_VOL_DIV_DN_8       0

#define UCD9081_VOL_REF         3.30                    /* 参考电压值 */

#define UCD9081_VOL_RAIL_1      1.00                    /* 监控的标准电压值 */
#define UCD9081_VOL_RAIL_2      1.20
#define UCD9081_VOL_RAIL_3      1.20
#define UCD9081_VOL_RAIL_4      1.80
#define UCD9081_VOL_RAIL_5      2.50
#define UCD9081_VOL_RAIL_6      3.30
#define UCD9081_VOL_RAIL_7      0
#define UCD9081_VOL_RAIL_8      0
#endif

#if (defined(BOARD_6911_FABRIC))
#define RAIL_VOL_DIV_UP_1       1.96
#define RAIL_VOL_DIV_DN_1       1.96
#define RAIL_VOL_DIV_UP_2       0
#define RAIL_VOL_DIV_DN_2       0
#define RAIL_VOL_DIV_UP_3       0
#define RAIL_VOL_DIV_DN_3       0
#define RAIL_VOL_DIV_UP_4       0
#define RAIL_VOL_DIV_DN_4       0
#define RAIL_VOL_DIV_UP_5       1.96
#define RAIL_VOL_DIV_DN_5       1.96
#define RAIL_VOL_DIV_UP_6       0
#define RAIL_VOL_DIV_DN_6       0
#define RAIL_VOL_DIV_UP_7       0
#define RAIL_VOL_DIV_DN_7       0
#define RAIL_VOL_DIV_UP_8       0
#define RAIL_VOL_DIV_DN_8       0

#define UCD9081_VOL_REF         3.30                    /* 参考电压值 */

#define UCD9081_VOL_RAIL_1      3.30                    /* 监控的标准电压值 */
#define UCD9081_VOL_RAIL_2      1.20
#define UCD9081_VOL_RAIL_3      1.00
#define UCD9081_VOL_RAIL_4      1.80
#define UCD9081_VOL_RAIL_5      2.50
#define UCD9081_VOL_RAIL_6      0
#define UCD9081_VOL_RAIL_7      0
#define UCD9081_VOL_RAIL_8      0
#endif

#define UCD9081_REG_RAIL_1      0x00                    /* 寄存器地址 */
#define UCD9081_REG_RAIL_2      0x02
#define UCD9081_REG_RAIL_3      0x04
#define UCD9081_REG_RAIL_4      0x06
#define UCD9081_REG_RAIL_5      0x08
#define UCD9081_REG_RAIL_6      0x0a
#define UCD9081_REG_RAIL_7      0x0c
#define UCD9081_REG_RAIL_8      0x0e
#define UCD9081_REG_ERR         0x20                    /* 0x20 - 0x26 */
#define UCD9081_REG_STATUS      0x26
#define UCD9081_REG_VER         0x27
#define UCD9081_REG_RAIL_STAT   0x28                    /* 0x28 - 0x29 */
#define UCD9081_REG_FLASH_LCK   0x2e
#define UCD9081_REG_RESTART     0x2f
#define UCD9081_REG_WADDR1      0x30
#define UCD9081_REG_WADDR2      0x31
#define UCD9081_REG_WDATA1      0x32
#define UCD9081_REG_WDATA2      0x33

#define UCD9081_STATUS_ERR_I2C  0x80                    /* I2C PHY layer error */
#define UCD9081_STATUS_ERR_RAIL 0x40                    /* RAIL error pending */
#define UCD9081_STATUS_ERR_NVLG 0x20                    /* ERROR points to run-time error log / non-volatile log */
#define UCD9081_STATUS_ERR_FW   0x10                    /* Device firmware error detected, device is idle */
#define UCD9081_STATUS_ERR_PRM  0x08                    /* Parameters invalid, last config loaded */
#define UCD9081_STATUS_ERR_REG  0x03                    /* 00 = No error
                                                           01 = Invalid address
                                                           10 = Read access error
                                                           11 = Write access error */
#define UCD9081_ERR_CODE_NULL   0x00                    /* Null alarm */
#define UCD9081_ERR_CODE_NSTART 0x01                    /* Supply did not start */
#define UCD9081_ERR_CODE_OV     0x02                    /* Sustained overvoltage detected */
#define UCD9081_ERR_CODE_UV     0x03                    /* Sustained undervoltage detected */
#define UCD9081_ERR_CODE_GOV    0x04                    /* overvoltage glitch detected */
#define UCD9081_ERR_CODE_GUV    0x05                    /* undervoltage glitch detected */

#define UCD9081_CTL_RESET       0x00
#define UCD9081_CTL_SHUTDOWN    0xc0

#define UCD9081_MIN             (0)                     /* 预设的公式原始参数 */
#define UCD9081_MAX             (5)
#define UCD9081_M               (1)
#define UCD9081_B               (0)
#define UCD9081_K1              (0)
#define UCD9081_K2              (-1)

#define UCD9081_VAL_M           ((uint8_t)UCD9081_M)    /* 预设的公式转换后参数 */
#define UCD9081_VAL_B           ((uint8_t)UCD9081_B)
#define UCD9081_VAL_K1          ((uint8_t)UCD9081_K1)
#define UCD9081_VAL_K2          ((uint8_t)UCD9081_K2)
#define UCD9081_VAL_MIN         ((uint8_t)SENSOR_VAL2RAW(UCD9081_MIN, UCD9081_M, UCD9081_B, UCD9081_K1, UCD9081_K2))
#define UCD9081_VAL_MAX         ((uint8_t)SENSOR_VAL2RAW(UCD9081_MAX, UCD9081_M, UCD9081_B, UCD9081_K1, UCD9081_K2))

#define VOL_2_UV(v)             ((float)(v * 0.90))     /* 欠压 -10% */
#define VOL_2_OV(v)             ((float)(v * 1.05))     /* 过压 +05% */

void ucd9081_int_handler(void *param);
void ucd9081_scan_function(void *arg);
void ucd9081_sett_function(void *arg);


typedef struct ucd9081_rail_vol_tbl {
	float rail_vol;     /* referece voltage */
	float rail_uv;      /* under voltage */
	float rail_ov;      /* over voltage */
	float r_div_up;     /* up resistance */
	float r_div_dn;     /* down resistance */
	uint8_t rail_reg;   /* voltage register */
} UCD9081_VOL_RAIL_TBL;

SDR_RECORD_FULL ucd9081_sr[MAX_UCD9081_SENSOR_COUNT];
sensor_data_t ucd9081_sd[MAX_UCD9081_SENSOR_COUNT];
I2C_DEVICE ucd9081_dev;

const IO_INT ucd9081_int =
{
    .peripheral     = GPIO_CHIP_UCD9081_GPIO2_PERIPH,
    .port           = GPIO_CHIP_UCD9081_GPIO2_PORT,
    .pins           = GPIO_CHIP_UCD9081_GPIO2_PIN,
    .interrupt      = GPIO_CHIP_UCD9081_GPIO2_INT,
    .handler        = ucd9081_int_handler,
    .param          = NULL,
};

const UCD9081_VOL_RAIL_TBL ucd9081_vol_tbl[MAX_UCD9081_SENSOR_COUNT] =
{
    {   /* rail 0 */
        .rail_vol = (float)(UCD9081_VOL_RAIL_1),
        .rail_uv  = (float)VOL_2_UV(UCD9081_VOL_RAIL_1),
        .rail_ov  = (float)VOL_2_OV(UCD9081_VOL_RAIL_1),
        .r_div_up = (float)(RAIL_VOL_DIV_UP_1),
        .r_div_dn = (float)(RAIL_VOL_DIV_DN_1),
        .rail_reg = UCD9081_REG_RAIL_1,
    },
    {   /* rail 1 */
        .rail_vol = (float)(UCD9081_VOL_RAIL_2),
        .rail_uv  = (float)VOL_2_UV(UCD9081_VOL_RAIL_2),
        .rail_ov  = (float)VOL_2_OV(UCD9081_VOL_RAIL_2),
        .r_div_up = (float)(RAIL_VOL_DIV_UP_2),
        .r_div_dn = (float)(RAIL_VOL_DIV_DN_2),
        .rail_reg = UCD9081_REG_RAIL_2,
    },
    {   /* rail 2 */
        .rail_vol = (float)(UCD9081_VOL_RAIL_3),
        .rail_uv  = (float)VOL_2_UV(UCD9081_VOL_RAIL_3),
        .rail_ov  = (float)VOL_2_OV(UCD9081_VOL_RAIL_3),
        .r_div_up = (float)(RAIL_VOL_DIV_UP_3),
        .r_div_dn = (float)(RAIL_VOL_DIV_DN_3),
        .rail_reg = UCD9081_REG_RAIL_3,
    },
    {   /* rail 3 */
        .rail_vol = (float)(UCD9081_VOL_RAIL_4),
        .rail_uv  = (float)VOL_2_UV(UCD9081_VOL_RAIL_4),
        .rail_ov  = (float)VOL_2_OV(UCD9081_VOL_RAIL_4),
        .r_div_up = (float)(RAIL_VOL_DIV_UP_4),
        .r_div_dn = (float)(RAIL_VOL_DIV_DN_4),
        .rail_reg = UCD9081_REG_RAIL_4,
    },
    {   /* rail 4 */
        .rail_vol = (float)(UCD9081_VOL_RAIL_5),
        .rail_uv  = (float)VOL_2_UV(UCD9081_VOL_RAIL_5),
        .rail_ov  = (float)VOL_2_OV(UCD9081_VOL_RAIL_5),
        .r_div_up = (float)(RAIL_VOL_DIV_UP_5),
        .r_div_dn = (float)(RAIL_VOL_DIV_DN_5),
        .rail_reg = UCD9081_REG_RAIL_5,
    },
    {   /* rail 5 */
        .rail_vol = (float)(UCD9081_VOL_RAIL_6),
        .rail_uv  = (float)VOL_2_UV(UCD9081_VOL_RAIL_6),
        .rail_ov  = (float)VOL_2_OV(UCD9081_VOL_RAIL_6),
        .r_div_up = (float)(RAIL_VOL_DIV_UP_6),
        .r_div_dn = (float)(RAIL_VOL_DIV_DN_6),
        .rail_reg = UCD9081_REG_RAIL_6,
    },
    {   /* rail 6 */
        .rail_vol = (float)(UCD9081_VOL_RAIL_7),
        .rail_uv  = (float)VOL_2_UV(UCD9081_VOL_RAIL_7),
        .rail_ov  = (float)VOL_2_OV(UCD9081_VOL_RAIL_7),
        .r_div_up = (float)(RAIL_VOL_DIV_UP_7),
        .r_div_dn = (float)(RAIL_VOL_DIV_DN_7),
        .rail_reg = UCD9081_REG_RAIL_7,
    },
    {   /* rail 7 */
        .rail_vol = (float)(UCD9081_VOL_RAIL_8),
        .rail_uv  = (float)VOL_2_UV(UCD9081_VOL_RAIL_8),
        .rail_ov  = (float)VOL_2_OV(UCD9081_VOL_RAIL_8),
        .r_div_up = (float)(RAIL_VOL_DIV_UP_8),
        .r_div_dn = (float)(RAIL_VOL_DIV_DN_8),
        .rail_reg = UCD9081_REG_RAIL_8,
    },
};

uint8_t ucd9081_raw2val(uint8_t ucd9081_id, uint16_t raw)
{
    float val = 0;

    if (ucd9081_vol_tbl[ucd9081_id].r_div_up == 0.0) {
        val = ((float)(raw) * UCD9081_VOL_REF / 1024);         /* 寄存器电压转换 */
    } else {
        val = ((float)(raw) * UCD9081_VOL_REF / 1024) *
            ((ucd9081_vol_tbl[ucd9081_id].r_div_up + ucd9081_vol_tbl[ucd9081_id].r_div_dn) / ucd9081_vol_tbl[ucd9081_id].r_div_dn);
    }
    return (uint8_t)lround(SENSOR_VAL2RAW(val, UCD9081_M, UCD9081_B, UCD9081_K1, UCD9081_K2));
}

void ucd9081_int_handler(void *param)
{
    uint8_t buffer[6];

    /* read status form ucd9081 */
    I2C_i2c1_slave_dev_set(&ucd9081_dev, UCD9081_REG_STATUS, (uint8_t*)&buffer[0], 1);
    I2C_i2c1_master_read(&ucd9081_dev);

    if (buffer[0] & UCD9081_STATUS_ERR_RAIL)
    {
        /* read error form ucd9081 */
        I2C_i2c1_slave_dev_set(&ucd9081_dev, UCD9081_REG_ERR, (uint8_t*)&buffer[0], 6);
        I2C_i2c1_master_read(&ucd9081_dev);

        /* TODO:
         * bring a over & alert to system
         */
        if ((buffer[0] >> 5) & UCD9081_ERR_CODE_OV)
        {
            //sd->lower_critical_threshold = 1;
        }
        if ((buffer[0] >> 5) & UCD9081_ERR_CODE_UV)
        {
            //sd->upper_critical_threshold = 1;
        }
    }

}

void ucd9081_init_chip(void)
{
    I2C_i2c1_slave_dev_init(&ucd9081_dev, UCD9081_SLAVE_ADDR, 1);
    //IO_gpio_int_register(&ucd9081_int);
}

void ucd9081_scan_function(void *arg)
{
    sensor_data_t *sd = (sensor_data_t *)arg;
    uint32_t error;
    uint8_t ucd9081_id;
    uint8_t buffer[6];

    ucd9081_id = sd->local_sensor_id;
    if (ucd9081_id >= MAX_UCD9081_SENSOR_COUNT) {
        sd->unavailable = 1;
        return;
    }

    /* read voltage from ucd9081 */
    I2C_i2c1_slave_dev_set(&ucd9081_dev, ucd9081_vol_tbl[ucd9081_id].rail_reg, (uint8_t*)&buffer[0], 2);
    error = I2C_i2c1_master_read(&ucd9081_dev);
    if (error != 0) {
        //sd->recv2 = error & 0x1f;
        sd->unavailable = 1;
        return;
    } else {
        sd->unavailable = 0;
    }

    sd->last_sensor_reading = ucd9081_raw2val(ucd9081_id, ((buffer[0] & 0x3) << 8 | buffer[1]));

    sd->lower_non_critical_threshold = 0;
    sd->lower_critical_threshold = 0;
    sd->lower_non_recoverable_threshold = 0;
    sd->upper_non_critical_threshold = 0;
    sd->upper_critical_threshold = 0;
    sd->upper_non_recoverable_threshold = 0;

    /* read status form ucd9081 */
    I2C_i2c1_slave_dev_set(&ucd9081_dev, UCD9081_REG_STATUS, (uint8_t*)&buffer[0], 1);
    I2C_i2c1_master_read(&ucd9081_dev);

    if (buffer[0] & UCD9081_STATUS_ERR_RAIL)
    {
        /* read error form ucd9081 */
        I2C_i2c1_slave_dev_set(&ucd9081_dev, UCD9081_REG_ERR, (uint8_t*)&buffer[0], 6);
        I2C_i2c1_master_read(&ucd9081_dev);

        if ((buffer[0] >> 5) & UCD9081_ERR_CODE_OV)
        {
            sd->lower_critical_threshold = 1;
        }
        if ((buffer[0] >> 5) & UCD9081_ERR_CODE_UV)
        {
            sd->upper_critical_threshold = 1;
        }
    }
}

void ucd9081_sett_function(void *arg)
{
    sensor_data_t *sd = (sensor_data_t *)arg;
    uint8_t sensor_id = sd->sensor_id;

    (void)sensor_id;

    /* TODO: 2012/9/26
     * Read the ucd9081 512B flash parameter, re-config and write it to flash back.
     */
}

void ucd9081_shutdown(void *arg)
{
    /* TODO: 2012/9/29
     * Shutdown the ucd9081.
     */
    uint8_t buffer[1] = {UCD9081_CTL_SHUTDOWN};
    uint32_t error;

    I2C_i2c1_slave_dev_set(&ucd9081_dev, UCD9081_REG_RESTART, (uint8_t*)&buffer[0], 1);
    error = I2C_i2c1_master_write(&ucd9081_dev);

    if (error) {
        /* save error log */
    }
}

void ucd9081_init_sensor_record(uint8_t i)
{
    char id_string[16];

    /* SENSOR RECORD HEADER */
    ucd9081_sr[i].header.record_id = 0;                         /* -WILL BE CHANGED- */
    ucd9081_sr[i].header.sdr_version = 0x51;
    ucd9081_sr[i].header.record_type = SDR_RECORD_TYPE_FULL_SENSOR;
    ucd9081_sr[i].header.record_len = sizeof(ucd9081_sr[i]);

    /* RECORD KEY BYTES */
    ucd9081_sr[i].key.owner_id = 0;                             /* 7-bit system software ID */
    ucd9081_sr[i].key.id_type = 1;                              /* System software type */
    ucd9081_sr[i].key.channel_num = 0;
    ucd9081_sr[i].key.sensor_owner_lun = 0;
    ucd9081_sr[i].key.sensor_number = 0;                        /* -CHANGE- this will get replaced by the actual
                                                                    sensor number when we register the SDR */

    /* RECORD BODY BYTES */
    ucd9081_sr[i].entity_id.id = ENTITY_ID_SYSTEM_BOARD;        /* physical entity the sensor is monitoring */
    ucd9081_sr[i].entity_id.entity_type = 0;                    /* treat entity as a physical entity */
    ucd9081_sr[i].entity_id.entity_instance_num = 0;

    /* Sensor Default (power up) State */
    ucd9081_sr[i].initialization.powerup_sensor_scanning = 1;   /* sensor scanning enabled */
    ucd9081_sr[i].initialization.powerup_evt_generation = 0;    /* event generation disabled */
    ucd9081_sr[i].initialization.init_sensor_type = 0;
    ucd9081_sr[i].initialization.init_hysteresis = 0;
    ucd9081_sr[i].initialization.init_thresholds = 0;
    ucd9081_sr[i].initialization.init_events = 0;
    ucd9081_sr[i].initialization.init_scanning = 1;             /* accepts scanning bit in the SetSensorEventEnable command */

    /* Sensor Capabilities */
    ucd9081_sr[i].capabilities.ignore_sensor = 0;               /* Ignore sensor if entity is not present or disabled. */
    ucd9081_sr[i].capabilities.sensor_rearm_support = 1;        /* automatically rearms itself when the event clears */
    ucd9081_sr[i].capabilities.sensor_hysteresis_support = 2;   /* hysteresis is readable and settable */
    ucd9081_sr[i].capabilities.sensor_threshold_access = 2;     /* thresholds are readable and settable
                                                                   per Reading Mask and Settable Threshold Mask, respectively */
    ucd9081_sr[i].capabilities.event_msg_control = 0;           /* per threshold/discrete-state event
                                                                   enable/disable control (implies that entire
                                                                   sensor and global disable are also supported) */
    ucd9081_sr[i].sensor_type = SENSOR_TYPE_VOLTAGE;            /* From Table 42-3, Sensor Type Codes */
    ucd9081_sr[i].event_type_code = 1;                          /* From Table 42-1, Threshold */

    /* Assertion Event Mask */
    ucd9081_sr[i].mask.type.threshold.assert_lower_non_critical_low = 0;
    ucd9081_sr[i].mask.type.threshold.assert_lower_non_critical_high = 0;
    ucd9081_sr[i].mask.type.threshold.assert_lower_critical_low = 0;
    ucd9081_sr[i].mask.type.threshold.assert_lower_critical_high = 1;
    ucd9081_sr[i].mask.type.threshold.assert_lower_non_recoverable_low = 0;
    ucd9081_sr[i].mask.type.threshold.assert_lower_non_recoverable_high = 0;
    ucd9081_sr[i].mask.type.threshold.assert_upper_non_critical_low = 0;
    ucd9081_sr[i].mask.type.threshold.assert_upper_non_critical_high = 0;
    ucd9081_sr[i].mask.type.threshold.assert_upper_critical_low = 0;
    ucd9081_sr[i].mask.type.threshold.assert_upper_critical_high = 1;
    ucd9081_sr[i].mask.type.threshold.assert_upper_non_recoverable_low = 0;
    ucd9081_sr[i].mask.type.threshold.assert_upper_non_recoverable_high = 0;

    /* Lower/Upper Threshold Reading Mask
     * for Command 'Get Sensor Reading'
     */
    ucd9081_sr[i].mask.type.threshold.status_lower_non_critical = 0;
    ucd9081_sr[i].mask.type.threshold.status_lower_critical = 1;
    ucd9081_sr[i].mask.type.threshold.status_lower_non_recoverable = 0;
    ucd9081_sr[i].mask.type.threshold.status_upper_non_critical = 0;
    ucd9081_sr[i].mask.type.threshold.status_upper_critical = 1;
    ucd9081_sr[i].mask.type.threshold.status_upper_non_recoverable = 0;

    /* Deassertion Event Mask */
    ucd9081_sr[i].mask.type.threshold.dassert_lower_non_critical_low = 0;
    ucd9081_sr[i].mask.type.threshold.dassert_lower_non_critical_high = 0;
    ucd9081_sr[i].mask.type.threshold.dassert_lower_critical_low = 0;
    ucd9081_sr[i].mask.type.threshold.dassert_lower_critical_high = 1;
    ucd9081_sr[i].mask.type.threshold.dassert_lower_non_recoverable_low = 0;
    ucd9081_sr[i].mask.type.threshold.dassert_lower_non_recoverable_high = 0;
    ucd9081_sr[i].mask.type.threshold.dassert_upper_non_critical_low = 0;
    ucd9081_sr[i].mask.type.threshold.dassert_upper_non_critical_high = 0;
    ucd9081_sr[i].mask.type.threshold.dassert_upper_critical_low = 0;
    ucd9081_sr[i].mask.type.threshold.dassert_upper_critical_high = 1;
    ucd9081_sr[i].mask.type.threshold.dassert_upper_non_recoverable_low = 0;
    ucd9081_sr[i].mask.type.threshold.dassert_upper_non_recoverable_high = 0;

    /* Settable Threshold Mask, Readable Threshold Mask
     * for Command 'Set Sensor Thresholds' and 'Get Sensor Thresholds'
     */
    ucd9081_sr[i].mask.type.threshold.lower_non_critical_readable = 0;
    ucd9081_sr[i].mask.type.threshold.lower_critical_readable = 1;
    ucd9081_sr[i].mask.type.threshold.lower_non_recoverable_readable = 0;
    ucd9081_sr[i].mask.type.threshold.upper_non_critical_readable = 0;
    ucd9081_sr[i].mask.type.threshold.upper_critical_readable = 1;
    ucd9081_sr[i].mask.type.threshold.upper_non_recoverable_readable = 0;

    ucd9081_sr[i].mask.type.threshold.lower_non_critical_settable = 0;
    ucd9081_sr[i].mask.type.threshold.lower_critical_settable = 0;
    ucd9081_sr[i].mask.type.threshold.lower_non_recoverable_settable = 0;
    ucd9081_sr[i].mask.type.threshold.upper_non_critical_settable = 0;
    ucd9081_sr[i].mask.type.threshold.upper_critical_settable = 0;
    ucd9081_sr[i].mask.type.threshold.upper_non_recoverable_settable = 0;

    /* Unit 1, 2, 3 */
    ucd9081_sr[i].unit.analog_data_format = 2;                  /* 2's complement (signed) */
    ucd9081_sr[i].unit.rate_unit = 0;                           /* none */
    ucd9081_sr[i].unit.modifier_unit = 0;                       /* 00b = none */
    ucd9081_sr[i].unit.percentage = 0;                          /* not a percentage value */
    ucd9081_sr[i].unit.unit_2_base = SENSOR_UNIT_VOLTS;         /* Base Unit */
    ucd9081_sr[i].unit.unit_3_modifier = 0;                     /* no modifier unit */
    ucd9081_sr[i].linearization = 0;                            /* Linear */

    ucd9081_sr[i].mtol.m_ls = FORMULA_M_LS(UCD9081_VAL_M);
    ucd9081_sr[i].mtol.m_ms = FORMULA_M_MS(UCD9081_VAL_M);
    ucd9081_sr[i].mtol.tolerance = 2;

	ucd9081_sr[i].bacc.b_ls = FORMULA_B_LS(UCD9081_VAL_B);
	ucd9081_sr[i].bacc.b_ms = FORMULA_B_MS(UCD9081_VAL_B);
	ucd9081_sr[i].bacc.accuracy_ls = 0;
	ucd9081_sr[i].bacc.accuracy_ms = 0;
    ucd9081_sr[i].bacc.accuracy_exp = 0;
    ucd9081_sr[i].bacc.sensor_dir = 0;
    ucd9081_sr[i].bacc.b_exponent = UCD9081_VAL_K1;
    ucd9081_sr[i].bacc.r_exponent = UCD9081_VAL_K2;

    ucd9081_sr[i].analog_flags.nominal_reading_specified = 0;
    ucd9081_sr[i].analog_flags.normal_max_specified = 1;
    ucd9081_sr[i].analog_flags.normal_min_specified = 1;

    ucd9081_sr[i].sensor_maximum_reading = UCD9081_VAL_MAX;
    ucd9081_sr[i].sensor_minimum_reading = UCD9081_VAL_MIN;

    ucd9081_sr[i].upper_non_recoverable_threshold = 0;
    ucd9081_sr[i].upper_critical_threshold =
        (uint8_t)lrint(SENSOR_VAL2RAW(ucd9081_vol_tbl[i].rail_ov, UCD9081_M, UCD9081_B, UCD9081_K1, UCD9081_K2));
    ucd9081_sr[i].upper_non_critical_threshold = 0;
    ucd9081_sr[i].lower_non_recoverable_threshold = 0;
    ucd9081_sr[i].lower_critical_threshold =
        (uint8_t)lrint(SENSOR_VAL2RAW(ucd9081_vol_tbl[i].rail_uv, UCD9081_M, UCD9081_B, UCD9081_K1, UCD9081_K2));
    ucd9081_sr[i].lower_non_critical_threshold = 0;
    ucd9081_sr[i].positive_going_threshold_hysteresis_value = 0;
    ucd9081_sr[i].negative_going_threshold_hysteresis_value = 0;
    ucd9081_sr[i].reserved2 = 0;
    ucd9081_sr[i].reserved3 = 0;
    ucd9081_sr[i].oem = 0;
    ucd9081_sr[i].id_type_length.type = 3;                      /* 11 = 8-bit ASCII + Latin 1. */
	snprintf(id_string, 16, "ucd9081-%d", i);
    ucd9081_sr[i].id_type_length.length = strlen(id_string);    /* length of following data, in characters */
    memcpy(ucd9081_sr[i].id_string_bytes, id_string, strlen(id_string));

    ucd9081_sd[i].sensor_id = 0;
    ucd9081_sd[i].last_sensor_reading = 0;
    ucd9081_sd[i].scan_period = 0;
    ucd9081_sd[i].scan_function = ucd9081_scan_function;
    ucd9081_sd[i].sett_function = ucd9081_sett_function;
    ucd9081_sd[i].event_messages_enabled = 1;
    ucd9081_sd[i].sensor_scanning_enabled = 1;
    ucd9081_sd[i].unavailable = 0;
    ucd9081_sd[i].local_sensor_id = i;
    ucd9081_sd[i].recv1 = 0;
    ucd9081_sd[i].recv2 = 0;
    ucd9081_sd[i].recv3 = 0;

	sensor_add(&ucd9081_sd[i], SDR_RECORD_TYPE_FULL_SENSOR, &ucd9081_sr[i]);
}

void ucd9081_init(void)
{
    uint8_t i;

    ucd9081_init_chip();

    for (i = 0; i < MAX_UCD9081_SENSOR_COUNT; i++)
    {
        ucd9081_init_sensor_record(i);
    }
}

#endif  // IPMI_CHIP_UCD9081

