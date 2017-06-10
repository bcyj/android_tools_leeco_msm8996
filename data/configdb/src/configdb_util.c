/*!
  @file
  configdb_util.c

  @brief
  This file implements utility functions used by the configuration parser.

*/

/*===========================================================================

  Copyright (c) 2011,2013-2014 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
01/07/11   sg      initial revision

===========================================================================*/

/*===========================================================================

                      INCLUDE FILES FOR MODULE

===========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include "configdb.h"
#include "configdb_util.h"

/* External Function Definitions */

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

int32 configdb_util_dup_string
(
   char **copy,
   const xmllib_string_s_type *src
)
{
   int32 ret = CFGDB_EFAIL;

   if (NULL == src || NULL == copy)
   {
      CFGDB_LOG_ERR("configdb_util_dup_string: bad parameter(s)\n");
      return CFGDB_EBADPARAM;
   }

   if (0 == src->len)
   {
      *copy = NULL;
   }
   else
   {
      uint32 len = (src->len > CFGDB_MAX_STRING_SIZE) ? CFGDB_MAX_STRING_SIZE
                                                      : src->len;
      if (NULL == (*copy = malloc((size_t)(len+1))))
      {
         CFGDB_LOG_ERR("configdb_util_dup_string: memory allocation failed\n");
         ret = CFGDB_EMEMORY;
      }
      else
      {
         strlcpy(*copy, src->string, (size_t)(len+1));
         ret = CFGDB_SUCCESS;
      }
   }

   return ret;
}


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

int32 configdb_util_icmp_string
(
   const xmllib_string_s_type *xml_str,
   const char *str
)
{
   int32 ret = -1;

   if (NULL != xml_str && NULL != str)
   {
      size_t len = strlen(str);

      /* If the lengths match, do the string comparison */
      if (xml_str->len == len)
      {
         ret = std_strnicmp(xml_str->string, str, len);
      }
   }

   return ret;
}


/*===========================================================================
  FUNCTION:  configdb_util_clip_chars
  ===========================================================================*/
  /*!
      @brief
      This function gets rid of the specified characters at either end
      of the input string

      @params
      clip_str [in]    - characters to clip from input string
      str      [inout] - input string with the characters clipped

      @return
      CFGDB_EBADPARAM
      CFGDB_SUCCESS
  */
/*=========================================================================*/

int32 configdb_util_clip_chars
(
   const char *clip_str,
   char *str
)
{
   char *begin = NULL, *end = NULL, *nbegin = NULL;

   if (NULL == clip_str || NULL == str)
   {
      CFGDB_LOG_ERR("configdb_util_clip_chars: bad parameter(s)\n");
      return CFGDB_EBADPARAM;
   }

   begin = str;
   end   = str + strlen(str);

   /* Find the new beginning by discarding the clip characters at the start */
   for (nbegin = begin; nbegin < end && NULL != strchr(clip_str, *nbegin); nbegin++)
      ;

   if (end > nbegin)
   {
      char *nend;

      /* Find the new end by discarding the clip characters at the end */
      for (nend = end-1; nend > nbegin && NULL != strchr(clip_str, *nend); nend--)
         ;

      /* Update the end */
      *(nend + 1) = '\0';
   }

   /* Move the new string to the beginning of the array */
   while ('\0' != (*begin++ = *nbegin++))
      ;

   return CFGDB_SUCCESS;
}


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

int32 configdb_util_isnumeric
(
   const char *str
)
{
   int32 ret_val = TRUE;

   if (NULL == str || '\0' == *str)
   {
      return FALSE;
   }

   for (; *str != '\0'; ++str)
   {
      if (!isdigit(*str))
      {
         ret_val = FALSE;
         break;
      }
   }

   return ret_val;
}


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
      conv_size [in]  - size of conv_data

      @return
      CFGDB_EBADPARAM
      CFGDB_EMEMORY
      CFGDB_SUCCESS
  */
/*=========================================================================*/

