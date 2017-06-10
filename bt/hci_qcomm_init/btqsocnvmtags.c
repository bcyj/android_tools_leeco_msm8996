/**
  @file btqsocnvmtags.c

  This file contains definitions of static tags: Fixed and run-time 
  tags that are chosen based on the run-time host parameters. It also 
  creates the set of static tags that need to be sent to the SoC at 
  run-time. This is done based on the chip type and the run-time 
  parameters of the host.
*/

/*--------------------------------------------------------------
     Copyright (c) 2008-2014 Qualcomm Technologies, Inc.
     All rights reserved.
     Qualcomm Technologies Confidential and Proprietary
--------------------------------------------------------------*/
/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order. Please
  use ISO format for dates.

  $Header: //source/qcom/qct/wconnect/bthost/soccfg/main/latest/src/btqsocnvmtags.c#60 $
  $DateTime: 2012/03/26 04:26:46 $
  $Author: c_vdomak $

  when        who  what, where, why
  ----------  ---  -----------------------------------------------------------
  2012-10-01   dv  Added support for wcn2243 2.1 SOC.
  2012-05-11   rk  Added Tag63 for WCN2243 to have BT PTA in low priority
                   when A2DP Streaming with flush timeout is enabled.
  2012-05-10   rk  Setting Tag74 to Rom default for WCN2243 + AR6k Wlan SoC
                   and correcting Tag57 for WCN2243.
  2012-03-26   rr  Updated BHA B0, MBA B1 and 4025 B3 NVM
  2012-02-29   rr  Added/Modified power class configurations for LE SoC
                   on WCN2243 and WCN3660
  2012-01-20   rk  Increased the warm up time(10ms) for Bahama XO based targets
  2011-10-13   ar  Removed the changes to disable LE through NVM.
  2011-08-18   rr  BT3.0 and 4.0 spec, Bluetooth Local Feature Mask and Bluetooth
                   Version Information configured using NVM tags
  2011-08-16   ar  Add support to disable LE through NVM. Disable LE for Riva 1.0
  2011-07-28   rs  For BTS 4025 B2 no 2 EV3 , 2EV5 , 3EV5, 3EV5
  2011-07-18   bn  Added support for 8960 for sending NVM tags to RIVA.
  2011-06-22   rk  Added poke command for BAHAMA B0
  2011-03-03   rr  Updated BHA B0 NVM (Power Class 2 added).
  2011-02-21   rr  Correcting tag#32 updates for 4025 B3.
  2010-12-09   tw  Updated MBA B1 NVM
  2010-11-05   tw  Updated MBA B1 NVM
  2010-10-25   tw  Added support for 4025 B3, BHA A0 updated
  2010-10-06   tw  BHA A0 NVM update + fix for BHA A0 tag 44
  2010-10-04   tw  4025 B2 NVM update, change to patch tag 101
  2010-07-21   rr  For BTS4025 and beyond, Tag 6: no EV4 & EV5 eSCO packets 
                   for best WLAN Coex.
  2010-08-11   rr  Fixed compiler warnings.
  2010-07-26   rr  4025 B2 NVM update.
  2010-07-23   rr  Added support for Bahama A0 (Manihiki)
  2010-07-13   rr  Marimba B1 NVM update.
  2010-07-05   rr  4025 B2 NVM update.
  2010-06-01  akg  Fix for CLK_PWR_REQ and other tag37 configuration for 
                   Marimba B0/B1 Sw-IBS. 
  2010-05-31  akg  InBand Sleep Disabled in FTM Mode. This causes FTM mode
                   fail on SCMM BMP
  2010-05-10   rr  Marimba B1 (aka 2.1) NVM update. (CR#235036 ROM patch).
                   Tag 6 for all Marimba in sync with I&T released NVM.
                   (3.0 spec) 
  2010-04-20   rr  Overriding BT_ACTIVE duration for all Marimba 
                   (ROM default: 10us too short).
  2010-04-27   rr  4025 B2 NVM update.
  2010-04-27   rr  Adding runtime configuration support for selecting BTC
                   tags (4025 B0/B1).
  2010-04-23  akg  Disable DeepSleep Mode for Marimba B0 & Marimba A0.
  2010-04-23   ns  Incase of Marimba do not write LMP version to SOCs (Tag78)
  2010-03-26   rr  Marimba B1 (aka 2.1) NVM update. Updated Tag 17 to byte 
                   length of 8 from 7 including Software in Band Sleep
  2010-03-10   rr  4020 BD B1 NVM update.
  2010-03-10   rr  Updated 4025 B2. ROM patch for CR#189653 - pabBT 
                   Scatternet: AG(master)+FTP(slave)+DUNP (master/slave) 
                   concurrent connections fails.
  2010-03-03   ag  Bringing up Logger Redirection Feature on Marimba. 
                   Change tag_38 parameters
  2010-02-23   rr  Updated Marimba B1. Removed not applicable CUSTOM CLASS
                   tag 36 from init_mba_b1_entries.
  2010-02-19   rr  Added support for Marimba B1
  2010-01-28  dgh  Added new power class to support custom power settings
                   on Hagrid daugthercard.
  2010-01-27  dgh  Added runtime configuration for selecting WLAN coexistence
                   tags.
  2010-01-13  dgh  Changed XO warmup time from 5 ms to 8 ms.
  2009-11-20  dgh  Updated NVM version for 4025 B2.
  2009-11-16   sp  Fixed Marimba A0 Tag17 length
  2009-11-16   sp  Fixed Marimba Tag17 length
  2009-11-04  dgh  Added define for BT_QSOC_VS_EDL_DNLOAD_REQ since it was 
                   moved out of btqsocnvmprivate.h.  Made tag #71 a runtime
                   tag.
  2009-10-07   sp  Added Tag38 for soc re-direct logging for Marimaba
  2009-10-07   sp  Added support for Marimba B0
  2009-10-05  dgh  Updated 4025 B1 and B2 NVM to latest released tags.
  2009-10-05  dgh  Fixed compiler warnings.
  2009-09-10  dgh  Doxygenated.  Updated with 4025 B2 NVM tags.
  2009-08-27   ks  Add changes, featurization for XO Support.   
  2009-07-07   jn  Updated NVM version for 4025 B1.
  2009-07-01   jn  Updated 4025 B1 patches.  
  2009-07-01   rb  Inital version
===============================================================================*/
/*******************************************************************************
 * Previous btqsocnvm.c revision history

  when        who  what, where, why
  ----------  ---  -----------------------------------------------------------
  2009-07-07   jn  Updated NVM version for 4025 B1.
  2009-07-01   jn  Updated 4025 B1 patches.
  2009-06-02   sa  Support for Class 2 device.
  2009-05-19   sa  Updated patch for 4025 B1.
  2009-04-24   sa  Updated patch for 4025 B1.
  2009-04-07   rb  Updated for 4025 B1 19.2MHz and 4021 B1 
  2009-03-14   sa  Added Support for CLASS 2 device.
  2009-03-14   sa  Updated patch for 4025 B1 (32 MHz only).
  2009-02-13   rb  Added support for sw in-band
  2009-02-02   sa  Updated patch for 4025 B1 
  2009-01-12   sa  Updated patch for 4021 B1 (19.2 MHz).
  2009-01-05   sa  Fixed Compiler Warning on 76XX.
  2008-12-19   sa  Updated patch for 4025 B0.
  2008-11-25   sa  Updated patch for 4020 BD B0 (19.2 MHz only).
  2008-11-19   sa  Fixed sending TAG 114 & 115 twice.
  2008-11-17   sa  Update patch for 4025 B1.
  2008-11-17   sa  Resolved Crash for unsupported SOC.
  2008-11-14   sa  Support for NVM Automation.
  2008-11-06   sa  Updated NVM for 4025 B1
  2008-11-03   sa  Removed Compilation Warning by removing data structures 
                   that are not required for R3 BD.
  2008-10-30   sa  Resolved Dependency between TAG 6 and TAG 78 for BT 2.1 
                   support.
  2008-10-30   jn  Fix tag 37 and remove tag 76 for all 4025 revs. Fix typo in
                   4025 B1 RAM index.
  2008-10-27   sa  Added support to read BT 2.1 support from a nv item in 
                   NV browser at run-time.
  2008-10-13   sa  Removing support for 4020 R3 BD and 4020 R4
  2008-10-10   sa  Updated NVM tag 55d for 4025 B1 and upgraded to ver 07.02 
                   (19.2 MHz) and ver 0D.02 (32MHz) 
  2008-09-10   jn  Update NVM for 4020 BD B0.
  2008-08-30   rb  Update NVM for 4020 BD B0. Added preliminary support for 
                   4020 BD B1.
  2008-08-20   rb  Added support for 4025 B1
  2008-08-20   jn  Fix date in the previous checkin.
  2008-08-19   jn  Update patch for 4020 R3. Update NVM for 4020 BD B0.
  2008-08-12   rb  Added new patches and updates for 4025 B0
  2008-07-28   sa  Added support to configure soc logging enabled at run-time.
  2008-07-28   rb  Fix an error in patch 108 for 4025 A0
  2008-07-24   sp  Correct previous mistake for FFA SoC logging enable
  2008-07-16   sp  Disable SoC logging for QSC6240/QSC6270
  2008-07-06   rb  Patches for 4021 B1, 4020BD B0 (19.2MHz and 32MHz)
  2008-06-12   rb  Patches for BT connection loss in sniff mode and updates for 
                   4021 B1, 4020BD B0 and 4020 B0 (19.2MHz and 32MHz)
  2008-05-21   rb  Patches for esco issue on 4020 BD B0 
  2008-05-15   rb  Support for 4025 B0 Beta and Patches for esco issue. 
  2008-05-06   rb  Patches for 4025 A0
  2008-03-31   rb  Patches for 4020 BD B0 and 4021 B1
  2008-03-21   rb  Update 4020 BD B0 with correct f/w version nos.
  2008-03-13   rb  Add support for 4020 BD B0 & 4021 B1 and updated patches
                   for 4020 A0 and 4020 B0

*
*   #2          29 Feb 2008           JN
*   Fixed features mask when BT2.1 features is enabled but
*   SoC does not support BT2.1.
*
*   #1          31 Jan 2008           BH
*   Branched and renamed file from WM.
*
*   #5          31 Jan 2008           RH
*   Merged in bretth's latest update.
*
*   #4          24 Jan 2008           BH
*   Updated NVMs to the following values 
 ********************************************************************************/

/******************************************************************************
                  -------------- R3 ----------------
                  - 19.2 Mhz Clock: 07.12  (08/13/08)
                  - 32 Mhz Clock: 0D.13    (08/13/08)
                  ----------- 4020BD A0 ------------
                  - 19.2 Mhz Clock: 07.0B  (01/23/08)
                  - 32 Mhz Clock: 0D.0A    (01/23/08)
                  ----------- 4020BD B0 ------------
                  - 19.2 Mhz Clock: 07.0D  (11/20/08)
                  - 32 Mhz Clock: 0D.08    (07/01/08)
                  ------------- 4021 B1 -------------
                  - 19.2 Mhz Clock: 07.09  (03/26/09)
                  - 32 Mhz Clock: 0D.08    (03/26/09)
                  ------------- 4021 C0  -------------
                  - 19.2 Mhz Clock: 07.08 
                  - 32 Mhz Clock: 0D.04 
                  ------------- 4025 A0  -------------
                  - 19.2 Mhz Clock: 07.04 
                  - 32 Mhz Clock: 0D.01 
                  ------------- 4025 B0  -------------
                  - 19.2 Mhz Clock: 07.08  (08/01/08)
                  - 32 Mhz Clock: 0D.00
                  ------------- 4025 B1  -------------
                  - 19.2 Mhz Clock: 07.0B (BT 2.1), 17.0B (CLASS 2)
                                    47.0C (BT 2.0)  (06/24/09)
                  - 32 Mhz Clock:   0D.0C (BT 2.1), 1D.0B (CLASS 2)
                                    4D.0D (BT 2.0)  (06/24/09)
                  ----------- 4020BD B1 ------------
                  - 19.2 Mhz Clock: 07.0A  (08/05/08)
                  - 32 Mhz Clock: 0D.08    (07/01/08)
                  ----------- Marimba B1 ------------
                  - 19.2 Mhz Clock: 07.18  (CLASS 1.5) (02/19/2010)
                  - 19.2 Mhz Clock: 07.18  (CLASS 2) (02/19/2010)
                  ------------- 4025 B2  -------------
                  - 19.2 Mhz Clock: 07.02 (BT 2.1), 17.02 (CLASS 2)
                                    47.02 (BT 2.0)  (03/10/2010)
                  - 32 Mhz Clock:   0D.03 (BT 2.1), 1D.02 (CLASS 2)
                                    4D.02 (BT 2.0)  (03/10/2010)
                  ----------- 4020BD B1 ------------
                  - 19.2 Mhz Clock: 07.03 (BT 2.1)
                                    47.03 (BT 2.0)  (03/10/2010)
                  - 32 Mhz Clock:   0D.02 (BT 2.1)
                                    4D.02 (BT 2.0)  (03/10/2010)
                  ----------- Marimba B1 ------------
                  - 19.2 Mhz Clock: 07.1C  (CLASS 1.5) (03/26/2010)
                  - 19.2 Mhz Clock: 07.1C  (CLASS 2) (03/26/2010)
                  ------------- 4025 B2  -------------
                  - 19.2 Mhz Clock: 07.07 (BT 2.1), 17.07 (CLASS 2)
                                    47.07 (BT 2.0)  (10/04/2010)
                  - 32 Mhz Clock:   0D.08 (BT 2.1), 1D.07 (CLASS 2)
                                    4D.07 (BT 2.0)  (10/04/2010)                  
                  ----------- Marimba B1 ------------
                  - 19.2 Mhz Clock: 07.20  (CLASS 1.5) (05/10/2010)
                  - 19.2 Mhz Clock: 07.20  (CLASS 2) (05/10/2010)
                           ----------- Marimba B1 ------------
                  - 19.2 Mhz Clock: 07.24  (CLASS 1.5) (07/13/2010)
                  - 19.2 Mhz Clock: 07.24  (CLASS 2) (07/13/2010)         
                           ----------- Bahama A0 ------------
                  - 19.2 Mhz Clock: 07.12  (CLASS 1) (10/06/2010)
********************************************************************************/
/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#ifdef ANDROID
#include <cutils/properties.h>
#endif

#include "AEEstd.h"
#include "btqsocnvmplatform.h"
#include "btqsocnvmprivate.h"
#include "btqsocnvmclass2.h"

/* The Clock Stabilization time for XO crystals is longer than TCXO. So BT 
   modules using XO needs a larger SoC Power on Count to hold off SoC Powerup 
   for this delay. SoC NVM Tag 37 configures the SoC Sleep/Wakeup parameters.
*/
#define TAG_37_SLEEP_PARAMS_MIN_BCST_BYTE_POS   (16)
#define TAG_37_SLEEP_PARAMS_PWR_ON_CNT_BYTE_POS (17)

#define TAG_37_SLEEP_PARAMS_MIN_BCST_VAL_XO		(0x80)
#define TAG_37_SLEEP_PARAMS_PWR_ON_CNT_VAL_XO	(0x7E)  
#define TAG_37_SLEEP_PARAMS_MIN_BCST_VAL		(0x34)
#define TAG_37_SLEEP_PARAMS_PWR_ON_CNT_VAL		(0x32)  

/* For Coex targets (BT4025 & Marimba), prevent multi-slot eSCO packets */
/* Applicable in Coex targets */
#if defined(FEATURE_BT_WLAN_COEXISTENCE)
	//Incase of Coex, prevent multi-slot eSCO packet
	/* Byte 4: disable EV4, EV5 (non-EDR) */
	#define TAG_6_BYTE_4_BT_2_1						(0x98)   
	#define TAG_6_BYTE_4_NO_BT_2_1					(0x98)
	/* Byte 5: disable EDR eSCO 3 slot */
	#define TAG_6_BYTE_5_BT_2_1						(0x7F)   
	#define TAG_6_BYTE_5_NO_BT_2_1					(0x79)
#else
	//Incase of Non-Coex, dont prevent multi-slot eSCO packet
	#define TAG_6_BYTE_4_BT_2_1						(0x9B)
	#define TAG_6_BYTE_4_NO_BT_2_1					(0x9B)
	#define TAG_6_BYTE_5_BT_2_1						(0xFF)
	#define TAG_6_BYTE_5_NO_BT_2_1					(0xF9)
#endif  /* FEATURE_BT_WLAN_COEXISTENCE */

#define TAG_6_BYTE_4_BT_3_0    TAG_6_BYTE_4_BT_2_1
#define TAG_6_BYTE_5_BT_3_0    TAG_6_BYTE_5_BT_2_1

#define NVM_TAG_HEADER_SIZE     4
#define NVM_TAG_6_BYTE_4_INDEX  4
#define NVM_TAG_6_BYTE_5_INDEX  5
#define NVM_TAG_6_BYTE_6_INDEX  6
/*****************************************************************************/
/*********************     Common  NVM  Entries   ****************************/
/*****************************************************************************/

/* Same Entry For R3, Unknown */
/* #4 Min. encryption key size - set to 0x01 */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_4[] = \
{
  0x04, 0x01, 0x04, 0x01, 
  0x01
};
#define CFG_TAG_4_R3        (&bt_qsoc_cfg_tag_4[0])
#define CFG_TAG_4_4020BD_B0 (&bt_qsoc_cfg_tag_4[0])
#define CFG_TAG_4_4020BD_B1 (&bt_qsoc_cfg_tag_4[0])
#define CFG_TAG_4_4021_B1   (&bt_qsoc_cfg_tag_4[0]) 
#define CFG_TAG_4_4025_A0   (&bt_qsoc_cfg_tag_4[0])
#define CFG_TAG_4_4025_B0   (&bt_qsoc_cfg_tag_4[0])
#define CFG_TAG_4_4025_B1   (&bt_qsoc_cfg_tag_4[0])
#define CFG_TAG_4_4025_B2   (&bt_qsoc_cfg_tag_4[0])
#define CFG_TAG_4_4025_B3   (&bt_qsoc_cfg_tag_4[0])
#define CFG_TAG_4_MBA_A0    (&bt_qsoc_cfg_tag_4[0])
#define CFG_TAG_4_MBA_B0    (&bt_qsoc_cfg_tag_4[0])
#define CFG_TAG_4_MBA_B1    (&bt_qsoc_cfg_tag_4[0]) 
#define CFG_TAG_4_UNKNOWN   (&bt_qsoc_cfg_tag_4[0])

/* Same Entry For R3, Unknown */
/* #5 Max. encryption key size - set to 0x10 */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_5[] = \
{
  0x04, 0x01, 0x05, 0x01, 
  0x10
};
#define CFG_TAG_5_R3        (&bt_qsoc_cfg_tag_5[0]) 
#define CFG_TAG_5_4020BD_B0 (&bt_qsoc_cfg_tag_5[0]) 
#define CFG_TAG_5_4020BD_B1 (&bt_qsoc_cfg_tag_5[0]) 
#define CFG_TAG_5_4021_B1   (&bt_qsoc_cfg_tag_5[0]) 
#define CFG_TAG_5_4025_A0   (&bt_qsoc_cfg_tag_5[0]) 
#define CFG_TAG_5_4025_B0   (&bt_qsoc_cfg_tag_5[0])
#define CFG_TAG_5_4025_B1   (&bt_qsoc_cfg_tag_5[0])
#define CFG_TAG_5_4025_B2   (&bt_qsoc_cfg_tag_5[0])
#define CFG_TAG_5_4025_B3   (&bt_qsoc_cfg_tag_5[0])
#define CFG_TAG_5_MBA_A0    (&bt_qsoc_cfg_tag_5[0])
#define CFG_TAG_5_MBA_B0    (&bt_qsoc_cfg_tag_5[0])
#define CFG_TAG_5_MBA_B1    (&bt_qsoc_cfg_tag_5[0]) 
#define CFG_TAG_5_UNKNOWN   (&bt_qsoc_cfg_tag_5[0]) 

/* Same Entry For R3, Unknown */
/* #6 BT supported features mask - TBD */

static bt_qsoc_cfg_element_type bt_qsoc_cfg_tag_6_bt_2_1_supported[] = \
{
  0x0B, 0x01, 0x06, 0x08,
  0xFF, 0xFE, 0x8F, 0xFE, TAG_6_BYTE_4_BT_2_1, TAG_6_BYTE_5_BT_2_1, 0x59, 0x83
};


static bt_qsoc_cfg_element_type bt_qsoc_cfg_tag_6_bt_2_1_not_supported[] = \
{
  0x0B, 0x01, 0x06, 0x08,
  0xFF, 0xFE, 0x8F, 0xFE, TAG_6_BYTE_4_NO_BT_2_1, TAG_6_BYTE_5_NO_BT_2_1, 0x00, 0x80
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_6_pre_4025_bt_2_1_supported[] = \
{
  0x0B, 0x01, 0x06, 0x08, 
  0xFF, 0xFE, 0x8F, 0xFE, 0x9B, 0xFF, 0x59, 0x83
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_6_pre_4025_bt_2_1_not_supported[] = \
{
  0x0B, 0x01, 0x06, 0x08, 
  0xFF, 0xFE, 0x8F, 0xFE, 0x9B, 0xF9, 0x00, 0x80
};

#if defined (FEATURE_BT_3_0) || defined (FEATURE_BT_4_0)
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_6_bt_3_0_supported[] = \
{
  0x0B, 0x01, 0x06, 0x08,
  0xFF, 0xFE, 0x8F, 0xFE, TAG_6_BYTE_4_BT_3_0, TAG_6_BYTE_5_BT_3_0, 0x59, 0x87
};

//Mask to disable EDR eSCO 3 Mbps and 3-slot EDR eSCO packets
//Disabling EV4/EV5 packets
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_6_bha_bt_3_0_supported[] = \
{
  0x0B, 0x01, 0x06, 0x08,
  0xFF, 0xFE, 0x8F, 0xFE, 0x98, 0x3F, 0x59, 0x87
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_78_bt_3_0_supported[] = \
{
  0x05, 0x01, 0x4E, 0x02,
  0x05, 0x05
};
#endif

#if defined (FEATURE_BT_4_0)
//Mask to disable EDR eSCO 3 Mbps and 3-slot EDR eSCO packets
//Disabling EV4/EV5 packets
//LE Supported: byte 4, bit 6
//Enabling Simultaneous LE and BR/EDR: byte 6, bit 1
static bt_qsoc_cfg_element_type bt_qsoc_cfg_tag_6_bha_bt_4_0_supported[] = \
{
  0x0B, 0x01, 0x06, 0x08,
  0xFF, 0xFE, 0x8F, 0xFE, 0xD8, 0x3F, 0x5B, 0x87
};
#endif

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_6_4020bd_b0_bt_2_1_supported[] = \
{
  0x0B, 0x01, 0x06, 0x08, 
  0xFF, 0xFE, 0x8F, 0x7E, 0x98, 0x1F, 0x59, 0x83 
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_6_4020bd_b0_bt_2_1_not_supported[] = \
{
  0x0B, 0x01, 0x06, 0x08, 
  0xFF, 0xFE, 0x8F, 0x7E, 0x98, 0x19, 0x00, 0x80
};

/* R3 does not support BT2.1 */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_6_r3[] = \
{
  0x0B, 0x01, 0x06, 0x08, 
  0xFF, 0xFE, 0x8F, 0xFE, 0x9B, 0xF9, 0x00, 0x80
};

#define CFG_TAG_6_R3        (&bt_qsoc_cfg_tag_6_r3[0]) 
#define CFG_TAG_6_4025_A0   (&bt_qsoc_cfg_tag_6_bt_2_1_supported[0]) 
#define CFG_TAG_6_UNKNOWN   (&bt_qsoc_cfg_tag_6_bt_2_1_not_supported[0]) 


/* Same Entry For R3, Unknown */
/* #17 HCI Transport : User Defined Baud 
* The values below specifies transport - H4 with used defined
* baud rate (0xFF third byte from the last). Actual baud rate is
* specified through tag #47
*/
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_17[] = \
{
/* set running baud rate to value defined by BT_QSOC_BAUD_RATE */
#ifdef FEATURE_BT_QSOC_INBAND_SLEEP
  0x08, 0x01, 0x11, 0x05, 
  0x4A, 0x01, BT_QSOC_BAUD_RATE, 0x00, 0x00 
#else
  0x08, 0x01, 0x11, 0x05, 
  0x0A, 0x01, BT_QSOC_BAUD_RATE, 0x00, 0x00
#endif /* FEATURE_BT_QSOC_INBAND_SLEEP */
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_17_4025[] = \
{
/* set running baud rate to value defined by BT_QSOC_BAUD_RATE */
#ifdef FEATURE_BT_QSOC_INBAND_SLEEP
  0x08, 0x01, 0x11, 0x05, 
  0x4A, 0x01, BT_QSOC_BAUD_RATE, 0x00, 0x00
#elif defined(FEATURE_BT_QSOC_SW_INBAND_SLEEP)
  0x0A, 0x01, 0x11, 0x07, 
  0x8A, 0x01, BT_QSOC_BAUD_RATE, 0x00, 0x00, 0x0A, 0x01
#else
  0x08, 0x01, 0x11, 0x05, 
  0x0A, 0x01, BT_QSOC_BAUD_RATE, 0x00, 0x00
#endif /* FEATURE_BT_QSOC_INBAND_SLEEP */   
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_17_unknown[] = \
{
/* set running baud rate to value defined by BT_QSOC_BAUD_RATE */
#ifdef FEATURE_BT_QSOC_INBAND_SLEEP
  0x08, 0x01, 0x11, 0x05, 
  0x4A, 0x01, BT_QSOC_BAUD_RATE, 0x00, 0x00
#elif defined(FEATURE_BT_QSOC_SW_INBAND_SLEEP)
  0x0B, 0x01, 0x11, 0x08, 
  0x8A, 0x01, BT_QSOC_BAUD_RATE, 0x00, 0x00, 0x0A, 0x0A, 0x01
#else
  0x08, 0x01, 0x11, 0x05, 
  0x0A, 0x01, BT_QSOC_BAUD_RATE, 0x00, 0x00
#endif /* FEATURE_BT_QSOC_INBAND_SLEEP */   
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_17_mba_a0[] = \
{
/* set running baud rate to value defined by BT_QSOC_BAUD_RATE */
#ifdef FEATURE_BT_QSOC_INBAND_SLEEP
  0x0A, 0x01, 0x11, 0x07, 
  0x4A, 0x01, BT_QSOC_BAUD_RATE, 0x00, 0x00, 0x00, 0x00
#elif defined(FEATURE_BT_QSOC_SW_INBAND_SLEEP)
  0x0A, 0x01, 0x11, 0x07, 
  0x8A, 0x01, BT_QSOC_BAUD_RATE, 0x00, 0x00, 0x0A, 0x01
#else
  0x0A, 0x01, 0x11, 0x07, 
  0x0A, 0x01, BT_QSOC_BAUD_RATE, 0x00, 0x00, 0x00, 0x00
#endif /* FEATURE_BT_QSOC_INBAND_SLEEP */   
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_17_mba_a0_FTM[] = \
{
  0x0A, 0x01, 0x11, 0x07, 
  0x0A, 0x01, BT_QSOC_BAUD_RATE, 0x00, 0x00, 0x00, 0x00

};


static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_17_mba_b0[] = \
{
/* set running baud rate to value defined by BT_QSOC_BAUD_RATE */
#ifdef FEATURE_BT_QSOC_INBAND_SLEEP
  0x0B, 0x01, 0x11, 0x08, 
  0x4A, 0x01, BT_QSOC_BAUD_RATE, 0x00, 0x00, 0x00, 0x00, 0x00
#elif defined(FEATURE_BT_QSOC_SW_INBAND_SLEEP)
  0x0B, 0x01, 0x11, 0x08, 
  0x8A, 0x01, BT_QSOC_BAUD_RATE, 0x00, 0x00, 0x0A, 0x01, 0x00
#else
  0x0B, 0x01, 0x11, 0x08, 
  0x0A, 0x01, BT_QSOC_BAUD_RATE, 0x00, 0x00, 0x00, 0x00, 0x00
#endif /* FEATURE_BT_QSOC_INBAND_SLEEP */   
};


static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_17_mba_b0_FTM[] = \
{
  0x0B, 0x01, 0x11, 0x08, 
  0x0A, 0x01, BT_QSOC_BAUD_RATE, 0x00, 0x00, 0x00, 0x00, 0x00

};


static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_17_mba_b1[] = \
{
/* set running baud rate to value defined by BT_QSOC_BAUD_RATE */
#ifdef FEATURE_BT_QSOC_INBAND_SLEEP
  0x0B, 0x01, 0x11, 0x08, 
  0x4A, 0x01, BT_QSOC_BAUD_RATE, 0x00, 0x00, 0x00, 0x00, 0x00
#elif defined(FEATURE_BT_QSOC_SW_INBAND_SLEEP)
  0x0B, 0x01, 0x11, 0x08, 
  0x8A, 0x01, BT_QSOC_BAUD_RATE, 0x00, 0x00, 0x0A, 0x01, 0x00
#else
  0x0B, 0x01, 0x11, 0x08, 
  0x0A, 0x01, BT_QSOC_BAUD_RATE, 0x00, 0x00, 0x00, 0x00, 0x00
#endif /* FEATURE_BT_QSOC_INBAND_SLEEP */   
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_17_mba_b1_FTM[] = \
{
  0x0B, 0x01, 0x11, 0x08, 
  0x0A, 0x01, BT_QSOC_BAUD_RATE, 0x00, 0x00, 0x00, 0x00, 0x00

};


static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_17_bha_a0[] = \
{
/* set running baud rate to value defined by BT_QSOC_BAUD_RATE */
#ifdef FEATURE_BT_QSOC_INBAND_SLEEP
  0x0B, 0x01, 0x11, 0x08,
  0x4A, 0x01, BT_QSOC_BAUD_RATE, 0x00, 0x00, 0x00, 0x00, 0x00
#elif defined(FEATURE_BT_QSOC_SW_INBAND_SLEEP)
  0x0B, 0x01, 0x11, 0x08, 
  0x8A, 0x01, BT_QSOC_BAUD_RATE, 0x00, 0x00, 0x04, 0x0A, 0x00
#else
  0x0B, 0x01, 0x11, 0x08,
  0x0A, 0x01, BT_QSOC_BAUD_RATE, 0x00, 0x00, 0x00, 0x00, 0x00
#endif /* FEATURE_BT_QSOC_INBAND_SLEEP */   
};

#define CFG_TAG_17_R3        (&bt_qsoc_cfg_tag_17[0]) 
#define CFG_TAG_17_4020BD_B0 (&bt_qsoc_cfg_tag_17[0]) 
#define CFG_TAG_17_4020BD_B1 (&bt_qsoc_cfg_tag_17[0]) 
#define CFG_TAG_17_4021_B1   (&bt_qsoc_cfg_tag_17[0]) 
#define CFG_TAG_17_4025_A0   (&bt_qsoc_cfg_tag_17[0]) 
#define CFG_TAG_17_4025_B0   (&bt_qsoc_cfg_tag_17[0]) 
#define CFG_TAG_17_4025_B1   (&bt_qsoc_cfg_tag_17_4025[0]) 
#define CFG_TAG_17_4025_B2   (&bt_qsoc_cfg_tag_17_4025[0])
#define CFG_TAG_17_4025_B3   (&bt_qsoc_cfg_tag_17_4025[0])
#define CFG_TAG_17_MBA_A0    (&bt_qsoc_cfg_tag_17_mba_a0[0]) 
#define CFG_TAG_17_MBA_B0    (&bt_qsoc_cfg_tag_17_mba_b0[0]) 
#define CFG_TAG_17_MBA_B1    (&bt_qsoc_cfg_tag_17_mba_b1[0])
#define CFG_TAG_17_BHA_A0    (&bt_qsoc_cfg_tag_17_bha_a0[0])
#define CFG_TAG_17_BHA_B0    (&bt_qsoc_cfg_tag_17_bha_a0[0])
#define CFG_TAG_17_UNKNOWN   (&bt_qsoc_cfg_tag_17_unknown[0]) 
#define CFG_TAG_17_MBA_A0_FTM    (&bt_qsoc_cfg_tag_17_mba_a0_FTM[0]) 
#define CFG_TAG_17_MBA_B0_FTM    (&bt_qsoc_cfg_tag_17_mba_b0_FTM[0]) 
#define CFG_TAG_17_MBA_B1_FTM    (&bt_qsoc_cfg_tag_17_mba_b1_FTM[0])


/* Same Entry For R3, Unknown */
/* #27 Sleep Enable Mask - enable sleep */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_27[] = \
{
#if defined(FEATURE_BT_QSOC_SLEEP) || defined(FEATURE_BT_QSOC_SW_INBAND_SLEEP)
  0x04, 0x01, 0x1B, 0x01, 
  0x01
#else
  0x04, 0x01, 0x1B, 0x01, 
  0x00
#endif /* FEATURE_BT_QSOC_SLEEP */
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_27_unknown[] = \
{
  0x04, 0x01, 0x1B, 0x01, 
  0x00
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_27_riva[] = \
{
  0x04, 0x01, 0x1B, 0x01,
  0x00
};

#define CFG_TAG_27_R3        (&bt_qsoc_cfg_tag_27[0]) 
#define CFG_TAG_27_4020BD_B0 (&bt_qsoc_cfg_tag_27[0]) 
#define CFG_TAG_27_4020BD_B1 (&bt_qsoc_cfg_tag_27[0]) 
#define CFG_TAG_27_4021_B1   (&bt_qsoc_cfg_tag_27[0]) 
#define CFG_TAG_27_4025_A0   (&bt_qsoc_cfg_tag_27[0])
#define CFG_TAG_27_4025_B0   (&bt_qsoc_cfg_tag_27[0])
#define CFG_TAG_27_4025_B1   (&bt_qsoc_cfg_tag_27[0])
#define CFG_TAG_27_4025_B2   (&bt_qsoc_cfg_tag_27[0])
#define CFG_TAG_27_4025_B3   (&bt_qsoc_cfg_tag_27[0])
//Currently, Deep sleep disabled on Marimba A0/B0.
#define CFG_TAG_27_MBA_A0    (&bt_qsoc_cfg_tag_27_unknown[0])
#define CFG_TAG_27_MBA_B0    (&bt_qsoc_cfg_tag_27_unknown[0])
#define CFG_TAG_27_MBA_B1    (&bt_qsoc_cfg_tag_27[0])
#define CFG_TAG_27_BHA_A0    (&bt_qsoc_cfg_tag_27[0])
#define CFG_TAG_27_BHA_B0    (&bt_qsoc_cfg_tag_27[0])
#define CFG_TAG_27_UNKNOWN   (&bt_qsoc_cfg_tag_27_unknown[0]) 

/* #28 Low Power Clock Control parameters - LPO type - Digital 
* Minimun Clk Cal. period - 0x00001000 (4096 * 312.5 = 1.28sec)
* Crystal stabilization time - not applicable
*/
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_28[] = \
{
  0x0A, 0x01, 0x1C, 0x07, 
  0x00, 0x10, 0x00, 0x00, 0x2C, 0x01, 0x01
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_28_4020bd_b0[] = \
{
  0x12, 0x01, 0x1C, 0x0F, 
  0x00, 0x10, 0x00, 0x00, 0x2C, 0x01, 0x01, 0x00, 
  0x00, 0xF0, 0x00, 0x00, 0xFF, 0xFF, 0x00
};

#define CFG_TAG_28_4025_A0   (&bt_qsoc_cfg_tag_28_4020bd_b0[0])
#define CFG_TAG_28_UNKNOWN   (&bt_qsoc_cfg_tag_28[0]) 

/* #32 USB product ID tag - used for versioning NVM releases 
*/
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_19p2MHz_4025_a0[] = \
{
  0x05, 0x01, 0x20, 0x02, 
  0x04, 0x07    // version 07.04
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_32MHz_4025_a0[] = \
{
  0x05, 0x01, 0x20, 0x02, 
  0x01, 0x0D    // version 0D.01
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_19p2MHz_4025_b1_bt_2_0[] = \
{
  0x05, 0x01, 0x20, 0x02, 
  0x0D, 0x47    // version 47.0D
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_19p2MHz_4025_b1_bt_2_1[] = \
{
  0x05, 0x01, 0x20, 0x02, 
  0x0C, 0x07    // version 07.0C
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_32MHz_4025_b1_bt_2_0[] = \
{
  0x05, 0x01, 0x20, 0x02, 
  0x0E, 0x4D    // version 4D.0E
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_32MHz_4025_b1_bt_2_1[] = \
{
  0x05, 0x01, 0x20, 0x02, 
  0x0D, 0x0D    // version 0D.0D
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_19p2MHz_4025_b2_bt_2_0[] = \
{
  0x05, 0x01, 0x20, 0x02,
  0x08, 0x47    // version 47.08
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_19p2MHz_4025_b2_bt_2_1[] = \
{
  0x05, 0x01, 0x20, 0x02,
  0x08, 0x07    // version 07.08
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_32MHz_4025_b2_bt_2_0[] = \
{
  0x05, 0x01, 0x20, 0x02,
  0x08, 0x4D    // version 4D.08
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_32MHz_4025_b2_bt_2_1[] = \
{
  0x05, 0x01, 0x20, 0x02,
  0x09, 0x0D    // version 0D.09
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_19p2MHz_4020bd_b0_bt_2_1[] = \
{
  0x05, 0x01, 0x20, 0x02, 
  0x0D, 0x07    // version 07.0D
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_19p2MHz_4020bd_b0_bt_2_0[] = \
{
  0x05, 0x01, 0x20, 0x02, 
  0x0C, 0x67    // version 67.0C
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_19p2MHz_mba_a0[] = \
{
  0x05, 0x01, 0x20, 0x02, 
  0x0B, 0x07    // version 07.0B

};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_19p2MHz_mba_b0[] = \
{
  0x05, 0x01, 0x20, 0x02, 
  0x10, 0x07    // version 07.10

};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_19p2MHz_mba_b1[] = \
{
  0x05, 0x01, 0x20, 0x02, 
  0x3C, 0x40    // version 40.3C

};

#define CFG_TAG_19P2_4020BD_B0_BT2_1 (&bt_qsoc_cfg_tag_32_19p2MHz_4020bd_b0_bt_2_1[0])
#define CFG_TAG_19P2_4020BD_B0_BT2_0 (&bt_qsoc_cfg_tag_32_19p2MHz_4020bd_b0_bt_2_0[0])

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_19p2MHz_4020bd_b1_bt_2_0[] = \
{
  0x05, 0x01, 0x20, 0x02, 
  0x03, 0x47    // version 47.03
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_19p2MHz_4020bd_b1_bt_2_1[] = \
{
  0x05, 0x01, 0x20, 0x02, 
  0x03, 0x07    // version 07.03
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_32MHz_4020bd_b1_bt_2_0[] = \
{
  0x05, 0x01, 0x20, 0x02, 
  0x02, 0x4D    // version 4D.02
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_32MHz_4020bd_b1_bt_2_1[] = \
{
  0x05, 0x01, 0x20, 0x02, 
  0x02, 0x0D    // version 0D.02
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_19p2MHz_4025_b3_bt_2_0[] = \
{
  0x05, 0x01, 0x20, 0x02, 
  0x01, 0x00    // version 00.01
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_19p2MHz_4025_b3_bt_2_1[] = \
{
  0x05, 0x01, 0x20, 0x02, 
  0x01, 0x10    // version 10.01
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_32MHz_4025_b3_bt_2_0[] = \
{
  0x05, 0x01, 0x20, 0x02, 
  0x01, 0x01    // version 01.01
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_32_32MHz_4025_b3_bt_2_1[] = \
{
  0x05, 0x01, 0x20, 0x02, 
  0x01, 0x11    // version 11.01
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_35_riva[] = \
{
  0x26, 0x01, 0x23, 0x23,
  0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
  0x58, 0x59, 0x19, 0x39, 0x3A, 0x3B, 0x3C, 0x3D,
  0x3E, 0x3F, 0x3F, 0x3F, 0x01, 0x09, 0x11, 0x21,
  0x22, 0x23, 0x24, 0x25, 0x26, 0x3F, 0xF5, 0xF5,
  0x13, 0x1F, 0x44
};

/* #36 Remove lowest power mode (01-07) 
*/
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_36_4025_a0[] = \
{
  0x26, 0x01, 0x24, 0x23, 
  0x09, 0x09, 0x07, 0x04, 0x09, 0x00, 0x01, 0x02, 
  0x03, 0x04, 0x05, 0x06, 0x07, 0x07, 0x07, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x02, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 
  0x07, 0x08, 0x09
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_36_4025_CLASS1[] = \
{
  0x26, 0x01, 0x24, 0x23,
  0x09, 0x09, 0x07, 0x04, 0x09, 0x00, 0x01, 0x02, 
  0x03, 0x04, 0x05, 0x06, 0x07, 0x07, 0x07, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 
  0x02, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 
  0x07, 0x08, 0x09
};
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_36_riva_CLASS1[] = \
{
  0x0F, 0x01, 0x24, 0x0C,
  0xFF, 0x03, 0x07, 0x09, 0x09, 0x09, 0x00, 0x00,
  0x09, 0x09, 0x04, 0x00
};
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_83_riva_CLASS1[] = \
{
  0x06, 0x01, 0x53, 0x03,
  0x09, 0x09, 0x09
};

#define CFG_TAG_36_4025_A0       (&bt_qsoc_cfg_tag_36_4025_a0[0])
/* LE SoC power class configurations are same on WCN2243 & WCN3660
 */
#define CFG_TAG_83_BHA_B0_CLASS1 (&bt_qsoc_cfg_tag_83_riva_CLASS1[0])
#define CFG_TAG_83_BHA_B0_CLASS2 (&bt_qsoc_cfg_tag_83_riva_CLASS2[0])

/** Custom power levels for the Hagrid 1 RF card on 7x30 SURF */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_36_mba_b0_custom[] = \
{
  0x2A, 0x01, 0x24, 0x27,
  0x06, 0x06, 0x06, 0x02, 0x06, 0x00, 0x01, 0x02, 
  0x03, 0x04, 0x05, 0x06, 0x07, 0x07, 0x07, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x02, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 
  0x07, 0x08, 0x09, 0x06, 0x00, 0x06, 0x00
};

/** Custom power levels for the Hagrid 3 RF card on 7x30 fusion ffa's */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_36_mba_b1_custom[] = \
{
  0x2A, 0x01, 0x24, 0x27,
  0x05, 0x05, 0x05, 0x02, 0x05, 0x00, 0x01, 0x02,
  0x03, 0x04, 0x05, 0x05, 0x05, 0x05, 0x05, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
  0x02, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
  0x07, 0x08, 0x09, 0x05, 0x00, 0x05, 0x00
};

/* #37 Sleep parameters - refer to documentation for values used */

// Generic w/ XO support disabled
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_37_clock_sharing[] = \
{
  0x19, 0x01, 0x25, 0x16, 
  0x00, 0x00, 0x02, 0x01, 0x01, 0x12, 0x01, 0x00,
  0x00, 0x12, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 
  TAG_37_SLEEP_PARAMS_MIN_BCST_VAL, TAG_37_SLEEP_PARAMS_PWR_ON_CNT_VAL,
  0x02, 0x00, 0x00, 0x00
};

// Generic w/ XO support enabled
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_37_clock_sharing_xo[] = \
{
  0x19, 0x01, 0x25, 0x16, 
  0x00, 0x00, 0x02, 0x01, 0x01, 0x12, 0x01, 0x00,
  0x00, 0x12, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 
  TAG_37_SLEEP_PARAMS_MIN_BCST_VAL_XO, TAG_37_SLEEP_PARAMS_PWR_ON_CNT_VAL_XO,
  0x02, 0x00, 0x00, 0x00
};

// Marimba A0 w/ XO support disabled
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_37_clk_sharing_mba_a0[] = \
{
  0x18, 0x01, 0x25, 0x15, 
  0x00, 0x00, 0x02, 0x01, 0x00, 0x00, 0x03, 0x03,
  0x00, 0x00, 0x0B, 0x0B, 0x00, 0x00, 0x04, 0x00,
  TAG_37_SLEEP_PARAMS_MIN_BCST_VAL, TAG_37_SLEEP_PARAMS_PWR_ON_CNT_VAL,
  0x30, 0x00, 0x00
};

// Marimba A0 w/ XO support enabled
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_37_clk_sharing_mba_a0_xo[] = \
{
  0x18, 0x01, 0x25, 0x15, 
  0x00, 0x00, 0x02, 0x01, 0x00, 0x00, 0x03, 0x03,
  0x00, 0x00, 0x0B, 0x0B, 0x00, 0x00, 0x04, 0x00,
  TAG_37_SLEEP_PARAMS_MIN_BCST_VAL_XO, TAG_37_SLEEP_PARAMS_PWR_ON_CNT_VAL_XO,
  0x30, 0x00, 0x00
};

// Marimba B0 w/ XO support disabled
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_37_clk_sharing_mba_b0[] = \
{
  0x18, 0x01, 0x25, 0x15, 
  0x00, 0x00, 0x02, 0x01, 0x01, 0x03, 0x03, 0x03,
  0x00, 0x0B, 0x0B, 0x0B, 0x00, 0x00, 0x04, 0x00,
  TAG_37_SLEEP_PARAMS_MIN_BCST_VAL, TAG_37_SLEEP_PARAMS_PWR_ON_CNT_VAL,
  0x02, 0x00, 0x00
};

// Marimba B0 w/ XO support enabled
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_37_clk_sharing_mba_b0_xo[] = \
{
  0x18, 0x01, 0x25, 0x15, 
  0x00, 0x00, 0x02, 0x01, 0x01, 0x03, 0x03, 0x03,
  0x00, 0x0B, 0x0B, 0x0B, 0x00, 0x00, 0x04, 0x00,
  TAG_37_SLEEP_PARAMS_MIN_BCST_VAL_XO, TAG_37_SLEEP_PARAMS_PWR_ON_CNT_VAL_XO,
  0x02, 0x00, 0x00
};

// 4025 w/ XO support disabled
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_37_clock_sharing_4025[] =
{
  0x19, 0x01, 0x25, 0x16, 
  0x00, 0x00, 0x02, 0x01, 0x01, 0x12, 0x1D, 0x00, 
  0x00, 0x12, 0x1D, 0x00, 0x00, 0x00, 0x00, 0x00, 
  TAG_37_SLEEP_PARAMS_MIN_BCST_VAL, TAG_37_SLEEP_PARAMS_PWR_ON_CNT_VAL,
  0x02, 0x00, 0x00, 0x00
};

// 4025 w/ XO support enabled
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_37_clock_sharing_4025_xo[] =
{
  0x19, 0x01, 0x25, 0x16, 
  0x00, 0x00, 0x02, 0x01, 0x01, 0x12, 0x1D, 0x00, 
  0x00, 0x12, 0x1D, 0x00, 0x00, 0x00, 0x00, 0x00, 
  TAG_37_SLEEP_PARAMS_MIN_BCST_VAL_XO, TAG_37_SLEEP_PARAMS_PWR_ON_CNT_VAL_XO,
  0x02, 0x00, 0x00, 0x00
};

// Marimba B1 w/ XO support disabled
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_37_clk_sharing_mba_b1[] = \
{
  0x18, 0x01, 0x25, 0x15, 
  0x00, 0x00, 0x02, 0x01, 0x01, 0x03, 0x03, 0x03,
  0x00, 0x0B, 0x0B, 0x0B, 0x00, 0x00, 0x04, 0x00,
  TAG_37_SLEEP_PARAMS_MIN_BCST_VAL, TAG_37_SLEEP_PARAMS_PWR_ON_CNT_VAL,
  0x02, 0x00, 0x00
};

// Marimba B1 w/ XO support enabled
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_37_clk_sharing_mba_b1_xo[] = \
{
  0x18, 0x01, 0x25, 0x15, 
  0x00, 0x00, 0x02, 0x01, 0x01, 0x03, 0x03, 0x03,
  0x00, 0x0B, 0x0B, 0x0B, 0x00, 0x00, 0x04, 0x00,
  TAG_37_SLEEP_PARAMS_MIN_BCST_VAL_XO, TAG_37_SLEEP_PARAMS_PWR_ON_CNT_VAL_XO,
  0x02, 0x00, 0x00
};

// Bahama A0
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_37_clk_sharing_bha_a0[] = \
{
  0x18, 0x01, 0x25, 0x15, 
  0x00, 0x00, 0x02, 0x02, 0x01, 0x03, 0x03, 0x03,
  0x00, 0x0B, 0x0B, 0x0B, 0x00, 0x00, 0x04, 0x00,
  0x80, 0x7E, 0x02, 0x00, 0x00
};

// Bahama B0
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_37_clk_sharing_bha_b0[] = \
{
  0x18, 0x01, 0x25, 0x15,
  0x00, 0x00, 0x02, 0x03, 0x01, 0x01, 0x03, 0x03,
  0x01, 0x01, 0x0B, 0x0B, 0x00, 0x00, 0x04, 0x00,
  0xA0, 0xA0, 0x02, 0x00, 0x00
};

// Bahama B0 With TCXO
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_37_clk_sharing_bha_b0_tcxo[] = \
{
  0x18, 0x01, 0x25, 0x15,
  0x00, 0x00, 0x02, 0x03, 0x01, 0x01, 0x03, 0x03,
  0x01, 0x01, 0x0B, 0x0B, 0x00, 0x00, 0x04, 0x00,
  TAG_37_SLEEP_PARAMS_MIN_BCST_VAL, TAG_37_SLEEP_PARAMS_PWR_ON_CNT_VAL,
  0x02, 0x00, 0x00
};
// 4025 B3 w/ XO support disabled
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_37_clock_sharing_4025_b3[] =
{
  0x19, 0x01, 0x25, 0x16, 
  0x00, 0x00, 0x02, 0x01, 0x01, 0x12, 0x1D, 0x00, 
  0x00, 0x12, 0x1D, 0x00, 0x00, 0x00, 0x00, 0x00, 
  TAG_37_SLEEP_PARAMS_MIN_BCST_VAL, TAG_37_SLEEP_PARAMS_PWR_ON_CNT_VAL,
  0x30, 0x00, 0x00, 0x00
};

// 4025 B3 w/ XO support enabled
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_37_clock_sharing_4025_b3_xo[] =
{
  0x19, 0x01, 0x25, 0x16, 
  0x00, 0x00, 0x02, 0x01, 0x01, 0x12, 0x1D, 0x00, 
  0x00, 0x12, 0x1D, 0x00, 0x00, 0x00, 0x00, 0x00, 
  TAG_37_SLEEP_PARAMS_MIN_BCST_VAL_XO, TAG_37_SLEEP_PARAMS_PWR_ON_CNT_VAL_XO,
  0x30, 0x00, 0x00, 0x00
};

#define CFG_TAG_37_CLK_SHARING_R3       \
                  (&bt_qsoc_cfg_tag_37_clock_sharing[0])
#define CFG_TAG_37_CLK_SHARING_R3_XO    \
                  (&bt_qsoc_cfg_tag_37_clock_sharing_xo[0])
#define CFG_TAG_37_CLK_SHARING_4020BD_B0    \
                  (&bt_qsoc_cfg_tag_37_clock_sharing[0])
#define CFG_TAG_37_CLK_SHARING_4020BD_B0_XO \
                  (&bt_qsoc_cfg_tag_37_clock_sharing_xo[0])
#define CFG_TAG_37_CLK_SHARING_4020BD_B1	  \
									(&bt_qsoc_cfg_tag_37_clock_sharing[0])
#define CFG_TAG_37_CLK_SHARING_4020BD_B1_XO \
                  (&bt_qsoc_cfg_tag_37_clock_sharing_xo[0])
#define CFG_TAG_37_CLK_SHARING_4021_B1      \
                  (&bt_qsoc_cfg_tag_37_clock_sharing[0])
#define CFG_TAG_37_CLK_SHARING_4021_B1_XO   \
                  (&bt_qsoc_cfg_tag_37_clock_sharing_xo[0])

#define CFG_TAG_37_CLK_SHARING_4025_A0    \
                                (&bt_qsoc_cfg_tag_37_clock_sharing_4025[0])
#define CFG_TAG_37_CLK_SHARING_4025_A0_XO \
                                (&bt_qsoc_cfg_tag_37_clock_sharing_4025_xo[0])
#define CFG_TAG_37_CLK_SHARING_4025_B0    \
                                (&bt_qsoc_cfg_tag_37_clock_sharing_4025[0]) 
#define CFG_TAG_37_CLK_SHARING_4025_B0_XO \
                                (&bt_qsoc_cfg_tag_37_clock_sharing_4025_xo[0])
#define CFG_TAG_37_CLK_SHARING_4025_B1    \
                                (&bt_qsoc_cfg_tag_37_clock_sharing_4025[0]) 
#define CFG_TAG_37_CLK_SHARING_4025_B1_XO \
                                (&bt_qsoc_cfg_tag_37_clock_sharing_4025_xo[0])
#define CFG_TAG_37_CLK_SHARING_4025_B2    \
                                (&bt_qsoc_cfg_tag_37_clock_sharing_4025[0]) 
#define CFG_TAG_37_CLK_SHARING_4025_B2_XO \
                                (&bt_qsoc_cfg_tag_37_clock_sharing_4025_xo[0])
#define CFG_TAG_37_CLK_SHARING_4025_B3    \
                                (&bt_qsoc_cfg_tag_37_clock_sharing_4025_b3[0]) 
#define CFG_TAG_37_CLK_SHARING_4025_B3_XO \
                                (&bt_qsoc_cfg_tag_37_clock_sharing_4025_b3_xo[0])

#define CFG_TAG_37_CLK_SHARING_MBA_A0     \
                                (&bt_qsoc_cfg_tag_37_clk_sharing_mba_a0[0]) 
#define CFG_TAG_37_CLK_SHARING_MBA_A0_XO  \
                                (&bt_qsoc_cfg_tag_37_clk_sharing_mba_a0_xo[0])
#define CFG_TAG_37_CLK_SHARING_MBA_B0     \
                                (&bt_qsoc_cfg_tag_37_clk_sharing_mba_b0[0])
#define CFG_TAG_37_CLK_SHARING_MBA_B0_XO  \
                                (&bt_qsoc_cfg_tag_37_clk_sharing_mba_b0_xo[0]) 
#define CFG_TAG_37_CLK_SHARING_MBA_B1     \
                                (&bt_qsoc_cfg_tag_37_clk_sharing_mba_b1[0]) 
#define CFG_TAG_37_CLK_SHARING_MBA_B1_XO  \
                                (&bt_qsoc_cfg_tag_37_clk_sharing_mba_b1_xo[0]) 

#define CFG_TAG_37_CLK_SHARING_BHA_A0     \
                                (&bt_qsoc_cfg_tag_37_clk_sharing_bha_a0[0])
#define CFG_TAG_37_CLK_SHARING_BHA_B0     \
                                (&bt_qsoc_cfg_tag_37_clk_sharing_bha_b0[0])
#define CFG_TAG_37_CLK_SHARING_BHA_B0_TCXO     \
                                (&bt_qsoc_cfg_tag_37_clk_sharing_bha_b0_tcxo[0])

#define CFG_TAG_37_CLK_SHARING_UNKNOWN    (&bt_qsoc_cfg_tag_37_clock_sharing[0])
#define CFG_TAG_37_CLK_SHARING_UNKNOWN_XO \
                                (&bt_qsoc_cfg_tag_37_clock_sharing_xo[0])

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_37_no_clock_sharing[] = \
{
  0x19, 0x01, 0x25, 0x16, 
  0x00, 0x00, 0x02, 0x01, 0x01, 0x12, 0x01, 0x00,
  0x00, 0x12, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x34, 0x32, 0x30, 0x00, 0x00, 0x00
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_37_no_clock_sharing_4025[] = \
{
  0x19, 0x01, 0x25, 0x16, 
  0x00, 0x00, 0x02, 0x01, 0x01, 0x12, 0x1D, 0x00,
  0x00, 0x12, 0x1D, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x34, 0x32, 0x30, 0x00, 0x00, 0x00
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_37_no_clk_sharing_mba_a0[] = \
{
  0x18, 0x01, 0x25, 0x15, 
  0x00, 0x00, 0x02, 0x01, 0x01, 0x03, 0x03, 0x03,
  0x00, 0x03, 0x0B, 0x0B, 0x00, 0x00, 0x04, 0x00,
  0x34, 0x32, 0x30, 0x00, 0x00
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_37_no_clk_sharing_bha_a0[] = \
{
  0x18, 0x01, 0x25, 0x15, 
  0x00, 0x00, 0x02, 0x02, 0x01, 0x03, 0x03, 0x03,
  0x00, 0x03, 0x0B, 0x0B, 0x00, 0x00, 0x04, 0x00,
  0x80, 0x7E, 0x02, 0x00, 0x00
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_37_no_clk_sharing_bha_b0[] = \
{
  0x18, 0x01, 0x25, 0x15,
  0x00, 0x00, 0x02, 0x03, 0x01, 0x01, 0x03, 0x03,
  0x01, 0x01, 0x0B, 0x0B, 0x00, 0x00, 0x04, 0x00,
  0x80, 0x7E, 0x02, 0x00, 0x00
};

#define CFG_TAG_37_NO_CLK_SHARING_R3        (&bt_qsoc_cfg_tag_37_no_clock_sharing[0]) 
#define CFG_TAG_37_NO_CLK_SHARING_4020BD_B0 (&bt_qsoc_cfg_tag_37_no_clock_sharing[0]) 
#define CFG_TAG_37_NO_CLK_SHARING_4020BD_B1 (&bt_qsoc_cfg_tag_37_no_clock_sharing[0]) 
#define CFG_TAG_37_NO_CLK_SHARING_4021_B1   (&bt_qsoc_cfg_tag_37_no_clock_sharing[0]) 
#define CFG_TAG_37_NO_CLK_SHARING_4025_A0 \
                                       (&bt_qsoc_cfg_tag_37_no_clock_sharing_4025[0]) 
#define CFG_TAG_37_NO_CLK_SHARING_4025_B0 \
                                       (&bt_qsoc_cfg_tag_37_no_clock_sharing_4025[0]) 
#define CFG_TAG_37_NO_CLK_SHARING_4025_B1 \
                                       (&bt_qsoc_cfg_tag_37_no_clock_sharing_4025[0]) 
#define CFG_TAG_37_NO_CLK_SHARING_4025_B2 \
                                       (&bt_qsoc_cfg_tag_37_no_clock_sharing_4025[0]) 
#define CFG_TAG_37_NO_CLK_SHARING_4025_B3 \
                                       (&bt_qsoc_cfg_tag_37_no_clock_sharing_4025[0]) 
#define CFG_TAG_37_NO_CLK_SHARING_MBA_A0 \
                                       (&bt_qsoc_cfg_tag_37_no_clk_sharing_mba_a0[0]) 
#define CFG_TAG_37_NO_CLK_SHARING_BHA_A0 \
                                       (&bt_qsoc_cfg_tag_37_no_clk_sharing_bha_a0[0])
#define CFG_TAG_37_NO_CLK_SHARING_BHA_B0 \
                                       (&bt_qsoc_cfg_tag_37_no_clk_sharing_bha_b0[0])
#define CFG_TAG_37_NO_CLK_SHARING_UNKNOWN   (&bt_qsoc_cfg_tag_37_no_clock_sharing[0]) 

/* #38 Debug control - Disable logger output 
* logger buffer - 300 bytes used only when logger is enabled 
*/
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_38_soc_logging[] = \
{
/* Turn on only for BT2.1 and Surf.
    Targets that have common FFA and SURF builds shouldn't turn it on */
  0x07, 0x01, 0x26, 0x04, 
  0x02, 0x00, 0x2C, 0x01
};


static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_38_soc_logging_marimba_a0[] = \
{
/* Turn on only for BT2.1 and Surf.
    Targets that have common FFA and SURF builds shouldn't turn it on */
  0x0C, 0x01, 0x26, 0x09, 
  0x02, 0x00, 0x64, 0x00, 0x2C, 0x05, 0x00, 0x1F, 0x20
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_38_soc_logging_marimba_b0[] = \
{
/* Turn on only for BT2.1 and Surf.
    Targets that have common FFA and SURF builds shouldn't turn it on */
  0x0C, 0x01, 0x26, 0x09, 
  0x02, 0x00, 0x64, 0x00, 0x2C, 0x05, 0x00, 0x1F, 0x20
};
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_38_soc_logging_marimba_b1[] = \
{
/* Turn on only for BT2.1 and Surf.
    Targets that have common FFA and SURF builds shouldn't turn it on */
  0x0C, 0x01, 0x26, 0x09, 
  0x02, 0x00, 0x64, 0x00, 0x2C, 0x05, 0x00, 0x1F, 0x20
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_38_soc_logging_bha_a0[] = \
{
  0x0C, 0x01, 0x26, 0x09, 
  0x02, 0x00, 0x64, 0x00, 0x2C, 0x05, 0x00, 0x1F, 0x20
};

#define CFG_TAG_38_SOC_LOGGING_R3        (&bt_qsoc_cfg_tag_38_soc_logging[0])
#define CFG_TAG_38_SOC_LOGGING_4020BD_B0 (&bt_qsoc_cfg_tag_38_soc_logging[0])
#define CFG_TAG_38_SOC_LOGGING_4020BD_B1 (&bt_qsoc_cfg_tag_38_soc_logging[0])
#define CFG_TAG_38_SOC_LOGGING_4021_B1   (&bt_qsoc_cfg_tag_38_soc_logging[0])
#define CFG_TAG_38_SOC_LOGGING_4025_A0   (&bt_qsoc_cfg_tag_38_soc_logging[0])
#define CFG_TAG_38_SOC_LOGGING_4025_B0   (&bt_qsoc_cfg_tag_38_soc_logging[0])
#define CFG_TAG_38_SOC_LOGGING_4025_B1   (&bt_qsoc_cfg_tag_38_soc_logging[0])
#define CFG_TAG_38_SOC_LOGGING_4025_B2   (&bt_qsoc_cfg_tag_38_soc_logging[0])
#define CFG_TAG_38_SOC_LOGGING_4025_B3   (&bt_qsoc_cfg_tag_38_soc_logging[0])
#define CFG_TAG_38_SOC_LOGGING_MBA_A0    (&bt_qsoc_cfg_tag_38_soc_logging_marimba_a0[0])
#define CFG_TAG_38_SOC_LOGGING_MBA_B0    (&bt_qsoc_cfg_tag_38_soc_logging_marimba_b0[0])
#define CFG_TAG_38_SOC_LOGGING_MBA_B1    (&bt_qsoc_cfg_tag_38_soc_logging_marimba_b1[0])
#define CFG_TAG_38_SOC_LOGGING_BHA_A0    (&bt_qsoc_cfg_tag_38_soc_logging_bha_a0[0])
#define CFG_TAG_38_SOC_LOGGING_BHA_B0    (&bt_qsoc_cfg_tag_38_soc_logging_bha_a0[0])
#define CFG_TAG_38_SOC_LOGGING_UNKNOWN   (&bt_qsoc_cfg_tag_38_soc_logging[0])

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_38_soc_no_logging[] = \
{
  0x07, 0x01, 0x26, 0x04, 
  0x00, 0x00, 0x2C, 0x01
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_38_soc_no_logging_bha_a0[] = \
{
  0x0C, 0x01, 0x26, 0x09, 
  0x00, 0x00, 0x2C, 0x01, 0x2C, 0x05, 0x00, 0x1F, 0x00
};

#define CFG_TAG_38_SOC_NO_LOGGING_R3        (&bt_qsoc_cfg_tag_38_soc_no_logging[0])
#define CFG_TAG_38_SOC_NO_LOGGING_4020BD_B0 (&bt_qsoc_cfg_tag_38_soc_no_logging[0])
#define CFG_TAG_38_SOC_NO_LOGGING_4020BD_B1 (&bt_qsoc_cfg_tag_38_soc_no_logging[0])
#define CFG_TAG_38_SOC_NO_LOGGING_4021_B1   (&bt_qsoc_cfg_tag_38_soc_no_logging[0])
#define CFG_TAG_38_SOC_NO_LOGGING_4025_A0   (&bt_qsoc_cfg_tag_38_soc_no_logging[0])
#define CFG_TAG_38_SOC_NO_LOGGING_4025_B0   (&bt_qsoc_cfg_tag_38_soc_no_logging[0])
#define CFG_TAG_38_SOC_NO_LOGGING_4025_B1   (&bt_qsoc_cfg_tag_38_soc_no_logging[0])
#define CFG_TAG_38_SOC_NO_LOGGING_4025_B2   (&bt_qsoc_cfg_tag_38_soc_no_logging[0])
#define CFG_TAG_38_SOC_NO_LOGGING_4025_B3   (&bt_qsoc_cfg_tag_38_soc_no_logging[0])
#define CFG_TAG_38_SOC_NO_LOGGING_MBA_A0    (&bt_qsoc_cfg_tag_38_soc_no_logging[0])
#define CFG_TAG_38_SOC_NO_LOGGING_MBA_B0    (&bt_qsoc_cfg_tag_38_soc_no_logging[0])
#define CFG_TAG_38_SOC_NO_LOGGING_MBA_B1    (&bt_qsoc_cfg_tag_38_soc_no_logging[0])
#define CFG_TAG_38_SOC_NO_LOGGING_BHA_A0    (&bt_qsoc_cfg_tag_38_soc_no_logging_bha_a0[0])
#define CFG_TAG_38_SOC_NO_LOGGING_BHA_B0    (&bt_qsoc_cfg_tag_38_soc_no_logging_bha_a0[0])
#define CFG_TAG_38_SOC_NO_LOGGING_UNKNOWN   (&bt_qsoc_cfg_tag_38_soc_no_logging[0])

/* #39 RM and LC Override Enable - Optimize RM register settings 
 * Enables RM register override 
 */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_39_4025_a0[] = \
{
  0x07, 0x01, 0x27, 0x04, 
  0x13, 0x01, 0x00, 0x00
};

#define CFG_TAG_39_4025_A0           (&bt_qsoc_cfg_tag_39_4025_a0[0])

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_39_riva[] = \
{
  0x07, 0x01, 0x27, 0x04,
  0x72, 0x00, 0x00, 0x00
};

/* #40 LC Burst Timer Override - Overrides the burst timers for each
 * burst.
 */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_40[] = \
{
  0x15, 0x01, 0x28, 0x12, 
  0x32, 0x05, 0x67, 0x0D, 0x42, 0x12, 0x43, 0x13,
  0x1C, 0x0C, 0x17, 0x10, 0x59, 0x13, 0x8D, 0x04, 
  0x3D, 0x08
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_40_4025_b0[] = \
{
  0x15, 0x01, 0x28, 0x12, 
  0x32, 0x05, 0x07, 0x0D, 0xE2, 0x11, 0x43, 0x13,
  0x1C, 0x0C, 0x17, 0x10, 0x59, 0x13, 0x8D, 0x04,
  0x3D, 0x08 
};
#define CFG_TAG_40_4025_A0   (&bt_qsoc_cfg_tag_40[0])
#define CFG_TAG_40_4025_B0   (&bt_qsoc_cfg_tag_40_4025_b0[0])

 
/* #41 Radio Register Override - Configures values for RM registers in
* Addr/Value pairs. 
*/
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_41_19p2MHz_4025_a0[] = \
{
  0x29, 0x01, 0x29, 0x26, 
  0x11, 0x00, 0x00, 0x00, 0x1E, 0x1E, 0x20, 0x62,
  0x41, 0x1A, 0x5A, 0x07, 0xA4, 0x65, 0xA5, 0x32,
  0xB2, 0x02, 0xB3, 0x02, 0xB4, 0x02, 0xB5, 0x01, 
  0xB6, 0x01, 0xB7, 0x01, 0xB8, 0x00, 0xB9, 0x00, 
  0x76, 0x03, 0xAD, 0x04, 0xC6, 0x3D 
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_41_32MHz_4025_a0[] = \
{
  0x29, 0x01, 0x29, 0x26, 
  0x11, 0x00, 0x00, 0x00, 0x1E, 0x1E, 0x20, 0x62,
  0x41, 0x12, 0x5A, 0x07, 0xA4, 0xC8, 0xA5, 0x42,
  0xB2, 0x02, 0xB3, 0x02, 0xB4, 0x02, 0xB5, 0x01, 
  0xB6, 0x01, 0xB7, 0x01, 0xB8, 0x00, 0xB9, 0x00, 
  0x76, 0x03, 0xAD, 0x04, 0xC6, 0x3D 
};

#define CFG_TAG_41_19P2MHZ_4025_A0   (&bt_qsoc_cfg_tag_41_19p2MHz_4025_a0[0]) 

#define CFG_TAG_41_32MHZ_4025_A0     (&bt_qsoc_cfg_tag_41_32MHz_4025_a0[0]) 

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_41_19p2MHz_4020_bd_b1[] = \
{
  0x3D, 0x01, 0x29, 0x3A, 
  0x1B, 0x00, 0x00, 0x00, 0x06, 0x1A, 0x3D, 0xFB,
  0x3E, 0x0F, 0x41, 0x1D, 0x42, 0x00, 0x43, 0xCC, 
  0x49, 0xF3, 0x4A, 0xF5, 0x5D, 0x06, 0x5E, 0x06,
  0x5F, 0x03, 0x64, 0x01, 0x72, 0x20, 0x75, 0x01,
  0x92, 0x00, 0x93, 0x00, 0x9C, 0x03, 0x1C, 0xF6,
  0x1D, 0x06, 0x20, 0x62, 0x1E, 0x0F, 0xA6, 0xAA,
  0xA7, 0x55, 0xA8, 0x15, 0xA4, 0x65, 0xA5, 0x32,
  0x30, 0x07 
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_41_32MHz_4020_bd_b1[] = \
{
  0x3B, 0x01, 0x29, 0x38, 
  0x1A, 0x00, 0x00, 0x00, 0x06, 0x1A, 0x3D, 0xFB,
  0x3E, 0x0F, 0x41, 0x15, 0x42, 0x00, 0x43, 0xCC,
  0x49, 0xF4, 0x4A, 0xF5, 0x5D, 0x06, 0x5E, 0x06,
  0x5F, 0x03, 0x64, 0x01, 0x72, 0x20, 0x75, 0x01,
  0x92, 0x02, 0x9C, 0x00, 0x1C, 0xF6, 0x1D, 0x06,
  0x20, 0x62, 0x1E, 0x0F, 0xA6, 0xAA, 0xA7, 0x55,
  0xA8, 0x15, 0xA4, 0xC8, 0xA5, 0x42, 0x30, 0x07
};

#define CFG_TAG_41_19P2MHZ_4020_BD_B1   (&bt_qsoc_cfg_tag_41_19p2MHz_4020_bd_b1[0]) 
#define CFG_TAG_41_32MHZ_4020_BD_B1     (&bt_qsoc_cfg_tag_41_32MHz_4020_bd_b1[0]) 

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_41_riva[] = \
{
  0x7F, 0x01, 0x29, 0x7C,
  0x3C, 0x00, 0x00, 0x00, 0x5C, 0x58, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00
};

/* #44 Codec configuration - Codec interface configuration */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_44[] = \
{
  0x0D, 0x01, 0x2C, 0x0A, 
  0x10, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00,
  0x0F, 0x0F
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_44_bha_a0[] = \
{
  0x2B, 0x01, 0x2C, 0x28,
  0x0F, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50,
  0xFF, 0x10, 0x02, 0x02, 0x01, 0x00, 0x02, 0x01,
  0x12, 0x28, 0xA0, 0x62, 0x03, 0x64, 0x01, 0x01,
  0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0,
  0xFF, 0x10, 0x02, 0x01, 0x00, 0x05, 0x01, 0x26,
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_44_bha_b0[] = \
{
  0x2C, 0x01, 0x2C, 0x29,
  0x2F, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50,
  0xFF, 0x10, 0x02, 0x02, 0x01, 0x00, 0x14, 0x01,
  0x06, 0x28, 0xA0, 0x62, 0x03, 0x64, 0x01, 0x01,
  0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0,
  0xFF, 0x10, 0x02, 0x01, 0x00, 0x14, 0x01, 0x02,
  0x03
};

#define CFG_TAG_44_R3        (&bt_qsoc_cfg_tag_44[0]) 
#define CFG_TAG_44_4020BD_B0 (&bt_qsoc_cfg_tag_44[0]) 
#define CFG_TAG_44_4020BD_B1 (&bt_qsoc_cfg_tag_44[0]) 
#define CFG_TAG_44_4021_B1   (&bt_qsoc_cfg_tag_44[0]) 
#define CFG_TAG_44_4025_A0   (&bt_qsoc_cfg_tag_44[0]) 
#define CFG_TAG_44_4025_B0   (&bt_qsoc_cfg_tag_44[0]) 
#define CFG_TAG_44_4025_B1   (&bt_qsoc_cfg_tag_44[0]) 
#define CFG_TAG_44_4025_B2   (&bt_qsoc_cfg_tag_44[0])
#define CFG_TAG_44_4025_B3   (&bt_qsoc_cfg_tag_44[0])
#define CFG_TAG_44_MBA_A0    (&bt_qsoc_cfg_tag_44[0]) 
#define CFG_TAG_44_MBA_B0    (&bt_qsoc_cfg_tag_44[0]) 
#define CFG_TAG_44_MBA_B1    (&bt_qsoc_cfg_tag_44[0])
#define CFG_TAG_44_BHA_A0    (&bt_qsoc_cfg_tag_44_bha_a0[0])
#define CFG_TAG_44_BHA_B0    (&bt_qsoc_cfg_tag_44_bha_b0[0])
#define CFG_TAG_44_UNKNOWN   (&bt_qsoc_cfg_tag_44[0]) 


/* #45 CVSD Gain Volume Setting -  */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_45[] = \
{
  0x09, 0x01, 0x2D, 0x06, 
  0x02, 0x00, 0x00, 0x00, 0x0F, 0x0F
};
#define CFG_TAG_45_R3        (&bt_qsoc_cfg_tag_45[0]) 
#define CFG_TAG_45_4020BD_B0 (&bt_qsoc_cfg_tag_45[0]) 
#define CFG_TAG_45_4020BD_B1 (&bt_qsoc_cfg_tag_45[0]) 
#define CFG_TAG_45_4021_B1   (&bt_qsoc_cfg_tag_45[0]) 
#define CFG_TAG_45_4025_A0   (&bt_qsoc_cfg_tag_45[0]) 
#define CFG_TAG_45_4025_B0   (&bt_qsoc_cfg_tag_45[0]) 
#define CFG_TAG_45_4025_B1   (&bt_qsoc_cfg_tag_45[0]) 
#define CFG_TAG_45_4025_B2   (&bt_qsoc_cfg_tag_45[0])
#define CFG_TAG_45_4025_B3   (&bt_qsoc_cfg_tag_45[0])
#define CFG_TAG_45_MBA_A0    (&bt_qsoc_cfg_tag_45[0]) 
#define CFG_TAG_45_MBA_B0    (&bt_qsoc_cfg_tag_45[0]) 
#define CFG_TAG_45_MBA_B1    (&bt_qsoc_cfg_tag_45[0]) 
#define CFG_TAG_45_UNKNOWN   (&bt_qsoc_cfg_tag_45[0]) 

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_45_riva[] = \
{
  0x7F, 0x01, 0x2D, 0x7C,
  0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC5, 0x00,
  0xCA, 0x0A, 0xF5, 0x01, 0xF6, 0x00, 0x64, 0x38,
  0x65, 0x1C, 0xB4, 0x07, 0xB6, 0x2C, 0xB7, 0x08,
  0xB9, 0x08, 0x06, 0x1F, 0x0A, 0x12, 0x26, 0x02,
  0x27, 0x12, 0x2A, 0x0C, 0x2F, 0x1F, 0x30, 0x1F,
  0x31, 0x1F, 0x32, 0x1F, 0x2B, 0x07, 0x2D, 0x04,
  0x39, 0xC4, 0x3A, 0xCF, 0x3B, 0xFF, 0x6F, 0x1F,
  0x92, 0x03, 0x93, 0x23, 0x97, 0x21, 0x9B, 0x10,
  0x9C, 0x03, 0x9D, 0x88, 0xA0, 0x40, 0xA1, 0x01,
  0xA2, 0x40, 0xA3, 0x40, 0xA4, 0x21, 0xAB, 0x36,
  0xAD, 0xF1, 0xF1, 0x66, 0x07, 0x04, 0x0B, 0x05,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00
};

/* #46 Voice Settings -  */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_46[] = \
{
  0x05, 0x01, 0x2E, 0x02, 
  0x40, 0x00
};
#define CFG_TAG_46_R3        (&bt_qsoc_cfg_tag_46[0]) 
#define CFG_TAG_46_4020BD_B0 (&bt_qsoc_cfg_tag_46[0]) 
#define CFG_TAG_46_4020BD_B1 (&bt_qsoc_cfg_tag_46[0]) 
#define CFG_TAG_46_4021_B1   (&bt_qsoc_cfg_tag_46[0]) 
#define CFG_TAG_46_4025_A0   (&bt_qsoc_cfg_tag_46[0]) 
#define CFG_TAG_46_4025_B0   (&bt_qsoc_cfg_tag_46[0]) 
#define CFG_TAG_46_4025_B1   (&bt_qsoc_cfg_tag_46[0]) 
#define CFG_TAG_46_4025_B2   (&bt_qsoc_cfg_tag_46[0])
#define CFG_TAG_46_4025_B3   (&bt_qsoc_cfg_tag_46[0]) 
#define CFG_TAG_46_MBA_A0    (&bt_qsoc_cfg_tag_46[0]) 
#define CFG_TAG_46_MBA_B0    (&bt_qsoc_cfg_tag_46[0]) 
#define CFG_TAG_46_MBA_B1    (&bt_qsoc_cfg_tag_46[0]) 
#define CFG_TAG_46_UNKNOWN   (&bt_qsoc_cfg_tag_46[0]) 


/* Tag 53 for WLAN Coex */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_53[] = \
{
#ifdef FEATURE_BT_WLAN_COEXISTENCE
  0x0B, 0x01, 0x35, 0x08, 
  0x02, 0x08, 0x70, 0x80, 0x04, 0x00, 0x08, 0x00
#else
  0x0B, 0x01, 0x35, 0x08, 
  0x00, 0x00, 0x70, 0x80, 0x04, 0x00, 0x08, 0x00 
#endif /* FEATURE_BT_WLAN_COEXISTENCE */
};

//Overriding BT_ACTIVE duration for all Marimba (ROM default: 10us too short).
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_53_mba[] = \
{
#ifdef FEATURE_BT_WLAN_COEXISTENCE
  0x0C, 0x01, 0x35, 0x09,
  0x02, 0x08, 0x70, 0x80, 0x04, 0x00, 0x08, 0x00, 0x0A
#else
  0x0C, 0x01, 0x35, 0x09, 
  0x00, 0x00, 0x70, 0x80, 0x04, 0x00, 0x08, 0x00, 0x00 
#endif /* FEATURE_BT_WLAN_COEXISTENCE */
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_53_bha[] = \
{
#ifdef FEATURE_BT_WLAN_COEXISTENCE
  0x11, 0x01, 0x35, 0x0E,
  0x02, 0x00, 0x80, 0x04, 0x00, 0x08, 0x00, 0x0A, 0x00,
  0x00, 0x20, 0x03, 0x70, 0x00 
#else
  0x11, 0x01, 0x35, 0x0E, 
  0x00, 0x00, 0x80, 0x04, 0x00, 0x08, 0x00, 0x00, 0x00,
  0x00, 0x20, 0x03, 0x70, 0x00  
#endif /* FEATURE_BT_WLAN_COEXISTENCE */
};

#define CFG_TAG_53_R3        (&bt_qsoc_cfg_tag_53[0]) 
#define CFG_TAG_53_4020BD_B0 (&bt_qsoc_cfg_tag_53[0]) 
#define CFG_TAG_53_4020BD_B1 (&bt_qsoc_cfg_tag_53[0]) 
#define CFG_TAG_53_4021_B1   (&bt_qsoc_cfg_tag_53[0])  
#define CFG_TAG_53_4025_A0   (&bt_qsoc_cfg_tag_53[0])  
#define CFG_TAG_53_4025_B0   (&bt_qsoc_cfg_tag_53[0])  
#define CFG_TAG_53_4025_B1   (&bt_qsoc_cfg_tag_53[0])  
#define CFG_TAG_53_4025_B2   (&bt_qsoc_cfg_tag_53[0])
#define CFG_TAG_53_4025_B3   (&bt_qsoc_cfg_tag_53[0])
#define CFG_TAG_53_MBA_A0    (&bt_qsoc_cfg_tag_53_mba[0]) 
#define CFG_TAG_53_MBA_B0    (&bt_qsoc_cfg_tag_53_mba[0]) 
#define CFG_TAG_53_MBA_B1    (&bt_qsoc_cfg_tag_53_mba[0]) 
#define CFG_TAG_53_BHA_A0    (&bt_qsoc_cfg_tag_53_bha[0])
#define CFG_TAG_53_BHA_B0    (&bt_qsoc_cfg_tag_53_bha[0])
#define CFG_TAG_53_UNKNOWN   (&bt_qsoc_cfg_tag_53[0]) 

/* Tag 55 - GPIOs for Sleep, Atheros Coex */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_55[] = \
{
  0x27, 0x01, 0x37, 0x24, 

#if defined( FEATURE_BT_QSOC_SLEEP ) && !defined( FEATURE_BT_QSOC_INBAND_SLEEP )
  0x04, 0x15, /* HOST_WAKE (4) => GPIO #21 */
#else
  0xFF, 0x00, /* No Function */
#endif /* FEATURE_BT_QSOC_SLEEP && !FEATURE_BT_QSOC_INBAND_SLEEP */

#ifdef FEATURE_BT_WLAN_COEXISTENCE
  0x09, 0x02, /* PTA_RF_Active(9) => GPIO #2 */
  0x0A, 0x01, /* PTA_Status(10) => GPIO #1 */
  0x0C, 0x00, /* PTA_TxConfx(12) => GPIO #0 */
#else
  0xFF, 0x00, /* No Function */
  0xFF, 0x00, /* No Function */
  0xFF, 0x00, /* No Function */
#endif /* FEATURE_BT_WLAN_COEXISTENCE */
  /* No Function */
  0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 
  0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 
  0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 
  0xFF, 0x00, 0xFF, 0x00
};

/* Tag 55 - GPIOs for Sleep, Atheros Coex for BTS 4025 
 * These mappings are valid for UART and in-band sleep UART.  
 * If support for H5 (3-wire UART) is ever added to the host, a different mapping will be required. 
 * */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_55_4025[] = \
{
  0x27, 0x01, 0x37, 0x24, 

#if defined( FEATURE_BT_QSOC_SLEEP ) && !defined( FEATURE_BT_QSOC_INBAND_SLEEP )
  0x04, 0x15, /* HOST_WAKE (4) => GPIO #21 */
#else
  0xFF, 0x00, /* No Function */
#endif /* FEATURE_BT_QSOC_SLEEP && !FEATURE_BT_QSOC_INBAND_SLEEP */

#ifdef FEATURE_BT_WLAN_COEXISTENCE
  0x09, 0x04, 
  0x0A, 0x03,
  0x0C, 0x01A,
#else
  0xFF, 0x00, /* No Function */
  0xFF, 0x00, /* No Function */
  0xFF, 0x00, /* No Function */
#endif /* FEATURE_BT_WLAN_COEXISTENCE */
  /* No Function */
  0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 
  0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 
  0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 
  0xFF, 0x00, 0xFF, 0x00
};

#define CFG_TAG_55_R3        (&bt_qsoc_cfg_tag_55[0]) 
#define CFG_TAG_55_4020BD_B0 (&bt_qsoc_cfg_tag_55[0]) 
#define CFG_TAG_55_4020BD_B1 (&bt_qsoc_cfg_tag_55[0]) 
#define CFG_TAG_55_4021_B1   (&bt_qsoc_cfg_tag_55[0]) 
#define CFG_TAG_55_4025_A0   (&bt_qsoc_cfg_tag_55_4025[0]) 
#define CFG_TAG_55_4025_B0   (&bt_qsoc_cfg_tag_55_4025[0]) 
#define CFG_TAG_55_4025_B1   (&bt_qsoc_cfg_tag_55_4025[0]) 
#define CFG_TAG_55_4025_B2   (&bt_qsoc_cfg_tag_55_4025[0])
#define CFG_TAG_55_4025_B3   (&bt_qsoc_cfg_tag_55_4025[0]) 
#define CFG_TAG_55_UNKNOWN   (&bt_qsoc_cfg_tag_55[0]) 


/* #57 - AFH parameters */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_57[] = \
{
  0x20, 0x01, 0x39, 0x1D, 
  0x02, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x40,
  0x00, 0x00, 0x0A, 0x05, 0x13, 0x4F, 0x02, 0x08,
  0x01, 0x0C, 0x01, 0x05, 0x01
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_57_non_libra[] = \
{
  0x24, 0x01, 0x39, 0x21, 
  0x02, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x40,
  0x00, 0x00, 0x0A, 0x05, 0x13, 0x4F, 0x02, 0x08,
  0x01, 0x0C, 0x01, 0x05, 0x01, 0x01, 0x00, 0x01, 
  0x01
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_57_bha_default[] = \
{
  0x26, 0x01, 0x39, 0x23,
  0x02, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x40,
  0x00, 0x00, 0x0A, 0x05, 0x13, 0x4F, 0x02, 0x08,
  0x01, 0x0C, 0x01, 0x05, 0x01, 0x01, 0x00, 0x01,
  0x01, 0x01, 0x01
};

/* Libra/Volans specific AFH configuration */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_57_mba_libra[] = \
{
  0x24, 0x01, 0x39, 0x21,
  0x03, 0x00, 0x3C, 0x00, 0x14, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x0A, 0x05, 0x13, 0x4F, 0x01, 0x08,
  0x01, 0x10, 0x03, 0x05, 0x01, 0x01, 0x00, 0x01,
  0x01
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_57_4025_libra[] = \
{
  0x24, 0x01, 0x39, 0x21,
  0x03, 0x00, 0x3C, 0x00, 0x14, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x0A, 0x05, 0x13, 0x4F, 0x01, 0x08,
  0x01, 0x10, 0x03, 0x05, 0x01, 0x01, 0x00, 0x00,
  0x01
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_57_bha[] = \
{
  0x26, 0x01, 0x39, 0x23,
  0x03, 0x00, 0x3C, 0x00, 0x14, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x0A, 0x05, 0x13, 0x4F, 0x01, 0x08,
  0x01, 0x10, 0x03, 0x05, 0x01, 0x01, 0x00, 0x01,
  0x01, 0x01, 0x01 
};

#define CFG_TAG_57_R3        (&bt_qsoc_cfg_tag_57[0]) 
#define CFG_TAG_57_4020BD_B0 (&bt_qsoc_cfg_tag_57[0]) 
#define CFG_TAG_57_4020BD_B1 (&bt_qsoc_cfg_tag_57[0]) 
#define CFG_TAG_57_4021_B1   (&bt_qsoc_cfg_tag_57[0]) 
#define CFG_TAG_57_4025_A0   (&bt_qsoc_cfg_tag_57[0]) 
#define CFG_TAG_57_UNKNOWN   (&bt_qsoc_cfg_tag_57[0]) 

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_63_bha[] = \
{
  0x30, 0x01, 0x3F, 0x2D,
  0x05, 0x40, 0x0B, 0x01, 0x00, 0x07, 0x07, 0x01,
  0x06, 0x06, 0x06, 0x01, 0x02, 0x02, 0x02, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x06, 0x03, 0x06, 0x02, 0x03,
  0x02, 0x00, 0x02, 0x02, 0x06, 0x01, 0x01, 0x04,
  0x04, 0x00, 0x05, 0x01, 0x01
};

/* #70 Internal IO Resistor Settings - Configures the internal 
 * pullup/pulldown resistor settings for external I/O pins on the BTS4020 */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_70[] = \
{
  0x0F, 0x01, 0x46, 0x0C, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00
};
#define CFG_TAG_70_R3        (&bt_qsoc_cfg_tag_70[0]) 
#define CFG_TAG_70_4020BD_B0 (&bt_qsoc_cfg_tag_70[0]) 
#define CFG_TAG_70_4020BD_B1 (&bt_qsoc_cfg_tag_70[0]) 
#define CFG_TAG_70_4021_B1   (&bt_qsoc_cfg_tag_70[0]) 
#define CFG_TAG_70_4025_A0   (&bt_qsoc_cfg_tag_70[0]) 
#define CFG_TAG_70_4025_B0   (&bt_qsoc_cfg_tag_70[0]) 
#define CFG_TAG_70_4025_B1   (&bt_qsoc_cfg_tag_70[0]) 
#define CFG_TAG_70_4025_B2   (&bt_qsoc_cfg_tag_70[0])
#define CFG_TAG_70_4025_B3   (&bt_qsoc_cfg_tag_70[0])
#define CFG_TAG_70_MBA_A0    (&bt_qsoc_cfg_tag_70[0]) 
#define CFG_TAG_70_MBA_B0    (&bt_qsoc_cfg_tag_70[0]) 
#define CFG_TAG_70_MBA_B1    (&bt_qsoc_cfg_tag_70[0]) 
#define CFG_TAG_70_UNKNOWN   (&bt_qsoc_cfg_tag_70[0]) 

/* #72 - HCI supported override */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_72[] = \
{
  0x44, 0x01, 0x48, 0x41,
  0X40, /* Length is a part of payload */ 
  0xFF, 0xFF, 0xFF, 0xFF, 0xCF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_72_4025_bt_2_0[] = \
{
  0x19, 0x01, 0x48, 0x16,   
  0x15, 0xFF, 0xFF, 0xFF, 0x03, 0xCE, 0xFF, 0xEF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0xF2, 0x0F, 0xE8, 
  0xFE, 0x3F, 0x00, 0x00, 0x00, 0x00
};
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_72_4020_bd_bt_2_0[] = \
{
  0x19, 0x01, 0x48, 0x16,
  0x15, 0xFF, 0xFF, 0xFF, 0x03, 0xCE, 0xFF, 0xEF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0xF2, 0x0F, 0xE8, 
  0xFE, 0x3F, 0x00, 0x00, 0x00, 0x00      
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_72_4020_bd_b1_bt_2_1[] = \
{
  0x19, 0x01, 0x48, 0x16,
  0x15, 0xFF, 0xFF, 0xFF, 0xFF, 0xCF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF 
};

#define CFG_TAG_72_4025_A0   (&bt_qsoc_cfg_tag_72[0]) 
#define CFG_TAG_72_4025_B0   (&bt_qsoc_cfg_tag_72[0]) 
#define CFG_TAG_72_4025_B1   (&bt_qsoc_cfg_tag_72_4025_bt_2_0[0]) 
#define CFG_TAG_72_4025_B2   (&bt_qsoc_cfg_tag_72_4025_bt_2_0[0])
#define CFG_TAG_72_4025_B3   (&bt_qsoc_cfg_tag_72_4025_bt_2_0[0]) 
#define CFG_TAG_72_MBA_A0    (&bt_qsoc_cfg_tag_72_4025_bt_2_0[0]) 
#define CFG_TAG_72_MBA_B0    (&bt_qsoc_cfg_tag_72_4025_bt_2_0[0]) 
#define CFG_TAG_72_MBA_B1    (&bt_qsoc_cfg_tag_72_4025_bt_2_0[0]) 
#define CFG_TAG_72_4020_BD_B0_BT2_1 (&bt_qsoc_cfg_tag_72[0])
#define CFG_TAG_72_4020_BD_B0_BT2_0 (&bt_qsoc_cfg_tag_72_4020_bd_bt_2_0[0])
#define CFG_TAG_72_4020_BD_B1_BT2_1 (&bt_qsoc_cfg_tag_72_4020_bd_b1_bt_2_1[0])
#define CFG_TAG_72_4020_BD_B1_BT2_0 (&bt_qsoc_cfg_tag_72_4020_bd_bt_2_0[0])

/* Tag 74 -- Libra Coexistence related. */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_74_libra[] = \
{
  0x07, 0x01, 0x4A, 0x04,
  0xD7, 0x29, 0xC7, 0x2A
};

/* #76 Switcher LDO Settings 
 */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_76[] = \
{
#ifdef FEATURE_BT_AUTOMATIC_SWITCHER
  0x0B, 0x01, 0x4C, 0x08, 
  0xBC, 0x80, 0x20, 0x03, 0x5E, 0x00, 0x29, 0x00
#else
  0x0B, 0x01, 0x4C, 0x08,
  0xBC, 0x82, 0x00, 0x02, 0x54, 0x1C, 0x20, 0x00
#endif /* FEATURE_BT_AUTOMATIC_SWITCHER */
};
/* BTS 4025 does not have freg, so this is not required */
#define CFG_TAG_76_R3        (&bt_qsoc_cfg_tag_76[0]) 
#define CFG_TAG_76_4020BD_B0 (&bt_qsoc_cfg_tag_76[0]) 
#define CFG_TAG_76_4020BD_B1 (&bt_qsoc_cfg_tag_76[0]) 
#define CFG_TAG_76_4021_B1   (&bt_qsoc_cfg_tag_76[0]) 
#define CFG_TAG_76_UNKNOWN   (&bt_qsoc_cfg_tag_76[0]) 

/* #78 Bluetooth version information - Bluetooth HCI and 
   Link Manager version parameters 
*/
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_78_bt_2_1_supported[] = \
{
  0x05, 0x01, 0x4E, 0x02, 
  0x04, 0x04
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_78_bt_2_1_not_supported[] = \
{
  0x05, 0x01, 0x4E, 0x02, 
  0x03, 0x03
};
#define CFG_TAG_78_4025_A0   (&bt_qsoc_cfg_tag_78_bt_2_1_supported[0]) 
#define CFG_TAG_78_UNKNOWN   (&bt_qsoc_cfg_tag_78_bt_2_1_not_supported[0]) 

/* #84 */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_84_riva[] = \
{
  0xB1, 0x01, 0x54, 0xAE,
  0x53, 0x63, 0x63, 0x63, 0x00, 0x73, 0x73, 0x73,
  0x73, 0x01, 0x01, 0x37, 0x02, 0x01, 0x37, 0x00,
  0x01, 0x30, 0x03, 0x01, 0x37, 0x04, 0x01, 0x37,
  0x05, 0x01, 0x37, 0x06, 0x01, 0x37, 0x77, 0x73,
  0x53, 0x10, 0x01, 0x37, 0x11, 0x01, 0x37, 0x13,
  0x02, 0x37, 0x16, 0x02, 0x37, 0x17, 0x02, 0x37,
  0x21, 0x03, 0x37, 0x33, 0x03, 0x37, 0x3C, 0x3C,
  0x3B, 0x3B, 0x3B, 0x3B, 0x3A, 0x3A, 0x3A, 0x3B,
  0x3B, 0x3A, 0x3B, 0x3B, 0x3B, 0x3C, 0x3C, 0x3B,
  0x3B, 0x3B, 0x3B, 0x3A, 0x32, 0x3A, 0x3B, 0x3B,
  0x3A, 0x3B, 0x3B, 0x3B, 0x3C, 0x3C, 0x3B, 0x3B,
  0x3B, 0x3B, 0x3A, 0x3A, 0x3A, 0x3B, 0x3B, 0x3A,
  0x3B, 0x3B, 0x3B, 0x35, 0x35, 0x36, 0x35, 0x35,
  0x36, 0x36, 0xBB, 0x36, 0x36, 0x36, 0x36, 0x36,
  0x37, 0x37, 0x5C, 0x5C, 0x5C, 0x5C, 0x59, 0x59,
  0x5C, 0x2B, 0x59, 0x5C, 0x59, 0x59, 0x5C, 0x59,
  0x59, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
  0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
  0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF,
  0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

/* #86 */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_86_riva[] = \
{
  0xC5, 0x01, 0x56, 0xC2,
  0x01, 0xA4, 0x02, 0x07, 0x08, 0xB5, 0xAD, 0xA5,
  0x9D, 0x92, 0x8E, 0x89, 0x84, 0x7E, 0x87, 0x89,
  0x01, 0x49, 0x49, 0x00, 0x49, 0x49, 0x01, 0x09,
  0x09, 0x00, 0x11, 0x15, 0x00, 0xEF, 0xEF, 0x01,
  0x09, 0x08, 0x02, 0x09, 0x08, 0x03, 0x09, 0x08,
  0x04, 0x09, 0x08, 0x05, 0x7A, 0x7B, 0x7A, 0x7B,
  0x7A, 0x7B, 0x7A, 0x7B, 0x7A, 0x7B, 0x7A, 0x7B,
  0x79, 0x7D, 0x79, 0x7D, 0x79, 0x7D, 0x79, 0x7D,
  0x79, 0x7D, 0x79, 0x7D, 0x79, 0x7D, 0x79, 0x7D,
  0x79, 0x7D, 0x7A, 0x7B, 0x7A, 0x7B, 0x7A, 0x7B,
  0x7A, 0x7B, 0x7A, 0x7B, 0x7A, 0x7B, 0x79, 0x7D,
  0x79, 0x7D, 0x79, 0x7D, 0x79, 0x7D, 0x79, 0x7D,
  0x79, 0x7D, 0x79, 0x7D, 0x79, 0x7D, 0x79, 0x7D,
  0x79, 0x7A, 0x79, 0x7A, 0x79, 0x7A, 0x79, 0x7A,
  0x79, 0x7A, 0x79, 0x7A, 0x78, 0x7A, 0x28, 0x28,
  0x78, 0x7A, 0x78, 0x7A, 0x78, 0x7A, 0x78, 0x7A,
  0x78, 0x7A, 0x78, 0x7A, 0x78, 0x7A, 0x79, 0x7A,
  0x79, 0x7A, 0x79, 0x7A, 0x79, 0x7A, 0x79, 0x7A,
  0x79, 0x7A, 0x78, 0x7A, 0x78, 0x7A, 0x78, 0x7A,
  0x78, 0x7A, 0x78, 0x7A, 0x78, 0x7A, 0x78, 0x7A,
  0x78, 0x7A, 0x78, 0x7A, 0x79, 0x7A, 0x79, 0x7A,
  0x79, 0x7A, 0x79, 0x7A, 0x79, 0x7A, 0x79, 0x7A,
  0x78, 0x7A, 0x78, 0x7A, 0x78, 0x7A, 0x78, 0x7A,
  0x78, 0x7A, 0x78, 0x7A, 0x78, 0x7A, 0x78, 0x7A,
  0x78, 0x7A
};
/* #95 BT Local Features - eSCO CQDDR evaluation - disabled 
 *                         L2CAP Flow Control - enabled
 *                         ACL Automatic Flush - enabled
 */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_95[] = \
{
  0x05, 0x01, 0x5F, 0x02, 
  0x00, 0x03
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_95_4025_b0[] = \
{
  0x0E, 0x01, 0x5F, 0x0B, 
  0x00, 0x03, 0x01, 0x0D, 0x00, 0x00, 0x00, 0x02,
  0x00, 0x02, 0x02

};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_95_riva[] = \
{
  0x05, 0x01, 0x5F, 0x02, 
  0x00, 0x0B
};
#define CFG_TAG_95_R3        (&bt_qsoc_cfg_tag_95[0]) 
#define CFG_TAG_95_4020BD_B0 (&bt_qsoc_cfg_tag_95[0]) 
#define CFG_TAG_95_4020BD_B1 (&bt_qsoc_cfg_tag_95[0]) 
#define CFG_TAG_95_4021_B1   (&bt_qsoc_cfg_tag_95[0]) 
#define CFG_TAG_95_4025_A0   (&bt_qsoc_cfg_tag_95[0]) 
#define CFG_TAG_95_4025_B0   (&bt_qsoc_cfg_tag_95_4025_b0[0]) 
#define CFG_TAG_95_UNKNOWN   (&bt_qsoc_cfg_tag_95[0]) 

/* #97 System Clock Request */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_97_clock_sharing[] = \
{
  /* NVM: Active High */
  0x05, 0x01, 0x61, 0x02, 
  0x00, 0x01
};

#define CFG_TAG_97_CLK_SHARING_R3        (&bt_qsoc_cfg_tag_97_clock_sharing[0]) 
#define CFG_TAG_97_CLK_SHARING_4020BD_B0 (&bt_qsoc_cfg_tag_97_clock_sharing[0]) 
#define CFG_TAG_97_CLK_SHARING_4020BD_B1 (&bt_qsoc_cfg_tag_97_clock_sharing[0]) 
#define CFG_TAG_97_CLK_SHARING_4021_B1   (&bt_qsoc_cfg_tag_97_clock_sharing[0]) 
#define CFG_TAG_97_CLK_SHARING_4025_A0   (&bt_qsoc_cfg_tag_97_clock_sharing[0])  
#define CFG_TAG_97_CLK_SHARING_4025_B0   (&bt_qsoc_cfg_tag_97_clock_sharing[0])  
#define CFG_TAG_97_CLK_SHARING_4025_B1   (&bt_qsoc_cfg_tag_97_clock_sharing[0])  
#define CFG_TAG_97_CLK_SHARING_4025_B2   (&bt_qsoc_cfg_tag_97_clock_sharing[0])
#define CFG_TAG_97_CLK_SHARING_4025_B3   (&bt_qsoc_cfg_tag_97_clock_sharing[0])
#define CFG_TAG_97_CLK_SHARING_MBA_A0    (&bt_qsoc_cfg_tag_97_clock_sharing[0])  
#define CFG_TAG_97_CLK_SHARING_MBA_B0    (&bt_qsoc_cfg_tag_97_clock_sharing[0])  
#define CFG_TAG_97_CLK_SHARING_MBA_B1    (&bt_qsoc_cfg_tag_97_clock_sharing[0])  
#define CFG_TAG_97_CLK_SHARING_UNKNOWN   (&bt_qsoc_cfg_tag_97_clock_sharing[0]) 

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_97_no_clock_sharing[] = \
{
  /* NVM: Active Undefined */
  0x05, 0x01, 0x61, 0x02, 
  0x00, 0xFF
};
#define CFG_TAG_97_NO_CLK_SHARING_R3        (&bt_qsoc_cfg_tag_97_no_clock_sharing[0]) 
#define CFG_TAG_97_NO_CLK_SHARING_4020BD_B0 (&bt_qsoc_cfg_tag_97_no_clock_sharing[0]) 
#define CFG_TAG_97_NO_CLK_SHARING_4020BD_B1 (&bt_qsoc_cfg_tag_97_no_clock_sharing[0]) 
#define CFG_TAG_97_NO_CLK_SHARING_4021_B1   (&bt_qsoc_cfg_tag_97_no_clock_sharing[0]) 
#define CFG_TAG_97_NO_CLK_SHARING_4025_A0   (&bt_qsoc_cfg_tag_97_no_clock_sharing[0]) 
#define CFG_TAG_97_NO_CLK_SHARING_4025_B0   (&bt_qsoc_cfg_tag_97_no_clock_sharing[0]) 
#define CFG_TAG_97_NO_CLK_SHARING_4025_B1   (&bt_qsoc_cfg_tag_97_no_clock_sharing[0]) 
#define CFG_TAG_97_NO_CLK_SHARING_4025_B2   (&bt_qsoc_cfg_tag_97_no_clock_sharing[0])
#define CFG_TAG_97_NO_CLK_SHARING_4025_B3   (&bt_qsoc_cfg_tag_97_no_clock_sharing[0])
#define CFG_TAG_97_NO_CLK_SHARING_MBA_A0    (&bt_qsoc_cfg_tag_97_no_clock_sharing[0]) 
#define CFG_TAG_97_NO_CLK_SHARING_UNKNOWN   (&bt_qsoc_cfg_tag_97_no_clock_sharing[0]) 

/* #99 System Reference Clock Control */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_99_19p2MHz[] = \
{
  /* 19.2 Mhz */
  0x04, 0x01, 0x63, 0x01, 
  0x07
};

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_99_32MHz[] = \
{
  /* 32 Mhz */
  0x04, 0x01, 0x63, 0x01, 
  0x0D
};

#define CFG_TAG_99_19P2MHZ_R3         (&bt_qsoc_cfg_tag_99_19p2MHz[0]) 
#define CFG_TAG_99_19P2MHZ_4020_BD_B1 (&bt_qsoc_cfg_tag_99_19p2MHz[0]) 
#define CFG_TAG_99_19P2MHZ_4025_A0    (&bt_qsoc_cfg_tag_99_19p2MHz[0]) 
#define CFG_TAG_99_19P2MHZ_4025_B0    (&bt_qsoc_cfg_tag_99_19p2MHz[0]) 
#define CFG_TAG_99_19P2MHZ_4025_B1    (&bt_qsoc_cfg_tag_99_19p2MHz[0]) 
#define CFG_TAG_99_19P2MHZ_UNKNOWN    (&bt_qsoc_cfg_tag_99_19p2MHz[0]) 
 
#define CFG_TAG_99_32MHZ_R3           (&bt_qsoc_cfg_tag_99_32MHz[0]) 
#define CFG_TAG_99_32MHZ_4020_BD_B1   (&bt_qsoc_cfg_tag_99_32MHz[0]) 
#define CFG_TAG_99_32MHZ_4025_A0      (&bt_qsoc_cfg_tag_99_32MHz[0]) 
#define CFG_TAG_99_32MHZ_4025_B0      (&bt_qsoc_cfg_tag_99_32MHz[0]) 
#define CFG_TAG_99_32MHZ_4025_B1      (&bt_qsoc_cfg_tag_99_32MHz[0]) 
#define CFG_TAG_99_32MHZ_UNKNOWN      (&bt_qsoc_cfg_tag_99_32MHz[0]) 


/* #116 HW Encryption Enable Code 
*/
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_116[] = \
{
  0x13, 0x01, 0x74, 0x10, 
  0x02, 0x4A, 0x01, 0x03, 0x11, 0x43, 0x08, 0x60, 
  0x70, 0x47, 0x00, 0x00, 0x48, 0x00, 0x00, 0xB0
};

#define CFG_TAG_116_4025_A0   (&bt_qsoc_cfg_tag_116[0]) 
#define CFG_TAG_116_UNKNOWN   (&bt_qsoc_cfg_tag_116[0]) 

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_119_4025_a0[] = \
{
      0xFB, 0x01, 0x77, 0xF8, 
      0x3D, 0x3E, 0xA7, 0xAA, 0xAB, 0xAC, 0xA6, 0xA8,
      0x1B, 0x09, 0xB7, 0x62, 0x80, 0x27, 0xBB, 0x16,
      0x1B, 0x09, 0xB7, 0x62, 0x80, 0x27, 0xBB, 0x16, 
      0x1B, 0x09, 0xB7, 0x62, 0x80, 0x27, 0xBB, 0x16, 
      0x1B, 0x09, 0xB7, 0x62, 0x80, 0x27, 0xBB, 0x16,
      0x1B, 0x09, 0xB7, 0x62, 0x80, 0x27, 0xBB, 0x16,
      0x1B, 0x09, 0xB7, 0x62, 0x80, 0x27, 0xBB, 0x16, 
      0x1B, 0x09, 0xB7, 0x62, 0x80, 0x27, 0xBB, 0x16, 
      0x1B, 0x09, 0xB7, 0x62, 0x80, 0x27, 0xBB, 0x16, 
      0x1B, 0x09, 0xB7, 0x62, 0x84, 0x07, 0xBB, 0x16, 
      0x1B, 0x09, 0xFB, 0x62, 0x84, 0x07, 0xFF, 0x17, 
      0xFB, 0x08, 0xB7, 0x40, 0x84, 0x00, 0xBB, 0x16, 
      0xFB, 0x08, 0xB7, 0x40, 0x84, 0x00, 0xBB, 0x16,
      0xFB, 0x08, 0xB7, 0x40, 0x84, 0x00, 0xBB, 0x16, 
      0xFB, 0x08, 0xB7, 0x40, 0x84, 0x00, 0xBB, 0x16,
      0xFB, 0x08, 0xB7, 0x40, 0x84, 0x00, 0xBB, 0x16,
      0xFB, 0x08, 0xB7, 0x40, 0x84, 0x00, 0xBB, 0x16,
      0xFB, 0x08, 0xB7, 0x40, 0x84, 0x00, 0xBB, 0x16,
      0xFB, 0x08, 0xB7, 0x40, 0x84, 0x00, 0xBB, 0x16,
      0xBB, 0x08, 0xB7, 0x41, 0xC4, 0x00, 0xBB, 0x16,
      0xDB, 0x09, 0xB7, 0x42, 0x84, 0x07, 0xBB, 0x16, 
      0xFB, 0x08, 0xB7, 0x40, 0x84, 0x00, 0xBB, 0x12,
      0xFB, 0x08, 0xB7, 0x40, 0x84, 0x00, 0xBB, 0x12, 
      0xFB, 0x08, 0xB7, 0x40, 0x84, 0x00, 0xBB, 0x12,
      0xFB, 0x08, 0xB7, 0x40, 0x84, 0x00, 0xBB, 0x12,
      0xFB, 0x08, 0xB7, 0x40, 0x84, 0x00, 0xBB, 0x12, 
      0xFB, 0x08, 0xB7, 0x40, 0x84, 0x00, 0xBB, 0x12,
      0xFB, 0x08, 0xB7, 0x40, 0x84, 0x00, 0xBB, 0x12, 
      0xFB, 0x08, 0xB7, 0x40, 0x84, 0x00, 0xBB, 0x12, 
      0xBB, 0x08, 0xB7, 0x41, 0xC4, 0x00, 0xBB, 0x12, 
      0xDB, 0x09, 0xB7, 0x42, 0x84, 0x07, 0xBB, 0x12, 
};

#define CFG_TAG_119_4025_A0 (&bt_qsoc_cfg_tag_119_4025_a0[0]) 

/* #123 Temperature Calibration Configuration */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_123_4025_a0[] = \
{
  0x14, 0x01, 0x7b, 0x11, 
  0x16, 0x13, 0x10, 0x0D, 0x0A, 0x08, 0x06, 0x01, 
  0x03, 0x05, 0x07, 0x09, 0x0B, 0x0D, 0x0F, 0x01, 
  0x00
};

/* #151 */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_tag_151_riva[] = \
{
  0x5D, 0x01, 0x97, 0x5A,
  0xFF, 0xFF, 0x07, 0x00, 0x46, 0x00, 0x8B, 0x00,
  0xB6, 0x00, 0xA7, 0x00, 0x96, 0x00, 0x8E, 0x00,
  0x88, 0x00, 0x81, 0x00, 0x7B, 0x00, 0x4C, 0x00,
  0x70, 0x00, 0x5A, 0x0D, 0x6A, 0x0D, 0x7A, 0x0D,
  0x8A, 0x0D, 0x9A, 0x0D, 0x10, 0x0E, 0x20, 0x0E,
  0x40, 0x0E, 0x50, 0x00, 0xD7, 0x02, 0x5C, 0x02,
  0x8F, 0x01, 0x8F, 0x01, 0x6C, 0x01, 0x6C, 0x01,
  0x00, 0x00, 0xF8, 0x08, 0x01, 0x00, 0x1F, 0x00,
  0x0A, 0x02, 0x55, 0x02, 0x00, 0x35, 0xF4, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x0C, 0x03, 0xBC, 0x02,
  0x31, 0x02, 0xD4, 0x01, 0xE0, 0x01, 0xA4, 0x01,
  0x00, 0x00
};

#define CFG_TAG_123_4025_A0   (&bt_qsoc_cfg_tag_123_4025_a0[0]) 

/******************************************************************
 *            R3 BD (4020BD) B0 AND B1 R O M  P A T C H E S              *
 ******************************************************************/
/* #106 Rom Insertion Patch 
 */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_4020bd_b0_tag_106[] = \
{
  0x37, 0x01, 0x6A, 0x34,
  0x50, 0x81, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00,
  0x84, 0x00, 0x28, 0x00, 0x49, 0x08, 0x47, 0x09,
  0x00, 0x00, 0x00, 0x08, 0x40, 0x06, 0x49, 0x08,
  0x40, 0x01, 0x99, 0x01, 0x29, 0x01, 0xD0, 0x01,
  0x49, 0x08, 0x47, 0x01, 0x49, 0x08, 0x47, 0x61,
  0x81, 0x00, 0x70, 0x59, 0x81, 0x00, 0x70, 0xF9,
  0xCC, 0xFF, 0xFF, 0x00

};
#define CFG_TAG_106_4020BD_B0       (&bt_qsoc_cfg_4020bd_b0_tag_106[0])


/* #107 Rom Insertion Patch 
 */

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_4020bd_b0_tag_107[] = \
{
  0x3B, 0x01, 0x6B, 0x38,
  0x50, 0x82, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00,
  0x84, 0x00, 0x2D, 0x00, 0x49, 0x08, 0x47, 0x09,
  0x00, 0x00, 0x00, 0x18, 0x71, 0xDE, 0x80, 0x20,
  0x69, 0x06, 0x4E, 0xE1, 0x68, 0xE0, 0x31, 0x49,
  0x78, 0x00, 0x29, 0x00, 0xD0, 0x37, 0x40, 0x02,
  0x4E, 0xF1, 0x43, 0x00, 0x4E, 0x30, 0x47, 0x59,
  0x82, 0x00, 0x70, 0xF9, 0xCC, 0xFF, 0xFF, 0x00
};

#define CFG_TAG_107_4020BD_B0       (&bt_qsoc_cfg_4020bd_b0_tag_107[0])


/* #108 Rom Insertion Patch 
 */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_4020bd_b0_tag_108[] = \
{
  0x5D, 0x01, 0x6C, 0x5A, 
  0xC8, 0x6D, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00,
  0x84, 0x08, 0x4F, 0x00, 0x49, 0x08, 0x47, 0x09,
  0x00, 0x00, 0x00, 0x8E, 0x21, 0x18, 0x73, 0x20,
  0x6F, 0x0A, 0x5A, 0x00, 0x21, 0x0B, 0x4B, 0x00,
  0xF0, 0x1A, 0xF8, 0x00, 0x28, 0x0C, 0xD1, 0x20,
  0x6F, 0x00, 0x21, 0x80, 0x30, 0xC2, 0x89, 0x20,
  0x1C, 0x07, 0x4B, 0x00, 0xF0, 0x10, 0xF8, 0x21,
  0x1C, 0x01, 0x20, 0x05, 0x4B, 0x00, 0xF0, 0x0B,
  0xF8, 0x01, 0x48, 0x00, 0x47, 0x00, 0x00, 0xE1,
  0x6D, 0x00, 0x70, 0x29, 0x82, 0x00, 0x70, 0xD9,
  0x21, 0x01, 0x70, 0x81, 0x5B, 0x00, 0x70, 0x18,
  0x47, 0x00
};
#define CFG_TAG_108_4020BD_B0       (&bt_qsoc_cfg_4020bd_b0_tag_108[0])


/******************************************************************
 *             4 0 2 5  A 0  R O M  P A T C H E S                 *
 ******************************************************************/

/* #107 Rom Insertion Patch #7
 * TT-4801: 4020 BD B0: In-Band Sleep Does not Respond to Un-Break
 */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_4025_a0_tag_107[] = \
{
  0x22, 0x01, 0x6B, 0x1F, 
  0x80, 0xE4, 0x03, 0x70, 0x00, 0x00, 0x00, 0x00,
  0x80, 0x04, 0x14, 0x05, 0x00, 0x00, 0x00, 0x01,
  0x49, 0x88, 0x70, 0x01, 0x49, 0x08, 0x47, 0xDC,
  0x6C, 0x00, 0x00, 0xED, 0x83, 0x03, 0x70 
};
#define CFG_TAG_107_4025_A0       (&bt_qsoc_cfg_4025_a0_tag_107[0]) 

/* #108 Rom Insertion Patch #8
 * TT-4765: 4025_A0: Inquiry with SCO and Sniff causes the device to abort.
 */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_4025_a0_tag_108[] = \
{
  0x12, 0x01, 0x6C, 0x0F, 
  0xA0, 0x6A, 0x02, 0x70, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x04, 0x04, 0xC0, 0x46, 0x61, 0x7E 
};
#define CFG_TAG_108_4025_A0       (&bt_qsoc_cfg_4025_a0_tag_108[0]) 

/* #111 Rom Insertion Patch #11
 * TT-4724: 4025 A0V0: Legacy pairing failure using WinMobile stack
 */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_4025_a0_tag_111[] = \
{
  0x44, 0x01, 0x6F, 0x41, 
  0x80, 0x6F, 0x01, 0x70, 0x00, 0x00, 0x00, 0x00, 
  0x84, 0x08, 0x36, 0x00, 0x48, 0x00, 0x47, 0x09, 
  0x00, 0x00, 0x00, 0x16, 0x1C, 0x08, 0x69, 0x00, 
  0x28, 0x08, 0xD0, 0x0D, 0x1C, 0x20, 0x1C, 0x18, 
  0x30, 0x06, 0x49, 0x00, 0xF0, 0x0C, 0xF8, 0x29, 
  0x1C, 0x00, 0x20, 0x28, 0x61, 0xB7, 0x78, 0x01, 
  0x20, 0x07, 0x40, 0x00, 0x48, 0x00, 0x47, 0x89, 
  0x6F, 0x01, 0x70, 0x67, 0xAD, 0x02, 0x70, 0x08, 
  0x47
};
#define CFG_TAG_111_4025_A0       (&bt_qsoc_cfg_4025_a0_tag_111[0]) 

/* #112 Rom Insertion Patch #12
 * TT-4724: 4025 A0V0: Legacy pairing failure using WinMobile stack 
 */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_4025_a0_tag_112[] = \
{
  0x5A, 0x01, 0x70, 0x57, 
  0x0C, 0xD9, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 
  0x84, 0x08, 0x4C, 0x00, 0x48, 0x00, 0x47, 0x09, 
  0x00, 0x00, 0x00, 0xA0, 0x6B, 0x40, 0x30, 0x40, 
  0x79, 0x00, 0x28, 0x01, 0xD0, 0x04, 0x28, 0x0B, 
  0xD1, 0x04, 0x21, 0x08, 0x9A, 0x20, 0x1C, 0x08, 
  0xB4, 0x0A, 0x4B, 0x00, 0xF0, 0x04, 0xF8, 0x08, 
  0xBC, 0x00, 0x28, 0x04, 0xD0, 0x05, 0xE0, 0x18, 
  0x47, 0x04, 0x21, 0x02, 0x48, 0x00, 0x47, 0x02, 
  0x48, 0x00, 0x47, 0x02, 0x48, 0x00, 0x47, 0x41, 
  0xD9, 0x00, 0x70, 0x1F, 0xD9, 0x00, 0x70, 0x43, 
  0xE5, 0x00, 0x70, 0x3D, 0x71, 0x01, 0x70
};
#define CFG_TAG_112_4025_A0       (&bt_qsoc_cfg_4025_a0_tag_112[0]) 
  
/* #113 Rom Insertion Patch #13
 * TT-4722: 4025 A0V0: Refresh_Encryption_Key not reported as a 
 * supported HCI Command
 */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_4025_a0_tag_113[] = \
{
  0x12, 0x01, 0x71, 0x0F, 
  0x38, 0xB1, 0x03, 0x70, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x04, 0x04, 0xE8, 0xFE, 0x3F, 0xF7
};
#define CFG_TAG_113_4025_A0       (&bt_qsoc_cfg_4025_a0_tag_113[0]) 

/* #114 Rom Insertion Patch #14
 * BTS 4025 A0: Patch the Desense feature - Add 2 register and 
 * update Rx Register settings
 */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_4025_a0_19p2MHz_tag_114[] = \
{
  0x4A, 0x01, 0x72, 0x47, 
  0x2C, 0x3B, 0x03, 0x70, 0x00, 0x00, 0x00, 0x00, 
  0x84, 0x08, 0x3C, 0x00, 0x48, 0x00, 0x47, 0x09, 
  0x00, 0x00, 0x00, 0x0A, 0x48, 0x11, 0x21, 0x01, 
  0x60, 0x1A, 0x21, 0x41, 0x61, 0x15, 0x21, 0x81, 
  0x61, 0x03, 0x21, 0x81, 0x62, 0x07, 0x48, 0x06, 
  0x21, 0x01, 0x60, 0x00, 0x21, 0x81, 0x62, 0x41, 
  0x66, 0xC1, 0x66, 0x01, 0x67, 0x10, 0x21, 0x81, 
  0x66, 0x96, 0x21, 0x41, 0x67, 0x70, 0x47, 0x04, 
  0x01, 0x00, 0xA8, 0x9C, 0x01, 0x00, 0xA8
};
#define CFG_TAG_114_19P2MHZ_4025_A0       (&bt_qsoc_cfg_4025_a0_19p2MHz_tag_114[0]) 

/* #115 Rom Insertion Patch #15
 * BTS 4025 A0: Patch the Desense feature - Add 2 register and 
 * update Rx Register settings
 */
static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_4025_a0_19p2MHz_tag_115[] = \
{
  0x2E, 0x01, 0x73, 0x2B, 
  0x58, 0x3B, 0x03, 0x70, 0x00, 0x00, 0x00, 0x00, 
  0x84, 0x08, 0x20, 0x00, 0x48, 0x00, 0x47, 0x09, 
  0x00, 0x00, 0x00, 0x19, 0x20, 0x48, 0x60, 0x18, 
  0x20, 0x88, 0x61, 0x23, 0x20, 0xC8, 0x61, 0x04, 
  0x22, 0xCA, 0x62, 0x00, 0x49, 0x08, 0x47, 0x61, 
  0x3B, 0x03, 0x70
};
#define CFG_TAG_115_19P2MHZ_4025_A0       (&bt_qsoc_cfg_4025_a0_19p2MHz_tag_115[0]) 

/******************************************************************
 *             4 0 2 5  B 0  R O M  P A T C H E S                 *
 ******************************************************************/

/* TT 5047: BTS 4025 B0 - Watchdog reset occurs when LPPS is enabled */

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_4025_b0_tag_112[] = \
{
  0x36, 0x01, 0x70, 0x33, 
  0x68, 0x21, 0x03, 0x70, 0x00, 0x00, 0x00, 0x00,
  0x84, 0x08, 0x28, 0x00, 0x48, 0x00, 0x47, 0x09,
  0x00, 0x00, 0x00, 0x01, 0x25, 0xA5, 0x60, 0x03,
  0x4E, 0x75, 0x60, 0x03, 0x49, 0x00, 0x20, 0x08,
  0x63, 0x03, 0x48, 0x00, 0x47, 0x00, 0x00, 0x00,
  0x10, 0x00, 0xB0, 0xC0, 0xF0, 0x0F, 0xB0, 0x71,
  0x21, 0x03, 0x70
};
#define CFG_TAG_112_4025_B0       (&bt_qsoc_cfg_4025_b0_tag_112[0]) 

/* TT-5021: 4025 B0 TX mode fails while in DUT */

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_4025_b0_tag_113[] = \
{
  0x12, 0x01, 0x71, 0x0F, 
  0x04, 0x5A, 0x02, 0x70, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x04, 0x04, 0xC9, 0x69, 0x40, 0x1A 
};
#define CFG_TAG_113_4025_B0       (&bt_qsoc_cfg_4025_b0_tag_113[0]) 

/* TT-5026: 4025 B0 Max length RNR followed by Write EIR Data results
 * in no events
 */

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_4025_b0_tag_114[] = \
{
  0x12, 0x01, 0x72, 0x0F, 
  0x14, 0xD0, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x04, 0x04, 0xC0, 0x46, 0xC0, 0x46 
};
#define CFG_TAG_114_4025_B0       (&bt_qsoc_cfg_4025_b0_tag_114[0]) 

/* TT-5025: 4025 B0 HCI_RESET in the middle of Remote Name causes
 * memory allocation issue
 */

static bt_qsoc_cfg_cst_element_type bt_qsoc_cfg_4025_b0_tag_115[] = \
{
  0x3E, 0x01, 0x73, 0x3B, 
  0xD0, 0x54, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00,
  0x84, 0x08, 0x30, 0x00, 0x4E, 0x30, 0x47, 0x09,
  0x00, 0x00, 0x00, 0xA0, 0x6B, 0x00, 0x28, 0x04,
  0xD0, 0x00, 0xF0, 0x05, 0xF8, 0xA0, 0x6B, 0x00,
  0xF0, 0x04, 0xF8, 0x02, 0x4E, 0x30, 0x47, 0x02,
  0x4E, 0x30, 0x47, 0x02, 0x4E, 0x30, 0x47, 0xDB,
  0x54, 0x00, 0x70, 0x35, 0x7B, 0x00, 0x70, 0x8B,
  0xB7, 0x02, 0x70
};
#define CFG_TAG_115_4025_B0       (&bt_qsoc_cfg_4025_b0_tag_115[0]) 

/*********** R U N  T I M E ****************/
#define CFG_TAG_2_RAM (&bt_qsoc_cfg_tag_2_ram[0])
static bt_qsoc_cfg_element_type bt_qsoc_cfg_tag_2_ram[] = \
{
  0x09, 0x01, 0x02, 0x06, 
  0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA
};

/* Customer specific configuration for BTS. Any cusomter specific values 
 * must be added here.
 */
 
#define BT_QSOC_VS_EDL_DNLOAD_REQ        (0x01)
 
const bt_qsoc_poke8_tbl_struct_type bt_qsoc_vs_poke8_tbl_r3[] =
{
  /* Patch R3: Rx Abort Disable for Test Modes (#1) 
    * In-band sleep poke #1 for SoC un-break refresh
    */
    {
      0x42, 
      {
        BT_QSOC_VS_EDL_DNLOAD_REQ, 0x40, 0xD4, 0x7A, 0x00, 0x00, 
        0x1F, 0x70, 0x03, 0x70, 0x81, 0x71, 0x03, 0x70, 
        0x00, 0x00, 0x00, 0x00, 0xED, 0x6B, 0x03, 0x70, 
        0x00, 0x00, 0x00, 0x00, 0x0B, 0x72, 0x03, 0x70, 
        0x8F, 0x72, 0x03, 0x70, 0x01, 0x7B, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x1D, 0x73, 0x03, 0x70, 0x01, 0x49, 0x88, 0x70, 
        0x01, 0x49, 0x08, 0x47, 0xDC, 0x6C, 0x00, 0x00, 
        0xD1, 0x72, 0x03, 0x70
      }
    },

#ifdef FEATURE_BT_QSOC_INBAND_SLEEP
   /* In-band sleep poke #2 for SoC un-break refresh 
   */
   {
      0x0A, 
      {
        BT_QSOC_VS_EDL_DNLOAD_REQ, 0x08, 0x10, 0x03, 0x00, 0x00, 
        0xD4, 0x7A, 0x00, 0x00
      }
   },
#endif /* FEATURE_BT_QSOC_INBAND_SLEEP */

};

/* Without Poke on Bahama B0, the internal regulators weren't functional and
    and the SoC will not respond to HCI_RESET */
const bt_qsoc_poke8_tbl_struct_type bt_qsoc_vs_poke8_tbl_bha_b0[] =
{
    {
      0x0A,
      {
        0x0C, 0x08, 0x38, 0x00, 0x00, 0xA4, 0x01, 0x01,
        0x03, 0x03
      }
    },
};


/*==============================================================
FUNCTION:  build_tag_71
==============================================================*/
/**
  Build a buffer size tag (#71) for the given configuration parameters.
  
  If num_rx_buf and num_tx_buf are zero it will trigger the use of
  the default parameter set.

  @see bt_qsoc_config_params_struct_type
  @see BT_QSOC_MAX_HC_RX_LEN
  @see BT_QSOC_MAX_HC_TX_LEN
  @see BT_QSOC_MAX_HC_NUM_RX_BUF
  @see BT_QSOC_MAX_HC_NUM_TX_BUF
  @see BT_QSOC_SCO_BUF_SIZE
  
  @return  Returns a NVM string for tag #71.

  @sideeffects Undetermined.
*/
static bt_qsoc_cfg_cst_element_type *build_tag_71
(
  /** [in] Parameters that determine the run time configuration */
  bt_qsoc_config_params_struct_type *param_ptr
)
{
  /* Note on storage...  This array is static because a pointer to
     the memory is returned from this function and added to the linked
     list of NVM data.
   */
  static bt_qsoc_cfg_element_type tag_71[] =
  {
    0x0A, 0x01, 0x47, 0x07,     /* 0..3 */
    BT_QSOC_MAX_HC_RX_LEN_LSB,  /*  4 */
    BT_QSOC_MAX_HC_RX_LEN_MSB,  /*  5 */
    BT_QSOC_MAX_HC_TX_LEN_LSB,  /*  6 */
    BT_QSOC_MAX_HC_TX_LEN_MSB,  /*  7 */ 
    BT_QSOC_MAX_HC_NUM_RX_BUF,  /*  8 */ 
    BT_QSOC_MAX_HC_NUM_TX_BUF,  /*  9 */
    BT_QSOC_SCO_BUF_SIZE        /* 10 */
  };

  if( NULL != param_ptr )
  {
    if( param_ptr->num_rx_buf > 0 )
    { 
      tag_71[4]  = (uint8)(param_ptr->max_hc_rx_len & 0x00FF);
      tag_71[5]  = (uint8)((param_ptr->max_hc_rx_len >> 8)& 0x00FF);
      tag_71[8]  = param_ptr->num_rx_buf;
    }
    
    if( param_ptr->num_tx_buf > 0 )
    { 
      tag_71[6]  = (uint8)(param_ptr->max_hc_tx_len & 0x00FF);
      tag_71[7]  = (uint8)((param_ptr->max_hc_tx_len >> 8)& 0x00FF);
      tag_71[9]  = param_ptr->num_tx_buf;
    }
    
    if( param_ptr->sco_buf_size > 0 )
    { 
      tag_71[10] = param_ptr->sco_buf_size;
    }
  }

  return &tag_71[0];
} /* build_tag_71 */


/*==============================================================
FUNCTION:  build_srl_tag_71
==============================================================*/
/**
  Build a buffer size tag (#71) for the given configuration parameters.
  
  If num_rx_buf and num_tx_buf are zero it will trigger the use of
  the default parameter set.
  This function is simillar to build_tag_71 but Rx/Tx Buf numbers have been 
  changed for SOC Redirect Logging

  @see bt_qsoc_config_params_struct_type
  @see BT_QSOC_MAX_HC_RX_LEN
  @see BT_QSOC_MAX_HC_TX_LEN
  @see BT_QSOC_MAX_HC_NUM_RX_BUF
  @see BT_QSOC_MAX_HC_NUM_TX_BUF
  @see BT_QSOC_SCO_BUF_SIZE
  
  @return  Returns a NVM string for tag #71.

  @sideeffects Undetermined.
*/
static bt_qsoc_cfg_cst_element_type *build_srl_tag_71
(
  /** [in] Parameters that determine the run time configuration */
  bt_qsoc_config_params_struct_type *param_ptr
)
{
  /* Note on storage...  This array is static because a pointer to
     the memory is returned from this function and added to the linked
     list of NVM data.
   */
  static bt_qsoc_cfg_element_type tag_srl_71[] =
  {
    0x0A, 0x01, 0x47, 0x07,     /* 0..3 */
    BT_QSOC_MAX_HC_RX_LEN_LSB,  /*  4 */
    BT_QSOC_MAX_HC_RX_LEN_MSB,  /*  5 */
    BT_QSOC_MAX_HC_TX_LEN_LSB,  /*  6 */
    BT_QSOC_MAX_HC_TX_LEN_MSB,  /*  7 */ 
    BT_QSOC_MAX_HC_NUM_RX_BUF_SOC_REDIRECT_LOGGING ,  /*  8 */ 
    BT_QSOC_MAX_HC_NUM_TX_BUF_SOC_REDIRECT_LOGGING ,  /*  9 */
    BT_QSOC_SCO_BUF_SIZE        /* 10 */
  };
  
  if( NULL != param_ptr )
  {
    if( param_ptr->num_rx_buf > 0 )
    { 
      tag_srl_71[4]  = (uint8)(param_ptr->max_hc_rx_len & 0x00FF);
      tag_srl_71[5]  = (uint8)((param_ptr->max_hc_rx_len >> 8)& 0x00FF);
      tag_srl_71[8]  = param_ptr->num_rx_buf;
    }
    
    if( param_ptr->num_tx_buf > 0 )
    { 
      tag_srl_71[6]  = (uint8)(param_ptr->max_hc_tx_len & 0x00FF);
      tag_srl_71[7]  = (uint8)((param_ptr->max_hc_tx_len >> 8)& 0x00FF);
      tag_srl_71[9]  = param_ptr->num_tx_buf;
    }
    
    if( param_ptr->sco_buf_size > 0 )
    { 
      tag_srl_71[10] = param_ptr->sco_buf_size;
    }
  }

  return &tag_srl_71[0];
} /* build_tag_srl_71 */


/*==============================================================
FUNCTION:  init_4020_R3_entries
==============================================================*/
/**
  Initialize the fixed and runtime data for the 4020 R3 SOC.

  @see bt_qsoc_config_params_struct_type
  
  @sideeffects Undetermined.
*/
static void init_4020_R3_entries
(
  /** [in] Parameters that determine the run time configuration */
  bt_qsoc_config_params_struct_type *param_ptr
)
{
  /* Add fixed NVM data */
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_2_RAM,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_4_R3,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_5_R3,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_6_R3,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_17_R3,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_27_R3,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_44_R3,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_45_R3,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_46_R3,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_53_R3,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_55_R3,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_57_R3,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_70_R3,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)build_tag_71(param_ptr),FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_76_R3,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_95_R3,FALSE);

  /* Add runtime NVM data */
  if(param_ptr->clock_sharing == BT_SOC_CLOCK_SHARING_ENABLED)
  {
    if(BT_QSOC_IS_XO_SUPPORT_ENABLED(param_ptr->is_xo_support_enabled))
    {
      bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_CLK_SHARING_R3_XO,FALSE);
    }
    else
    {
      bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_CLK_SHARING_R3,FALSE);
    }
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_97_CLK_SHARING_R3,FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_NO_CLK_SHARING_R3,FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_97_NO_CLK_SHARING_R3,FALSE);
  }

  if(BT_QSOC_SOC_LOGGING_ENABLED(param_ptr->clock_sharing))
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_38_SOC_LOGGING_R3,FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_38_SOC_NO_LOGGING_R3,FALSE);
  } 
} /* init_4020_R3_entries */

/*==============================================================
FUNCTION:  init_4020BD_B0_entries
==============================================================*/
/**
  Initialize the fixed and runtime data for the 4020 BD B0 SOC.

  @see bt_qsoc_config_params_struct_type
  
  @sideeffects Undetermined.
*/
static void init_4020BD_B0_entries
(
  /** [in] Parameters that determine the run time configuration */
  bt_qsoc_config_params_struct_type *param_ptr
)
{
  /* Add fixed NVM data */
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_2_RAM,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_4_4020BD_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_5_4020BD_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_17_4020BD_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_27_4020BD_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_44_4020BD_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_45_4020BD_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_46_4020BD_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_53_4020BD_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_55_4020BD_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_57_4020BD_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_70_4020BD_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)build_tag_71(param_ptr),FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_76_4020BD_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_95_4020BD_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_106_4020BD_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_107_4020BD_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_108_4020BD_B0,FALSE);

  /* Add runtime NVM data */
  if(param_ptr->clock_sharing == BT_SOC_CLOCK_SHARING_ENABLED)
  {
    if(BT_QSOC_IS_XO_SUPPORT_ENABLED(param_ptr->is_xo_support_enabled))
    {
      bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_CLK_SHARING_4020BD_B0_XO,
                              FALSE);
    }
    else
    {
      bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_CLK_SHARING_4020BD_B0,FALSE);
    }
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_97_CLK_SHARING_4020BD_B0,FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_NO_CLK_SHARING_4020BD_B0,FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_97_NO_CLK_SHARING_4020BD_B0,FALSE);
  }

  if(BT_QSOC_SOC_LOGGING_ENABLED(param_ptr->soc_logging))
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_38_SOC_LOGGING_4020BD_B0,FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_38_SOC_NO_LOGGING_4020BD_B0,FALSE);
  } 

  if(BT_2_1_FEATURE_SUPPORTED(param_ptr->bt_2_1_lisbon_disabled))
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_4020bd_b0_bt_2_1_supported[0],\
                            FALSE);

    if(param_ptr->refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
    {
      bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_19P2_4020BD_B0_BT2_1,FALSE);
    }

    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_72_4020_BD_B0_BT2_1,FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_2_1_supported[0],\
                            FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_4020bd_b0_bt_2_1_not_supported[0],\
                            FALSE);

    if(param_ptr->refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
    {
      bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_19P2_4020BD_B0_BT2_0,FALSE);
    }

    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_72_4020_BD_B0_BT2_0,FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_2_1_not_supported[0],\
                            FALSE);
  }
} /* init_4020BD_B0_entries */

/*==============================================================
FUNCTION:  init_4020BD_B1_entries
==============================================================*/
/**
  Initialize the fixed and runtime data for the 4020 BD B1 SOC.

  @see bt_qsoc_config_params_struct_type
  
  @sideeffects Undetermined.
*/
static void init_4020BD_B1_entries
(
  /** [in] Parameters that determine the run time configuration */
  bt_qsoc_config_params_struct_type *param_ptr
)
{
  /* Add fixed NVM data */
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_2_RAM,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_4_4020BD_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_5_4020BD_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_17_4020BD_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_27_4020BD_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_44_4020BD_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_45_4020BD_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_46_4020BD_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_53_4020BD_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_55_4020BD_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_57_4020BD_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_70_4020BD_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)build_tag_71(param_ptr),FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_76_4020BD_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_95_4020BD_B1,FALSE);

  /* Add runtime NVM data */
  if(param_ptr->clock_sharing == BT_SOC_CLOCK_SHARING_ENABLED)
  {
    if(BT_QSOC_IS_XO_SUPPORT_ENABLED(param_ptr->is_xo_support_enabled))
    {
      bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_CLK_SHARING_4020BD_B1_XO,
                              FALSE);
    }
    else
    {
      bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_CLK_SHARING_4020BD_B1,FALSE);
    }
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_97_CLK_SHARING_4020BD_B1,FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_NO_CLK_SHARING_4020BD_B1,FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_97_NO_CLK_SHARING_4020BD_B1,FALSE);
  }

  if(BT_QSOC_SOC_LOGGING_ENABLED(param_ptr->soc_logging))
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_38_SOC_LOGGING_4020BD_B1,FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_38_SOC_NO_LOGGING_4020BD_B1,FALSE);
  } 

  if(BT_2_1_FEATURE_SUPPORTED(param_ptr->bt_2_1_lisbon_disabled))
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_pre_4025_bt_2_1_supported[0],
                            FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_72_4020_BD_B1_BT2_1,FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_2_1_supported[0],
                            FALSE);
    /* update version tags */
    if(param_ptr->refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
    {
        bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_32_19p2MHz_4020bd_b1_bt_2_1[0],
                                FALSE);
    }
    else
    {
        bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_32_32MHz_4020bd_b1_bt_2_1[0],
                                FALSE);
    }
  }
  else  
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_pre_4025_bt_2_1_not_supported[0],
                            FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_72_4020_BD_B1_BT2_0,FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_2_1_not_supported[0],
                            FALSE);
    /* update version tags */
    if(param_ptr->refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
    {
        bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_32_19p2MHz_4020bd_b1_bt_2_0[0],
                                FALSE);
    }
    else
    {
        bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_32_32MHz_4020bd_b1_bt_2_0[0],
                                FALSE);
    }
  }

  if(param_ptr->refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_41_19P2MHZ_4020_BD_B1,FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_99_19P2MHZ_4020_BD_B1,FALSE);
  }
  else 
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_41_32MHZ_4020_BD_B1,FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_99_32MHZ_4020_BD_B1,FALSE);
  }    
} /* init_4020BD_B1_entries */

/*==============================================================
FUNCTION:  init_4021_B1_entries
==============================================================*/
/**
  Initialize the fixed and runtime data for the 4021 B1 SOC.

  @see bt_qsoc_config_params_struct_type
  
  @sideeffects Undetermined.
*/
static void init_4021_B1_entries
(
  /** [in] Parameters that determine the run time configuration */
  bt_qsoc_config_params_struct_type *param_ptr
)
{
  /* Add fixed NVM data */
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_2_RAM,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_4_4021_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_5_4021_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_17_4021_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_27_4021_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_44_4021_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_45_4021_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_46_4021_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_53_4021_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_55_4021_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_57_4021_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_70_4021_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)build_tag_71(param_ptr),FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_76_4021_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_95_4021_B1,FALSE);

  /* Add runtime NVM data */
  if(param_ptr->clock_sharing == BT_SOC_CLOCK_SHARING_ENABLED)
  {
    if(BT_QSOC_IS_XO_SUPPORT_ENABLED(param_ptr->is_xo_support_enabled))
    {
      bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_CLK_SHARING_4021_B1_XO,
                              FALSE);
    }
    else
    {
      bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_CLK_SHARING_4021_B1,FALSE);
    }
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_97_CLK_SHARING_4021_B1,FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_NO_CLK_SHARING_4021_B1,FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_97_NO_CLK_SHARING_4021_B1,FALSE);
  }

  if(BT_QSOC_SOC_LOGGING_ENABLED(param_ptr->soc_logging))
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_38_SOC_LOGGING_4021_B1,FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_38_SOC_NO_LOGGING_4021_B1,FALSE);
  }

  if(BT_2_1_FEATURE_SUPPORTED(param_ptr->bt_2_1_lisbon_disabled))
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_pre_4025_bt_2_1_supported[0],\
                            FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_2_1_supported[0],\
                            FALSE);
  }
  else  
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_pre_4025_bt_2_1_not_supported[0],\
                            FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_2_1_not_supported[0],\
                            FALSE);
  }
} /* init_4021_B1_entries */

/*==============================================================
FUNCTION:  init_4025_A0_entries
==============================================================*/
/**
  Initialize the fixed and runtime data for the 4025 A0 SOC.

  @see bt_qsoc_config_params_struct_type
  
  @sideeffects Undetermined.
*/
static void init_4025_A0_entries
(
  /** [in] Parameters that determine the run time configuration */
  bt_qsoc_config_params_struct_type *param_ptr
)
{
  /* Add fixed NVM data */
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_2_RAM,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_4_4025_A0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_5_4025_A0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_6_4025_A0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_28_4025_A0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_36_4025_A0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_39_4025_A0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_40_4025_A0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_44_4025_A0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_45_4025_A0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_46_4025_A0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_53_4025_A0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_55_4025_A0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_57_4025_A0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_70_4025_A0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)build_tag_71(param_ptr),FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_72_4025_A0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_78_4025_A0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_95_4025_A0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_107_4025_A0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_108_4025_A0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_111_4025_A0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_112_4025_A0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_111_4025_A0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_113_4025_A0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_116_4025_A0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_119_4025_A0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_123_4025_A0,FALSE);

  /* Add runtime NVM data */
  if(param_ptr->refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_32_19p2MHz_4025_a0[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_41_19P2MHZ_4025_A0,FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_99_19P2MHZ_4025_A0,FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_114_19P2MHZ_4025_A0,FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_115_19P2MHZ_4025_A0,FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_32_32MHz_4025_a0[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_41_32MHZ_4025_A0,FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_99_32MHZ_4025_A0,FALSE);
  }

  if(param_ptr->clock_sharing == BT_SOC_CLOCK_SHARING_ENABLED)
  {
    if(BT_QSOC_IS_XO_SUPPORT_ENABLED(param_ptr->is_xo_support_enabled))
    {
      bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_CLK_SHARING_4025_A0_XO,
                              FALSE);
    }
    else
    {
      bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_CLK_SHARING_4025_A0,FALSE);
    }
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_97_CLK_SHARING_4025_A0,FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_NO_CLK_SHARING_4025_A0,FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_97_NO_CLK_SHARING_4025_A0,FALSE);
  }
} /* init_4025_A0_entries */

/*==============================================================
FUNCTION:  init_4025_B0_entries
==============================================================*/
/**
  Initialize the fixed and runtime data for the 4025 B0 SOC.

  @see bt_qsoc_config_params_struct_type
  
  @sideeffects Undetermined.
*/
static void init_4025_B0_entries
(
  /** [in] Parameters that determine the run time configuration */
  bt_qsoc_config_params_struct_type *param_ptr
)
{
  /* Add fixed NVM data */
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_2_RAM,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_4_4025_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_5_4025_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_17_4025_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_27_4025_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_40_4025_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_44_4025_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_45_4025_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_46_4025_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_53_4025_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_55_4025_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_70_4025_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)build_tag_71(param_ptr),FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_72_4025_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_95_4025_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_112_4025_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_113_4025_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_114_4025_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_115_4025_B0,FALSE);

  /* Add runtime NVM data */
  if(param_ptr->wlan_type == BT_QSOC_WLAN_LIBRA)
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_57_4025_libra[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_74_libra[0],FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_57_non_libra[0],FALSE);
    // use tag 74 ROM default when not attached to Libra
  }

  if(param_ptr->clock_sharing == BT_SOC_CLOCK_SHARING_ENABLED)
  {
    if(BT_QSOC_IS_XO_SUPPORT_ENABLED(param_ptr->is_xo_support_enabled))
    {
      bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_CLK_SHARING_4025_B0_XO,
                              FALSE);
    }
    else
    {
      bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_CLK_SHARING_4025_B0,FALSE);
    }
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_97_CLK_SHARING_4025_B0,FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_NO_CLK_SHARING_4025_B0,FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_97_NO_CLK_SHARING_4025_B0,FALSE);
  }

  if(BT_QSOC_SOC_LOGGING_ENABLED(param_ptr->soc_logging))
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_38_SOC_LOGGING_4025_B0,FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_38_SOC_NO_LOGGING_4025_B0,FALSE);
  }

  if(BT_2_1_FEATURE_SUPPORTED(param_ptr->bt_2_1_lisbon_disabled))
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bt_2_1_supported[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_2_1_supported[0],FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bt_2_1_not_supported[0],\
                            FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_2_1_not_supported[0],\
                            FALSE);
  }
} /* init_4025_B0_entries */

