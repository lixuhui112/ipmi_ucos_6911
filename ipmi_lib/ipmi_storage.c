//*****************************************************************************
//
// ipmi_storage.c - IPMI Command for Storage
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#include "ipmi_lib/ipmi.h"
#include "app/lib_common.h"

/******************************************************************************
* IPMI EEPROM STORAGE DESIGN
* used chip at24c64 page size 32b, max size 8k
*
*       at24c64
* +-+-+-+-+-+---+---+---+  0
* | CPU reverse space   |
* +-+-+-+-+-+---+---+---+  2k
* | ARM reverse space   |
* +-+-+-+-+-+---+---+---+  4k
* | IPMI SDR space      |  (record size 64 byte, max record 64)
* +-+-+-+-+-+---+---+---+  6k
* | IPMI SEL space      |  (record size 16 byte, max record 128)
* +-+-+-+-+-+---+---+---+  8k
*
*
******************************************************************************/

uint16_t g_sel_sdr_status = 0;

void ipmi_storage_status_set(uint16_t status)
{
    g_sel_sdr_status |= status;
}

void ipmi_storage_status_clr(uint16_t status)
{
    g_sel_sdr_status &= ~status;
}

void ipmi_storage_status_get(uint16_t *status)
{
    *status = g_sel_sdr_status;
}

int ipmi_cmd_storage(struct ipmi_ctx *ctx_cmd)
{
    DEBUG("ipmi_cmd_storage\r\n");

    switch (ctx_cmd->req.msg.cmd)
    {
        /* FRU Device Commands **********************************************/
        case GET_FRU_INVENTORY_AREA_INFO:                   /* 0x10 */
        case READ_FRU_DATA:                                 /* 0x11 */
        case WRITE_FRU_DATA:                                /* 0x12 */
            ipmi_cmd_invalid(ctx_cmd);
            break;

        /* SDR Device Commands **********************************************/
        case GET_SDR_REPOS_INFO:                            /* 0x20 */
            ipmi_get_sdr_repository_info(ctx_cmd);
            break;

        case GET_SDR_REPOS_ALLOC_INFO:                      /* 0x21 */
            ipmi_get_sdr_repository_allocation_info(ctx_cmd);
            break;

        case RESERVE_SDR_REPOSITORY:                        /* 0x22 */
            ipmi_reserve_sdr_repository(ctx_cmd);
            break;

        case GET_SDR:                                       /* 0x23 */
            ipmi_get_sdr(ctx_cmd);
            break;

        case ADD_SDR:                                       /* 0x24 */
            ipmi_add_sdr(ctx_cmd);
            break;

        case PARTIAL_ADD_SDR:                               /* 0x25 */
            ipmi_partial_add_sdr(ctx_cmd);
            break;

        case DELETE_SDR:                                    /* 0x26 */
            ipmi_delete_sdr(ctx_cmd);
            break;

        case CLEAR_SDR_REPOSITORY:                          /* 0x27 */
            ipmi_clear_sdr_repository(ctx_cmd);
            break;

        case GET_SDR_REPOSITORY_TIME:                       /* 0x28 */
            ipmi_get_sdr_repository_time(ctx_cmd);
            break;

        case SET_SDR_REPOSITORY_TIME:                       /* 0x29 */
            ipmi_set_sdr_repository_time(ctx_cmd);
            break;

        case ENTER_SDR_REPOS_UPDATE_MODE:                   /* 0x2a */
            ipmi_enter_sdr_repository_update_mode(ctx_cmd);
            break;

        case EXIT_SDR_REPOS_UPDATE_MODE:                    /* 0x2b */
            ipmi_exit_sdr_repository_update_mode(ctx_cmd);
            break;

        case RUN_INITIALIZATION_AGENT:                      /* 0x2c */
            ipmi_cmd_invalid(ctx_cmd);
            break;

        /* SEL Device Commands **********************************************/
        case GET_SEL_INFO:                                  /* 0x40 */
            ipmi_get_sel_info(ctx_cmd);
            break;

        case GET_SEL_ALLOCATION_INFO:                       /* 0x41 */
            ipmi_get_sel_allocation_info(ctx_cmd);
            break;

        case RESERVE_SEL:                                   /* 0x42 */
            ipmi_reserve_sel(ctx_cmd);
            break;

        case GET_SEL_ENTRY:                                 /* 0x43 */
            ipmi_get_sel_entry(ctx_cmd);
            break;

        case ADD_SEL_ENTRY:                                 /* 0x44 */
            ipmi_add_sel_entry(ctx_cmd);
            break;

        case DELETE_SEL_ENTRY:                              /* 0x46 */
            ipmi_del_sel_entry(ctx_cmd);
            break;

        case CLEAR_SEL:                                     /* 0x47 */
            ipmi_clear_sel_entry(ctx_cmd);
            break;

        case GET_SEL_TIME:                                  /* 0x48 */
            ipmi_get_sel_time(ctx_cmd);
            break;

        case SET_SEL_TIME:                                  /* 0x49 */
            ipmi_set_sel_time(ctx_cmd);
            break;

        case PARTIAL_ADD_SEL_ENTRY:                         /* 0x45 */
        case GET_AUXILIARY_LOG_STATUS:                      /* 0x5A */
        case SET_AUXILIARY_LOG_STATUS:                      /* 0x5B */
        case GET_SEL_TIME_UTC_OFFSET:                       /* 0x5C */
        case SET_SEL_TIME_UTC_OFFSET:                       /* 0x5D */
        default:
            ipmi_cmd_invalid(ctx_cmd);
            break;
    }

    return 0;
}



