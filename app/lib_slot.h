//*****************************************************************************
//
// lib_i2c.h - I2C routines for the IPMI application.
//
//*****************************************************************************

#ifndef __LIB_SLOT_H__
#define __LIB_SLOT_H__

#ifdef __cplusplus
extern "C"
{
#endif

int Slot_Present_Check(void);
int Slot_Num_Init(void);
int Slot_Type_Init(void);
int Slot_Power_Open(void);
int Slot_HotSwap_Open(void);

#ifdef __cplusplus
}
#endif

#endif // __LIB_SLOT_H__