/*==============================================================
FUNCTION:  init_4025_B1_entries
==============================================================*/
/**
  Initialize the fixed and runtime data for the 4025 B1 SOC.

  @see bt_qsoc_config_params_struct_type
  
  @sideeffects Undetermined.
*/
static void init_4025_B1_entries
(
  /** [in] Parameters that determine the run time configuration */
  bt_qsoc_config_params_struct_type *param_ptr
)
{
  /* Add fixed NVM data */
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_2_RAM,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_4_4025_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_5_4025_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_17_4025_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_27_4025_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_44_4025_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_45_4025_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_46_4025_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_53_4025_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_55_4025_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_70_4025_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)build_tag_71(param_ptr),FALSE);
  
  /* Add runtime NVM data */
  if(param_ptr->wlan_type == BT_QSOC_WLAN_LIBRA)
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_57_4025_libra[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_74_libra[0],FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_57_non_libra[0],FALSE);
    // use tag 74 ROM default when not attached to Libra
  }

  if(param_ptr->clock_sharing == BT_SOC_CLOCK_SHARING_ENABLED)
  {
    if(BT_QSOC_IS_XO_SUPPORT_ENABLED(param_ptr->is_xo_support_enabled))
    {
      bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_CLK_SHARING_4025_B1_XO,
                              FALSE);
    }
    else
    {
      bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_CLK_SHARING_4025_B1,FALSE);
    }
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_97_CLK_SHARING_4025_B1,FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_NO_CLK_SHARING_4025_B1,FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_97_NO_CLK_SHARING_4025_B1,FALSE);
  }

  if(BT_QSOC_SOC_LOGGING_ENABLED(param_ptr->soc_logging))
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_38_SOC_LOGGING_4025_B1,FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_38_SOC_NO_LOGGING_4025_B1,FALSE);
  }

  if(BT_2_1_FEATURE_SUPPORTED(param_ptr->bt_2_1_lisbon_disabled))
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bt_2_1_supported[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_2_1_supported[0],FALSE);

    if(param_ptr->refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
    {
      bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_32_19p2MHz_4025_b1_bt_2_1[0],\
                              FALSE);
    }
    else
    {
      bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_32_32MHz_4025_b1_bt_2_1[0],\
                              FALSE);
    }      
  }
  else  
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bt_2_1_not_supported[0],\
                            FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_2_1_not_supported[0],\
                            FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_72_4025_B1,FALSE);

    if(param_ptr->refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
    {
      if(param_ptr->bt_qsoc_bredr_dev_class == BT_QSOC_DEV_CLASS2)
      {
        bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_32_19P2MHz_4025_b1_CLASS2[0],\
                                FALSE);
      }
      else
      {
        bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_32_19p2MHz_4025_b1_bt_2_0[0],\
                                FALSE);
      }
    }
    else 
    {
      if(param_ptr->bt_qsoc_bredr_dev_class == BT_QSOC_DEV_CLASS2)
      {
        bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_32_32MHz_4025_b1_CLASS2[0],\
                                FALSE);
      }
      else
      {
        bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_32_32MHz_4025_b1_bt_2_0[0],\
                                FALSE);
      }
    }    
  }

  if(param_ptr->bt_qsoc_bredr_dev_class == BT_QSOC_DEV_CLASS2)
  {       
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_36_4025_CLASS2[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_56_4025_CLASS2[0],FALSE);   
    if(param_ptr->refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
    {
      bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_41_19P2MHz_4025_CLASS2[0],\
                              FALSE);
    }
    else
    {
      bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_41_32MHz_4025_CLASS2[0],\
                              FALSE);
    }   
  }
  else
  {       
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_36_4025_CLASS1[0],FALSE);
  }
} /* init_4025_B1_entries */

