/*============================================================================

   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __FLASH_INTERFACE_H__
#define __FLASH_INTERFACE_H__

#include "camera.h"
#include "tgtcommon.h"
#include "intf_comm_data.h"

typedef enum {
  FLASH_GET_STATE,
  FLASH_GET_MODE,
//  FLASH_GET_STROBE_STATE,
} flash_get_type_t;
/*
typedef enum {
  FLASH_SET_LED_STATE,
  FLASH_SET_LED_MODE,
} flash_led_set_type_t;

typedef enum {
  FLASH_SET_STROBE_STATE,
  FLASH_SET_STROBE_CHARGE,
} flash_strobe_set_type_t;
*/

typedef enum {
  FLASH_SET_STATE,
  FLASH_SET_MODE,
} flash_set_type_t;

typedef enum {
  STROBE_FLASH_NOT_READY,
  STROBE_FLASH_READY,
} flash_strobe_ready_t;

typedef enum {
  STROBE_FLASH_CHARGE_DISABLE,
  STROBE_FLASH_CHARGE_ENABLE,
} flash_strobe_charge_enable_t;

typedef enum {
  STROBE_FLASH_UNINIT,
  STROBE_FLASH_INIT,
  STROBE_FLASH_FAIL,
} flash_strobe_state_t;

typedef struct {
  uint32_t led_state; /* MSM_CAMERA_LED_OFF, etc */
  led_mode_t led_mode;
} flash_led_data_t;

typedef struct {
  flash_strobe_ready_t strobe_ready;
  flash_strobe_state_t strobe_state;
} flash_strobe_data_t;

typedef struct {
  flash_get_type_t type;
  flash_led_data_t data;
} flash_led_get_t;

typedef struct {
  flash_set_type_t type;
  flash_led_data_t data;
} flash_led_set_t;

typedef struct {
  flash_get_type_t type;
  flash_strobe_data_t data;
} flash_strobe_get_t;

typedef struct {
  flash_set_type_t type;
  flash_strobe_charge_enable_t charge_enable;
} flash_strobe_set_t;

typedef struct {
  uint32_t fd;
/* TODO */
} flash_init_data_t;

/********************************
     LED Interface APIs
*********************************/
int flash_led_comp_create();
uint32_t flash_led_client_open(module_ops_t *ops);
int flash_led_comp_destroy();
#if 0
uint32_t flash_led_interface_create(void);
int flash_led_interface_destroy(uint32_t handle);
int  flash_led_interface_init(uint32_t handle, flash_init_data_t *flash_init_data);
int  flash_led_get_params(uint32_t handle, flash_led_get_t *flash_get);
int  flash_led_set_params(uint32_t handle, flash_led_set_t *flash_set);
#endif
/********************************
     Strobe Interface APIs
*********************************/
int flash_strobe_comp_create();
uint32_t flash_strobe_client_open(module_ops_t *ops);
int flash_strobe_comp_destroy();
#if 0
uint32_t flash_strobe_interface_create(void);
int flash_strobe_interface_destroy(uint32_t handle);
int  flash_strobe_interface_init(uint32_t handle, flash_init_data_t *flash_init_data);
int  flash_strobe_get_params(uint32_t handle, flash_strobe_get_t *flash_get);
int  flash_strobe_set_params(uint32_t handle, flash_strobe_set_t *flash_set);
#endif
#endif /* __FLASH_INTERFACE_H__ */
