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

#define IPMI_SDR_HEADER_OFFSET      0x1000  /* 0k (cpu) 2k (arm) 4k (sdr) 6k (sel) 8k */


void ipmi_get_sdr_repository_info(struct ipmi_ctx *ctx_cmd);

void ipmi_sdr_init(void);

#endif