/*==============================================================
FUNCTION:  init_4025_B2_entries
==============================================================*/
/**
  Initialize the fixed and runtime data for the 4025 B2 SOC.

  @see bt_qsoc_config_params_struct_type
  
  @sideeffects Undetermined.
*/
static void init_4025_B2_entries
(
  /** [in] Parameters that determine the run time configuration */
  bt_qsoc_config_params_struct_type *param_ptr
)
{
  /* Add fixed NVM data */
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_2_RAM,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_4_4025_B2,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_5_4025_B2,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_17_4025_B2,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_27_4025_B2,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_44_4025_B2,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_45_4025_B2,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_46_4025_B2,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_53_4025_B2,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_55_4025_B2,FALSE);
  // now a runtime tag -- bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_57_4025_B2,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_70_4025_B2,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)build_tag_71(param_ptr),FALSE);
  
  /* Add runtime NVM data */
  if(param_ptr->wlan_type == BT_QSOC_WLAN_LIBRA)
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_57_4025_libra[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_74_libra[0],FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_57_non_libra[0],FALSE);
    // use tag 74 ROM default when not attached to Libra
  }

  if(param_ptr->clock_sharing == BT_SOC_CLOCK_SHARING_ENABLED)
  {
    if(BT_QSOC_IS_XO_SUPPORT_ENABLED(param_ptr->is_xo_support_enabled))
    {
      bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_CLK_SHARING_4025_B2_XO,
                              FALSE);
    }
    else
    {
      bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_CLK_SHARING_4025_B2,FALSE);
    }
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_97_CLK_SHARING_4025_B2,FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_NO_CLK_SHARING_4025_B2,FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_97_NO_CLK_SHARING_4025_B2,FALSE);
  }

  if(BT_QSOC_SOC_LOGGING_ENABLED(param_ptr->soc_logging))
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_38_SOC_LOGGING_4025_B2,FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_38_SOC_NO_LOGGING_4025_B2,FALSE);
  }
  /* Disabling the following in Bluetooth Supported Features Mask
     Byte#5, Bit5: EDR eSCO DQPSM mode  Bit6: EDR eSCO 8 DPSK mode
      Bit7: EDR eSCO 3 slot
  */
  bt_qsoc_cfg_tag_6_bt_2_1_supported[NVM_TAG_HEADER_SIZE + NVM_TAG_6_BYTE_5_INDEX] &= 0x1F;
  if(BT_2_1_FEATURE_SUPPORTED(param_ptr->bt_2_1_lisbon_disabled))
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bt_2_1_supported[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_2_1_supported[0],FALSE);

    if(param_ptr->refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
    {
      if(param_ptr->bt_qsoc_bredr_dev_class == BT_QSOC_DEV_CLASS2)
      {
        bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_32_19P2MHz_4025_b2_CLASS2[0],\
                                FALSE);
      }
      else
      {
        bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_32_19p2MHz_4025_b2_bt_2_1[0],\
                                FALSE);
      }
    }
    else
    {
      if(param_ptr->bt_qsoc_bredr_dev_class == BT_QSOC_DEV_CLASS2)
      {
        bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_32_32MHz_4025_b2_CLASS2[0],\
                                FALSE);
      }
      else
      {
        bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_32_32MHz_4025_b2_bt_2_1[0],\
                                FALSE);
      }
    }       
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bt_2_1_not_supported[0],\
                            FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_2_1_not_supported[0],\
                            FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_72_4025_B2,FALSE);

    if(param_ptr->refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
    {
      bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_32_19p2MHz_4025_b2_bt_2_0[0],\
                              FALSE);
    }
    else 
    {
      bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_32_32MHz_4025_b2_bt_2_0[0],\
                              FALSE);
    }    
  }

  if(param_ptr->bt_qsoc_bredr_dev_class == BT_QSOC_DEV_CLASS2)
  {      
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_36_4025_CLASS2[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_56_4025_CLASS2[0],FALSE);

    if(param_ptr->refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
    {
      bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_41_19P2MHz_4025_CLASS2[0],\
                              FALSE);
    }
    else
    {
      bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_41_32MHz_4025_CLASS2[0],\
                              FALSE);
    }
  }
  else
  {      
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_36_4025_CLASS1[0],\
                            FALSE);
  }
} /* init_4025_B2_entries */


