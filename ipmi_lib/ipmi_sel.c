//*****************************************************************************
//
// ipmi_sel.c - IPMI Command for System Event Log (SEL) Commands
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#include "ipmi_lib/ipmi.h"
#include "app/lib_common.h"
#include <string.h>
#include <time.h>

/******************************************************************************
* SEL STORAGE DESIGN
* 1) write to last, read from first
* 2) the last unit is next write uint
* 3) if last = first, first must move to next unit
*
*    first      last
*      |          |
* +-+-+-+-+-+---+---+---+
* |0|1|2|3|4|...|126|127|
* +-+-+-+-+-+---+---+---+
*
******************************************************************************/

extern uint32_t g_sel_sdr_time;
extern uint16_t g_sel_sdr_status;
sel_event_header g_ipmi_sel_header;
uint16_t g_sel_reservation_id = 0;


uint16_t ipmi_get_valid_sel_record(uint16_t record_offset, uint16_t max_entry, uint16_t record_id, sel_event_record *sel_record)
{
    uint32_t error;
    uint16_t i;

    for (i = 0; i < max_entry; i++) {
        // read sel record from eeprom
        error = at24xx_read(IPMI_SEL_STORAGE_OFFSET + record_offset, (uint8_t*)sel_record, sizeof(sel_event_record));
        if (error) {
            return 0xffff;
        }

        // search specify record_id
        if (record_id != 0) {
            if (sel_record->record_id == record_id) {
                return i;
            }
        // search any valid record_id
        } else {
            if (sel_record->record_id != 0 && sel_record->record_id != 0xffff) {
                return i;
            }
        }

        record_offset = (record_offset + IPMI_MAX_SEL_BYTES) % IPMI_MAX_SEL_SPACE;
    }

    return 0xffff;
}


void ipmi_get_sel_info(struct ipmi_ctx *ctx_cmd)
{
    struct sel_info_rsp *req = (struct sel_info_rsp *)(&ctx_cmd->rsp.data[0]);

    req->version = IPMI_SEL_VERSION;
    req->number_of_entries = B16_H2L(g_ipmi_sel_header.number_of_entries);
    req->free_space = B16_H2L(g_ipmi_sel_header.free_space);
    req->most_recent_addition_timestamp = B32_H2L(g_ipmi_sel_header.most_recent_addition_timestamp);
    req->most_recent_erase_timestamp = B32_H2L(g_ipmi_sel_header.most_recent_erase_timestamp);
    req->operation_support = OVERFLOW_FLAG |
                             DELETE_SEL_COMMAND_SUPPORTED |
                             RESERVE_SEL_COMMAND_SUPPORTED |
                             GET_SEL_ALLOCATION_INFORMATION_COMMAND_SUPPORTED;

    ipmi_cmd_ok(ctx_cmd, sizeof(struct sel_info_rsp));
}

void ipmi_get_sel_allocation_info(struct ipmi_ctx *ctx_cmd)
{
    struct sel_allocation_info_rsp *req = (struct sel_allocation_info_rsp *)(&ctx_cmd->rsp.data[0]);

    req->number_of_possible_allocation_units = B16_H2L(g_ipmi_sel_header.number_of_entries);
    req->allocation_unit_size_in_bytes = B16_H2L(g_ipmi_sel_header.number_of_entries * IPMI_MAX_SEL_BYTES);
    req->number_of_free_allocation_units = B16_H2L(IPMI_MAX_SEL_ENTRIES - g_ipmi_sel_header.number_of_entries);
    req->largest_free_block_in_allocation_units = B16_H2L((IPMI_MAX_SEL_ENTRIES - g_ipmi_sel_header.number_of_entries) * IPMI_MAX_SEL_BYTES);
    req->maximum_record_size_in_allocation_units = IPMI_MAX_SEL_BYTES;

    ipmi_cmd_ok(ctx_cmd, sizeof(struct sel_allocation_info_rsp));
}

