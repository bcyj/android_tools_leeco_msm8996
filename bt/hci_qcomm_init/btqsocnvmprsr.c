/**
  @file btqsocnvmprsr.c

  This module implements routines needed to parse the nvm string into 
  individual tags. Some of the parsing routines below are shared by:

  - Automatic mode where the nvm tags are read from a string 
  - File system mode where the nvm tags are read from a file on the phone's
    file system
*/

/*--------------------------------------------------------------
  Copyright (c) 2008-2012 Qualcomm Technologies, Inc.
  All rights reserved.
  Qualcomm Technologies Confidential and Proprietary
--------------------------------------------------------------*/

/*=============================================================================

EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order. Please
use ISO format for dates.

$Header: //source/qcom/qct/wconnect/bthost/soccfg/main/latest/src/btqsocnvmprsr.c#15 $
$DateTime: 2011/07/26 15:05:55 $
$Author: kgeib $

when        who  what, where, why
----------  ---  -----------------------------------------------------------
2012-10-01   DV  Added support for wcn2243 2.1 SOC.
2012-08-28   rr  Fix memory leak when duplicate NVM tags exist.
2011-07-18   bn  Added support for 8960 for sending NVM tags to RIVA.
2011-03-01   rr  Deprecating unused formal paramters to parser routines.
2010-10-25   tw Added support for 4025 B3
2010-07-23   rr  Added support for Bahama A0 (Manihiki)
2010-03-26   rr  Runtime NVM mode selection for initialising NVM configuration
2010-02-19   rr  Added support for Marimba B1
2009-11-30  dgh  Fixed bug in auto-mode string selection for 4025 B2.
2009-10-07   sp  Added support for Marimba B0
2009-10-05  dgh  Fixed compiler warning.
2009-09-16  dgh  Fixed bug in bt_qsoc_nvm_init_auto_mode.
2009-09-14  dgh  Made bt_qsoc_nvm_init_hybrid_mode dependent on the EFS mode 
                 feature variable (FEATURE_BT_QSOC_NVM_EFS_MODE).
2009-07-25  dgh  Initial version of restructured file.  Previous history below.

=============================================================================*/

/*******************************************************************************
 * Previous btqsocnvm.c revision history
 *
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

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "AEEstd.h"
#include "btqsocnvmplatform.h"
#include "btqsocnvmprivate.h"

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
#define BT_QSOC_NVM_TAG_WRITE       1    /**< Indicates the tag is to be written */
#define BT_QSOC_NVM_MAX_NUM_TAGS    256  /**< Indicated total num of possible NVM 
                                            tags */
#define BT_QSOC_NVM_MAX_TAG_LEN     261  /**< Max tag length + 4 bytes for header
                                            1 byte for end of string */

/** Number of bitmap words.  Add one extra element just in case the max 
    number of tags is not a multiple of 32. */
#define TAG_BITMAP_ELEMENTS (BT_QSOC_NVM_MAX_NUM_TAGS/32 + 1)

/* These macros are used internally by parser to parse the tag information  
 * It parses the string to get "Tag Number =", "Tag Length=", "Tag Value=" */

#define BT_QSOC_NVM_TAG_NUM_STR_LEN    9  
#define BT_QSOC_NVM_TAG_LEN_STR_LEN   12   
#define BT_QSOC_NVM_TAG_VAL_STR_LEN   11                                       
#define BT_QSOC_NVM_NUMBER_LEN         5

#define BT_QSOC_NVM_PARSE_RES_BUF_LEN      16   /**< length of temp buffer */

#define  BT_QSOC_NVM_PARSER_MAGIC 0x51434F4D  /**< magic number to initialization detection */

#define TAG_OFFSET 2  /**< offset into a NVM string where the tag number is stored */

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/

/** simple linked list to hold NVM data */
typedef struct _bt_qsoc_nvm_element_type
{
  /** pointer to the next NVM element */
  struct _bt_qsoc_nvm_element_type *next_element;

  /** pointer to the NVM data */
  uint8 *nvm_data;

  /** True if nvm_data was dynamically allocated */
  boolean is_on_heap;
} bt_qsoc_nvm_element_type;

/** Structure to hold parser data */
typedef struct
{
  uint32 parser_magic;                /**< "magic number" to detect an 
                                           initialized structure. */
  bt_qsoc_nvm_element_type *head_tag; /**< pointer to the start of a linked 
                                           list of NVM data elements */
  bt_qsoc_nvm_element_type *next_tag; /**< pointer to the next unsent tag */
} bt_qsoc_nvm_parser_type;