/*==============================================================
FUNCTION:  init_4025_B3_entries
==============================================================*/
/**
  Initialize the fixed and runtime data for the 4025 B3 SOC.

  @see bt_qsoc_config_params_struct_type
  
  @sideeffects Undetermined.
*/
static void init_4025_B3_entries
(
  /** [in] Parameters that determine the run time configuration */
  bt_qsoc_config_params_struct_type *param_ptr
)
{
  /* Add fixed NVM data */
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_2_RAM,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_4_4025_B3,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_5_4025_B3,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_17_4025_B3,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_27_4025_B3,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_44_4025_B3,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_45_4025_B3,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_46_4025_B3,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_53_4025_B3,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_55_4025_B3,FALSE);
  // now a runtime tag -- bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_57_4025_B2,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_70_4025_B3,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)build_tag_71(param_ptr),FALSE);
  
  /* Add runtime NVM data */
  if(param_ptr->wlan_type == BT_QSOC_WLAN_LIBRA)
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_57_4025_libra[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_74_libra[0],FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_57_non_libra[0],FALSE);
    // use tag 74 ROM default when not attached to Libra
  }

  if(param_ptr->clock_sharing == BT_SOC_CLOCK_SHARING_ENABLED)
  {
    if(BT_QSOC_IS_XO_SUPPORT_ENABLED(param_ptr->is_xo_support_enabled))
    {
      bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_CLK_SHARING_4025_B3_XO,
                              FALSE);
    }
    else
    {
      bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_CLK_SHARING_4025_B3,FALSE);
    }
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_97_CLK_SHARING_4025_B3,FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_NO_CLK_SHARING_4025_B3,FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_97_NO_CLK_SHARING_4025_B3,FALSE);
  }

  if(BT_QSOC_SOC_LOGGING_ENABLED(param_ptr->soc_logging))
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_38_SOC_LOGGING_4025_B3,FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_38_SOC_NO_LOGGING_4025_B3,FALSE);
  }

  if(BT_2_1_FEATURE_SUPPORTED(param_ptr->bt_2_1_lisbon_disabled))
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bt_2_1_supported[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_2_1_supported[0],FALSE);

    if(param_ptr->refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
    {
      if(param_ptr->bt_qsoc_bredr_dev_class == BT_QSOC_DEV_CLASS2)
      {
        bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_32_19P2MHz_4025_b3_CLASS2[0],\
                                FALSE);
      }
      else
      {
        bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_32_19p2MHz_4025_b3_bt_2_1[0],\
                                FALSE);
      }
    }
    else
    {
      if(param_ptr->bt_qsoc_bredr_dev_class == BT_QSOC_DEV_CLASS2)
      {
        bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_32_32MHz_4025_b3_CLASS2[0],\
                                FALSE);
      }
      else
      {
        bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_32_32MHz_4025_b3_bt_2_1[0],\
                                FALSE);
      }
    }       
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bt_2_1_not_supported[0],\
                            FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_2_1_not_supported[0],\
                            FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_72_4025_B2,FALSE);

    if(param_ptr->refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
    {
      bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_32_19p2MHz_4025_b3_bt_2_0[0],\
                              FALSE);
    }
    else 
    {
      bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_32_32MHz_4025_b3_bt_2_0[0],\
                              FALSE);
    }    
  }

  if(param_ptr->bt_qsoc_bredr_dev_class == BT_QSOC_DEV_CLASS2)
  {      
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_36_4025_CLASS2[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_56_4025_CLASS2[0],FALSE);

    if(param_ptr->refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
    {
      bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_41_19P2MHz_4025_CLASS2[0],\
                              FALSE);
    }
    else
    {
      bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_41_32MHz_4025_CLASS2[0],\
                              FALSE);
    }
  }
  else
  {      
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_36_4025_CLASS1[0],\
                            FALSE);
  }
} /* init_4025_B3_entries */