void ipmi_reserve_sel(struct ipmi_ctx *ctx_cmd)
{
    if (g_sel_sdr_status & IPMI_STORAGE_ERASE_PROCESS) {
        ipmi_cmd_err(ctx_cmd, IPMI_CC_SEL_ERASE_PROGRESS);
        return;
    }

    if (!++g_sel_reservation_id)
        g_sel_reservation_id++;


    ctx_cmd->rsp.data[0] = g_sel_reservation_id & 0xff;
    ctx_cmd->rsp.data[1] = (g_sel_reservation_id >> 8) & 0xff;

    ipmi_cmd_ok(ctx_cmd, 2);
}

void ipmi_get_sel_entry(struct ipmi_ctx *ctx_cmd)
{
    struct get_sel_entry_req *req = (struct get_sel_entry_req *)(&ctx_cmd->req.data[0]);
    struct get_sel_entry_rsp *rsp = (struct get_sel_entry_rsp *)(&ctx_cmd->rsp.data[0]);
    sel_event_record search_sel;
    uint16_t record_id, record_offset, record_size, rev_id;
    uint16_t i, record_index;

    rev_id = B16_L2H(req->reservation_id);
    if (rev_id != 0 && g_sel_reservation_id != rev_id) {
        ipmi_cmd_err(ctx_cmd, IPMI_CC_RES_CANCELED);
        return;
    }

    record_id = B16_L2H(req->sel_record_id);

    if (req->offset_into_record > IPMI_MAX_SEL_BYTES) {
        ipmi_cmd_err(ctx_cmd, IPMI_CC_REQ_DATA_INV_LENGTH);
        return;
    }

    if (record_id == 0xffff) {
        // get the last sel entry
        record_offset = (IPMI_SEL_INDEX_DEC(g_ipmi_sel_header.last_entry_index) * IPMI_MAX_SEL_BYTES);
    } else {
        // get the first or any other sel entry
        record_offset = (g_ipmi_sel_header.first_entry_index * IPMI_MAX_SEL_BYTES);
    }

    // search sel record in eeprom
    for (i = 0; i < g_ipmi_sel_header.number_of_entries; i++) {

        record_index = ipmi_get_valid_sel_record(record_offset, IPMI_MAX_SEL_ENTRIES, 0, &search_sel);
        if (record_index == 0xffff) {
            ipmi_cmd_err(ctx_cmd, IPMI_CC_OUT_OF_SPACE);
            return;
        }

        // next record_offset
        record_offset = (record_offset + IPMI_MAX_SEL_BYTES) % IPMI_MAX_SEL_SPACE;

        // finded the record_id or get the first/last record
        if (record_id == (search_sel.record_id) || record_id == 0x0 || record_id == 0xffff) {

            // copy record to response
            if (req->bytes_to_read + req->offset_into_record > IPMI_MAX_SEL_BYTES) {
                record_size = IPMI_MAX_SEL_BYTES - req->offset_into_record;
                memcpy(rsp->record_data, (char*)(&search_sel) + req->offset_into_record, record_size);
            } else {
                record_size = req->bytes_to_read;
                memcpy(rsp->record_data, (char*)(&search_sel) + req->offset_into_record, record_size);
            }

            // get the next record_id
            if (i == g_ipmi_sel_header.number_of_entries - 1 || record_id == 0xffff) {
                rsp->next_sel_record_id = 0xffff;
            } else {
                record_index = ipmi_get_valid_sel_record(record_offset, IPMI_MAX_SEL_ENTRIES-1, 0, &search_sel);
                if (record_index != 0xffff) {
                    rsp->next_sel_record_id = B16_H2L(search_sel.record_id);
                } else {
                    rsp->next_sel_record_id = 0xffff;
                }
            }

            ipmi_cmd_ok(ctx_cmd, record_size + 2);
            return;
        }
    }
    ipmi_cmd_err(ctx_cmd, IPMI_CC_REQ_DATA_NOT_PRESENT);
}

