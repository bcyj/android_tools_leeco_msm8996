#ifndef _BT_QSOC_NVM_BTS_CLASS2_SUPPORT_H
#define _BT_QSOC_NVM_BTS_CLASS2_SUPPORT_H 

/**
  @file btqsocnvmclass2.h

  This file contains definitions of static tags that are specific to Class II
  configurations.  These tags make up the set of differences between the released
  Class I and Class II NVM tags.
*/

/*--------------------------------------------------------------
     Copyright (c) 2009-2013 Qualcomm Technologies, Inc.
     All rights reserved.
     Qualcomm Technologies Confidential and Proprietary
--------------------------------------------------------------*/
/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order. Please
  use ISO format for dates.

  $Header: //source/qcom/qct/wconnect/bthost/soccfg/main/latest/src/btqsocnvmclass2.h#27 $
  $DateTime: 2011/08/04 00:05:46 $
  $Author: tylerw $

  when        who  what, where, why
  ----------  ---  -----------------------------------------------------------
  2013-07-05   dv  Updated BHA B1 NVM(70.06)
  2013-05-09   dv  Updated BHA B1 NVM(70.05)
  2013-03-01   dv  Updated BHA B0 NVM(70.17)
  2013-03-01   dv  Updated BHA B1 NVM(70.03)
  2012-12-14   dv  Updated BHA B0 NVM(70.16)
  2012-10-01   dv  Added support for wcn2243 2.1 SOC.
  2012-05-28   dv  Updated BHA B0 NVM(70.13)
  2012-02-22   dv  Updated BHA B0 NVM
  2011-05-20   rr  Updated BHA B0 NVM
  2011-03-03   rr  Updated BHA B0 NVM (Power Class 2 added).
  2010-12-09   tw  Updated MBA B1 NVM
  2010-11-05   tw  Updated MBA B1 NVM
  2010-10-25   tw  Added support for 4025 B3
  2010-07-26   rr  4025 B2 NVM update.
  2010-07-13   rr  Marimba B1 NVM update.
  2010-07-05   rr  4025 B2 NVM update.
  2010-05-10   rr  Marimba B1 (aka 2.1) NVM update. (CR#235036 ROM patch).
  2010-04-27   rr  4025 B2 NVM update.
  2010-03-10   rr  Updated 4025 B2. ROM patch for CR#189653 - pabBT Scatternet: AG(master)+FTP(slave)+DUNP (master/slave) concurrent connections fails.
  2010-02-19   rr  Added support for Marimba B1
  2009-11-20  dgh  Updated NVM version for 4025 B2. 
  2009-10-05  dgh  Updated 4025 B1 and B2 NVM to latest released tags.
  2009-08-14  dgh  Added support for 4025 B2.
  2009-07-07   jn  Updated NVM version for 4025 B1.
  2009-05-19   sa  Updated patch for 4025 B1 (19.2 & 32 MHz).
  2009-04-24   sa  Updated patch for 4025 B1 (19.2 & 32 MHz).
  2009-02-13   sa  Updated patch for 4025 B1 (32 MHz only).

*   #1         02 Feb  2009          SA
*   Created new file to support class 2 nv cmds for 4025 B1.

===========================================================================*/

/*===========================================================================

                         INCLUDE FILES FOR MODULE

===========================================================================*/

/*=========================================================================*/
/*                               TYPEDEFS                                  */
/*=========================================================================*/


/*===========================================================================
                                 Globals
===========================================================================*/


static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_19P2MHz_4025_b1_CLASS2[] = \
{
  0x05, 0x01, 0x20, 0x02, 
  0x0D, 0x17    // version 17.0D
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_32MHz_4025_b1_CLASS2[] = \
{
  0x05, 0x01, 0x20, 0x02, 
  0x0E, 0x1D    // version 1D.0E
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_36_4025_CLASS2[] = \
{
   0x26, 0x01, 0x24, 0x23,
   0x07, 0x07, 0x07, 0x02, 0x07, 0x00, 0x01, 0x02, 
   0x03, 0x04, 0x05, 0x06, 0x07, 0x07, 0x07, 0x00, 
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 
   0x02, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 
   0x07, 0x08, 0x09
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_19P2MHz_4025_b2_CLASS2[] = \
{
  0x05, 0x01, 0x20, 0x02, 
  0x08, 0x17    // version 17.08
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_32MHz_4025_b2_CLASS2[] = \
{
  0x05, 0x01, 0x20, 0x02, 
  0x08, 0x1D    // version 1D.08
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_19P2MHz_4025_b3_CLASS2[] = \
{
  0x05, 0x01, 0x20, 0x02, 
  0x01, 0x18    // version 18.01
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_32MHz_4025_b3_CLASS2[] = \
{
  0x05, 0x01, 0x20, 0x02, 
  0x01, 0x19    // version 19.01
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_41_19P2MHz_4025_CLASS2[] = \
{
  0x29, 0x01, 0x29, 0x26,
  0x11, 0x00, 0x00, 0x00, 0x42, 0x33, 0xA5, 0x31, 
  0xB1, 0x07, 0xC1, 0x70, 0x1C, 0xFA, 0x1D, 0x04, 
  0x1E, 0x1F, 0x20, 0x6C, 0x3D, 0x67, 0x3E, 0xC9, 
  0xA7, 0x64, 0xAA, 0x42, 0xAB, 0x94, 0xAC, 0x8F, 
  0xA6, 0x27, 0xA8, 0x00, 0xCF, 0x04
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_41_32MHz_4025_CLASS2[] = \
{
  0x2B, 0x01, 0x29, 0x28,
  0x12, 0x00, 0x00, 0x00, 0x42, 0x33, 0xA5, 0x31, 
  0xB1, 0x07, 0xC1, 0x70, 0x1C, 0xFA, 0x1D, 0x04, 
  0x1E, 0x1F, 0x20, 0x6C, 0x3D, 0x67, 0x3E, 0xC9, 
  0xA7, 0x64, 0xAA, 0x42, 0xAB, 0x94, 0xAC, 0x8F, 
  0xA6, 0x27, 0xA8, 0x00, 0xCF, 0x04, 0x5B, 0x06
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_56_4025_CLASS2[] = \
{
  0x04, 0x01, 0x38, 0x01,
  0x04
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_19p2MHz_mba_b1_CLASS2[] = \
{
  0x05, 0x01, 0x20, 0x02, 
  0x3C, 0x50    // version 50.3C

};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_36_mba_b1_CLASS2[] = \
{
   0x2A, 0x01, 0x24, 0x27,
   0x07, 0x07, 0x07, 0x02, 0x07, 0x00, 0x01, 0x02,
   0x03, 0x04, 0x05, 0x06, 0x07, 0x07, 0x07, 0x00, 
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
   0x02, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 
   0x07, 0x08, 0x09, 0x07, 0x00, 0x07, 0x00
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_56_mba_b1_CLASS2[] = \
{
  0x06, 0x01, 0x38, 0x03,
  0x04, 0x04, 0x04
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_19p2MHz_bha_b0_CLASS2[] = \
{
  0x05, 0x01, 0x20, 0x02,
  0x17, 0x70    // version 70.17
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_36_bha_b0_CLASS2[] = \
{
   0x0F, 0x01, 0x24, 0x0C,
   0xFF, 0x00, 0x07, 0x07, 0x07, 0x07, 0x00, 0x00,
   0x07, 0x07, 0x02, 0x00
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_19p2MHz_bha_b1_CLASS2[] = \
{
  0x05, 0x01, 0x20, 0x02,
  0x06, 0x70    // version 70.06
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_36_riva_CLASS2[] = \
{
   0x0F, 0x01, 0x24, 0x0C,
   0xFF, 0x00, 0x07, 0x07, 0x07, 0x07, 0x00, 0x00,
   0x07, 0x07, 0x02, 0x00
};
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_83_riva_CLASS2[] = \
{
   0x06, 0x01, 0x53, 0x03,
   0x07, 0x07, 0x07
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_56_bha_b0_CLASS2[] = \
{
  0x06, 0x01, 0x38, 0x03,
  0x04, 0x04, 0x04
};

#endif /* _BT_QSOC_NVM_BTS_CLASS2_SUPPORT_H  */