/*==============================================================
FUNCTION:  init_mba_a0_entries
==============================================================*/
/**
  Initialize the fixed and runtime data for the Marimba A0 SOC.

  @see bt_qsoc_config_params_struct_type
  
  @sideeffects Undetermined.
*/
static void init_mba_a0_entries
(
  /** [in] Parameters that determine the run time configuration */
  bt_qsoc_config_params_struct_type *param_ptr
)
{
  /* Add fixed NVM data */
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_2_RAM,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_4_MBA_A0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_5_MBA_A0,FALSE);

  /* Disable Sleep (HW/SW Inband) in FTM Mode */
  if(BT_QSOC_IS_FTM_MODE(param_ptr->is_ftm_mode))
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_17_MBA_A0_FTM,FALSE); 
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_17_MBA_A0,FALSE);
  }
  //Currently, deep sleep is disabled. 
  // (see CFG_TAG_27_MBA_A0 definition)
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_27_MBA_A0,FALSE); 
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_44_MBA_A0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_45_MBA_A0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_46_MBA_A0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_53_MBA_A0,FALSE);
  // now a runtime tag -- bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_57_MBA_A0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_70_MBA_A0,FALSE);
 if(BT_QSOC_SOC_LOGGING_ENABLED(param_ptr->soc_logging))
  {
    bt_qsoc_nvm_add_element((uint8 *)build_srl_tag_71(param_ptr),FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_38_SOC_LOGGING_MBA_A0,FALSE);
  }
  else  
  {
    bt_qsoc_nvm_add_element((uint8 *)build_tag_71(param_ptr),FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_38_SOC_NO_LOGGING_MBA_A0,FALSE);
  }

  /* Add runtime NVM data */
  if(param_ptr->wlan_type == BT_QSOC_WLAN_LIBRA)
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_57_mba_libra[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_74_libra[0],FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_57_non_libra[0],FALSE);
    // use tag 74 ROM default when not attached to Libra
  }

  if(param_ptr->clock_sharing == BT_SOC_CLOCK_SHARING_ENABLED)
  {
    if(BT_QSOC_IS_XO_SUPPORT_ENABLED(param_ptr->is_xo_support_enabled))
    {
      bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_CLK_SHARING_MBA_A0_XO,FALSE);
    }
    else
    {
      bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_CLK_SHARING_MBA_A0,FALSE);
    }
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_97_CLK_SHARING_MBA_A0,FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_NO_CLK_SHARING_MBA_A0,FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_97_NO_CLK_SHARING_MBA_A0,FALSE);
  }

  if(BT_2_1_FEATURE_SUPPORTED(param_ptr->bt_2_1_lisbon_disabled))
  {
#if !defined (FEATURE_BT_3_0) && !defined (FEATURE_BT_4_0)
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bt_2_1_supported[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_2_1_supported[0],FALSE);
#else
    //Since BT Core on Marimba supports 3.0 and so also the host,
    //even if FEATURE_BT_4_0 is defined we need to advertise as 3.0
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bt_3_0_supported[0],FALSE);
#endif //Bluetooth version information ROM defaulted, implying 3.0 spec

    if(param_ptr->refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
    {
      bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_32_19p2MHz_mba_a0[0],FALSE);
    }
    else
    {
        /* Fill in if 32MHz support is enabled */
    }      
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bt_2_1_not_supported[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_2_1_not_supported[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_72_MBA_A0,FALSE);

    if(param_ptr->refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
    {
      /* Same release no. for 2.0 and 2.1 NVM releases */
      bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_32_19p2MHz_mba_a0[0],FALSE);
    }
    else 
    {
      /* Fill in if 32MHz support is enabled */
    }    
   }

  /* Add class2 support when available*/
  if(param_ptr->bt_qsoc_bredr_dev_class == BT_QSOC_DEV_CLASS2)
  {
    if(param_ptr->refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
    {
    }
    else
    {
    }
  }
  else
  {       
  }
} /* init_mba_a0_entries */