void ipmi_add_sel_entry(struct ipmi_ctx *ctx_cmd)
{
    uint32_t error;
    uint16_t record_offset;
    sel_event_record new_sel;
    sel_event_record *req = (sel_event_record *)(&ctx_cmd->req.data[0]);
    struct add_sel_entry_rsp *rsp = (struct add_sel_entry_rsp *)(&ctx_cmd->rsp.data[0]);

    // save the sel record
    memcpy(&new_sel, req, IPMI_MAX_SEL_BYTES);

    // auto set the sel record id
    new_sel.record_id = ++g_ipmi_sel_header.count_of_entries;

    // auto set the sel record timestamp
    if ((new_sel.record_type == 0x02) || (new_sel.record_type >= 0xc0 && new_sel.record_type <= 0xdf)) {
        new_sel.sel_type.standard_type.timestamp = B32_H2L(g_sel_sdr_time);
    }

    // write the sel record to eeprom
    record_offset = IPMI_SEL_STORAGE_OFFSET + (g_ipmi_sel_header.last_entry_index * IPMI_MAX_SEL_BYTES);
    error = at24xx_write(record_offset, (uint8_t*)&new_sel, sizeof(sel_event_record));
    if (error) {
        ipmi_cmd_err(ctx_cmd, IPMI_CC_REQ_DATA_NOT_PRESENT);
        return;
    }

    // add the first/last index and number of entries
    // overflow support
    g_ipmi_sel_header.last_entry_index = IPMI_SEL_INDEX_INC(g_ipmi_sel_header.last_entry_index);
    if (g_ipmi_sel_header.last_entry_index == g_ipmi_sel_header.first_entry_index) {
        g_ipmi_sel_header.first_entry_index = IPMI_SEL_INDEX_INC(g_ipmi_sel_header.first_entry_index);
    } else {
        g_ipmi_sel_header.number_of_entries++;
        g_ipmi_sel_header.free_space -= IPMI_MAX_SEL_BYTES;
        g_ipmi_sel_header.alloc_space += IPMI_MAX_SEL_BYTES;
    }

    // change the addition timestamp
    // TODO: change OSTimeGet to RTC time
    g_ipmi_sel_header.most_recent_addition_timestamp = B32_H2L(g_sel_sdr_time);

    // write the sel header to eeprom
    error = at24xx_write(IPMI_SEL_HEADER_OFFSET, (uint8_t*)&g_ipmi_sel_header, sizeof(sel_event_header));
    if (error) {
        ipmi_cmd_err(ctx_cmd, IPMI_CC_REQ_DATA_NOT_PRESENT);
        return;
    }

    // return added record sel id
    rsp->added_record_id = B16_H2L(new_sel.record_id);

    ipmi_cmd_ok(ctx_cmd, 2);
}

uint16_t ipmi_add_sel(struct standard_spec_sel_rec *standard_sel)
{
    sel_event_record new_sel;
    uint16_t record_offset;
    uint32_t error;

    memcpy(&new_sel.sel_type.standard_type, standard_sel, sizeof(struct standard_spec_sel_rec));
    new_sel.record_id = ++g_ipmi_sel_header.count_of_entries;
    new_sel.record_type = 0x02;
    new_sel.sel_type.standard_type.timestamp = g_sel_sdr_time;

    // write the sel record to eeprom
    record_offset = IPMI_SEL_STORAGE_OFFSET + (g_ipmi_sel_header.last_entry_index * IPMI_MAX_SEL_BYTES);
    error = at24xx_write(record_offset, (uint8_t*)&new_sel, sizeof(sel_event_record));
    if (error) {
        return 0;
    }

    // add the first/last index and number of entries
    // overflow support
    g_ipmi_sel_header.last_entry_index = IPMI_SEL_INDEX_INC(g_ipmi_sel_header.last_entry_index);
    if (g_ipmi_sel_header.last_entry_index == g_ipmi_sel_header.first_entry_index) {
        g_ipmi_sel_header.first_entry_index = IPMI_SEL_INDEX_DEC(g_ipmi_sel_header.first_entry_index);
    } else {
        g_ipmi_sel_header.number_of_entries++;
        g_ipmi_sel_header.free_space -= IPMI_MAX_SEL_BYTES;
        g_ipmi_sel_header.alloc_space += IPMI_MAX_SEL_BYTES;
    }

    // change the addition timestamp
    // TODO: change OSTimeGet to RTC time
    g_ipmi_sel_header.most_recent_addition_timestamp = g_sel_sdr_time;

    // write the sel header to eeprom
    error = at24xx_write(IPMI_SEL_HEADER_OFFSET, (uint8_t*)&g_ipmi_sel_header, sizeof(sel_event_header));
    if (error) {
        return 0;
    }

    // return added record sel id
    return new_sel.record_id;
}

