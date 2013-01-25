//*****************************************************************************
//
// ipmi_se.c - IPMI Command for Sensor/Event
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#include "ipmi_lib/ipmi.h"
#include "app/lib_common.h"
#include <string.h>

#define MAX_SDR_COUNT               32
#define MAX_SENSOR_COUNT            32

// in this implementation: number of sensors == number of SDRs
uint16_t current_sensor_count = 0;
uint16_t current_sdr_count = 0;
uint16_t sdr_reservation_id = 0;

sdr_record_entry_t sdr_entry_table[MAX_SDR_COUNT];
sensor_data_t *sensor[MAX_SENSOR_COUNT];
extern uint32_t g_sel_sdr_time;


void ipmi_sensor_scan_period(void)
{
    uint8_t i;

    for (i = 0; i < MAX_SENSOR_COUNT; i++)
    {
        if (sensor[i] != NULL && sensor[i]->scan_period) {
            if ((sensor[i]->last_scan_timestamp + sensor[i]->scan_period) <= g_sel_sdr_time) {
                sensor[i]->scan_function((void *)sensor[i]);
                if (sensor[i]->retry == 0) {  // sensor is scanning ok
                    sensor[i]->last_scan_timestamp = g_sel_sdr_time;
                }
                break;
            }
        }
    }
}

uint8_t sdr_record_len(uint8_t sdr_type)
{
    switch (sdr_type)
    {
        case SDR_RECORD_TYPE_FULL_SENSOR:
            return sizeof(SDR_RECORD_FULL);
        case SDR_RECORD_TYPE_COMPACT_SENSOR:
            return sizeof(SDR_RECORD_COMPACT);
        case SDR_RECORD_TYPE_MC_DEVICE_LOCATOR:
            return sizeof(SDR_RECORD_MC_LOCATOR);
        default:
            return 0;
    }
}

int sensor_add(sensor_data_t *sensor_data, uint8_t sdr_type, void *sdr)
{
    if (current_sensor_count >= MAX_SENSOR_COUNT)
        return -1;
    if (current_sdr_count >= MAX_SDR_COUNT)
        return -1;

    sensor[current_sensor_count] = sensor_data;
    sensor_data->sensor_id = current_sensor_count;
    sensor_data->sdr_record = &sdr_entry_table[current_sdr_count];

    sdr_entry_table[current_sdr_count].record_id = current_sdr_count;
    sdr_entry_table[current_sdr_count].record_type = sdr_type;
    sdr_entry_table[current_sdr_count].record_len = sdr_record_len(sdr_type);
    sdr_entry_table[current_sdr_count].record_ptr = sdr;

    ((SDR_RECORD_HEADER_KEY*)sdr)->header.record_id = current_sdr_count;

    if (sdr_type == SDR_RECORD_TYPE_FULL_SENSOR ||
        sdr_type == SDR_RECORD_TYPE_COMPACT_SENSOR) {
        ((SDR_RECORD_HEADER_KEY*)sdr)->key.sensor_number = current_sensor_count;
    }

    current_sensor_count++;
    current_sdr_count++;

    return 0;
}

/*======================================================================*/
/*
 *   Sensor Device Commands
 *
 *   Mandatory Commands
 *       Get Device SDR Info
 *       Get Device SDR
 *       Reserve Device SDR Repository
 *       Get Sensor Reading
 *
 *       Using NETFN_EVENT_REQ/NETFN_EVENT_RESP
 */
/*======================================================================*/