/*==============================================================
FUNCTION:  init_mba_b0_entries
==============================================================*/
/**
  Initialize the fixed and runtime data for the Marimba B0 SOC.

  @see bt_qsoc_config_params_struct_type

  @sideeffects Undetermined.
*/
static void init_mba_b0_entries
(
  /** [in] Parameters that determine the run time configuration */
  bt_qsoc_config_params_struct_type *param_ptr
)
{
  /* Add fixed NVM data */
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_2_RAM,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_4_MBA_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_5_MBA_B0,FALSE);

  /* Disable Sleep (HW/SW Inband) in FTM Mode */
  if(BT_QSOC_IS_FTM_MODE(param_ptr->is_ftm_mode))
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_17_MBA_B0_FTM,FALSE); // TBD !!!!
    
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_17_MBA_B0,FALSE); // TBD !!!!
  }

  //Currently, deep sleep is disabled for sleep to work. 
  // (see CFG_TAG_27_MBA_B0 definition)
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_27_MBA_B0,FALSE); 
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_44_MBA_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_45_MBA_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_46_MBA_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_53_MBA_B0,FALSE);
  // now a runtime tag -- bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_57_MBA_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_70_MBA_B0,FALSE);
  /* Check if SoC Logger Redirection Enabled - 
  Different Rx/Tx Buff numbers for both cases*/
  if(BT_QSOC_SOC_LOGGING_ENABLED(param_ptr->soc_logging))
  {
    bt_qsoc_nvm_add_element((uint8 *)build_srl_tag_71(param_ptr),FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_38_SOC_LOGGING_MBA_B0,FALSE);
  }
  else  
  {
    bt_qsoc_nvm_add_element((uint8 *)build_tag_71(param_ptr),FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_38_SOC_NO_LOGGING_MBA_B0,FALSE);
  }

  /* Add runtime NVM data */
  if(param_ptr->wlan_type == BT_QSOC_WLAN_LIBRA)
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_57_mba_libra[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_74_libra[0],FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_57_non_libra[0],FALSE);
    // use tag 74 ROM default when not attached to Libra
  }

  if(param_ptr->clock_sharing == BT_SOC_CLOCK_SHARING_ENABLED)
  {
    if(BT_QSOC_IS_XO_SUPPORT_ENABLED(param_ptr->is_xo_support_enabled))
    {
      /* TBD ? */
      bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_CLK_SHARING_MBA_B0_XO,FALSE);
    }
    else
    {
      /* TBD? */
      bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_CLK_SHARING_MBA_B0,FALSE);
    }
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_97_CLK_SHARING_MBA_B0,FALSE);
  }
  else
  {
     /* Fill in if Clock sharing is disabled*/
  }

  if(BT_2_1_FEATURE_SUPPORTED(param_ptr->bt_2_1_lisbon_disabled))
  {
#if !defined (FEATURE_BT_3_0) && !defined (FEATURE_BT_4_0)
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bt_2_1_supported[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_2_1_supported[0],FALSE);
#else
    //Since BT Core on Marimba supports 3.0 and so also the host,
    //even if FEATURE_BT_4_0 is defined we need to advertise as 3.0
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bt_3_0_supported[0],FALSE);
#endif //Bluetooth version information ROM defaulted, implying 3.0 spec

    if(param_ptr->refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
    {
      bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_32_19p2MHz_mba_b0[0],FALSE); // TBD !!!
    }
    else
    {
        /* Fill in if 32MHz support is enabled */
    }      
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bt_2_1_not_supported[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_2_1_not_supported[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_72_MBA_B0,FALSE);

    if(param_ptr->refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
    {
      /* Same release no. for 2.0 and 2.1 NVM releases */
      bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_32_19p2MHz_mba_b0[0],FALSE);
    }
    else 
    {
      /* Fill in if 32MHz support is enabled */
    }    
   }

  /* Add class2 support when available*/
  if(param_ptr->bt_qsoc_bredr_dev_class == BT_QSOC_DEV_CLASS2)
  {
    if(param_ptr->refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
    {
    }
    else
    {
    }
  }
  else if(param_ptr->bt_qsoc_bredr_dev_class == BT_QSOC_DEV_CLASS_CUSTOM)
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_36_mba_b0_custom[0],FALSE);
  }
  else
  {       
  }
} /* init_mba_b0_entries */

