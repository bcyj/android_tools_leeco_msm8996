#ifndef __LED_H__
#define __LED_H__

#include <stdint.h>
#include "flash_interface.h"

#define LED_NUM_SKIP_FRAMES 3

typedef uint32_t msm_camera_led_state_t;

typedef struct {
  int camfd;
  flash_led_data_t led_data;
} flash_led_ctrl_t;

#define LED_MAX_CLIENT_NUM 2

typedef struct {
  flash_led_ctrl_t led_ctrl;
  uint32_t obj_idx_mask;
  uint8_t client_idx;
  uint32_t handle;
  uint8_t my_comp_id;
  mctl_ops_t *ops;
} led_client_t;

typedef struct {
  pthread_mutex_t mutex;
  uint32_t led_handle_cnt;
  led_client_t client[LED_MAX_CLIENT_NUM];
} led_comp_root_t;

void led_ctrl_init(flash_led_ctrl_t *led_ctrl, int camfd);
void led_ctrl_release(void);

uint32_t get_led_state();
int8_t set_led_state(msm_camera_led_state_t new_led_state);
void update_led_state(msm_camera_led_state_t LED_state);

led_mode_t get_led_mode();
int8_t set_led_mode(led_mode_t led_mode);
#endif /* __LED_H__ */
