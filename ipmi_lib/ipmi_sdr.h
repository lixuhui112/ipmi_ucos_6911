//*****************************************************************************
//
// ipmi_sdr.h - IPMI Command Header File for Sensor Data Record Repository
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#ifndef __IPMI_SDR_H__
#define __IPMI_SDR_H__

#include "ipmi_lib/ipmi.h"
#include "app/lib_common.h"
#include <string.h>

#define IPMI_SDR_VERSION            0x51

#define IPMI_SDR_HEADER_OFFSET      0x1000  /* 0k (cpu) 2k (arm) 4k (sdr) 6k (sel) 8k */
#define IPMI_SDR_STORAGE_OFFSET     (IPMI_SDR_HEADER_OFFSET + (sizeof(sel_event_header)))
#define IPMI_MAX_SDR_BYTES          (64)
#define IPMI_HEADER_SDR_BYTES       (sizeof(struct sensor_record_header))
#define IPMI_MAX_SDR_ENTRIES        (64-1)
#define IPMI_MAX_SDR_SPACE          (IPMI_MAX_SDR_BYTES * IPMI_MAX_SDR_ENTRIES)
#define IPMI_SDR_INDEX_INC(X)       ((X + 1) % IPMI_MAX_SDR_ENTRIES)
#define IPMI_SDR_INDEX_DEC(X)       ((X + IPMI_MAX_SDR_ENTRIES - 1) % IPMI_MAX_SDR_ENTRIES)
#define IPMI_SDR_RECORD_INC(X)      ((X + IPMI_MAX_SDR_BYTES) % IPMI_MAX_SDR_SPACE)
#define IPMI_SDR_RECORD_DEC(X)      ((X + IPMI_MAX_SDR_SPACE - IPMI_MAX_SDR_BYTES) % IPMI_MAX_SDR_SPACE)

#pragma pack(1)
typedef struct sdr_repository_header_st {
    uint16_t number_of_entries;                     /* current numbers of sel entries */
    uint16_t count_of_entries;                      /* count of all entries */
    uint16_t free_space;                            /* free space in storage */
    uint16_t alloc_space;                           /* redundancy */
    uint16_t first_entry_index;
    uint16_t last_entry_index;
    uint32_t most_recent_addition_timestamp;
    uint32_t most_recent_erase_timestamp;
    uint32_t rev_0[3];                              /* fill to 32 byte */
    //uint32_t rev_1[8];                              /* fill to 64 byte */
} sdr_repository_header;

#define OVERFLOW_FLAG                                       0x80
#define DELETE_SDR_COMMAND_SUPPORTED                        0x08
#define PARTIAL_ADD_SDR_ENTRY_COMMAND_SUPPORTED             0x04
#define RESERVE_SDR_COMMAND_SUPPORTED                       0x02
#define GET_SDR_ALLOCATION_INFORMATION_COMMAND_SUPPORTED    0x01

struct sdr_info_rsp {
    uint8_t  version;                               /* number of records */
    uint16_t record_count;                          /*  */
    uint16_t free_space;
    uint32_t most_recent_addition_timestamp;
    uint32_t most_recent_erase_timestamp;
    uint8_t  operation_support;
};

struct sdr_allocation_info_rsp {
    uint16_t number_of_possible_allocation_units;
    uint16_t allocation_unit_size_in_bytes;
    uint16_t number_of_free_allocation_units;
    uint16_t largest_free_block_in_allocation_units;
    uint8_t maximum_record_size_in_allocation_units;
};

struct get_sdr_entry_req {
    uint16_t reservation_id;
    uint16_t sdr_record_id;
    uint8_t offset_into_record;
    uint8_t bytes_to_read;
};

struct get_sdr_entry_rsp {
    uint16_t next_sdr_record_id;                /* Next SDR Record ID, LS Byte */
    uint8_t record_data[IPMI_MAX_SDR_BYTES];    /* Record Data, 64 bytes for entire record */
};

struct add_sdr_entry_req {
    uint8_t record_data[IPMI_MAX_SDR_BYTES];
};

struct add_sdr_entry_rsp {
    uint16_t added_record_id;
};

struct partial_add_sdr_req {
    uint16_t reservation_id;
    uint16_t sdr_record_id;
    uint8_t offset_into_record;
#ifdef __LITTLE_ENDIAN__
    uint8_t in_progress:4,                      /* [3:0] - in progress
                                                   0h = partial add in progress.
                                                   1h = last record data being transferred with this request */
            reserved:4;                         /* [7:4] - reserved */
#else
    uint8_t reserved:4,
            erasure_progress:4;
#endif
    uint8_t record_data[IPMI_MAX_SDR_BYTES];    /* Record Data, 64 bytes for entire record */
};

struct partial_add_sdr_rsp {
    uint16_t added_record_id;
};

struct del_sdr_entry_req {
    uint16_t reservation_id;
    uint16_t sdr_record_id;
};

struct del_sdr_entry_rsp {
    uint16_t deleted_record_id;
};

#define INITIATE_ERASE              0xAA
#define GET_ERASURE_STATUS          0x00

struct clear_sdr_req {
    uint8_t reservation_id_lsb;
    uint8_t reservation_id_msb;
    uint8_t char_c;                             /* ¡®C¡¯ (43h) */
    uint8_t char_l;                             /* ¡®L¡¯ (4Ch) */
    uint8_t char_r;                             /* ¡®R¡¯ (52h) */
    uint8_t operation;                          /* AAh = initiate erase.
                                                   00h = get erasure status. */
};

struct clear_sdr_rsp {
#ifdef __LITTLE_ENDIAN__
    uint8_t erasure_progress:4,                 /* [3:0] - erasure progress
                                                   0h = erasure in progress.
                                                   1h = erase completed. */
            reserved:4;                         /* [7:4] - reserved */
#else
    uint8_t reserved:4,
            erasure_progress:4;
#endif
};

struct sdr_time_st {
    uint32_t timestamp;                         /* Time in four-byte format. LS byte first.
                                                   See Section 37, Timestamp Format. */
};
#pragma pack()

void ipmi_get_sdr_repository_info(struct ipmi_ctx *ctx_cmd);
void ipmi_get_sdr_repository_allocation_info(struct ipmi_ctx *ctx_cmd);
void ipmi_reserve_sdr_repository(struct ipmi_ctx *ctx_cmd);
void ipmi_get_sdr(struct ipmi_ctx *ctx_cmd);
void ipmi_add_sdr(struct ipmi_ctx *ctx_cmd);
void ipmi_partial_add_sdr(struct ipmi_ctx *ctx_cmd);
void ipmi_delete_sdr(struct ipmi_ctx *ctx_cmd);
void ipmi_clear_sdr_repository(struct ipmi_ctx *ctx_cmd);
void ipmi_get_sdr_repository_time(struct ipmi_ctx *ctx_cmd);
void ipmi_set_sdr_repository_time(struct ipmi_ctx *ctx_cmd);
void ipmi_enter_sdr_repository_update_mode(struct ipmi_ctx *ctx_cmd);
void ipmi_exit_sdr_repository_update_mode(struct ipmi_ctx *ctx_cmd);
void ipmi_run_initialization_agent(struct ipmi_ctx *ctx_cmd);
void ipmi_sdr_init(void);

#endif