void ipmi_get_device_sdr_info(struct ipmi_ctx *ctx_cmd)
{
    struct sdr_device_info_rs *sdr_info;

    sdr_info = (struct sdr_device_info_rs *)&ctx_cmd->rsp.data[0];

    /* operation bit [0]
       1b = Get SDR count. This returns the total number of SDRs in
       the device.
       0b = Get Sensor count. This returns the number of sensors
       implemented on LUN this command was addressed to */
    if (ctx_cmd->req_len == 0)                      // Get Sensor count in current lun
    {
        sdr_info->count = current_sensor_count;
    }
    else if (ctx_cmd->req_len == 1)
    {
        if (ctx_cmd->req.data[0] == 0x01)           // Get all SDRs count in the device.
        {
            sdr_info->count = current_sensor_count;
        }
        else if (ctx_cmd->req.data[0] == 0x00)      // Get Sensor count in current lun
        {
            sdr_info->count = 0;
        }
        else
        {
            ipmi_cmd_err(ctx_cmd, IPMI_CC_PARAM_OUT_OF_RANGE);
            return;
        }
    }
    else
    {
        ipmi_cmd_err(ctx_cmd, IPMI_CC_REQ_DATA_INV_LENGTH);
        return;
    }

    /* Flags:
       0b = static sensor population. The number of sensors handled by this
       device is fixed, and a query shall return records for all sensors.
       1b = dynamic sensor population. This device may have its sensor
       population vary during run time (defined as any time other that
       when an install operation is in progress). */
    sdr_info->flags = 0;

    /* Device LUNs
       [3] - 1b = LUN 3 has sensors
       [2] - 1b = LUN 2 has sensors
       [1] - 1b = LUN 1 has sensors
       [0] - 1b = LUN 0 has sensors */
    sdr_info->device_luns = 1;

    /* Four byte timestamp, or counter. Updated or incremented each time
       the sensor population changes. This field is not provided if the
       flags indicate a static sensor population.*/
    sdr_info->popChangeInd[0] = 0;
    sdr_info->popChangeInd[1] = 0;
    sdr_info->popChangeInd[2] = 0;
    sdr_info->popChangeInd[3] = 0;

    ipmi_cmd_ok(ctx_cmd, sizeof(struct sdr_device_info_rs));
}

void ipmi_reserve_device_sdr_repository(struct ipmi_ctx *ctx_cmd)
{
    struct reserve_device_sdr_repository_resp *reserve_sdr_resp;

    if (!++sdr_reservation_id)
        sdr_reservation_id++;

    reserve_sdr_resp = (struct reserve_device_sdr_repository_resp *)&ctx_cmd->rsp.data[0];
    reserve_sdr_resp->reservation_id_lsb = 0xff & sdr_reservation_id;
    reserve_sdr_resp->reservation_id_msb = sdr_reservation_id >> 8;

    ipmi_cmd_ok(ctx_cmd, sizeof(struct reserve_device_sdr_repository_resp));
}

void ipmi_get_device_sdr(struct ipmi_ctx *ctx_cmd)
{
    GET_DEVICE_SDR_CMD *req = (GET_DEVICE_SDR_CMD *)(&ctx_cmd->req.data[0]);
    GET_DEVICE_SDR_RESP *resp = (GET_DEVICE_SDR_RESP *)(&ctx_cmd->rsp.data[0]);
    unsigned short record_id, i, found = 0;

    /* if offset into record is zero we don't have to worry about the
     * reservation ids */
    if (req->offset != 0) {
        /* Otherwise check to see if we have the reservation */
        if (sdr_reservation_id != (req->reservation_id_msb << 8 | req->reservation_id_lsb)) {
            ipmi_cmd_err(ctx_cmd, IPMI_CC_RES_CANCELED);
            return;
        }
    }
    /* check if we have a valid record ID */
    record_id = req->record_id_msb << 8 | req->record_id_lsb;
    for (i = 0; i < current_sensor_count; i++) {
        if (sdr_entry_table[i].record_id == record_id) {
            found++;
            break;
        }
    }

    /* using record ID, offset and bytes to read fields, fill in the req_bytes field */
    if (found) {
        /* fill in the Record ID for next record */
        if (i + 1 < current_sensor_count) {
            resp->rec_id_next_lsb = sdr_entry_table[i+1].record_id & 0xff;
            resp->rec_id_next_msb = sdr_entry_table[i+1].record_id >> 8;
        } else {
            resp->rec_id_next_lsb = 0xff;
            resp->rec_id_next_msb = 0xff;
        }

        /* SDR Data goes in here */
        /* check req->bytes_to_read. FFh means read entire record. */
        if (req->bytes_to_read + req->offset > sdr_entry_table[i].record_len) {
            memcpy(resp->req_bytes,
                    (char*)(sdr_entry_table[i].record_ptr) + req->offset,
                    sdr_entry_table[i].record_len - req->offset);
            ipmi_cmd_ok(ctx_cmd, sdr_entry_table[i].record_len - req->offset + 2);
        } else {
            memcpy(resp->req_bytes, (char*)(sdr_entry_table[i].record_ptr) + req->offset, req->bytes_to_read);
            ipmi_cmd_ok(ctx_cmd, req->bytes_to_read + 2);
        }

        /* TODO return a 80h = record changed status if any of the record contents
           have been altered since the last time the Requester issued the request
           with 00h for the Offset into SDR field. This can be implemented by adding
           last_query_ts, and record_change_ts timestamps to the SDR_ENTRY struct.
        Q: shouldn't the reservation scheme take care of this */
    } else {
        ipmi_cmd_err(ctx_cmd, IPMI_CC_REQ_DATA_NOT_PRESENT);
    }
}


