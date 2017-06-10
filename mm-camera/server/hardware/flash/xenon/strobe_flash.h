#ifndef __STROBE_FLASH_H__
#define __STROBE_FLASH_H__

#include <stdint.h>
#include "camera.h"
#include "flash_interface.h"

typedef uint32_t msm_strobe_flash_charge_enable_t; 

typedef struct {
  int camfd;
  flash_strobe_data_t strobe_data;
  flash_strobe_charge_enable_t strobe_flash_charge_en;
} flash_strobe_ctrl_t;

#define STROBE_MAX_CLIENT_NUM 2

typedef struct {
  flash_strobe_ctrl_t strobe_ctrl;
  uint32_t obj_idx_mask;
  uint8_t client_idx;
  uint32_t handle;
  uint8_t my_comp_id;
  mctl_ops_t *ops;
} strobe_client_t;

typedef struct {
  pthread_mutex_t mutex;
  uint32_t strobe_handle_cnt;
  strobe_client_t client[STROBE_MAX_CLIENT_NUM];
} strobe_comp_root_t;

int8_t strobe_flash_device_init(flash_strobe_ctrl_t *, int);
int32_t strobe_flash_device_charge(
  flash_strobe_ctrl_t *,
  msm_strobe_flash_charge_enable_t);
flash_strobe_ready_t get_flash_ready_status(flash_strobe_ctrl_t *);
int8_t strobe_flash_device_release(flash_strobe_ctrl_t *strobe_flash_ctrl);

#endif /* __STROBE_FLASH_H__ */