void ipmi_del_sel_entry(struct ipmi_ctx *ctx_cmd)
{
    struct del_sel_entry_req *req = (struct del_sel_entry_req *)(&ctx_cmd->req.data[0]);
    struct del_sel_entry_rsp *rsp = (struct del_sel_entry_rsp *)(&ctx_cmd->rsp.data[0]);
    sel_event_record search_sel;
    uint16_t record_index, record_id, rev_id;
    uint16_t record_offset = 0;
    uint32_t error;

    rev_id = B16_L2H(req->reservation_id);
    if (rev_id != 0 && g_sel_reservation_id != rev_id) {
        ipmi_cmd_err(ctx_cmd, IPMI_CC_RES_CANCELED);
        return;
    }

    record_id = B16_L2H(req->sel_record_id);

    if (record_id == 0xffff) {

        // del the last sel entry
        record_offset = (IPMI_SEL_INDEX_DEC(g_ipmi_sel_header.last_entry_index) * IPMI_MAX_SEL_BYTES);

    } else if (record_id == 0x0) {

        // del the first or any other sel entry
        record_offset = (g_ipmi_sel_header.first_entry_index * IPMI_MAX_SEL_BYTES);

    } else {

        // search sel record in eeprom
        record_offset = (g_ipmi_sel_header.first_entry_index * IPMI_MAX_SEL_BYTES);

        record_index = ipmi_get_valid_sel_record(record_offset, IPMI_MAX_SEL_ENTRIES, record_id, &search_sel);

        // can not find the record entry
        if (record_index == 0xffff) {
            ipmi_cmd_err(ctx_cmd, IPMI_CC_REQ_DATA_NOT_PRESENT);
            return;
        } else {
            record_offset = (record_index * IPMI_MAX_SEL_BYTES);
        }
    }

    if (record_offset == 0) {

        // record not found
        ipmi_cmd_err(ctx_cmd, IPMI_CC_REQ_DATA_NOT_PRESENT);
        return;

    } else {

        // get the record id
        record_index = ipmi_get_valid_sel_record(record_offset, IPMI_MAX_SEL_ENTRIES, 0, &search_sel);
        if (record_index == 0xffff) {
            ipmi_cmd_err(ctx_cmd, IPMI_CC_OUT_OF_SPACE);
            return;
        }

        // founded the record and clear it!
        at24xx_clear(record_offset, IPMI_MAX_SEL_ENTRIES);

        // update the sel record header
        g_ipmi_sel_header.count_of_entries--;
        g_ipmi_sel_header.free_space += IPMI_MAX_SEL_BYTES;
        g_ipmi_sel_header.alloc_space -= IPMI_MAX_SEL_BYTES;

        if (record_id == 0x0) {
            g_ipmi_sel_header.first_entry_index = IPMI_SEL_INDEX_INC(g_ipmi_sel_header.first_entry_index);
        }
        if (record_id == 0xffff) {
            g_ipmi_sel_header.last_entry_index = IPMI_SEL_INDEX_DEC(g_ipmi_sel_header.last_entry_index);
        }
        g_ipmi_sel_header.most_recent_erase_timestamp = g_sel_sdr_time;

        // write the sel header to eeprom
        error = at24xx_write(IPMI_SEL_HEADER_OFFSET, (uint8_t*)&g_ipmi_sel_header, sizeof(sel_event_header));
        if (error) {
            ipmi_cmd_err(ctx_cmd, IPMI_CC_RES_CANCELED);
            return;
        }

        // return the deleted record id
        rsp->deleted_record_id = B16_H2L(search_sel.record_id);

        ipmi_cmd_ok(ctx_cmd, 2);
    }
}

void ipmi_get_sel_time(struct ipmi_ctx *ctx_cmd)
{
    struct sel_time_st *rsp = (struct sel_time_st *)(&ctx_cmd->rsp.data[0]);

    rsp->timestamp = B32_H2L(g_sel_sdr_time);

    ipmi_cmd_ok(ctx_cmd, sizeof(struct sel_time_st));
}

void ipmi_set_sel_time(struct ipmi_ctx *ctx_cmd)
{
    struct sel_time_st *req = (struct sel_time_st *)(&ctx_cmd->req.data[0]);

    g_sel_sdr_time = B32_L2H(req->timestamp);

    ipmi_cmd_ok(ctx_cmd, 0);
}