void ipmi_get_sensor_reading(struct ipmi_ctx *ctx_cmd)
{
    GET_SENSOR_READING_RESP *resp = (GET_SENSOR_READING_RESP *)(&ctx_cmd->rsp.data[0]);
    uint8_t sensor_number;
    int i, found = 0;

    sensor_number = ctx_cmd->req.data[0];

    /* Given the req->sensor_number return the reading */
    for (i = 0; i < current_sensor_count; i++) {
        if (sensor[i]->sensor_id == sensor_number) {
            found++;
            break;
        }
    }

    /* if this is a non-periodically scanned sensor, call the sensor scan
     * function to update the sensor reading*/
    if (found && !sensor[i]->scan_period)
        sensor[i]->scan_function((void *)sensor[i]);

    if (found) {
        resp->sensor_reading                    = sensor[i]->last_sensor_reading;
        resp->event_messages_enabled            = sensor[i]->event_messages_enabled;
        resp->sensor_scanning_enabled           = sensor[i]->sensor_scanning_enabled;
        resp->unavailable                       = sensor[i]->unavailable;
        resp->lower_non_critical_threshold      = sensor[i]->lower_non_critical_threshold;
        resp->lower_critical_threshold          = sensor[i]->lower_critical_threshold;
        resp->lower_non_recoverable_threshold   = sensor[i]->lower_non_recoverable_threshold;
        resp->upper_non_critical_threshold      = sensor[i]->upper_non_critical_threshold;
        resp->upper_critical_threshold          = sensor[i]->upper_critical_threshold;
        resp->upper_non_recoverable_threshold   = sensor[i]->upper_non_recoverable_threshold;
        ipmi_cmd_ok(ctx_cmd, sizeof(GET_SENSOR_READING_RESP));
    } else {
        ipmi_cmd_err(ctx_cmd, IPMI_CC_REQ_DATA_NOT_PRESENT);
    }
}

void ipmi_get_sensor_factors(struct ipmi_ctx *ctx_cmd)
{
    ipmi_cmd_invalid(ctx_cmd);
}

