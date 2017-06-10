/*============================================================================
   Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#ifndef mm_vpe_reg_pack_h
#define mm_vpe_reg_pack_h

#include <stdint.h>

/*--------------------------------------------*/
/* This part is the register definition.      */
/*--------------------------------------------*/
typedef struct mm_vpe_hw_version_packed {
  uint32_t        revision               : 8;
  uint32_t        release_phase          : 8;
  uint32_t        minor_version          : 8;
  uint32_t        major_version          : 8;
}__attribute__((packed, aligned(4))) mm_vpe_hw_version_packed;

/* this is the dimension of ROI.  width / height. */
typedef struct mm_vpe_src_size_packed {
  uint32_t        src_w                  : 13;
  uint32_t        /* reserved */         :  3;
  uint32_t        src_h                  : 13;
  uint32_t        /* reserved */         :  3;
}__attribute__((packed, aligned(4))) mm_vpe_src_size_packed;

typedef struct mm_vpe_src_ystride1_packed {
  uint32_t        srcp0_ystride          :  15;
  uint32_t        /* reserved */         :   1;
  uint32_t        srcp1_ystride          :  15;
  uint32_t        /* reserved */         :   1;
}__attribute__((packed, aligned(4))) mm_vpe_src_ystride1_packed;

typedef struct mm_vpe_src_format_packed {
  uint32_t    srcc0_bits                  :   2;
  uint32_t    srcc1_bits                  :   2;
  uint32_t    srcc2_bits                  :   2;
  uint32_t    srcc3_bits                  :   2;
  uint32_t    srcc3_en                    :   1;
  uint32_t    src_bpp                     :   2;
  uint32_t    /* reserved */              :   2;
  uint32_t    unpack_count                :   4;
  uint32_t    unpack_tight                :   1;
  uint32_t    unpack_align                :   1;
  uint32_t    fetch_planes                :   2;
  uint32_t    wmv9_mode                   :   1;
  uint32_t    /* reserved */              :  10;
}__attribute__((packed, aligned(4))) mm_vpe_src_format_packed;

typedef struct mm_vpe_op_mode_packed {
  uint32_t        scalex_en               :   1;
  uint32_t        scaley_en               :   1;
  uint32_t        src_data_format         :   1;
  uint32_t        convert_matrix_en       :   1;
  uint32_t        convert_matrix_sel      :   1;
  uint32_t        lut_c0_en               :   1;
  uint32_t        lut_c1_en               :   1;
  uint32_t        lut_c2_en               :   1;
  uint32_t        rot_en                  :   1;
  uint32_t        rot_mode                :   3;
  uint32_t        blend_en                :   1;
  uint32_t        blend_alpha_sel         :   2;
  uint32_t        blend_eq_sel            :   1;
  uint32_t        dither_en               :   1;
  uint32_t        /* reserved */          :   1;
  uint32_t        src_chroma_samp         :   2;
  uint32_t        src_chroma_site         :   1;
  uint32_t        dst_chroma_samp         :   2;
  uint32_t        dst_chroma_site         :   1;
  uint32_t        blend_transp_en         :   1;
  uint32_t        bg_chroma_samp          :   2;
  uint32_t        bg_chroma_site          :   1;
  uint32_t        deint_en                :   1;
  uint32_t        deint_odd_ref           :   1;
  uint32_t        dst_data_format         :   1;
  uint32_t        /* reserved */          :   1;
}__attribute__((packed, aligned(4))) mm_vpe_op_mode_packed;

typedef struct mm_vpe_out_format_packed {
  uint32_t        dstc0_bits          :   2;
  uint32_t        dstc1_bits          :   2;
  uint32_t        dstc2_bits          :   2;
  uint32_t        dstc3_bits          :   2;
  uint32_t        dstc3_en            :   1;
  uint32_t        pack_count          :   4;
  uint32_t        pack_tight          :   1;
  uint32_t        pack_align          :   1;
  uint32_t        reserved_bit15      :   1;
  uint32_t        dst_bpp             :   2;
  uint32_t        write_planes        :   2;
  uint32_t        comp_en             :   1;
  uint32_t        comp_mode           :   2;
  uint32_t        /* reserved */      :   9;
}__attribute__((packed, aligned(4))) mm_vpe_out_format_packed;

