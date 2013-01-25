//*****************************************************************************
//
// ipmi_sdr.c - IPMI Command for Sensor Data Record Repository
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#include "ipmi_lib/ipmi.h"
#include "app/lib_common.h"
#include <string.h>

extern uint32_t g_sel_sdr_time;
extern uint16_t g_sel_sdr_status;
sdr_repository_header g_ipmi_sdr_header;
uint16_t g_sdr_reservation_id = 0;

uint16_t ipmi_get_valid_sdr_record(uint16_t record_offset, uint16_t max_entry, uint16_t record_id, SDR_RECORD_FULL *sdr_record)
{
    uint32_t error;
    uint16_t i;

    for (i = 0; i < max_entry; i++) {
        // read sel record from eeprom
        error = at24xx_read(IPMI_SDR_STORAGE_OFFSET + record_offset, (uint8_t*)sdr_record, sizeof(SDR_RECORD_FULL));
        if (error) {
            return 0xffff;
        }

        // search specify record_id
        if (record_id != 0) {
            if (sdr_record->header.record_id == record_id) {
                return i;
            }
        // search any valid record_id
        } else {
            if (sdr_record->header.record_id != 0 && sdr_record->header.record_id != 0xffff) {
                return i;
            }
        }

        record_offset = (record_offset + IPMI_MAX_SDR_BYTES) % IPMI_MAX_SDR_SPACE;
    }

    return 0xffff;
}

void ipmi_get_sdr_repository_info(struct ipmi_ctx *ctx_cmd)
{
    struct sdr_info_rsp *rsp = (struct sdr_info_rsp *)(&ctx_cmd->rsp.data[0]);

    rsp->version = IPMI_SDR_VERSION;
    rsp->record_count = B16_H2L(g_ipmi_sdr_header.number_of_entries);
    rsp->free_space = B16_H2L(g_ipmi_sdr_header.free_space);
    rsp->most_recent_addition_timestamp = B32_H2L(g_ipmi_sdr_header.most_recent_addition_timestamp);
    rsp->most_recent_erase_timestamp = B32_H2L(g_ipmi_sdr_header.most_recent_erase_timestamp);
    rsp->operation_support = OVERFLOW_FLAG |
                             DELETE_SDR_COMMAND_SUPPORTED |
                             RESERVE_SDR_COMMAND_SUPPORTED |
                             GET_SDR_ALLOCATION_INFORMATION_COMMAND_SUPPORTED;

    ipmi_cmd_ok(ctx_cmd, sizeof(struct sel_info_rsp));
}

void ipmi_get_sdr_repository_allocation_info(struct ipmi_ctx *ctx_cmd)
{
    struct sdr_allocation_info_rsp *req = (struct sdr_allocation_info_rsp *)(&ctx_cmd->rsp.data[0]);

    req->number_of_possible_allocation_units = B16_H2L(g_ipmi_sdr_header.number_of_entries);
    req->allocation_unit_size_in_bytes = B16_H2L(g_ipmi_sdr_header.number_of_entries * IPMI_MAX_SDR_BYTES);
    req->number_of_free_allocation_units = B16_H2L(IPMI_MAX_SDR_ENTRIES - g_ipmi_sdr_header.number_of_entries);
    req->largest_free_block_in_allocation_units = B16_H2L((IPMI_MAX_SDR_ENTRIES - g_ipmi_sdr_header.number_of_entries) * IPMI_MAX_SDR_BYTES);
    req->maximum_record_size_in_allocation_units = IPMI_MAX_SDR_BYTES;

    ipmi_cmd_ok(ctx_cmd, sizeof(struct sdr_allocation_info_rsp));
}

void ipmi_reserve_sdr_repository(struct ipmi_ctx *ctx_cmd)
{
    if (g_sel_sdr_status & IPMI_STORAGE_ERASE_PROCESS) {
        ipmi_cmd_err(ctx_cmd, IPMI_CC_SEL_ERASE_PROGRESS);
        return;
    }

    if (!++g_sdr_reservation_id)
        g_sdr_reservation_id++;


    ctx_cmd->rsp.data[0] = g_sdr_reservation_id & 0xff;
    ctx_cmd->rsp.data[1] = (g_sdr_reservation_id >> 8) & 0xff;

    ipmi_cmd_ok(ctx_cmd, 2);
}