void ipmi_set_sensor_thresholds(struct ipmi_ctx *ctx_cmd)
{
    uint8_t sensor_number = ctx_cmd->req.data[0];
    SENSOR_THRESHOLDS *req = (SENSOR_THRESHOLDS *)&ctx_cmd->req.data[1];
    int i, found = 0;

    /* Given the req->sensor_number return the reading */
    for (i = 0; i < current_sensor_count; i++) {
        if (sensor[i]->sensor_id == sensor_number) {
            found++;
            break;
        }
    }

    if (found) {
        SDR_RECORD_FULL *fsdr = (SDR_RECORD_FULL *)sdr_entry_table[i].record_ptr;

        if (fsdr->event_type_code != 0x01) {
            ipmi_cmd_err(ctx_cmd, IPMI_CC_REQ_DATA_NOT_PRESENT);
            return;
        }

        sensor[i]->set_upper_non_recoverable_threshold  = req->set_upper_non_recoverable_threshold;
        sensor[i]->set_upper_critical_threshold         = req->set_upper_critical_threshold;
        sensor[i]->set_upper_non_critical_threshold     = req->set_upper_non_critical_threshold;
        sensor[i]->set_lower_non_critical_threshold     = req->set_lower_non_critical_threshold;
        sensor[i]->set_lower_critical_threshold         = req->set_lower_critical_threshold;
        sensor[i]->set_lower_non_recoverable_threshold  = req->set_lower_non_recoverable_threshold;

        if (req->set_upper_non_recoverable_threshold &&
                fsdr->mask.type.threshold.upper_non_recoverable_settable)
            fsdr->upper_non_recoverable_threshold = req->upper_non_recoverable_threshold;

        if (req->set_upper_critical_threshold &&
                fsdr->mask.type.threshold.upper_critical_settable)
            fsdr->upper_critical_threshold = req->upper_critical_threshold;

        if (req->set_upper_non_critical_threshold &&
                fsdr->mask.type.threshold.upper_non_critical_settable)
            fsdr->upper_non_critical_threshold = req->upper_non_critical_threshold;

        if (req->set_lower_non_critical_threshold &&
                fsdr->mask.type.threshold.lower_non_critical_settable)
            fsdr->lower_non_critical_threshold = req->lower_non_critical_threshold;

        if (req->set_lower_critical_threshold &&
                fsdr->mask.type.threshold.lower_critical_settable)
            fsdr->lower_critical_threshold = req->lower_critical_threshold;

        if (req->set_lower_non_recoverable_threshold &&
                fsdr->mask.type.threshold.lower_non_recoverable_settable)
            fsdr->lower_non_recoverable_threshold = req->lower_non_recoverable_threshold;

        sensor[i]->sett_function((void *)sensor[i]);

        ipmi_cmd_ok(ctx_cmd, 0);
    } else {
        ipmi_cmd_err(ctx_cmd, IPMI_CC_REQ_DATA_NOT_PRESENT);
    }
}

void ipmi_get_sensor_thresholds(struct ipmi_ctx *ctx_cmd)
{
    uint8_t sensor_number = ctx_cmd->req.data[0];
    SENSOR_THRESHOLDS *rsp = (SENSOR_THRESHOLDS *)&ctx_cmd->rsp.data[0];
    int i, found = 0;

    /* Given the req->sensor_number return the reading */
    for (i = 0; i < current_sensor_count; i++) {
        if (sensor[i]->sensor_id == sensor_number) {
            found++;
            break;
        }
    }

    if (found) {
        SDR_RECORD_FULL *fsdr = (SDR_RECORD_FULL *)sdr_entry_table[i].record_ptr;

        if (fsdr->event_type_code != 0x01) {
            ipmi_cmd_err(ctx_cmd, IPMI_CC_REQ_DATA_NOT_PRESENT);
            return;
        }

        rsp->set_upper_non_recoverable_threshold    = fsdr->mask.type.threshold.upper_non_recoverable_readable;
        rsp->set_upper_critical_threshold           = fsdr->mask.type.threshold.upper_critical_readable;
        rsp->set_upper_non_critical_threshold       = fsdr->mask.type.threshold.upper_non_critical_readable;
        rsp->set_lower_non_recoverable_threshold    = fsdr->mask.type.threshold.lower_non_recoverable_readable;
        rsp->set_lower_critical_threshold           = fsdr->mask.type.threshold.lower_critical_readable;
        rsp->set_lower_non_critical_threshold       = fsdr->mask.type.threshold.lower_non_critical_readable;

        if (fsdr->mask.type.threshold.upper_non_recoverable_readable)
            rsp->upper_non_recoverable_threshold = fsdr->upper_non_recoverable_threshold;

        if (fsdr->mask.type.threshold.upper_critical_readable)
            rsp->upper_critical_threshold = fsdr->upper_critical_threshold;

        if (fsdr->mask.type.threshold.upper_non_critical_readable)
            rsp->upper_non_critical_threshold = fsdr->upper_non_critical_threshold;

        if (fsdr->mask.type.threshold.lower_non_critical_readable)
            rsp->lower_non_critical_threshold = fsdr->lower_non_critical_threshold;

        if (fsdr->mask.type.threshold.lower_critical_readable)
            rsp->lower_critical_threshold = fsdr->lower_critical_threshold;

        if (fsdr->mask.type.threshold.lower_non_recoverable_readable)
            rsp->lower_non_recoverable_threshold = fsdr->lower_non_recoverable_threshold;

        ipmi_cmd_ok(ctx_cmd, sizeof(SENSOR_THRESHOLDS));
    } else {
        ipmi_cmd_err(ctx_cmd, IPMI_CC_REQ_DATA_NOT_PRESENT);
    }
}