typedef struct mm_vpe_out_size_packed {
  uint32_t        dst_w             : 12;
  uint32_t        /* reserved */    : 4;
  uint32_t        dst_h             : 12;
  uint32_t        /* reserved */    : 4;
}__attribute__((packed, aligned(4))) mm_vpe_out_size_packed;

typedef struct mm_vpe_out_ystride1_packed {
  uint32_t        dstp0_ystride        : 14;
  uint32_t        /* reserved */       : 2;
  uint32_t        dstp1_ystride        : 14;
  uint32_t        /* reserved */       : 2;
}__attribute__((packed, aligned(4))) mm_vpe_out_ystride1_packed;

typedef struct mm_vpe_out_xy_packed {
  uint32_t        dst_x                : 12;
  uint32_t        /* reserved */       : 4;
  uint32_t        dst_y                : 12;
  uint32_t        /* reserved */       : 4;
}__attribute__((packed, aligned(4))) mm_vpe_out_xy_packed;

/* this is for the offset (x,y) of ROI */
typedef struct mm_vpe_src_xy_packed {
  uint32_t        src_x              : 13;
  uint32_t        /* reserved */     : 3;
  uint32_t        src_y              : 13;
  uint32_t        /* reserved */     : 3;
}__attribute__((packed, aligned(4))) mm_vpe_src_xy_packed;

/* this is the actual buffer size.  Naming here is a little
   confusing.  */
typedef struct mm_vpe_src_image_size_packed {
  uint32_t        src_img_w           : 13;
  uint32_t        /* reserved */      : 3;
  uint32_t        src_img_h           : 13;
  uint32_t        /* reserved */      : 3;
}__attribute__((packed, aligned(4))) mm_vpe_src_image_size_packed;

typedef struct mm_vpe_scale_config_packed {
  uint32_t        scalex_unit_sel         : 1;
  uint32_t        scaley_unit_sel         : 1;
  uint32_t        x_table                 : 2;
  uint32_t        y_table                 : 2;
  uint32_t        svi_en                  : 1;
  uint32_t        sharpen_en              : 1;
  uint32_t        pix_repeat_mode         : 1;
  uint32_t        /* reserved */          : 23;
}__attribute__((packed, aligned(4))) mm_vpe_scale_config_packed;

typedef struct mm_vpe_deint_decision_packed {
  uint32_t        thresh                 : 11;
  uint32_t        /* reserved */         : 5;
  uint32_t        attenu                 : 3;
  uint32_t        /* reserved */         : 13;
}__attribute__((packed, aligned(4))) mm_vpe_deint_decision_packed;

/*  there are four coefficients 0-3 */
typedef struct mm_vpe_deint_coeff_packed {
  uint32_t        coeff                  :  10;
  uint32_t        /* reserved */         :  22;
}__attribute__((packed, aligned(4))) mm_vpe_deint_coeff_packed;

/*  there are four coefficients 0-127 for LSP */
typedef struct mm_vpe_scale_coeff_lsp_packed {
  uint32_t        coeff0                  :  10;
  uint32_t        /* reserved */          :   6;
  uint32_t        coeff1                  :  10;
  uint32_t        /* reserved */          :   6;
}__attribute__((packed, aligned(4))) mm_vpe_scale_coeff_lsp_packed;

/*  there are four coefficients 0-127 for MSP */
typedef struct mm_vpe_scale_coeff_msp_packed {
  uint32_t        coeff2                  :  10;
  uint32_t        /* reserved */          :   6;
  uint32_t        coeff3                  :  10;
  uint32_t        /* reserved */          :   6;
}__attribute__((packed, aligned(4))) mm_vpe_scale_coeff_msp_packed;

#endif //vpe_reg_pack_h
