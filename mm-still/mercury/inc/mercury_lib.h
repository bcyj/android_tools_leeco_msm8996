/* Copyright (c) 2012, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef MERCURY_LIB_H
#define MERCURY_LIB_H
#include <linux/msm_ion.h>


#define MSM_MERCURY_NAME "/dev/mercury0"

#define JPEGERR_SUCCESS              0
#define JPEGERR_EFAILED              1
#define JPEGERR_EMALLOC              2
#define JPEGERR_ENULLPTR             3
#define JPEGERR_EBADPARM             4
#define JPEGERR_EBADSTATE            5
#define JPEGERR_EUNSUPPORTED         6
#define JPEGERR_EUNINITIALIZED       7
#define JPEGERR_TAGABSENT            8
#define JPEGERR_TAGTYPE_UNEXPECTED   9
#define JPEGERR_THUMBNAIL_DROPPED    10
#define JPEGERR_ETIMEDOUT            11


#define JPEGD_MAXHUFFTABLES     8
#define JPEGD_MAXQUANTTABLES    4
#define JPEGD_MAXCOMPONENTS     4
#define JPEGD_MAXCOMPSINSCAN    4

struct mercury_buf {
    uint32_t type;
    int      fd;
    void     *vaddr;
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
};

struct mercury_evt {
    uint32_t type;
    uint32_t len;
    void     *value;
};

typedef enum {
  MERCURY_PMEM_ADSP,
  MERCURY_PMEM_SMIPOOL,
} mercury_pmem_region_type;

typedef void *mcr_obj_t;


typedef enum
{
    MCR_JPEG_H2V2      = 0,
    MCR_JPEG_H2V1      = 1,
    MCR_JPEG_H1V2      = 2,
    MCR_JPEG_H1V1      = 3,
    MCR_JPEG_GRAYSCALE = 4,

    MCR_JPEG_BS_H2V2      = 6,
    MCR_JPEG_BS_H2V1      = 7,
    MCR_JPEG_BS_H1V2      = 8,
    MCR_JPEG_BS_H1V1      = 9,

} mercury_subsampling_t;



typedef struct {
    uint8_t mcu_type;
    uint8_t output_format;
    uint8_t crcb_order;
    mercury_subsampling_t jpegdFormat;
    uint8_t scale_ratio ;
    uint8_t numofplanes  ;              /// defaut to two-plane
    } mercury_cmd_control_cfg_t;

typedef struct {
    uint16_t image_width;
    uint16_t image_height;
    uint8_t *bitstream_buf;
    uint32_t bitstream_length;
} mercury_cmd_readbus_cfg_t;

typedef struct {
    uint16_t image_width ;
    uint16_t image_height ;
    uint8_t sampling_factor;
    uint8_t wr_buf_format;
    uint8_t *y_buf;
    uint8_t *u_buf;
    uint8_t *v_buf;
    uint32_t num_of_planes;
    uint32_t scale_ratio;
} mercury_cmd_writebus_cfg_t;

typedef uint16_t* mercury_quant_table_t;

typedef struct {

    uint8_t                 qtable_present_flag;
    mercury_quant_table_t      p_qtables[4];
} mercury_cmd_quant_cfg_t;

typedef struct {
    uint8_t bits[17];
    uint8_t values[256];
} huff_table_t;

typedef struct {
    uint8_t                 htable_present_flag;
    huff_table_t      p_htables[8];
} mercury_cmd_huff_cfg_t;

typedef struct {
    uint8_t comp_id;
    uint8_t sampling_h;
    uint8_t sampling_v;
    uint8_t qtable_sel;
} mer_comp_info_t;

typedef struct {
    uint32_t                width;
    uint32_t                height;
    uint8_t                 precision;
    uint8_t                 num_comps;
    mer_comp_info_t      *p_comp_infos;
} mercury_cmd_sof_cfg_t;

typedef struct {
    uint8_t comp_id;
    uint8_t dc_selector;
    uint8_t ac_selector;

} mercury_comp_entropy_sel_t;

typedef struct {
    uint32_t                  offset;
    uint8_t                   num_selectors;
    uint8_t                   spec_start;
    uint8_t                   spec_end;
    uint8_t                   succ_approx_h;
    uint8_t                   succ_approx_l;
    mercury_comp_entropy_sel_t  *p_selectors;

} mecury_scan_info_t;

typedef struct {

    mecury_scan_info_t      **pp_scan_infos;
} mercury_cmd_sos_cfg_t;

int mercury_lib_input_buf_cfg(struct mercury_buf *buf);
int mercury_lib_output_buf_get(struct mercury_buf *buf);
int mercury_lib_input_buf_get(struct mercury_buf *buf);
int mercury_lib_output_buf_cfg(struct mercury_buf *buf);
int mercury_lib_jpegd_wr_op_cfg(uint8_t align, uint8_t flip, uint8_t mirror);
int mercury_lib_configure_baseline(mercury_cmd_quant_cfg_t, mercury_cmd_sof_cfg_t, mercury_cmd_huff_cfg_t, mercury_cmd_sos_cfg_t);
int mercury_lib_clear_sequential_op (mcr_obj_t mcr_obj);
int mercury_lib_set_sequential_op (mcr_obj_t mcr_obj);

#endif
