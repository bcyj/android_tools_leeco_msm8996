/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#ifndef JPEGD_LIB_H
#define JPEGD_LIB_H

#include <linux/ion.h>
#include "jpeg_common.h"
#include "jpegd.h"

/*============================================================================
   MACROS
============================================================================*/
#define MSM_JPEGD_NAME "/dev/jpeg2"

#define JPEGDERR_SUCCESS               0
#define JPEGDERR_EFAILED              -1
#define JPEGDERR_EMALLOC              -2
#define JPEGDERR_ENULLPTR             -3
#define JPEGDERR_EBADPARM             -4
#define JPEGDERR_EBADSTATE            -5
#define JPEGDERR_EUNSUPPORTED         -6
#define JPEGDERR_EUNINITIALIZED       -7
#define JPEGDERR_TAGABSENT            -8
#define JPEGDERR_TAGTYPE_UNEXPECTED   -9
#define JPEGDERR_THUMBNAIL_DROPPED    -10
#define JPEGDERR_ETIMEDOUT            -11

#define JPEGD_MAXHUFFTABLES     8
#define JPEGD_MAXQUANTTABLES    4
#define JPEGD_MAXCOMPONENTS     4
#define JPEGD_MAXCOMPSINSCAN    4

/*============================================================================
   DATA STRUCTURES
============================================================================*/
typedef struct {
  uint32_t width;
  uint32_t height;
  uint32_t rotation;
  jpegd_scale_type_t scale_factor;
  jpeg_subsampling_t format;
  uint32_t crcb_order;
  uint8_t  num_planes;
  uint32_t restart_interval;
  uint32_t actual_width;
  uint32_t actual_height;
} jpegd_base_config_t;

typedef struct {
  uint32_t type;
  int fd;
  void *vaddr;
  uint32_t y_off;
  uint32_t y_len;
  uint32_t framedone_len;
  uint32_t cbcr_off;
  uint32_t cbcr_len;
  uint32_t num_of_mcu_rows;
  uint32_t offset;
  int ion_fd_main;
  struct ion_allocation_data alloc_ion;
  struct ion_fd_data fd_ion_map;
  uint32_t pln2_off;
  uint32_t pln2_len;
} jpegd_buf;

typedef enum {
  JPEGD_EVT_FRAMEDONE,
  JPEGD_EVT_ERROR,
  JPEGD_EVT_INPUT_CONSUMED,
  JPEGD_EVT_OUTPUT_CONSUMED,
} jpegd_evt_type_t;

typedef struct {
  jpegd_evt_type_t type;
  union {
    int error_type;
  };
} jpegd_event_t;

typedef enum {
  JPEGD_H2V2      = 0,
  JPEGD_H2V1      = 1,
  JPEGD_H1V2      = 2,
  JPEGD_H1V1      = 3,
  JPEGD_GRAYSCALE = 4,
  JPEGD_FORMAT_MAX = 5
} jpegd_subsampling_t;

typedef struct {
  uint16_t image_width;
  uint16_t image_height;
  uint16_t actual_width;
  uint16_t actual_height;
} jpegd_image_info_t;

typedef uint16_t* jpegd_quant_table_t;

typedef struct {
  uint8_t qtable_present_flag;
  jpegd_quant_table_t p_qtables[4];
} jpegd_cmd_quant_cfg_t;

typedef struct {
  int bits[17];
  int values[256];
} huff_table_t;

typedef struct {
  uint8_t  htable_present_flag;
  huff_table_t  p_htables[8];
} jpegd_cmd_huff_cfg_t;

typedef struct {
  uint8_t comp_id;
  uint8_t dc_selector;
  uint8_t ac_selector;
} jpegd_comp_entropy_sel_t;

typedef struct {
  uint8_t flip;
  uint8_t rotation;
  uint8_t mal_en;
  uint8_t mal_boundary;
  uint8_t plane0_enable;
  uint8_t plane1_enable;
  uint8_t plane2_enable;
  uint8_t read_enable;
  uint8_t swc_fetch_enable;
  uint8_t bottom_vpad_enable;
  uint8_t cbcr_order;
  uint8_t memory_format;
  uint8_t burst_length_max;
  uint8_t byte_ordering;
} jpegd_cmd_fe_cfg;

typedef struct {
  uint8_t flip;
  uint8_t rotation;
  uint8_t pop_buff_on_eos;
  uint8_t mal_enable;
  uint8_t mal_boundary;
  uint8_t pln2_enable;
  uint8_t pln1_enable;
  uint8_t pln0_enable;
  uint8_t cbcr_order;
  uint8_t memory_format;
  uint8_t burst_length_max;
  uint8_t byte_ordering;
  jpegd_subsampling_t jpegdFormat;
} jpegd_cmd_we_cfg;

typedef enum {
  JPEGD_PLN_0,
  JPEGD_PLN_1,
  JPEGD_PLN_2,
  JPEGD_PLN_MAX
} jpegd_planes_t;

/*============================================================================
   FUNCTION DECL
============================================================================*/
typedef int (*jpegd_evt_handler) (void*, jpegd_event_t *);
//
int jpegd_lib_init(void **handle, void *p_user, jpegd_evt_handler evt_handler);
//
int jpegd_lib_release(void *handle);
//
int jpegd_lib_decode(void *handle);
//
int jpegd_lib_input_buf_cfg(void *handle, jpegd_buf *);
//
int jpegd_lib_output_buf_get(void *handle, jpegd_buf *);
//
int jpegd_lib_input_buf_get(void *handle, jpegd_buf *);
//
int jpegd_lib_output_buf_cfg(void *handle, jpegd_buf *);
//
int jpegd_lib_wait_done(void* handle);
//
int jpegd_lib_configure_baseline(
  void *handle,
  jpegd_cmd_quant_cfg_t *,
  jpegd_cmd_huff_cfg_t *,
  jpegd_base_config_t *);

#endif //JPEGD_LIB_H