/*==============================================================
FUNCTION:  init_mba_b1_entries
==============================================================*/
/**
  Initialize the fixed and runtime data for the Marimba B1 SOC.

  @see bt_qsoc_config_params_struct_type
  
  @sideeffects Undetermined.
*/
static void init_mba_b1_entries
(
  /** [in] Parameters that determine the run time configuration */
  bt_qsoc_config_params_struct_type *param_ptr
)
{
  /* Add fixed NVM data */
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_2_RAM,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_4_MBA_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_5_MBA_B1,FALSE);
  /* Disable Sleep (HW/SW Inband) in FTM Mode */
  if(BT_QSOC_IS_FTM_MODE(param_ptr->is_ftm_mode))
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_17_MBA_B1_FTM,FALSE); // TBD !!!!
    
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_17_MBA_B1,FALSE); // TBD !!!!
  }
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_27_MBA_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_44_MBA_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_45_MBA_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_46_MBA_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_53_MBA_B1,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_70_MBA_B1,FALSE);
  /* Check if SoC Logger Redirection Enabled - 
  Different Rx/Tx Buff numbers for both cases*/
  if(BT_QSOC_SOC_LOGGING_ENABLED(param_ptr->soc_logging))
  {
    bt_qsoc_nvm_add_element((uint8 *)build_srl_tag_71(param_ptr),FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_38_SOC_LOGGING_MBA_B1,FALSE);
  }
  else  
  {
    bt_qsoc_nvm_add_element((uint8 *)build_tag_71(param_ptr),FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_38_SOC_NO_LOGGING_MBA_B1,FALSE);
  }
  /* Add runtime NVM data */
  if(param_ptr->wlan_type == BT_QSOC_WLAN_LIBRA)
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_57_mba_libra[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_74_libra[0],FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_57_non_libra[0],FALSE);
    // use tag 74 ROM default when not attached to Libra
  }

  if(param_ptr->clock_sharing == BT_SOC_CLOCK_SHARING_ENABLED)
  {
    if(BT_QSOC_IS_XO_SUPPORT_ENABLED(param_ptr->is_xo_support_enabled))
    {
      /* TBD ? */
      bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_CLK_SHARING_MBA_B1_XO,FALSE);
    }
    else
    {
      /* TBD? */
      bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_CLK_SHARING_MBA_B1,FALSE);
    }
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_97_CLK_SHARING_MBA_B1,FALSE);
  }
  else
  {
     /* Fill in if Clock sharing is disabled*/
  }

  if(BT_2_1_FEATURE_SUPPORTED(param_ptr->bt_2_1_lisbon_disabled))
  {
#if !defined (FEATURE_BT_3_0) && !defined (FEATURE_BT_4_0)
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bt_2_1_supported[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_2_1_supported[0],FALSE);
#else
    //Since BT Core on Marimba supports 3.0 and so also the host,
    //even if FEATURE_BT_4_0 is defined we need to advertise as 3.0
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bt_3_0_supported[0],FALSE);
#endif //Bluetooth version information ROM defaulted, implying 3.0 spec

    if(param_ptr->refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
    {
      if(param_ptr->bt_qsoc_bredr_dev_class == BT_QSOC_DEV_CLASS2)
      {
        bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_32_19p2MHz_mba_b1_CLASS2[0],\
                                FALSE);
      }
      else
      {
        bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_32_19p2MHz_mba_b1[0],\
                                FALSE);
      }
    }
    else
    {
        /* Fill in if 32MHz support is enabled */
    }      
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bt_2_1_not_supported[0],\
                            FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_2_1_not_supported[0],\
                            FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_72_MBA_B1,FALSE);

    if(param_ptr->refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
    {
      /* Same release no. for 2.0 and 2.1 NVM releases */
      bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_32_19p2MHz_mba_b1[0],FALSE);
    }
    else 
    {
      /* Fill in if 32MHz support is enabled */
    }    
   }

  /* Add class2 support when available*/
  if(param_ptr->bt_qsoc_bredr_dev_class == BT_QSOC_DEV_CLASS2)
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_36_mba_b1_CLASS2[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_56_mba_b1_CLASS2[0],FALSE);

    if(param_ptr->refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
    {
    }
    else
    {
      /* Fill in if 32MHz support is enabled */
    }
  }
  else if(param_ptr->bt_qsoc_bredr_dev_class == BT_QSOC_DEV_CLASS_CUSTOM)
  {
    //Reducing power by disabling top 4 power levels where VCO pulling
    //is causing TX distortion.
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_36_mba_b1_custom[0],FALSE);
  }
  else
  {
    //Here tag 36 value from the NVM release string 
    //i.e. bt_qsoc_nvm_MARIMBA_B1.c would be sent.
    //And tag 56 value would be defaulted to ROM contents.
  }
  //tag 41 value is currently same for class 1.5 and class 2. 
  //Hence part of dynamic c-string class of NVM tags.
  
} /* init_mba_b1_entries */

