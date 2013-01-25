//*****************************************************************************
//
// ipmi_storage.h - IPMI Command Header File for Storage
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

#define IPMI_STORAGE_ERASE_PROCESS      0x0001

void ipmi_storage_status_set(uint16_t status);
void ipmi_storage_status_clr(uint16_t status);
void ipmi_storage_status_get(uint16_t *status);


int ipmi_cmd_storage(struct ipmi_ctx *ctx_cmd);