void ipmi_get_sensor_event_enable(struct ipmi_ctx *ctx_cmd)
{
    uint8_t sensor_number = ctx_cmd->req.data[0];
    SENSOR_EVENT_ENABLE *rsp = (SENSOR_EVENT_ENABLE *)&ctx_cmd->rsp.data[0];
    int i, found = 0;

    /* Given the req->sensor_number return the reading */
    for (i = 0; i < current_sensor_count; i++) {
        if (sensor[i]->sensor_id == sensor_number) {
            found++;
            break;
        }
    }

    if (found) {
        SDR_RECORD_FULL *fsdr = (SDR_RECORD_FULL *)sdr_entry_table[i].record_ptr;

        if (!IS_THRESHOLD_SENSOR(fsdr)) {
            ipmi_cmd_err(ctx_cmd, IPMI_CC_REQ_DATA_NOT_PRESENT);
            return;
        }

        rsp->sensor_scanning_enabled = sensor[i]->sensor_scanning_enabled;
        rsp->event_messages_enabled = sensor[i]->event_messages_enabled;

        if (rsp->event_messages_enabled)
        {
#define BIT_SET(bit,x,y)    ((x).bit = (y).bit)
            BIT_SET(assert_lower_non_critical_low, rsp->type.threshold, fsdr->mask.type.threshold);
            BIT_SET(assert_lower_non_critical_low, rsp->type.threshold, fsdr->mask.type.threshold);
            BIT_SET(assert_lower_non_critical_high, rsp->type.threshold, fsdr->mask.type.threshold);
            BIT_SET(assert_lower_critical_low, rsp->type.threshold, fsdr->mask.type.threshold);
            BIT_SET(assert_lower_critical_high, rsp->type.threshold, fsdr->mask.type.threshold);
            BIT_SET(assert_lower_non_recoverable_low, rsp->type.threshold, fsdr->mask.type.threshold);
            BIT_SET(assert_lower_non_recoverable_high, rsp->type.threshold, fsdr->mask.type.threshold);
            BIT_SET(assert_upper_non_critical_low, rsp->type.threshold, fsdr->mask.type.threshold);
            BIT_SET(assert_upper_non_critical_high, rsp->type.threshold, fsdr->mask.type.threshold);
            BIT_SET(assert_upper_critical_low, rsp->type.threshold, fsdr->mask.type.threshold);
            BIT_SET(assert_upper_critical_high, rsp->type.threshold, fsdr->mask.type.threshold);
            BIT_SET(assert_upper_non_recoverable_low, rsp->type.threshold, fsdr->mask.type.threshold);
            BIT_SET(assert_upper_non_recoverable_high, rsp->type.threshold, fsdr->mask.type.threshold);
            BIT_SET(dassert_lower_non_critical_low, rsp->type.threshold, fsdr->mask.type.threshold);
            BIT_SET(dassert_lower_non_critical_high, rsp->type.threshold, fsdr->mask.type.threshold);
            BIT_SET(dassert_lower_critical_low, rsp->type.threshold, fsdr->mask.type.threshold);
            BIT_SET(dassert_lower_critical_high, rsp->type.threshold, fsdr->mask.type.threshold);
            BIT_SET(dassert_lower_non_recoverable_low, rsp->type.threshold, fsdr->mask.type.threshold);
            BIT_SET(dassert_lower_non_recoverable_high, rsp->type.threshold, fsdr->mask.type.threshold);
            BIT_SET(dassert_upper_non_critical_low, rsp->type.threshold, fsdr->mask.type.threshold);
            BIT_SET(dassert_upper_non_critical_high, rsp->type.threshold, fsdr->mask.type.threshold);;
            BIT_SET(dassert_upper_critical_low, rsp->type.threshold, fsdr->mask.type.threshold);
            BIT_SET(dassert_upper_critical_high, rsp->type.threshold, fsdr->mask.type.threshold);
            BIT_SET(dassert_upper_non_recoverable_low, rsp->type.threshold, fsdr->mask.type.threshold);
            BIT_SET(dassert_upper_non_recoverable_high, rsp->type.threshold, fsdr->mask.type.threshold);
#undef BIT_SET
            ipmi_cmd_ok(ctx_cmd, sizeof(SENSOR_EVENT_ENABLE));
        } else {
            ipmi_cmd_ok(ctx_cmd, 1);
        }

    } else {
        ipmi_cmd_err(ctx_cmd, IPMI_CC_REQ_DATA_NOT_PRESENT);
    }
}