void ipmi_get_sdr(struct ipmi_ctx *ctx_cmd)
{
    struct get_sdr_entry_req *req = (struct get_sdr_entry_req *)(&ctx_cmd->req.data[0]);
    struct get_sdr_entry_rsp *rsp = (struct get_sdr_entry_rsp *)(&ctx_cmd->rsp.data[0]);
    SDR_RECORD_FULL search_sdr;
    uint16_t record_id, record_offset, record_size, rev_id;
    uint16_t i, record_index;

    rev_id = B16_L2H(req->reservation_id);
    if (rev_id != 0 && g_sdr_reservation_id != rev_id) {
        ipmi_cmd_err(ctx_cmd, IPMI_CC_RES_CANCELED);
        return;
    }

    record_id = B16_L2H(req->sdr_record_id);

    if (req->offset_into_record > IPMI_MAX_SDR_BYTES) {
        ipmi_cmd_err(ctx_cmd, IPMI_CC_REQ_DATA_INV_LENGTH);
        return;
    }

    if (record_id == 0xffff) {
        // get the last sel entry
        record_offset = (IPMI_SDR_INDEX_DEC(g_ipmi_sdr_header.last_entry_index) * IPMI_MAX_SDR_BYTES);
    } else {
        // get the first or any other sel entry
        record_offset = (g_ipmi_sdr_header.first_entry_index * IPMI_MAX_SDR_BYTES);
    }

    // search sel record in eeprom
    for (i = 0; i < g_ipmi_sdr_header.number_of_entries; i++) {

        record_index = ipmi_get_valid_sdr_record(record_offset, IPMI_MAX_SDR_ENTRIES, 0, &search_sdr);
        if (record_index == 0xffff) {
            ipmi_cmd_err(ctx_cmd, IPMI_CC_OUT_OF_SPACE);
            return;
        }

        // next record_offset
        record_offset = (record_offset + IPMI_MAX_SDR_BYTES) % IPMI_MAX_SDR_SPACE;

        // finded the record_id or get the first/last record
        if (record_id == (search_sdr.header.record_id) || record_id == 0x0 || record_id == 0xffff) {

            // copy record to response
            if (req->bytes_to_read + req->offset_into_record > IPMI_MAX_SDR_BYTES) {
                record_size = IPMI_MAX_SDR_BYTES - req->offset_into_record;
                memcpy(rsp->record_data, (char*)(&search_sdr) + req->offset_into_record, record_size);
            } else {
                record_size = req->bytes_to_read;
                memcpy(rsp->record_data, (char*)(&search_sdr) + req->offset_into_record, record_size);
            }

            // get the next record_id
            if (i == g_ipmi_sdr_header.number_of_entries - 1 || record_id == 0xffff) {
                rsp->next_sdr_record_id = 0xffff;
            } else {
                record_index = ipmi_get_valid_sdr_record(record_offset, IPMI_MAX_SDR_ENTRIES-1, 0, &search_sdr);
                if (record_index != 0xffff) {
                    rsp->next_sdr_record_id = B16_H2L(search_sdr.header.record_id);
                } else {
                    rsp->next_sdr_record_id = 0xffff;
                }
            }

            ipmi_cmd_ok(ctx_cmd, record_size + 2);
            return;
        }
    }
    ipmi_cmd_err(ctx_cmd, IPMI_CC_REQ_DATA_NOT_PRESENT);
}

