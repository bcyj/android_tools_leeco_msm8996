/*!
  @file
  configdb_util.h

  @brief
  This file declares utility functions used by the configuration parser.

*/
/*===========================================================================

  Copyright (c) 2011-2013 Qualcomm Technologies, Inc. All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential
===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header:  $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
01/07/11   sg      initial version

===========================================================================*/

#ifndef _CONFIGDB_UTIL_H
#define _CONFIGDB_UTIL_H

#include "comdef.h"
#include "configdb.h"
#include "configdbi.h"
#include "xmllib_common.h"
#include "ds_util.h"
#include <string.h>
#include "ds_string.h"

/* If any of the builds have an older version of ds_string.h that doesn't
   have this function defined */
#ifndef std_strnicmp
  #define std_strnicmp strncasecmp
#endif

#ifdef FEATURE_QMI_TEST

#include "tf_log.h"

#define  CFGDB_LOG              TF_MSG
#define  CFGDB_LOG_ERR          TF_MSG_ERROR
#define  CFGDB_LOG_HIGH         TF_MSG_HIGH
#define  CFGDB_LOG_MED          TF_MSG_MED
#define  CFGDB_LOG_LOW          TF_MSG_INFO
#define  CFGDB_LOG_DFLT         TF_MSG
#define  CFGDB_LOG_SYS_ERR      TF_MSG
#define  CFGDB_LOG_INIT         TF_MSG_INIT

#else

/* Logging macros */
#undef  DS_LOG_TAG
#define DS_LOG_TAG "QC-CONFIGDB-LIB"

#define  CFGDB_LOG               ds_log
#define  CFGDB_LOG_ERR           ds_log_err
#define  CFGDB_LOG_HIGH          ds_log_high
#define  CFGDB_LOG_MED           ds_log_med
#define  CFGDB_LOG_LOW           ds_log_low
#define  CFGDB_LOG_DFLT          ds_log_dflt
#define  CFGDB_LOG_SYS_ERR       ds_log_sys_err

#define  CFGDB_LOG_INIT          ds_log_init2
#endif

/* This function declaration not present in the header file.
   So, using forward declaration to avoid compilation warning */
extern void ds_log_init2(int threshold, int mode);

/*===========================================================================
  FUNCTION:  configdb_util_dup_string
  ===========================================================================*/
  /*!
      @brief
      This function extracts and duplicates the src string (xmllib_string_s_type)
      and returns a NULL terminated copy via the "copy" parameter.

      Note that memory allocated by this function for the "copy" must be
      released by the client of this function and also the input string is
      clipped at CFGDB_MAX_STRING_SIZE

      @params
      copy [out] - NULL terminated copy of the source string
      src  [in]  - Source xmllib string

      @return
      CFGDB_EBADPARAM
      CFGDB_EMEMORY
      CFGDB_SUCCESS
  */
/*=========================================================================*/
int32 configdb_util_dup_string(
   char **copy,
   const xmllib_string_s_type *src
);


/*===========================================================================
  FUNCTION:  configdb_util_icmp_string
  ===========================================================================*/
  /*!
      @brief
      This function returns the result of case insensitive comparison of a
      xmllib's string (xml_str) and a regular string (str)

      @params
      xml_str [in] - xmllib string
      str     [in] - regular string

      @return
      0 - if both strings are equal
      1 - First string has greater value
     -1 - Second string has greater value
  */
/*=========================================================================*/
int32 configdb_util_icmp_string(
   const xmllib_string_s_type *xml_str,
   const char *str
);


/*===========================================================================
  FUNCTION:  configdb_util_clip_chars
  ===========================================================================*/
  /*!
      @brief
      This function gets rid of the specified characters at either end
      of the input string

      @params
      clip_str [in] - characters to clip from input string
      str      [in] - input string with the characters clipped

      @return
      CFGDB_EBADPARAM
      CFGDB_SUCCESS
  */
/*=========================================================================*/
int32 configdb_util_clip_chars(
   const char *clip_str,
   char *str
);


/*===========================================================================
  FUNCTION:  configdb_util_is_numeric
  ===========================================================================*/
  /*!
      @brief
      This function verifies whether the input string contains all numeric
      characters

      @params
      str [in] - input string

      @return
      TRUE
      FALSE
  */
/*=========================================================================*/
int32 configdb_util_isnumeric(
   const char *str
);


/*===========================================================================
  FUNCTION:  configdb_util_default_string_converter
  ===========================================================================*/
  /*!
      @brief
      This is the converter function for the string type inherently supported by
      the parser

      @params
      src_data  [in]  - input string data
      conv_data [out] - storage for converted string value
      conv_size [in]  - memory allocated for conv_data

      @return
      CFGDB_ENOCONVERT
      CFGDB_EBADPARAM
      CFGDB_EMEMORY
      CFGDB_SUCCESS
  */
/*=========================================================================*/
int32 configdb_util_default_string_converter(
   const char *src_data,
   void       *conv_data,
   uint32      conv_size
);


/*===========================================================================
  FUNCTION:  configdb_util_default_int_converter
  ===========================================================================*/
  /*!
      @brief
      This is the converter function for the int type inherently supported by
      the parser

      str_data  [in]  - input string data
      conv_data [out] - storage for converted numeric value
      conv_size [in]  - memory allocated for conv_data

      @return
      CFGDB_ENOCONVERT
      CFGDB_EBADPARAM
      CFGDB_EMEMORY
      CFGDB_SUCCESS
  */
/*=========================================================================*/
int32 configdb_util_default_int_converter(
   const char *src_data,
   void       *conv_data,
   uint32      conv_size
);


/*===========================================================================
  FUNCTION:  configdb_util_free_tree
  ===========================================================================*/
  /*!
      @brief
      This function traverses the configdb tree and frees all the nodes

      Note that this is a recursive function

      @params
      root [in] - Root of the configdb tree to be freed

      @return
      None
  */
/*=========================================================================*/
void configdb_util_free_tree(
   configdb_node_t *root
);


/*===========================================================================
  FUNCTION:  configdb_util_append_child
  ===========================================================================*/
  /*!
      @brief
      This function appends a child node to the parent's list of children

      @params
      parent [in] - Parent node
      child  [in] - Child node to append

      @return
      CFGDB_EBADPARAM
      CFGDB_SUCCESS
  */
/*=========================================================================*/
int32 configdb_util_append_child(
   configdb_node_t *parent,
   configdb_node_t *child
);


/*===========================================================================
  FUNCTION:  configdb_util_print_tree
  ===========================================================================*/
  /*!
      @brief
      This function traverses the configdb tree and prints each node

      @params
      root [in] - Root of the configdb tree

      @return
      None
  */
/*=========================================================================*/
void configdb_util_print_tree(
   const configdb_node_t *root
);

#endif /* _CONFIGDB_UTIL_H */