void ipmi_get_sensor_event_status(struct ipmi_ctx *ctx_cmd)
{
    uint8_t sensor_number = ctx_cmd->req.data[0];
    SENSOR_EVENT_STATUS *rsp = (SENSOR_EVENT_STATUS *)&ctx_cmd->rsp.data[0];
    int i, found = 0;
    static struct {
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
    } last_threshold;

    /* Given the req->sensor_number return the reading */
    for (i = 0; i < current_sensor_count; i++) {
        if (sensor[i]->sensor_id == sensor_number) {
            found++;
            break;
        }
    }

    if (found) {
        rsp->event_messages_enabled = sensor[i]->event_messages_enabled;
        rsp->sensor_scanning_enabled = sensor[i]->sensor_scanning_enabled;
        rsp->unavailable = sensor[i]->unavailable;

        rsp->type.discrete.assertion_event_msg = 0;
        rsp->type.discrete.dassertion_event_msg = 0;

        if (last_threshold.lower_non_critical_threshold == 0 &&
            sensor[i]->lower_non_critical_threshold == 1) {
            rsp->type.threshold.assert_lower_non_critical_low = 1;
        }
        if (last_threshold.lower_non_critical_threshold == 1 &&
            sensor[i]->lower_non_critical_threshold == 0) {
            rsp->type.threshold.dassert_lower_non_critical_low = 1;
        }

        if (last_threshold.lower_critical_threshold == 0 &&
            sensor[i]->lower_critical_threshold == 1) {
            rsp->type.threshold.assert_lower_critical_low = 1;
        }
        if (last_threshold.lower_critical_threshold == 1 &&
            sensor[i]->lower_critical_threshold == 0) {
            rsp->type.threshold.dassert_lower_critical_low = 1;
        }

        if (last_threshold.lower_non_recoverable_threshold == 0 &&
            sensor[i]->lower_non_recoverable_threshold == 1) {
            rsp->type.threshold.assert_lower_non_recoverable_low = 1;
        }
        if (last_threshold.lower_non_recoverable_threshold == 1 &&
            sensor[i]->lower_non_recoverable_threshold == 0) {
            rsp->type.threshold.dassert_lower_non_recoverable_low = 1;
        }

        if (last_threshold.upper_non_critical_threshold == 0 &&
            sensor[i]->upper_non_critical_threshold == 1) {
            rsp->type.threshold.assert_upper_non_critical_high = 1;
        }
        if (last_threshold.upper_non_critical_threshold == 1 &&
            sensor[i]->upper_non_critical_threshold == 0) {
            rsp->type.threshold.dassert_upper_non_critical_high = 1;
        }

        if (last_threshold.upper_critical_threshold == 0 &&
            sensor[i]->upper_critical_threshold == 1) {
            rsp->type.threshold.assert_upper_critical_high = 1;
        }
        if (last_threshold.upper_critical_threshold == 1 &&
            sensor[i]->upper_critical_threshold == 0) {
            rsp->type.threshold.dassert_upper_critical_high = 1;
        }

        if (last_threshold.upper_non_recoverable_threshold == 0 &&
            sensor[i]->upper_non_recoverable_threshold == 1) {
            rsp->type.threshold.assert_upper_non_recoverable_high = 1;
        }
        if (last_threshold.upper_non_recoverable_threshold == 1 &&
            sensor[i]->upper_non_recoverable_threshold == 0) {
            rsp->type.threshold.dassert_upper_non_recoverable_high = 1;
        }

        last_threshold.lower_non_critical_threshold     = sensor[i]->lower_non_critical_threshold;
        last_threshold.lower_critical_threshold         = sensor[i]->lower_critical_threshold;
        last_threshold.lower_non_recoverable_threshold  = sensor[i]->lower_non_recoverable_threshold;
        last_threshold.lower_non_critical_threshold     = sensor[i]->lower_non_critical_threshold;
        last_threshold.lower_critical_threshold         = sensor[i]->lower_critical_threshold;
        last_threshold.lower_non_recoverable_threshold  = sensor[i]->lower_non_recoverable_threshold;

        ipmi_cmd_ok(ctx_cmd, sizeof(SENSOR_EVENT_STATUS));

    } else {
        ipmi_cmd_err(ctx_cmd, IPMI_CC_REQ_DATA_NOT_PRESENT);
    }
}

