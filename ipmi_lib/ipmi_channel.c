//*****************************************************************************
//
// ipmi_channel.c - IPMI Channel
//
// author:  qujianning@kyland.com.cn
// update:  2012/08/03
// version: v1.0
//
//*****************************************************************************

#include "ipmi.h"
#include "driverlib/sysctl.h"
#include "app/lib_common.h"

struct ipmi_channel_table_st {
    uint8_t channel_number;
    uint8_t channel_protocol;
    uint8_t channel_mediumtype;
    uint8_t channel_access_modes;
    uint8_t channel_privilege_levels;
};

struct ipmi_channel_table_st ipmi_channel_table[] =
{
    {
        .channel_number             = IPMI_CH_NUM_PRIMARY_IPMB,
        .channel_protocol           = IPMI_CH_PROTOCOL_IPMB,
        .channel_mediumtype         = IPMI_CH_MEDIUM_IPMB,
        .channel_access_modes       = IPMI_CH_SESSION_LESS,
        .channel_privilege_levels   = 0,
    },
    {
        .channel_number             = IPMI_CH_NUM_CONSOLE,
        .channel_protocol           = IPMI_CH_PROTOCOL_IPMB,
        .channel_mediumtype         = IPMI_CH_MEDIUM_SERIAL,
        .channel_access_modes       = IPMI_CH_SESSION_LESS,
        .channel_privilege_levels   = 0,
    },
    {
        .channel_number             = IPMI_CH_NUM_ICMB,
        .channel_protocol           = IPMI_CH_PROTOCOL_IPMB,
        .channel_mediumtype         = IPMI_CH_MEDIUM_SERIAL,
        .channel_access_modes       = IPMI_CH_SESSION_LESS,
        .channel_privilege_levels   = 0,
    },
    {
        .channel_number             = IPMI_CH_NUM_LAN,
        .channel_protocol           = IPMI_CH_PROTOCOL_IPMB,
        .channel_mediumtype         = IPMI_CH_MEDIUM_LAN,
        .channel_access_modes       = IPMI_CH_SESSION_MUILTY,
        .channel_privilege_levels   = 0,
    },
    {
        .channel_number             = IPMI_CH_NUM_SYS_INTERFACE,
        .channel_protocol           = IPMI_CH_PROTOCOL_SMIC,
        .channel_mediumtype         = IPMI_CH_MEDIUM_SYS,
        .channel_access_modes       = IPMI_CH_SESSION_LESS,
        .channel_privilege_levels   = 0,
    },
    {
        .channel_number             = IPMI_CH_NUM_MAX,
        .channel_protocol           = 0,
        .channel_mediumtype         = 0,
        .channel_access_modes       = 0,
        .channel_privilege_levels   = 0,
    },

};

uint8_t ipmi_get_channel_medium_type(uint8_t channel_number)
{
    for (uint8_t i = 0; i < 0xf && ipmi_channel_table[i].channel_number != IPMI_CH_NUM_MAX; i++)
    {
        if (channel_number == ipmi_channel_table[i].channel_number)
        {
            return ipmi_channel_table[i].channel_mediumtype;
        }
    }
    return 0;
}

uint8_t ipmi_get_channel_protocol_type(uint8_t channel_number)
{
    for (uint8_t i = 0; i < 0xf && ipmi_channel_table[i].channel_number != IPMI_CH_NUM_MAX; i++)
    {
        if (channel_number == ipmi_channel_table[i].channel_number)
        {
            return ipmi_channel_table[i].channel_protocol;
        }
    }
    return 0;
}

uint8_t ipmi_get_channel_session_support(uint8_t channel_number)
{
    for (uint8_t i = 0; i < 0xf && ipmi_channel_table[i].channel_number != IPMI_CH_NUM_MAX; i++)
    {
        if (channel_number == ipmi_channel_table[i].channel_number)
        {
            return ipmi_channel_table[i].channel_access_modes;
        }
    }
    return 0;
}


