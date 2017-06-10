/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __SCE40_REG_H__
#define __SCE40_REG_H__

#define ISP_SCE40_OFF 0x00000694
#define ISP_SCE40_LEN 34

typedef struct ISP_Skin_Enhan_Coordinates {
  /* ISP_SKIN_ENHAN_Cx_COORD_0  */
  int32_t      vertex00                 : 8;
  int32_t      vertex01                 : 8;
  int32_t      vertex02                 : 8;
  int32_t     /* reserved */            : 8;
  /* ISP_SKIN_ENHAN_Cx_COORD_1  */
  int32_t      vertex10                 : 8;
  int32_t      vertex11                 : 8;
  int32_t      vertex12                 : 8;
  int32_t     /* reserved */            : 8;
  /* ISP_SKIN_ENHAN_Cx_COORD_2  */
  int32_t      vertex20                 : 8;
  int32_t      vertex21                 : 8;
  int32_t      vertex22                 : 8;
  int32_t     /* reserved */            : 8;
  /* ISP_SKIN_ENHAN_Cx_COORD_3  */
  int32_t      vertex30                 : 8;
  int32_t      vertex31                 : 8;
  int32_t      vertex32                 : 8;
  int32_t     /* reserved */            : 8;
  /* ISP_SKIN_ENHAN_Cx_COORD_4  */
  int32_t      vertex40                 : 8;
  int32_t      vertex41                 : 8;
  int32_t      vertex42                 : 8;
  int32_t     /* reserved */            : 8;
}__attribute__((packed, aligned(4))) ISP_Skin_Enhan_Coordinates;

typedef struct ISP_Skin_Enhan_Coeff {
  /* coef_reg_0 */
  int32_t      coef00                    :  12;
  int32_t     /* reserved */             :   4;
  int32_t      coef01                    :  12;
  int32_t     /* reserved */             :   4;
  /* coef_reg_1 */
  int32_t      coef10                    :  12;
  int32_t     /* reserved */             :   4;
  int32_t      coef11                    :  12;
  int32_t     /* reserved */             :   4;
  /* coef_reg_2 */
  int32_t      coef20                    :  12;
  int32_t     /* reserved */             :   4;
  int32_t      coef21                    :  12;
  int32_t     /* reserved */             :   4;
  /* coef_reg_3 */
  int32_t      coef30                    :  12;
  int32_t     /* reserved */             :   4;
  int32_t      coef31                    :  12;
  int32_t     /* reserved */             :   4;
  /* coef_reg_4 */
  int32_t      coef40                    :  12;
  int32_t     /* reserved */             :   4;
  int32_t      coef41                    :  12;
  int32_t     /* reserved */             :   4;
  /* coef_reg_5 */
  int32_t      coef50                    :  12;
  int32_t     /* reserved */             :   4;
  int32_t      coef51                    :  12;
  int32_t     /* reserved */             :   4;
}__attribute__((packed, aligned(4))) ISP_Skin_Enhan_Coeff;

typedef struct ISP_Skin_Enhan_Offset {
  /*  offset 0 */
  int32_t      offset0                     :  17;
  int32_t     /* reserved */               :   3;
  uint32_t      shift0                     :   4;
  int32_t     /* reserved */               :   8;
  /*  offset 1 */
  int32_t      offset1                     :  17;
  int32_t     /* reserved */               :   3;
  uint32_t      shift1                     :   4;
  int32_t     /* reserved */               :   8;
  /*  offset 2 */
  int32_t      offset2                     :  17;
  int32_t     /* reserved */               :   3;
  uint32_t      shift2                     :   4;
  int32_t     /* reserved */               :   8;
  /*  offset 3 */
  int32_t      offset3                     :  17;
  int32_t     /* reserved */               :   3;
  uint32_t      shift3                     :   4;
  int32_t     /* reserved */               :   8;
  /*  offset 4 */
  int32_t      offset4                     :  17;
  int32_t     /* reserved */               :   3;
  uint32_t      shift4                     :   4;
  int32_t     /* reserved */               :   8;
  /*  offset 5 */
  int32_t      offset5                     :  17;
  int32_t     /* reserved */               :   3;
  uint32_t      shift5                     :   4;
  int32_t     /* reserved */               :   8;
}__attribute__((packed, aligned(4))) ISP_Skin_Enhan_Offset;

#endif //__SCE40_REG_H__
