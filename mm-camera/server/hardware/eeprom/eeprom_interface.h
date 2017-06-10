/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __EEPROM_INTERFACE_H__
#define __EEPROM_INTERFACE_H__
#include "tgtcommon.h"
#include "af_tuning.h"
/* Actuator Status codes */
#define EEPROM_SUCCESS                (0)
#define EEPROM_HANDLE_INVALID         (1)
#define EEPROM_SET_INVALID            (2)
#define EEPROM_GET_INVALID            (3)

typedef int32_t eeprom_status_t;

typedef enum {
  EEPROM_GET_SUPPORT,
  EEPROM_GET_CALIB_2D_DPC,
  EEPROM_GET_RAW_DATA,
} eeprom_get_type_t;

typedef enum {
  EEPROM_SET_CHROMATIX,
  EEPROM_SET_FOCUSPTR,
  EEPROM_SET_DOCALIB,
} eeprom_set_type_t;

typedef struct {
  pixels_array_t dpc_info;
  uint8_t is_eeprom_supported;
  struct msm_calib_raw *raw_data;
} eeprom_get_data_t;

typedef struct {
  eeprom_get_type_t type;
  eeprom_get_data_t data;
} eeprom_get_t;

typedef struct {
  chromatix_parms_type *chromatixPtr;
  af_tune_parms_t *aftune_ptr;
} eeprom_info_t;


typedef union {
  eeprom_info_t info;
} eeprom_set_data_t;

typedef struct {
  eeprom_set_type_t type;
  eeprom_set_data_t data;
} eeprom_set_t;

/********************************
     EEPROM Interface APIs
*********************************/
int eeprom_comp_create();
uint32_t eeprom_client_open(module_ops_t *ops);
int eeprom_comp_destroy();

#endif /* __EEPROM_INTERFACE_H__ */