void ipmi_get_sensor_type(struct ipmi_ctx *ctx_cmd)
{
    uint8_t sensor_number = ctx_cmd->req.data[0];
    SENSOR_TYPE_RSP *rsp = (SENSOR_TYPE_RSP *)&ctx_cmd->rsp.data[0];
    int i, found = 0;

    /* Given the req->sensor_number return the reading */
    for (i = 0; i < current_sensor_count; i++) {
        if (sensor[i]->sensor_id == sensor_number) {
            found++;
            break;
        }
    }

    if (found) {
        SDR_RECORD_FULL *fsdr = (SDR_RECORD_FULL *)sdr_entry_table[i].record_ptr;
        rsp->sensor_type = fsdr->sensor_type;
        rsp->event_type_code = fsdr->event_type_code;
        ipmi_cmd_ok(ctx_cmd, sizeof(SENSOR_TYPE_RSP));
    } else {
        ipmi_cmd_err(ctx_cmd, IPMI_CC_REQ_DATA_NOT_PRESENT);
    }
}

int ipmi_cmd_se(struct ipmi_ctx *ctx_cmd)
{
    DEBUG("ipmi_cmd_se\r\n");

    switch (ctx_cmd->req.msg.cmd)
    {
        /* Sensor Device Commands *******************************************/

        case GET_DEVICE_SDR_INFO:                           /* 0x20 */
            ipmi_get_device_sdr_info(ctx_cmd);
            break;

        case GET_DEVICE_SDR:                                /* 0x21 */
            ipmi_get_device_sdr(ctx_cmd);
            break;

        case GET_SDR_RESERVE_REPO:                          /* 0x22 */
            ipmi_reserve_device_sdr_repository(ctx_cmd);
            break;

        case GET_SENSOR_FACTORS:                            /* 0x23 */
            ipmi_get_sensor_factors(ctx_cmd);
            break;

        case SET_SENSOR_THRESHOLDS:                         /* 0x26 */
            ipmi_set_sensor_thresholds(ctx_cmd);
            break;

        case GET_SENSOR_THRESHOLDS:                         /* 0x27 */
            ipmi_get_sensor_thresholds(ctx_cmd);
            break;

        case GET_SENSOR_EVENT_ENABLE:                       /* 0x29 */
            ipmi_get_sensor_event_enable(ctx_cmd);
            break;

        case GET_SENSOR_EVENT_STATUS:                       /* 0x2b */
            ipmi_get_sensor_event_status(ctx_cmd);
            break;

        case GET_SENSOR_READING:                            /* 0x2d */
            ipmi_get_sensor_reading(ctx_cmd);
            break;

        case GET_SENSOR_TYPE:                               /* 0x2f */
            ipmi_get_sensor_type(ctx_cmd);
            break;

        case SET_SENSOR_HYSTERESIS:                         /* 0x24 */
        case GET_SENSOR_HYSTERESIS:                         /* 0x25 */
        case SET_SENSOR_EVENT_ENABLE:                       /* 0x28 */
        case RE_ARM_SENSOR_EVENTS:                          /* 0x2a */
        case SET_SENSOR_READING_STATUS:                     /* 0x30 */
        default:
            ipmi_cmd_invalid(ctx_cmd);
            break;
    }

    return 0;
}