int32 configdb_util_default_string_converter
(
   const char *src_data,
   void       *conv_data,
   uint32      conv_size
)
{
   int32 ret_val = CFGDB_EFAIL;

   if (NULL == src_data ||
       NULL == conv_data)
   {
      CFGDB_LOG_ERR("configdb_util_default_string_converter: "
                    "bad parameter(s)\n");
      ret_val = CFGDB_EBADPARAM;
   }
   else if (conv_size <= strlen(src_data))
   {
      CFGDB_LOG_ERR("configdb_util_default_string_converter: "
                    "not enough memory allocated to store converted data\n");
      ret_val = CFGDB_EMEMORY;
   }
   else
   {
      strlcpy((char *)conv_data, src_data, conv_size);
      ret_val = CFGDB_SUCCESS;
   }

   return ret_val;
}


/*===========================================================================
  FUNCTION:  configdb_util_default_int_converter
  ===========================================================================*/
  /*!
      @brief
      This is the converter function for the int type inherently supported by
      the parser

      src_data  [in]  - input string data
      conv_data [out] - storage for converted numeric value
      conv_size [in]  - size of conv_data

      @return
      CFGDB_EBADPARAM
      CFGDB_EMEMORY
      CFGDB_SUCCESS
  */
/*=========================================================================*/

int32 configdb_util_default_int_converter
(
   const char *src_data,
   void       *conv_data,
   uint32      conv_size
)
{
   int32 ret_val = CFGDB_EFAIL;

   if (NULL == src_data ||
       NULL == conv_data)
   {
      CFGDB_LOG_ERR("configdb_util_default_int_converter: bad parameter(s)\n");
      ret_val = CFGDB_EBADPARAM;
   }
   else if (conv_size < sizeof(int))
   {
      CFGDB_LOG_ERR("configdb_util_default_int_converter: "
                    "not enough memory allocated to store converted data\n");
      ret_val = CFGDB_EMEMORY;
   }
   else
   {
      char *end = NULL;
      int32 conv = (int32)strtol(src_data, &end, 0);

      /* Make sure that the entire src_data was successfully
         converted to a number */
      if (NULL != end && '\0' == *end)
      {
         *(int32 *)conv_data = conv;
         ret_val = CFGDB_SUCCESS;
      }
   }

   return ret_val;
}


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

void configdb_util_free_tree
(
   configdb_node_t *node
)
{
   if (NULL == node)
      return;

   configdb_util_free_tree(node->child);

   configdb_util_free_tree(node->sibling);

   /* Free the memory allocated for the node's contents and the node */
   CFGDB_FREE(node->name);
   CFGDB_FREE(node->type);
   CFGDB_FREE(node->value);
   CFGDB_FREE(node);
}


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

int32 configdb_util_append_child
(
   configdb_node_t *parent,
   configdb_node_t *child
)
{
   if (NULL == child || NULL == parent)
   {
      CFGDB_LOG_ERR("configdb_util_append_child: bad parameter(s)\n");
      return CFGDB_EBADPARAM;
   }

   /* If the parent node has no children */
   if (NULL == parent->child)
   {
      parent->child = child;
   }
   else
   {
      configdb_node_t *node;

      /* Go to the end of the list */
      for (node = parent->child; NULL != node->sibling; node = node->sibling)
         ;

      child->sibling = NULL;
      node->sibling = child;
   }

   /* Update the child's parent pointer */
   child->parent = parent;

   return CFGDB_SUCCESS;
}


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

void configdb_util_print_tree
(
   const configdb_node_t *node
)
{
   if (NULL == node)
      return;

   printf("name: %s\n", (node->name)?node->name:"null");
   printf("type: %s\n", (node->type)?node->type:"null");
   printf("value: %s\n", (node->value)?node->value:"null");
   printf("parent: %s\n", (node->parent)?node->parent->type:"null");
   printf("children: %u\n\n", node->child_count);

   configdb_util_print_tree(node->child);
   configdb_util_print_tree(node->sibling);
}