void ipmi_add_sdr(struct ipmi_ctx *ctx_cmd)
{
    uint32_t error;
    uint16_t record_offset;
    SDR_RECORD_FULL new_sdr;
    SDR_RECORD_FULL *req = (SDR_RECORD_FULL *)(&ctx_cmd->req.data[0]);
    struct add_sdr_entry_rsp *rsp = (struct add_sdr_entry_rsp *)(&ctx_cmd->rsp.data[0]);

    // save the sel record
    memcpy(&new_sdr, req, IPMI_MAX_SDR_BYTES);

    // auto set the sel record id
    new_sdr.header.record_id = ++g_ipmi_sdr_header.count_of_entries;

    // write the sel record to eeprom
    record_offset = IPMI_SDR_STORAGE_OFFSET + (g_ipmi_sdr_header.last_entry_index * IPMI_MAX_SDR_BYTES);
    error = at24xx_write(record_offset, (uint8_t*)&new_sdr, sizeof(SDR_RECORD_FULL));
    if (error) {
        ipmi_cmd_err(ctx_cmd, IPMI_CC_REQ_DATA_NOT_PRESENT);
        return;
    }

    // add the first/last index and number of entries
    // overflow support
    g_ipmi_sdr_header.last_entry_index = IPMI_SDR_INDEX_INC(g_ipmi_sdr_header.last_entry_index);
    if (g_ipmi_sdr_header.last_entry_index == g_ipmi_sdr_header.first_entry_index) {
        g_ipmi_sdr_header.first_entry_index = IPMI_SDR_INDEX_INC(g_ipmi_sdr_header.first_entry_index);
    } else {
        g_ipmi_sdr_header.number_of_entries++;
        g_ipmi_sdr_header.free_space -= IPMI_MAX_SDR_BYTES;
        g_ipmi_sdr_header.alloc_space += IPMI_MAX_SDR_BYTES;
    }

    // change the addition timestamp
    // TODO: change OSTimeGet to RTC time
    g_ipmi_sdr_header.most_recent_addition_timestamp = B32_H2L(g_sel_sdr_time);

    // write the sel header to eeprom
    error = at24xx_write(IPMI_SDR_HEADER_OFFSET, (uint8_t*)&g_ipmi_sdr_header, sizeof(sdr_repository_header));
    if (error) {
        ipmi_cmd_err(ctx_cmd, IPMI_CC_REQ_DATA_NOT_PRESENT);
        return;
    }

    // return added record sel id
    rsp->added_record_id = B16_H2L(new_sdr.header.record_id);

    ipmi_cmd_ok(ctx_cmd, 2);
}

void ipmi_partial_add_sdr(struct ipmi_ctx *ctx_cmd)
{
    uint32_t error;
    uint16_t record_offset, rev_id;
    SDR_RECORD_FULL new_sdr;
    struct partial_add_sdr_req *req = (struct partial_add_sdr_req *)(&ctx_cmd->req.data[0]);
    struct partial_add_sdr_rsp *rsp = (struct partial_add_sdr_rsp *)(&ctx_cmd->rsp.data[0]);

    rev_id = B16_L2H(req->reservation_id);
    if (rev_id != 0 && g_sdr_reservation_id != rev_id) {
        ipmi_cmd_err(ctx_cmd, IPMI_CC_RES_CANCELED);
        return;
    }

    // save the sel record
    memcpy(((char*)&new_sdr) + req->offset_into_record, req->record_data, ctx_cmd->req_len - 6);

    // auto set the sel record id
    new_sdr.header.record_id = ++g_ipmi_sdr_header.count_of_entries;

    // write the sel record to eeprom
    record_offset = IPMI_SDR_STORAGE_OFFSET + (g_ipmi_sdr_header.last_entry_index * IPMI_MAX_SDR_BYTES);
    error = at24xx_write(record_offset, (uint8_t*)&new_sdr, sizeof(SDR_RECORD_FULL));
    if (error) {
        ipmi_cmd_err(ctx_cmd, IPMI_CC_REQ_DATA_NOT_PRESENT);
        return;
    }

    // add the first/last index and number of entries
    // overflow support
    g_ipmi_sdr_header.last_entry_index = IPMI_SDR_INDEX_INC(g_ipmi_sdr_header.last_entry_index);
    if (g_ipmi_sdr_header.last_entry_index == g_ipmi_sdr_header.first_entry_index) {
        g_ipmi_sdr_header.first_entry_index = IPMI_SDR_INDEX_INC(g_ipmi_sdr_header.first_entry_index);
    } else {
        g_ipmi_sdr_header.number_of_entries++;
        g_ipmi_sdr_header.free_space -= IPMI_MAX_SDR_BYTES;
        g_ipmi_sdr_header.alloc_space += IPMI_MAX_SDR_BYTES;
    }

    // change the addition timestamp
    // TODO: change OSTimeGet to RTC time
    g_ipmi_sdr_header.most_recent_addition_timestamp = B32_H2L(g_sel_sdr_time);

    // write the sel header to eeprom
    error = at24xx_write(IPMI_SDR_HEADER_OFFSET, (uint8_t*)&g_ipmi_sdr_header, sizeof(sdr_repository_header));
    if (error) {
        ipmi_cmd_err(ctx_cmd, IPMI_CC_REQ_DATA_NOT_PRESENT);
        return;
    }

    // return added record sel id
    rsp->added_record_id = B16_H2L(new_sdr.header.record_id);

    ipmi_cmd_ok(ctx_cmd, 2);
}