/*==============================================================
FUNCTION:  init_bha_a0_entries
==============================================================*/
/**
  Initialize the fixed and runtime data for the Bahama A0 SOC.

  @see bt_qsoc_config_params_struct_type

  @sideeffects Undetermined.
*/
static void init_bha_a0_entries
(
  /** [in] Parameters that determine the run time configuration */
  bt_qsoc_config_params_struct_type *param_ptr
)
{
  /* Add fixed NVM data */
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_2_RAM,FALSE);
  if(BT_2_1_FEATURE_SUPPORTED(param_ptr->bt_2_1_lisbon_disabled))
  {
#if defined (FEATURE_BT_3_0)
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bha_bt_3_0_supported[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_3_0_supported[0],FALSE);
#elif defined (FEATURE_BT_4_0)
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bha_bt_4_0_supported[0],FALSE);
    //Bluetooth version information ROM defaulted, implying 4.0 spec
#else
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bt_2_1_supported[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_2_1_supported[0],FALSE);
#endif
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bt_2_1_not_supported[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_2_1_not_supported[0],FALSE);
  }
  
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_17_BHA_A0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_27_BHA_A0,FALSE);
  
  //tag 32/99 is as per dynamic set (C-String), 
  //since only 19p2 Master Ref CLK NVM released.
  
  //tag 36 is currently ROM defaulted as only class 1 NVM is released.
  
  if(param_ptr->clock_sharing == BT_SOC_CLOCK_SHARING_ENABLED)
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_CLK_SHARING_BHA_A0,FALSE);
  }
  else  
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_NO_CLK_SHARING_BHA_A0,FALSE);
  }

  if(BT_QSOC_SOC_LOGGING_ENABLED(param_ptr->soc_logging))
  {
    bt_qsoc_nvm_add_element((uint8 *)build_srl_tag_71(param_ptr),FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_38_SOC_LOGGING_BHA_A0,FALSE);
  }
  else  
  {
    bt_qsoc_nvm_add_element((uint8 *)build_tag_71(param_ptr),FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_38_SOC_NO_LOGGING_BHA_A0,FALSE);
  }

  //tag 44 needed to override voice settings defaults for interop with MSM
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_44_BHA_A0,FALSE);

  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_53_BHA_A0,FALSE);
  //For BT-WLAN coexistence, 
  //tag 55(GPIO Mapping)/57(AFH Parameters)
  //defaulted to ROM values. Based on WLAN type these parameters 
  //need to be tweaked.

  if(param_ptr->wlan_type == BT_QSOC_WLAN_LIBRA)
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_74_libra[0],FALSE);
  }
  // use tag 74 ROM default when not attached to Libra
  
  //Currently most of the NVM are from the dynamic set (i.e. C-String),
  //as currently only a variant of NVM for 19p2MHz Master Reference CLK
  //Power Class 1 and 3.0 spec is released. Once multiple variants based on 
  //Master Reference CLK/Power Class/Spec is released, changes have to be 
  //incorporated. 

} /* init_bha_a0_entries */

/*==============================================================
FUNCTION:  init_bha_b0_entries
==============================================================*/
/**
  Initialize the fixed and runtime data for the Bahama B0 SOC.

  @see bt_qsoc_config_params_struct_type

  @sideeffects Undetermined.
*/
static void init_bha_b0_entries
(
  /** [in] Parameters that determine the run time configuration */
  bt_qsoc_config_params_struct_type *param_ptr
)
{
  /* Add fixed NVM data */
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_2_RAM,FALSE);
  if(BT_2_1_FEATURE_SUPPORTED(param_ptr->bt_2_1_lisbon_disabled))
  {
#if defined (FEATURE_BT_3_0)
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bha_bt_3_0_supported[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_3_0_supported[0],FALSE);
#elif defined (FEATURE_BT_4_0)
    if(param_ptr->bt_qsoc_le_dev_class == BT_QSOC_DEV_CLASS2)
    {
       bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_83_BHA_B0_CLASS2,FALSE);
    }
    else if(param_ptr->bt_qsoc_le_dev_class == BT_QSOC_DEV_CLASS1)
    {
      bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_83_BHA_B0_CLASS1,FALSE);
    }

#ifdef ANDROID
    char le_support_disable[PROPERTY_VALUE_MAX];
    //Remove LE support from BT feature mask if LE is not supported on the target.
    property_get("ro.bluetooth.le.disable", le_support_disable, "false");
    if (!strcmp(le_support_disable, "true")) {
      bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bha_bt_3_0_supported[0],FALSE);
      bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_3_0_supported[0],FALSE);
    }
    else {
      bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bha_bt_4_0_supported[0],FALSE);
    }
#else
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bha_bt_4_0_supported[0],FALSE);
#endif

    //Bluetooth version information ROM defaulted, implying 4.0 spec
#else
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bt_2_1_supported[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_2_1_supported[0],FALSE);
#endif
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bt_2_1_not_supported[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_2_1_not_supported[0],FALSE);
  }

  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_17_BHA_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_27_BHA_B0,FALSE);


  if(param_ptr->clock_sharing == BT_SOC_CLOCK_SHARING_ENABLED)
  {
    if(BT_QSOC_IS_XO_SUPPORT_ENABLED(param_ptr->is_xo_support_enabled))
      {
        bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_CLK_SHARING_BHA_B0,FALSE);
      } else {
        bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_CLK_SHARING_BHA_B0_TCXO,FALSE);
      }
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_NO_CLK_SHARING_BHA_B0,FALSE);
  }

  if(BT_QSOC_SOC_LOGGING_ENABLED(param_ptr->soc_logging))
  {
    bt_qsoc_nvm_add_element((uint8 *)build_srl_tag_71(param_ptr),FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_38_SOC_LOGGING_BHA_B0,FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)build_tag_71(param_ptr),FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_38_SOC_NO_LOGGING_BHA_B0,FALSE);
  }

  //tag 44 needed to override voice settings defaults for interop with MSM
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_44_BHA_B0,FALSE);

  //For BT-WLAN coexistence
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_53_BHA_B0,FALSE);

  //tag 55(GPIO Mapping)
  //Based on WLAN type GPIO Mapping need to be set.

  //Tag 57 AFH Parameters. Need to be tweaked based on WLAN type
  //Currently we are using same values for both WCN and AR Wlan types
  bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_57_bha[0],FALSE);

  //Tag 63 QTA Coex Parameters. Need to be tweaked based on WLAN type
  //Currently we are using same values for both WCN and AR Wlan types
  bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_63_bha[0],FALSE);

  //Tag 74 Coex Burst Timers. Need to be tweaked based on WLAN type
  if(param_ptr->wlan_type == BT_QSOC_WLAN_LIBRA)
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_74_libra[0],FALSE);
  }
  else
  {
    // use tag 74 as ROM default when not attached to Libra
  }

  if(param_ptr->bt_qsoc_bredr_dev_class == BT_QSOC_DEV_CLASS2)
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_36_bha_b0_CLASS2[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_56_bha_b0_CLASS2[0],FALSE);
    bt_qsoc_nvm_add_element(
                    (uint8 *)&bt_qsoc_cfg_tag_32_19p2MHz_bha_b0_CLASS2[0],FALSE);
  }
  else if(param_ptr->bt_qsoc_bredr_dev_class == BT_QSOC_DEV_CLASS1)
  {
    //Here tag 32 and 36 value from the NVM release string
    //i.e. bt_qsoc_nvm_BAHAMA_B0.c would be sent.
    //And tag 56 value would be defaulted to ROM contents.
  }
} /* init_bha_b0_entries */

/*==============================================================
FUNCTION:  init_bha_b1_entries
==============================================================*/
/**
  Initialize the fixed and runtime data for the Bahama B1 SOC.

  @see bt_qsoc_config_params_struct_type

  @sideeffects Undetermined.
*/
static void init_bha_b1_entries
(
  /** [in] Parameters that determine the run time configuration */
  bt_qsoc_config_params_struct_type *param_ptr
)
{
  /* Add fixed NVM data */
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_2_RAM,FALSE);
  if(BT_2_1_FEATURE_SUPPORTED(param_ptr->bt_2_1_lisbon_disabled))
  {
#if defined (FEATURE_BT_3_0)
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bha_bt_3_0_supported[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_3_0_supported[0],FALSE);
#elif defined (FEATURE_BT_4_0)
    if(param_ptr->bt_qsoc_le_dev_class == BT_QSOC_DEV_CLASS2)
    {
       bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_83_BHA_B0_CLASS2,FALSE);
    }
    else if(param_ptr->bt_qsoc_le_dev_class == BT_QSOC_DEV_CLASS1)
    {
      bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_83_BHA_B0_CLASS1,FALSE);
    }

#ifdef ANDROID
    char le_support_disable[PROPERTY_VALUE_MAX];
    //Remove LE support from BT feature mask if LE is not supported on the target.
    property_get("ro.bluetooth.le.disable", le_support_disable, "false");
    if (!strcmp(le_support_disable, "true")) {
      bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bha_bt_3_0_supported[0],FALSE);
      bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_3_0_supported[0],FALSE);
    }
    else {
      bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bha_bt_4_0_supported[0],FALSE);
    }
#else
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bha_bt_4_0_supported[0],FALSE);
#endif

    //Bluetooth version information ROM defaulted, implying 4.0 spec
#else
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bt_2_1_supported[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_2_1_supported[0],FALSE);
#endif
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bt_2_1_not_supported[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_2_1_not_supported[0],FALSE);
  }

  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_17_BHA_B0,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_27_BHA_B0,FALSE);


  if(param_ptr->clock_sharing == BT_SOC_CLOCK_SHARING_ENABLED)
  {
    if(BT_QSOC_IS_XO_SUPPORT_ENABLED(param_ptr->is_xo_support_enabled))
      {
        bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_CLK_SHARING_BHA_B0,FALSE);
      } else {
        bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_CLK_SHARING_BHA_B0_TCXO,FALSE);
      }
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_NO_CLK_SHARING_BHA_B0,FALSE);
  }

  if(BT_QSOC_SOC_LOGGING_ENABLED(param_ptr->soc_logging))
  {
    bt_qsoc_nvm_add_element((uint8 *)build_srl_tag_71(param_ptr),FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_38_SOC_LOGGING_BHA_B0,FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)build_tag_71(param_ptr),FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_38_SOC_NO_LOGGING_BHA_B0,FALSE);
  }

  //tag 44 needed to override voice settings defaults for interop with MSM
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_44_BHA_B0,FALSE);

  //For BT-WLAN coexistence
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_53_BHA_B0,FALSE);

  //tag 55(GPIO Mapping)
  //Based on WLAN type GPIO Mapping need to be set.

  //Tag 57 AFH Parameters. Need to be tweaked based on WLAN type
  //Currently we are using same values for both WCN and AR Wlan types
  bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_57_bha[0],FALSE);

  //Tag 63 QTA Coex Parameters. Need to be tweaked based on WLAN type
  //Currently we are using same values for both WCN and AR Wlan types
  bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_63_bha[0],FALSE);

  //Tag 74 Coex Burst Timers. Need to be tweaked based on WLAN type
  if(param_ptr->wlan_type == BT_QSOC_WLAN_LIBRA)
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_74_libra[0],FALSE);
  }
  else
  {
    // use tag 74 as ROM default when not attached to Libra
  }

  if(param_ptr->bt_qsoc_bredr_dev_class == BT_QSOC_DEV_CLASS2)
  {
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_36_bha_b0_CLASS2[0],FALSE);
    bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_56_bha_b0_CLASS2[0],FALSE);
    bt_qsoc_nvm_add_element(
                    (uint8 *)&bt_qsoc_cfg_tag_32_19p2MHz_bha_b1_CLASS2[0],FALSE);
  }
  else if(param_ptr->bt_qsoc_bredr_dev_class == BT_QSOC_DEV_CLASS1)
  {
    //Here tag 32 and 36 value from the NVM release string
    //i.e. bt_qsoc_nvm_BAHAMA_B1.c would be sent.
    //And tag 56 value would be defaulted to ROM contents.
  }
} /* init_bha_b1_entries */

/*==============================================================
FUNCTION:  init_riva_entries
==============================================================*/
static void init_riva_entries
(
  /** [in] Parameters that determine the run time configuration */
  bt_qsoc_config_params_struct_type *param_ptr
)
{

   /* Add fixed NVM data */
   bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_2_RAM,FALSE);
   if (BT_2_1_FEATURE_SUPPORTED(param_ptr->bt_2_1_lisbon_disabled)) {
#if defined (FEATURE_BT_3_0)
       bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bha_bt_3_0_supported[0], FALSE);
       bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_3_0_supported[0], FALSE);
#elif defined (FEATURE_BT_4_0)
       bt_qsoc_nvm_add_element((uint8 *)& bt_qsoc_cfg_tag_95_riva[0], FALSE);
       if (param_ptr->bt_qsoc_le_dev_class == BT_QSOC_DEV_CLASS2) {
           bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_83_riva_CLASS2[0], FALSE);
       } else if (param_ptr->bt_qsoc_le_dev_class == BT_QSOC_DEV_CLASS1) {
           bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_83_riva_CLASS1[0], FALSE);
       }
     //Bluetooth version (implying 4.0 spec) & local supported features is ROM defaulted
#endif
   } else {
       bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_6_bt_2_1_not_supported[0], FALSE);
       bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_78_bt_2_1_not_supported[0], FALSE);
   }
   if (param_ptr->bt_qsoc_bredr_dev_class == BT_QSOC_DEV_CLASS2) {
       bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_36_riva_CLASS2[0], FALSE);
   } else if (param_ptr->bt_qsoc_bredr_dev_class == BT_QSOC_DEV_CLASS1) {
       bt_qsoc_nvm_add_element((uint8 *)&bt_qsoc_cfg_tag_36_riva_CLASS1[0], FALSE);
   }
}
/*==============================================================
FUNCTION:  init_unknown_entries
==============================================================*/
/**
  Initialize the fixed and runtime data for an unknown SOC.

  @see bt_qsoc_config_params_struct_type
  
  @sideeffects Undetermined.
*/
static void init_unknown_entries
(
  /** [in] Parameters that determine the run time configuration */
  bt_qsoc_config_params_struct_type *param_ptr
)
{
  /* Add fixed NVM data */
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_2_RAM,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_4_UNKNOWN,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_5_UNKNOWN,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_6_UNKNOWN,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_17_UNKNOWN,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_27_UNKNOWN,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_28_UNKNOWN,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_44_UNKNOWN,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_45_UNKNOWN,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_46_UNKNOWN,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_53_UNKNOWN,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_55_UNKNOWN,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_57_UNKNOWN,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_70_UNKNOWN,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)build_tag_71(param_ptr),FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_76_UNKNOWN,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_78_UNKNOWN,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_95_UNKNOWN,FALSE);
  bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_116_UNKNOWN,FALSE);

  /* Add runtime NVM data */
  if(param_ptr->refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_99_19P2MHZ_UNKNOWN,FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_99_32MHZ_UNKNOWN,FALSE);
  }

  if(param_ptr->clock_sharing == BT_SOC_CLOCK_SHARING_ENABLED)
  {
    if(BT_QSOC_IS_XO_SUPPORT_ENABLED(param_ptr->is_xo_support_enabled))
    {
      bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_CLK_SHARING_UNKNOWN_XO,FALSE);
    }
    else
    {
      bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_CLK_SHARING_UNKNOWN,FALSE);
    }
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_97_CLK_SHARING_UNKNOWN,FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_37_NO_CLK_SHARING_UNKNOWN,FALSE);
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_97_NO_CLK_SHARING_UNKNOWN,FALSE);
  }

  if(BT_QSOC_SOC_LOGGING_ENABLED(param_ptr->soc_logging))
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_38_SOC_LOGGING_UNKNOWN,FALSE);
  }
  else
  {
    bt_qsoc_nvm_add_element((uint8 *)CFG_TAG_38_SOC_NO_LOGGING_UNKNOWN,FALSE);
  }
} /* init_unknown_entries */

/*==============================================================
FUNCTION:  bt_qsoc_nvm_init_fixed_n_runtime_tbl
==============================================================*/
boolean bt_qsoc_nvm_init_fixed_n_runtime_tbl 
(
  /** [in] SOC version */
  bt_qsoc_enum_type bt_qsoc_type, 
  
  /** [in] Parameters that determine the run time configuration */
  bt_qsoc_config_params_struct_type *param_ptr
)
{
  boolean rval = FALSE;

  if( NULL != param_ptr )
  {
    /* Copy the BD Address into the RAM pointed by the table pointer */
    std_memmove( &bt_qsoc_cfg_tag_2_ram[4], &param_ptr->bd_address[0],
      BT_QSOC_MAX_BD_ADDRESS_SIZE );

    switch(bt_qsoc_type)
    {
      case BT_QSOC_R3:
        init_4020_R3_entries(param_ptr);
        rval = TRUE;
        break;

      case BT_QSOC_4020BD_B0:
        init_4020BD_B0_entries(param_ptr);
        rval = TRUE;
        break;

      case BT_QSOC_4020BD_B1:
        init_4020BD_B1_entries(param_ptr);
        rval = TRUE;
        break;

      case BT_QSOC_4021_B1:
         init_4021_B1_entries(param_ptr);
         rval = TRUE;
         break;

      case BT_QSOC_4025_A0:
        init_4025_A0_entries(param_ptr);
        rval = TRUE;
        break;

      case BT_QSOC_4025_B0:
        init_4025_B0_entries(param_ptr);
        rval = TRUE;
        break;

      case BT_QSOC_4025_B1:
        init_4025_B1_entries(param_ptr);
        rval = TRUE;
        break;

      case BT_QSOC_4025_B2:
        init_4025_B2_entries(param_ptr);
        rval = TRUE;
        break;

      case BT_QSOC_4025_B3:
        init_4025_B3_entries(param_ptr);
        rval = TRUE;
        break;

      case BT_QSOC_MBA_A0:
        init_mba_a0_entries(param_ptr);
        rval = TRUE;
        break;

      case BT_QSOC_MBA_B0:
        init_mba_b0_entries(param_ptr);
        rval = TRUE;
        break;

      case BT_QSOC_MBA_B1:
        init_mba_b1_entries(param_ptr);
        rval = TRUE;
        break;

      case BT_QSOC_BHA_A0:
        init_bha_a0_entries(param_ptr);
        rval = TRUE;
        break;

      case BT_QSOC_BHA_B0:
        init_bha_b0_entries(param_ptr);
        rval = TRUE;
        break;
      case BT_QSOC_BHA_B1:
        init_bha_b1_entries(param_ptr);
        rval = TRUE;
        break;
      case BT_QSOC_RIVA:
        init_riva_entries(param_ptr);
        rval = TRUE;
        break;
      case BT_QSOC_R3BD:    
      case BT_QSOC_R4:  
      case BT_QSOC_R2B:
      case BT_QSOC_R2C:
        /* Unsupported BTS versions */
        rval = FALSE;
        break;

      default:
        /* Unknown BTS versions */
        init_unknown_entries(param_ptr);
        rval = TRUE;
        break;
    }
  }

  return rval;
} /* bt_qsoc_nvm_init_fixed_n_runtime_tbl */
