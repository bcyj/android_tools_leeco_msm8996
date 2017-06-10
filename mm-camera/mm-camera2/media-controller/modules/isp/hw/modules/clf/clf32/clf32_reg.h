/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __CLF32_REG_H__
#define __CLF32_REG_H__

#define ISP_CLF32_CFG_OFF 0x000006B0
#define ISP_CLF32_CFG_LEN 18

typedef struct ISP_CLF_Cfg {
  uint32_t       colorconv_enable       :   1;
  uint32_t    /* reserved */            :  15;
  uint32_t       pipe_flush_cnt         :  13;
  uint32_t       pipe_flush_ovd         :   1;
  uint32_t       flush_halt_ovd         :   1;
  uint32_t    /* reserved */            :   1;

}__attribute__((packed, aligned(4))) ISP_CLF_Cfg;

typedef struct ISP_CLF_Luma_Cfg {
  /* CLF Luma cfg 0 */
  uint32_t       cutoff_1               :  12;
  uint32_t    /* reserved */            :   4;
  uint32_t       cutoff_2               :  12;
  uint32_t    /* reserved */            :   4;

  /* CLF Luma cfg 1 */
  uint32_t       cutoff_3               :  12;
  uint32_t    /* reserved */            :  20;

  /* CLF Luma cfg 2 */
  uint32_t       mult_neg               :  12;
  uint32_t    /* reserved */            :   4;
  uint32_t       mult_pos               :  12;
  uint32_t    /* reserved */            :   4;
}__attribute__((packed, aligned(4))) ISP_CLF_Luma_Cfg;

typedef struct ISP_CLF_Luma_Lut {
  /* CLF Luma LUT */
  int32_t        lut0                : 12;
  int32_t     /* reserved */         :  4;
  int32_t        lut1                : 12;
  int32_t     /* reserved */         :  4;
}__attribute__((packed, aligned(4))) ISP_CLF_Luma_Lut;

typedef struct ISP_CLF_Chroma_Coeff {
  /* CLF Chroma coef 0 */
  uint32_t       v_coeff0               :   7;
  uint32_t    /* reserved */            :   1;
  uint32_t       v_coeff1               :   6;
  uint32_t    /* reserved */            :  18;

  /* CLF Chroma coef 1 */
  uint32_t       h_coeff0               :   7;
  uint32_t    /* reserved */            :   1;
  uint32_t       h_coeff1               :   6;
  uint32_t    /* reserved */            :   2;
  uint32_t       h_coeff2               :   6;
  uint32_t    /* reserved */            :   2;
  uint32_t       h_coeff3               :   6;
  uint32_t    /* reserved */            :   2;
} __attribute__((packed, aligned(4))) ISP_CLF_Chroma_Coeff;

#endif //__CLF32_REG_H__