void ipmi_delete_sdr(struct ipmi_ctx *ctx_cmd)
{
    struct del_sdr_entry_req *req = (struct del_sdr_entry_req *)(&ctx_cmd->req.data[0]);
    struct del_sdr_entry_rsp *rsp = (struct del_sdr_entry_rsp *)(&ctx_cmd->rsp.data[0]);
    SDR_RECORD_FULL search_sdr;
    uint16_t record_index, record_id, rev_id;
    uint16_t record_offset = 0;
    uint32_t error;

    rev_id = B16_L2H(req->reservation_id);
    if (rev_id != 0 && g_sdr_reservation_id != rev_id) {
        ipmi_cmd_err(ctx_cmd, IPMI_CC_RES_CANCELED);
        return;
    }

    record_id = B16_L2H(req->sdr_record_id);

    if (record_id == 0xffff) {

        // del the last sel entry
        record_offset = (IPMI_SDR_INDEX_DEC(g_ipmi_sdr_header.last_entry_index) * IPMI_MAX_SDR_BYTES);

    } else if (record_id == 0x0) {

        // del the first or any other sel entry
        record_offset = (g_ipmi_sdr_header.first_entry_index * IPMI_MAX_SDR_BYTES);

    } else {

        // search sel record in eeprom
        record_offset = (g_ipmi_sdr_header.first_entry_index * IPMI_MAX_SDR_BYTES);

        record_index = ipmi_get_valid_sdr_record(record_offset, IPMI_MAX_SDR_ENTRIES, record_id, &search_sdr);

        // can not find the record entry
        if (record_index == 0xffff) {
            ipmi_cmd_err(ctx_cmd, IPMI_CC_REQ_DATA_NOT_PRESENT);
            return;
        } else {
            record_offset = (record_index * IPMI_MAX_SDR_BYTES);
        }
    }

    if (record_offset == 0) {

        // record not found
        ipmi_cmd_err(ctx_cmd, IPMI_CC_REQ_DATA_NOT_PRESENT);
        return;

    } else {

        // get the record id
        record_index = ipmi_get_valid_sdr_record(record_offset, IPMI_MAX_SDR_ENTRIES, 0, &search_sdr);
        if (record_index == 0xffff) {
            ipmi_cmd_err(ctx_cmd, IPMI_CC_OUT_OF_SPACE);
            return;
        }

        // founded the record and clear it!
        at24xx_clear(record_offset, IPMI_MAX_SDR_ENTRIES);

        // update the sel record header
        g_ipmi_sdr_header.count_of_entries--;
        g_ipmi_sdr_header.free_space += IPMI_MAX_SDR_BYTES;
        g_ipmi_sdr_header.alloc_space -= IPMI_MAX_SDR_BYTES;

        if (record_id == 0x0) {
            g_ipmi_sdr_header.first_entry_index = IPMI_SDR_INDEX_INC(g_ipmi_sdr_header.first_entry_index);
        }
        if (record_id == 0xffff) {
            g_ipmi_sdr_header.last_entry_index = IPMI_SDR_INDEX_DEC(g_ipmi_sdr_header.last_entry_index);
        }
        g_ipmi_sdr_header.most_recent_erase_timestamp = g_sel_sdr_time;

        // write the sel header to eeprom
        error = at24xx_write(IPMI_SDR_HEADER_OFFSET, (uint8_t*)&g_ipmi_sdr_header, sizeof(sdr_repository_header));
        if (error) {
            ipmi_cmd_err(ctx_cmd, IPMI_CC_RES_CANCELED);
            return;
        }

        // return the deleted record id
        rsp->deleted_record_id = B16_H2L(search_sdr.header.record_id);

        ipmi_cmd_ok(ctx_cmd, 2);
    }
}