void ipmi_clear_sel_entry(struct ipmi_ctx *ctx_cmd)
{
    struct clear_sel_req *req = (struct clear_sel_req *)(&ctx_cmd->req.data[0]);
    struct clear_sel_rsp *rsp = (struct clear_sel_rsp *)(&ctx_cmd->rsp.data[0]);
    uint32_t error;
    uint16_t status, rev_id;

    rev_id = (req->reservation_id_msb << 8 | req->reservation_id_lsb);
    if (rev_id != 0 && g_sel_reservation_id != rev_id) {
        ipmi_cmd_err(ctx_cmd, IPMI_CC_RES_CANCELED);
        return;
    }

    if (req->char_c != 'C' || req->char_l != 'L' || req->char_r != 'R') {
        ipmi_cmd_err(ctx_cmd, IPMI_CC_INV_DATA_FIELD_IN_REQ);
        return;
    }

    if (req->operation == INITIATE_ERASE) {
        // init erase
        ipmi_storage_status_set(IPMI_STORAGE_ERASE_PROCESS);

        // clear ipmi sel header
        error = at24xx_clear(IPMI_SEL_HEADER_OFFSET, sizeof(sel_event_header));
        if (error) {
            ipmi_storage_status_clr(IPMI_STORAGE_ERASE_PROCESS);
            ipmi_cmd_err(ctx_cmd, IPMI_CC_REQ_DATA_NOT_PRESENT);
            return;
        }

        // clear ipmi sel record storage
        error = at24xx_clear(IPMI_SEL_STORAGE_OFFSET, IPMI_MAX_SEL_SPACE);
        if (error) {
            ipmi_storage_status_clr(IPMI_STORAGE_ERASE_PROCESS);
            ipmi_cmd_err(ctx_cmd, IPMI_CC_REQ_DATA_NOT_PRESENT);
            return;
        }
        ipmi_storage_status_clr(IPMI_STORAGE_ERASE_PROCESS);

        // reinit ipmi sel header
        ipmi_sel_init();

        // clear is finished
        rsp->erasure_progress = 1;

        ipmi_cmd_ok(ctx_cmd, sizeof(struct clear_sel_rsp));
        return;

    } else if (req->operation == GET_ERASURE_STATUS) {
        // get erase status
        ipmi_storage_status_get(&status);
        if (status & IPMI_STORAGE_ERASE_PROCESS) {
            rsp->erasure_progress = 0;
        } else {
            rsp->erasure_progress = 1;
        }

        ipmi_cmd_ok(ctx_cmd, sizeof(struct clear_sel_rsp));
        return;
    }

    ipmi_cmd_err(ctx_cmd, IPMI_CC_INV_DATA_FIELD_IN_REQ);
}

void ipmi_sel_init(void)
{
    uint32_t error;

    /* if eeprom is error, or storage is empty
     * initial the sel header to full empty, and set free space to all space
     */
    error = at24xx_read(IPMI_SEL_HEADER_OFFSET, (uint8_t*)&g_ipmi_sel_header, sizeof(sel_event_header));

    /* if read error or empty eeprom, clear all the sel storage space */
    if (error || g_ipmi_sel_header.count_of_entries == 0xff) {
        g_ipmi_sel_header.number_of_entries = 0;
        g_ipmi_sel_header.count_of_entries = 0;
        g_ipmi_sel_header.free_space = 0;
        g_ipmi_sel_header.alloc_space = 0;
        g_ipmi_sel_header.first_entry_index = 0;
        g_ipmi_sel_header.last_entry_index = 0;
        g_ipmi_sel_header.most_recent_addition_timestamp = 0;
        g_ipmi_sel_header.most_recent_erase_timestamp = 0;
    }

    /* rebuild the free space */
    if (g_ipmi_sel_header.count_of_entries == 0) {
        g_ipmi_sel_header.free_space = IPMI_MAX_SEL_SPACE;
    }

    // write the sel header to eeprom
    error = at24xx_write(IPMI_SEL_HEADER_OFFSET, (uint8_t*)&g_ipmi_sel_header, sizeof(sel_event_header));
    if (error) {
        return;
    }
}