/*----------------------------------------------------------------------------
 * Global Data Definitions
 * -------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Static Variable Definitions
 * -------------------------------------------------------------------------*/
/** Pointer to the global parser information structure */
static bt_qsoc_nvm_parser_type *bt_qsoc_parser_ptr = NULL;

/** bitmap used to track tags as they are sent to the SOC */
static uint32 sent_tag_bitmap[TAG_BITMAP_ELEMENTS];

/*----------------------------------------------------------------------------
 * Static Function Declarations and Definitions
 * -------------------------------------------------------------------------*/

/*==============================================================
FUNCTION:  get_auto_mode_nvm_string
==============================================================*/
/**
  Returns an auto-mode NVM string (hard-coded NVM file data) for
  a given QSOC type and clock speed if one is available.

  @return  Auto-mode NVM string or NULL.
*/
static const char *get_auto_mode_nvm_string 
(
  /** Type of QSOC to query */
  bt_qsoc_enum_type qsoc_type,

  /** Reference clock of QSOC */
  bt_soc_refclock_type bt_refclock_type
)
{
  const char *r_val = NULL;

  switch(qsoc_type)
  {
    case BT_QSOC_R3:
      if(bt_refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
      {
        r_val = bt_qsoc_nvm_BTS4020_R3_19P2Mhz; 
      }
      else
      {
        r_val = bt_qsoc_nvm_BTS4020_R3_32Mhz;
      }       
      break;

    case BT_QSOC_4020BD_B0:
      if(bt_refclock_type == BT_SOC_REFCLOCK_19P2MHZ) 
      {  
        r_val = bt_qsoc_nvm_BTS4020_BDB0_19P2Mhz;
      }
      else
      {
        r_val = bt_qsoc_nvm_BTS4020_BDB0_32Mhz; 
      }
      break;

    case BT_QSOC_4020BD_B1:
      if(bt_refclock_type == BT_SOC_REFCLOCK_19P2MHZ) 
      {  
        r_val = bt_qsoc_nvm_BTS4020_BDB1_19P2Mhz;
      }
      else
      {
        r_val = bt_qsoc_nvm_BTS4020_BDB1_32Mhz;
      }
      break;

    case BT_QSOC_4021_B1:
      if(bt_refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
      {
        r_val = bt_qsoc_nvm_BTS4021_B1_19P2Mhz;
      }
      else
      {
        r_val = bt_qsoc_nvm_BTS4021_B1_32Mhz;
      }
      break;

    case BT_QSOC_4025_B0:
      if(bt_refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
      {
        r_val = bt_qsoc_nvm_BTS4025_B0_19P2Mhz;
      }
      else
      {
        r_val = bt_qsoc_nvm_BTS4025_B0_32Mhz;
      }
      break;

    case BT_QSOC_4025_B1:
      if(bt_refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
      {
        r_val = bt_qsoc_nvm_BTS4025_B1_19P2Mhz;
      }
      else
      {
        r_val = bt_qsoc_nvm_BTS4025_B1_32Mhz;
      }
      break;

    case BT_QSOC_4025_B2:
      if(bt_refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
      {
        r_val = bt_qsoc_nvm_BTS4025_B2_19P2Mhz;
      }
      else
      {
        r_val = bt_qsoc_nvm_BTS4025_B2_32Mhz;
      }
      break;

    case BT_QSOC_4025_B3:
      if(bt_refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
      {
        r_val = bt_qsoc_nvm_BTS4025_B3_19P2Mhz;
      }
      else
      {
        r_val = bt_qsoc_nvm_BTS4025_B3_32Mhz;
      }
      break;

    case BT_QSOC_MBA_A0:
      if(bt_refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
      {
        r_val = bt_qsoc_nvm_MARIMBA_A0;
      }
      break;

    case BT_QSOC_MBA_B0:
      if(bt_refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
      {
        r_val = bt_qsoc_nvm_MARIMBA_B0;
      }
      break;

    case BT_QSOC_MBA_B1:
      if(bt_refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
      {
        r_val = bt_qsoc_nvm_MARIMBA_B1;
      }
      break;

    case BT_QSOC_BHA_A0:
      if(bt_refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
      {
        r_val = bt_qsoc_nvm_BAHAMA_A0;
      }
      break;

    case BT_QSOC_BHA_B0:
      if(bt_refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
      {
        r_val = bt_qsoc_nvm_BAHAMA_B0;
      }
      break;
    case BT_QSOC_BHA_B1:
      if(bt_refclock_type == BT_SOC_REFCLOCK_19P2MHZ)
      {
        r_val = bt_qsoc_nvm_BAHAMA_B1;
      }

    default:
      break;
  }

  return r_val;
}

/*==============================================================
FUNCTION:  init_tag_list
==============================================================*/
/**
  Initialize the tag number bitmap.

  @see bt_qsoc_nvm_set_tag_sent
  @see was_tag_sent_to_soc
  
  @return  none
*/
static void init_tag_list()
{
  std_memset(&sent_tag_bitmap[0],0,sizeof(sent_tag_bitmap));
} /* init_tag_list */

/*==============================================================
FUNCTION:  was_tag_sent_to_soc
==============================================================*/
/**
  Checks to see if a tag number was already sent to the SOC.

  @see bt_qsoc_nvm_set_tag_sent
  @see init_tag_list
  
  @return  boolean: True: If tag was already sent to SOC.
                    False: otherwise.
*/
static boolean was_tag_sent_to_soc
(
  uint8 tag_num_to_find /**< [in] tag number to check */
)
{
  int tag_word, tag_bit;

  /* determine the offset into the sent_tag_bitmap array */
  tag_word = (tag_num_to_find & 0x00FF) >> 5;
  
  /* determine the offset into the selected bitmap word */
  tag_bit  = tag_num_to_find & 0x001F;

  return (sent_tag_bitmap[tag_word] & (1 << tag_bit)) > 0;
} /* was_tag_sent_to_soc */

/*==============================================================
FUNCTION:  free_nvm_list
==============================================================*/
/**
  This function deallocates memory allocated to the NVM list.

  @return none.
*/
static void free_nvm_list()
{
  bt_qsoc_nvm_element_type *elem_ptr, *temp_ptr;

  /* make sure this is allocated */
  if( NULL != bt_qsoc_parser_ptr )
  {
    if( bt_qsoc_parser_ptr->parser_magic == BT_QSOC_NVM_PARSER_MAGIC )
    {
      elem_ptr = bt_qsoc_parser_ptr->head_tag;
      while( NULL != elem_ptr )
      {
        if( elem_ptr->is_on_heap )
        {
          btqsocnvmplatform_free(elem_ptr->nvm_data);
        }
        temp_ptr = elem_ptr;
        elem_ptr = elem_ptr->next_element;
        btqsocnvmplatform_free(temp_ptr);
      }
    }
  }
} /* free_nvm_list */

/*----------------------------------------------------------------------------
 * Externalized Function Definitions
 * -------------------------------------------------------------------------*/

/*==============================================================
FUNCTION:  bt_qsoc_nvm_init_auto_mode
==============================================================*/
boolean bt_qsoc_nvm_init_auto_mode 
(
  /** [in] SOC version */
  bt_qsoc_enum_type bt_qsoc_type, 
  
  /** [in] Parameters that determine the run time configuration */
  bt_qsoc_config_params_struct_type *param_ptr
)
{
  boolean r_val = FALSE;
  const char *nvm_string_ptr = NULL;

  if( NULL != param_ptr ) 
  {
    /* initialize the parser */   
    r_val = bt_qsoc_nvm_init_parser();
    init_tag_list();  /* initialize tag bitmap */

    if( FALSE != r_val )
    {
      /* initialize the fixed (hard-coded) and runtime (tweaked) NVM tables. */
      r_val = bt_qsoc_nvm_init_fixed_n_runtime_tbl(bt_qsoc_type,
                                               param_ptr);
    }

    if(BT_QSOC_RIVA != bt_qsoc_type)
    {
      if( FALSE != r_val )
      {
         /* Get a pointer to the NVM file data (if any) */
         nvm_string_ptr = get_auto_mode_nvm_string(bt_qsoc_type,
         param_ptr->refclock_type);
      }

      if( NULL != nvm_string_ptr && FALSE != r_val )
      {  /* If there is NVM string data, initialize and parse it */
         r_val = bt_qsoc_nvm_parse_nvm_file(nvm_string_ptr);
      }
    }
  }
  return r_val;
} /* bt_qsoc_nvm_init_auto_mode */

/*==============================================================
FUNCTION:  bt_qsoc_nvm_init_hybrid_mode
==============================================================*/
boolean bt_qsoc_nvm_init_hybrid_mode 
(
  /** [in] SOC version */
  bt_qsoc_enum_type bt_qsoc_type, 
  
  /** [in] Parameters that determine the run time configuration */
  bt_qsoc_config_params_struct_type *param_ptr
)
{
  boolean r_val = FALSE;
  const char *nvm_string_ptr = NULL;

  if( NULL != param_ptr ) 
  {
    /* initialize the parser */   
    r_val = bt_qsoc_nvm_init_parser();
    init_tag_list();  /* initialize tag bitmap */

    if( FALSE != r_val )
    {
      /* initialize the fixed (hard-coded) and runtime (tweaked) NVM tables. */
      r_val = bt_qsoc_nvm_init_fixed_n_runtime_tbl(bt_qsoc_type,
                                               param_ptr);
    }

    if(BT_QSOC_RIVA != bt_qsoc_type)
    {
      if( FALSE != r_val )
      {
        /* parse the EFS-mode data */
        r_val = bt_qsoc_nvm_init_fs_mode();
      }

      if( FALSE != r_val )
      {
        /* Get a pointer to the NVM file data (if any) */
        nvm_string_ptr = get_auto_mode_nvm_string(bt_qsoc_type,
        param_ptr->refclock_type);
      }

      if( NULL != nvm_string_ptr && FALSE != r_val )
      { /* If there is NVM string data, parse it */
        r_val = bt_qsoc_nvm_parse_nvm_file(nvm_string_ptr);
      }
    }
  }

  return r_val;
} /* bt_qsoc_nvm_init_hybrid_mode */

/*==============================================================
FUNCTION:  bt_qsoc_nvm_close_nvm
==============================================================*/
boolean bt_qsoc_nvm_close_nvm 
(
  /** [in] BT QSOC NVM Mode */
  bt_qsoc_enum_nvm_mode mode
)
{
  /* Close file handle, deallocate any dynamic memory */
  if( NULL != bt_qsoc_parser_ptr )
  {
    if( bt_qsoc_parser_ptr->parser_magic == BT_QSOC_NVM_PARSER_MAGIC )
    {
      free_nvm_list();
    }
    btqsocnvmplatform_free(bt_qsoc_parser_ptr);
    bt_qsoc_parser_ptr = NULL;
  }

  /* Added featurization to avoid ARM9 BT FTM compilation error 
   * (no EFS support) */
  if( mode == NVM_EFS_MODE || mode == NVM_HYBRID_MODE )
  {
    btqsocnvmplatform_close_file();   
  }
  return TRUE;
} /* bt_qsoc_nvm_close_nvm */

/*==============================================================
FUNCTION:  bt_qsoc_nvm_set_tag_sent
==============================================================*/
void bt_qsoc_nvm_set_tag_sent
(
  /** [in] tag number to mark */
  uint8 tag_no
)
{
  int tag_word, tag_bit;

  /* determine the offset into the sent_tag_bitmap array */
  tag_word = (tag_no & 0x00FF) >> 5;

  /* determine the offset into the selected bitmap word */
  tag_bit  = tag_no & 0x001F;

  sent_tag_bitmap[tag_word] |= (1 << tag_bit);
} /* bt_qsoc_nvm_set_tag_sent */

/*==============================================================
FUNCTION:  bt_qsoc_nvm_get_next_nvm_tag
==============================================================*/
boolean bt_qsoc_nvm_get_next_nvm_tag 
(
  /** [out] pointer to storage for the returned tag */
  uint8** soc_nvm_ptr
)
{
  boolean r_val = FALSE;
  bt_qsoc_nvm_element_type *elem_ptr;

  *soc_nvm_ptr  = NULL;

  /* make sure list is allocated */
  if( NULL != bt_qsoc_parser_ptr )
  {
    if( bt_qsoc_parser_ptr->parser_magic == BT_QSOC_NVM_PARSER_MAGIC )
    {
      while( NULL != bt_qsoc_parser_ptr->next_tag )
      {
        elem_ptr = bt_qsoc_parser_ptr->next_tag;
        bt_qsoc_parser_ptr->next_tag = elem_ptr->next_element;

        if( was_tag_sent_to_soc(elem_ptr->nvm_data[TAG_OFFSET]) == FALSE )
        {
          *soc_nvm_ptr = elem_ptr->nvm_data;
          r_val = TRUE;
          break;
        }
      }
    }
  }

  return r_val;
} /* bt_qsoc_nvm_get_next_nvm_tag */


/*==============================================================
FUNCTION:  bt_qsoc_nvm_init_parser
==============================================================*/
boolean bt_qsoc_nvm_init_parser ()
{
  boolean r_val = FALSE;

  if( NULL != bt_qsoc_parser_ptr )
  {
    if( bt_qsoc_parser_ptr->parser_magic != BT_QSOC_NVM_PARSER_MAGIC )
    {
      btqsocnvmplatform_free(bt_qsoc_parser_ptr);
      bt_qsoc_parser_ptr = NULL;
    }
  }

  if( NULL == bt_qsoc_parser_ptr )
  {
    bt_qsoc_parser_ptr = btqsocnvmplatform_malloc(sizeof(*bt_qsoc_parser_ptr));
    if( NULL != bt_qsoc_parser_ptr )
    {
      std_memset(bt_qsoc_parser_ptr,0,sizeof(*bt_qsoc_parser_ptr));
    }
  }

  if( NULL != bt_qsoc_parser_ptr )
  {
    if( bt_qsoc_parser_ptr->parser_magic != BT_QSOC_NVM_PARSER_MAGIC )
    {
      if( NULL != bt_qsoc_parser_ptr->head_tag )
      {
        free_nvm_list();
        bt_qsoc_parser_ptr->head_tag = NULL;
      }

      bt_qsoc_parser_ptr->head_tag = 
        btqsocnvmplatform_malloc(sizeof(*(bt_qsoc_parser_ptr->head_tag)));
      if( NULL != bt_qsoc_parser_ptr->head_tag )
      {
        std_memset(bt_qsoc_parser_ptr->head_tag,0,
          sizeof(*(bt_qsoc_parser_ptr->head_tag)));
        bt_qsoc_parser_ptr->next_tag = bt_qsoc_parser_ptr->head_tag;
        bt_qsoc_parser_ptr->parser_magic = BT_QSOC_NVM_PARSER_MAGIC;
        r_val = TRUE;
      }
    }
    else
    { /* parser already initialized (in hybrid mode?) */
      r_val = TRUE;
    }
  }

  return r_val;
} /* bt_qsoc_nvm_init_parser */

/*==============================================================
FUNCTION:  bt_qsoc_nvm_parse_nvm_file
==============================================================*/
boolean bt_qsoc_nvm_parse_nvm_file 
(
  /** [in] pointer to NVM file data */
  const char* nvm_data_ptr
) 
{
  boolean r_val = FALSE;
  const char *tagnum_label_ptr;
  const char *taglen_label_ptr;
  const char *tagval_label_ptr;
  const char *temp_ptr;
  const char *end_ptr;
  const char *end_of_data_ptr;
  const char *next_tag_ptr;
  int error;
  int idx;
  int tagnum;
  int taglen;
  uint8 *nvm_string = NULL;

  if( NULL != nvm_data_ptr )
  {
    end_of_data_ptr = nvm_data_ptr + std_strlen(nvm_data_ptr);

    temp_ptr = std_strchr(nvm_data_ptr,'[');  /* seek a configuration block */

    while( NULL != temp_ptr )
    {
      /* see if it is a "[Tag..." block */
      temp_ptr = std_strstr(temp_ptr+1,"Tag");  
      if( NULL != temp_ptr )
      {
        /* see if it is a "[TagX]" block */
        std_scanul(temp_ptr+3, 10, &end_ptr, &error);
        if( 0 == error )
        {
          next_tag_ptr  = std_strchr(end_ptr+1,'[');
          tagnum_label_ptr = std_strstr(end_ptr+1,"TagNum"); 
          taglen_label_ptr = std_strstr(end_ptr+1,"TagLength"); 
          tagval_label_ptr = std_strstr(end_ptr+1,"TagValue");

          /* Sanity check the label pointers. If next_tag_ptr == NULL, */
          /* we are processing the last block so skip this check.      */
          if( NULL != next_tag_ptr && tagnum_label_ptr > next_tag_ptr &&
            taglen_label_ptr > next_tag_ptr && tagval_label_ptr > next_tag_ptr )
          {
            /* this was a malformed tag entry.  skip to next. */
            temp_ptr = next_tag_ptr;
            continue;
          }

          tagnum = -1;
          if( NULL != tagnum_label_ptr )
          {
            end_ptr = tagnum_label_ptr+6;
            while( end_ptr < end_of_data_ptr )
            {
              tagnum = std_scanul(end_ptr, 10, &end_ptr, &error);
              if( 0 == error )
              {
                break;
              }

              end_ptr++;
            }
          }

          if( tagnum <= 0 || tagnum > 255 )
          {
            /* The tag number is invalid.  skip to next. */
            temp_ptr = next_tag_ptr;
            continue;
          }

          taglen = 0;
          if( NULL != taglen_label_ptr )
          {
            end_ptr = taglen_label_ptr+9;
            while( end_ptr < end_of_data_ptr )
            {
              taglen = std_scanul(end_ptr, 10, &end_ptr, &error);
              if( 0 == error )
              {
                break;
              }

              end_ptr++;
            }
          }

          if( taglen <= 0 || taglen > 255 )
          {
            /* The tag length is invalid.  skip to next. */
            temp_ptr = next_tag_ptr;
            continue;
          }

          /* allocate memory for NVM string */
          nvm_string = btqsocnvmplatform_malloc(taglen+4);

          if( NULL != nvm_string )
          {
            /* Build the tag */
            nvm_string[3] = taglen;
            nvm_string[2] = tagnum;
            nvm_string[1] = BT_QSOC_NVM_TAG_WRITE;   
            nvm_string[0] = taglen + 3;

            if( NULL != tagval_label_ptr )
            {
              end_ptr = tagval_label_ptr+8;
              for( idx = 0; idx < taglen; idx++ )
              {
                while( end_ptr < end_of_data_ptr )
                {
                  nvm_string[idx+4] = (uint8) std_scanul(end_ptr, 16, &end_ptr, &error);
                  if( 0 == error )
                  {
                    break;
                  }

                  end_ptr++;
                }
              }

              bt_qsoc_nvm_add_element(nvm_string,TRUE);
              r_val = TRUE;   /* added at least one element so return TRUE */
            }
            else
            {
              btqsocnvmplatform_free(nvm_string);
              nvm_string = NULL;
            }
          }

          temp_ptr = next_tag_ptr;
        }
      }
    }
  }

  return r_val;
} /* bt_qsoc_nvm_parse_nvm_file */

/*==============================================================
FUNCTION:  bt_qsoc_nvm_add_element
==============================================================*/
void bt_qsoc_nvm_add_element
(
  /** NVM data to add to the list */
  uint8 *nvm_string,

  /** True if string was dynamically allocated */
  boolean is_on_heap
)
{
  bt_qsoc_nvm_element_type *elem_ptr, *prev_ptr;

  /* make sure this is allocated */
  if( NULL != bt_qsoc_parser_ptr )
  {
    if( bt_qsoc_parser_ptr->parser_magic == BT_QSOC_NVM_PARSER_MAGIC )
    {
      elem_ptr = prev_ptr = bt_qsoc_parser_ptr->head_tag;
      while( NULL != elem_ptr && NULL != elem_ptr->nvm_data )
      {
        if( elem_ptr->nvm_data[TAG_OFFSET] == nvm_string[TAG_OFFSET] )
        { /* Duplicate found!  Do not overwrite.
           * Release memory held on heap for the duplicate tag
           */
          if(is_on_heap)
          {
            btqsocnvmplatform_free(nvm_string);
            nvm_string = NULL;
          }
          return;
        }
        prev_ptr = elem_ptr;  /* save the previous element pointer */
        elem_ptr = elem_ptr->next_element;
      }

      if( NULL != elem_ptr && NULL == elem_ptr->nvm_data )
      { /* This element had no data so use it instead of allocating a new one. */
        elem_ptr->nvm_data = nvm_string;
        elem_ptr->is_on_heap = is_on_heap;
      }
      else if( NULL == elem_ptr && prev_ptr != NULL )
      { /* Tag wasn't found in list.  Allocate a new element. */
        prev_ptr->next_element = 
          btqsocnvmplatform_malloc(sizeof(*prev_ptr->next_element));

        elem_ptr = prev_ptr->next_element;

        if( elem_ptr != NULL )
        {
          std_memset(elem_ptr,0,sizeof(*elem_ptr));
          elem_ptr->nvm_data = nvm_string;
          elem_ptr->is_on_heap = is_on_heap;
        }
      }
    }
  }
} /* bt_qsoc_nvm_add_element */