void ipmi_clear_sdr_repository(struct ipmi_ctx *ctx_cmd)
{
    struct clear_sdr_req *req = (struct clear_sdr_req *)(&ctx_cmd->req.data[0]);
    struct clear_sdr_rsp *rsp = (struct clear_sdr_rsp *)(&ctx_cmd->rsp.data[0]);
    uint32_t error;
    uint16_t status, rev_id;

    rev_id = (req->reservation_id_msb << 8 | req->reservation_id_lsb);
    if (rev_id != 0 && g_sdr_reservation_id != rev_id) {
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
        error = at24xx_clear(IPMI_SDR_HEADER_OFFSET, sizeof(sdr_repository_header));
        if (error) {
            ipmi_storage_status_clr(IPMI_STORAGE_ERASE_PROCESS);
            ipmi_cmd_err(ctx_cmd, IPMI_CC_REQ_DATA_NOT_PRESENT);
            return;
        }

        // clear ipmi sel record storage
        error = at24xx_clear(IPMI_SDR_STORAGE_OFFSET, IPMI_MAX_SDR_SPACE);
        if (error) {
            ipmi_storage_status_clr(IPMI_STORAGE_ERASE_PROCESS);
            ipmi_cmd_err(ctx_cmd, IPMI_CC_REQ_DATA_NOT_PRESENT);
            return;
        }
        ipmi_storage_status_clr(IPMI_STORAGE_ERASE_PROCESS);

        // reinit ipmi sel header
        ipmi_sdr_init();

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

void ipmi_get_sdr_repository_time(struct ipmi_ctx *ctx_cmd)
{
    struct sdr_time_st *rsp = (struct sdr_time_st *)(&ctx_cmd->rsp.data[0]);

    rsp->timestamp = B32_H2L(g_sel_sdr_time);

    ipmi_cmd_ok(ctx_cmd, sizeof(struct sel_time_st));
}

void ipmi_set_sdr_repository_time(struct ipmi_ctx *ctx_cmd)
{
    struct sdr_time_st *req = (struct sdr_time_st *)(&ctx_cmd->req.data[0]);

    g_sel_sdr_time = B32_L2H(req->timestamp);

    ipmi_cmd_ok(ctx_cmd, 0);
}

void ipmi_enter_sdr_repository_update_mode(struct ipmi_ctx *ctx_cmd)
{
    ipmi_cmd_err(ctx_cmd, IPMI_CC_INV_CMD);
}

void ipmi_exit_sdr_repository_update_mode(struct ipmi_ctx *ctx_cmd)
{
    ipmi_cmd_err(ctx_cmd, IPMI_CC_INV_CMD);
}

void ipmi_run_initialization_agent(struct ipmi_ctx *ctx_cmd)
{
    ipmi_cmd_err(ctx_cmd, IPMI_CC_INV_CMD);
}


void ipmi_sdr_init(void)
{
    uint32_t error;

    /* if eeprom is error, or storage is empty
     * initial the sdr header to full empty, and set free space to all space
     */
    error = at24xx_read(IPMI_SDR_HEADER_OFFSET, (uint8_t*)&g_ipmi_sdr_header, sizeof(sdr_repository_header));

    /* if read error or empty eeprom, clear all the sdr storage space */
    if (error || g_ipmi_sdr_header.count_of_entries == 0xffff) {
        g_ipmi_sdr_header.number_of_entries = 0;
        g_ipmi_sdr_header.count_of_entries = 0;
        g_ipmi_sdr_header.free_space = 0;
        g_ipmi_sdr_header.alloc_space = 0;
        g_ipmi_sdr_header.first_entry_index = 0;
        g_ipmi_sdr_header.last_entry_index = 0;
        g_ipmi_sdr_header.most_recent_addition_timestamp = 0;
        g_ipmi_sdr_header.most_recent_erase_timestamp = 0;
    }

    /* rebuild the free space */
    if (g_ipmi_sdr_header.count_of_entries == 0) {
        g_ipmi_sdr_header.free_space = IPMI_MAX_SDR_SPACE;
    }

    // write the sel header to eeprom
    error = at24xx_write(IPMI_SDR_HEADER_OFFSET, (uint8_t*)&g_ipmi_sdr_header, sizeof(sdr_repository_header));
    if (error) {
        return;
    }
}


