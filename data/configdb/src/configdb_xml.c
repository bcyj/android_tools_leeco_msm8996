/*!
  @file
  configdb_xml.c

  @brief
  This file implements the XML specific parsing functionality.

*/

/*===========================================================================

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary and Confidential.

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
#include "xmllib_common.h"
#include "xmllib_parser.h"
#include "configdb.h"
#include "configdbi.h"
#include "configdb_xml.h"
#include "configdb_util.h"

/* Private Type definitions */

/* Max allowed size of the XML file (16 MB) */
#define CFGDB_XML_MAX_FILESIZE (16 << 20)

/* Defines for clipping space or space & quotes (single, double) */
#define CFGDB_XML_CLIP_SPACE        " "
#define CFGDB_XML_CLIP_SPACE_QUOTES " '\""

/* Structure for storing the xml data from file, current read position and
   size information */
struct xml_data
{
   uint32 size;  /* Actual size of the file data */
   uint32 read;  /* Current read position in buff */
   char *buff;   /* Buffer containing the xml file contents */
};


/* Private function declarations */

static int32 configdb_xml_peekbytes_cb(
   struct xmllib_metainfo_s_type *metainfo,
   int32                          offset,
   int32                          bytecount,
   uint8                         *buffer
);

static int32 configdb_xml_getbytes_cb(
   struct xmllib_metainfo_s_type *metainfo,
   int32                          bytecount,
   char                          *buffer
);

static int32 configdb_xml_parse(
   const char *xml_file,
   xmllib_parsetree_node_s_type **xml_root
);

static int32 configdb_xml_read_xml_file(
   const char *xm_file,
   struct xml_data *xml
);

static int32 configdb_xml_aggregate_clone_tree(
   const xmllib_parsetree_node_s_type *xml_root,
   configdb_node_t **cfgdb_root
);

static int32 configdb_xml_validate_parse_tree(
   xmllib_parsetree_node_s_type *xml_root
);

static configdb_node_t *configdb_xml_dup_node(
   const xmllib_parsetree_node_s_type *xml_node,
   configdb_node_t *parent_node,
   int32 *ret_val
);

static int32 configdb_xml_init_aggregate_node(
   const xmllib_parsetree_node_s_type *xml_node,
   configdb_node_t *cfgdb_node,
   configdb_node_t *parent_node
);

static const xmllib_parsetree_node_s_type *configdb_xml_find_root_element(
   const xmllib_parsetree_node_s_type *xml_root
);

/* Private Function Definitions */

/*===========================================================================
  FUNCTION:  configdb_xmllib_memalloc
  ===========================================================================*/
  /*!
      @brief
      This function is a wrapper to the malloc function

      @params
      size   [in]  - The size of memory to allocate

      @return
      The pointer to the allocated memory
  */
/*=========================================================================*/

static void * configdb_xmllib_memalloc
(
  int32 size
)
{
  return malloc((size_t)size);
}

/* External Function Definitions */

/*===========================================================================
  FUNCTION:  configdb_xml_init
  ===========================================================================*/
  /*!
      @brief
      This function parses the given xml_file, constructs a configdb tree and
      returns a pointer to the root of the tree via cfgdb_root

      @params
      xml_file   [in]  - XML file from which to construct the configdb tree
      cfgdb_root [out] - Root of the configdb tree

      @return
      CFGDB_ENOTFOUND
      CFGDB_EBADPARAM
      CFGDB_EMEMORY
      CFGDB_EFORMAT
      CFGDB_EFAIL
      CFGDB_SUCCESS
  */
/*=========================================================================*/

int32 configdb_xml_init
(
   const char *xml_file,
   configdb_node_t **cfgdb_root
)
{
   xmllib_parsetree_node_s_type *xml_root = NULL;
   int32 ret_val = CFGDB_EFAIL;

   if (NULL == cfgdb_root)
   {
      CFGDB_LOG_ERR("configdb_xml_init: bad parameter cfgdb_root NULL\n");
      return CFGDB_EBADPARAM;
   }

   /* Parse the xml file using xmllib and obtain the root */
   ret_val = configdb_xml_parse(xml_file, &xml_root);

   if (CFGDB_SUCCESS == ret_val)
   {
      /* Validate the xml tree returned by the xmllib */
      ret_val = configdb_xml_validate_parse_tree(xml_root);

      if (CFGDB_SUCCESS == ret_val)
      {
         /* Create a (almost) clone of the xmllib's tree while aggregating
            data into configdb tree */
         ret_val = configdb_xml_aggregate_clone_tree(xml_root, cfgdb_root);
      }

      /* Free up the xmllib's parse tree */
      xmllib_parser_free(free, xml_root);
   }

   return ret_val;
}


/* Private Function Definitions */

/*===========================================================================
  FUNCTION:  configdb_xml_parse
  ===========================================================================*/
  /*!
      @brief
      This function reads the given xml_file into a local data structure and
      uses xmllib to parse the xml data into a xml tree whose root is returned
      via xml_root parameter

      @params
      xml_file [in]  - Input XML from which to create the xmllib parse tree
      xml_root [out] - xmllib's root node

      @return
      CFGDB_ENOTFOUND
      CFGDB_EBADPARAM
      CFGDB_EMEMORY
      CFGDB_EFORMAT
      CFGDB_EFAIL
      CFGDB_SUCCESS
  */
/*=========================================================================*/

static int32 configdb_xml_parse
(
   const char *xml_file,
   xmllib_parsetree_node_s_type **xml_root
)
{
   struct xml_data xml_data = {0, 0, NULL};

   /* Update the xml_data structure with data from the xml file */
   int32 ret_val = configdb_xml_read_xml_file(xml_file, &xml_data);

   if (CFGDB_SUCCESS == ret_val)
   {
      xmllib_error_e_type error = 0;
      int32 xmllib_ret;
      struct xmllib_metainfo_s_type metainfo = {
                        &xml_data,
                        (xmllib_memalloc_fptr_type)configdb_xmllib_memalloc,
                        free,
                        configdb_xml_peekbytes_cb,
                        configdb_xml_getbytes_cb
                        };

      /* Invoke the XML parser and obtain the parse tree */
      xmllib_ret = xmllib_parser_parse(XMLLIB_ENCODING_UTF8,
                                       &metainfo,
                                       xml_root,
                                       &error);

      if (XMLLIB_SUCCESS == xmllib_ret)
      {
         ret_val = CFGDB_SUCCESS;
      }
      else
      {
         CFGDB_LOG_ERR("configdb_xml_parse: xmllib returned parse error\n");
         ret_val = CFGDB_EFORMAT;
      }
   }

   /* Free the buffer allocated by the xml file reading utility function */
   CFGDB_FREE(xml_data.buff);

   return ret_val;
}


/*===========================================================================
  FUNCTION:  configdb_xml_read_xml_file
  ===========================================================================*/
  /*!
      @brief
      This function is reads the given XML file and stores the contents in
      the given xml_data structure

      @params
      file_name [in]  -  XML file name
      xml_data  [out] -  Structure to read data into

      @return
      CFGDB_ENOTFOUND
      CFGDB_EMEMORY
      CFGDB_EFAIL
      CFGDB_SUCCESS
  */
/*=========================================================================*/

static int32 configdb_xml_read_xml_file
(
   const char *file_name,
   struct xml_data *xml_data
)
{
   FILE *fp = NULL;
   int32 ret_val = CFGDB_EFAIL;
   int32 cur_pos;

   fp = fopen(file_name, "rb");

   /* File not found at the given path */
   if (NULL == fp)
   {
      CFGDB_LOG_ERR("configdb_xml_read_xml_file: unable to open file %s\n",
                    file_name);
      ret_val = CFGDB_ENOTFOUND;
   }
   /* If seek to the end failed or file size is greater than what we support */
   else if (fseek(fp, (long)0, SEEK_END) ||
            ((cur_pos = (int32)ftell(fp)) < 0 || cur_pos > CFGDB_XML_MAX_FILESIZE) )
   {
      fclose(fp);
   }
   else
   {
      xml_data->size = (uint32)cur_pos;

      /* Allocate storage for reading the xml file into memory */
      if (NULL == (xml_data->buff = malloc(xml_data->size)))
      {
         CFGDB_LOG_ERR("configdb_xml_read_xml_file: failed to allocate "
                       "memory for read buffer\n");
         ret_val = CFGDB_EMEMORY;
      }
      /* Reset to the beginning of the file */
      else if (!fseek(fp, 0, SEEK_SET))
      {
         size_t read_size;

         /* Read the data from file to buffer */
         read_size = fread(xml_data->buff, (size_t)1, (size_t)xml_data->size, fp);

         if (!ferror(fp) && (read_size == xml_data->size))
         {
            xml_data->read = 0;
            ret_val = CFGDB_SUCCESS;
         }
      }

      fclose(fp);
   }

   return ret_val;
}


/*===========================================================================
  FUNCTION:  configdb_xml_peekbytes_cb
  ===========================================================================*/
  /*!
      @brief
      This function allows the xmllib to peek into the xml file without
      actually consuming data

      @params
      metainfo  [in]  - XML meta information struct
      offset    [in]  - Offset in the text
      bytecount [in]  - Number of bytes to return
      buffer    [out] - Buffer to return bytes in

      @return
      XMLLIB_ERROR
      XMLLIB_SUCCESS

      @note
      Dependency - Enough memory must be allocated by xmllib for the buffer
  */
/*=========================================================================*/

static int32 configdb_xml_peekbytes_cb
(
  struct xmllib_metainfo_s_type *metainfo,
  int32                          offset,
  int32                          bytecount,
  uint8                         *buffer
)
{
   struct xml_data *xml_data = NULL;

   /* Validate arguments */
   if ((NULL == metainfo)                       ||
       (NULL == (xml_data = metainfo->xmltext)) ||
       (offset < 0)                             ||
       (bytecount < 0)                          ||
       (NULL == buffer)                         ||
       (xml_data->read+(uint32)offset+(uint32)bytecount > xml_data->size))
   {
      return XMLLIB_ERROR;
   }

   memcpy(buffer, xml_data->buff+xml_data->read+offset, (size_t)bytecount);
   return XMLLIB_SUCCESS;
}


/*===========================================================================
  FUNCTION:  configdb_xml_getbytes_cb
  ===========================================================================*/
  /*!
      @brief
      This function allows the xmllib to consume the required number of bytes

      @params
      metainfo  [in]  - XML meta information struct
      bytecount [in]  - Number of bytes to return
      buffer    [out] - Optional buffer to copy bytes to


      @return
      XMLLIB_ERROR
      XMLLIB_SUCCESS

      @note
      Dependency - Enough memory must be allocated by xmllib for the buffer
  */
/*=========================================================================*/

static int32 configdb_xml_getbytes_cb
(
  struct xmllib_metainfo_s_type *metainfo,
  int32                          bytecount,
  char                          *buffer
)
{
   struct xml_data *xml_data = NULL;

   /* If requesting to read more than what we have return error */
   if ((NULL == metainfo)                       ||
       (NULL == (xml_data = metainfo->xmltext)) ||
       (bytecount < 0)                          ||
       (xml_data->read+(uint32)bytecount > xml_data->size))
   {
      return XMLLIB_ERROR;
   }

   /* If a valid buffer is given, copy the data */
   if (NULL != buffer)
   {
      memcpy(buffer, xml_data->buff+xml_data->read, (size_t)bytecount);
   }

   /* Increment to the next unread data block */
   xml_data->read += (uint32)bytecount;

   return XMLLIB_SUCCESS;
}


/*===========================================================================
  FUNCTION:  configdb_xml_validate_parse_tree
  ===========================================================================*/
  /*!
      @brief
      This function traverses the xml tree to makes sure that the elements
      are valid ("list", "listitem" or "data"), all "data" elements have a
      "type" attribute and the child node of "data" element is always a
      "content" node

      @params
      xml_node [in] - xmllib's parse tree node

      @return
      CFGDB_EFORMAT
      CFGDB_SUCCESS

      @note
      This is a recursive function

      Different types of nodes in a xmllib tree:
      * XMLLIB_PARSETREE_NODE_ELEMENT
      * XMLLIB_PARSETREE_NODE_EMPTYELEMENT
      * XMLLIB_PARSETREE_NODE_CONTENT
      * XMLLIB_PARSETREE_NODE_PI
      * XMLLIB_PARSETREE_NODE_COMMENT
      * XMLLIB_PARSETREE_NODE_XMLDECL
  */
/*=========================================================================*/

static int32 configdb_xml_validate_parse_tree
(
   xmllib_parsetree_node_s_type *xml_node
)
{
   int32 ret_val = CFGDB_SUCCESS;

   if (NULL == xml_node)
      return ret_val;

   /* Only xmllib nodes of type ELEMENT can have children */
   if (XMLLIB_PARSETREE_NODE_ELEMENT == xml_node->nodetype)
   {
      if (0 == configdb_util_icmp_string(&xml_node->payload.element.name,
                                         CFGDB_ELE_DATA))
      {
         /* Every "data" element must have a "type" attribute */
         xmllib_parsetree_attribute_s_type *attr = xml_node->payload.element.attribute_list;
         for (; NULL != attr; attr = attr->next)
         {
            if (0 == configdb_util_icmp_string(&attr->name, CFGDB_ATTR_TYPE) &&
                attr->value.len > 2) /* There are atleast 2 quote characters */
            {
               break;
            }
         }

         /* For every "data" element make sure there's a valid "type" attribute and
            a child "content" node */
         if (NULL == attr                             ||
             NULL == xml_node->payload.element.child  ||
             XMLLIB_PARSETREE_NODE_CONTENT != xml_node->payload.element.child->nodetype)
         {
            CFGDB_LOG_ERR("configdb_xml_validate_parse_tree: "
                          "data element is malformed\n");
            ret_val = CFGDB_EFORMAT;
         }
      }
      /* If the element name is not recognized, return format error */
      else if (0 != configdb_util_icmp_string(&xml_node->payload.element.name,
                                              CFGDB_ELE_LIST)                    &&
               0 != configdb_util_icmp_string(&xml_node->payload.element.name,
                                              CFGDB_ELE_LISTITEM))
      {
         CFGDB_LOG_ERR("configdb_xml_validate_parse_tree: "
                       "unrecognized element\n");
         ret_val = CFGDB_EFORMAT;
      }

      if (CFGDB_SUCCESS == ret_val)
         ret_val = configdb_xml_validate_parse_tree(xml_node->payload.element.child);
   }

   if (CFGDB_SUCCESS == ret_val)
      ret_val = configdb_xml_validate_parse_tree(xml_node->sibling);

   return ret_val;
}


/*===========================================================================
  FUNCTION:  configdb_xml_aggregate_clone_tree
  ===========================================================================*/
  /*!
      @brief
      This function traverses the xml tree, creates a corresponding configdb
      node and aggregates all the recognized attributes and data into the node

      @params
      xml_root   [in]  - Root of the xmllib's parse tree
      cfgdb_root [out] - Return pointer for root of the configdb tree

      @return
      CFGDB_EFAIL
      CFGDB_EMEMORY
      CFGDB_SUCCESS
  */
/*=========================================================================*/

static int32 configdb_xml_aggregate_clone_tree
(
   const xmllib_parsetree_node_s_type *xml_root,
   configdb_node_t **cfgdb_root
)
{
   int32 ret_val = CFGDB_EFAIL;
   const xmllib_parsetree_node_s_type *xml_root_ele = NULL;

   /* Get to the root element if there are comments or other nodes */
   if (NULL != (xml_root_ele = configdb_xml_find_root_element(xml_root)))
   {
      *cfgdb_root = NULL;

      /* Recursively duplicate and aggregate all the xml nodes */
      *cfgdb_root = configdb_xml_dup_node(xml_root_ele, NULL, &ret_val);

      /* In case of error free up any partial tree */
      if (CFGDB_SUCCESS != ret_val)
      {
         if (NULL != *cfgdb_root)
         {
            configdb_util_free_tree(*cfgdb_root);
            *cfgdb_root = NULL;
         }
      }
   }

   return ret_val;
}


/*===========================================================================
  FUNCTION:  configdb_xml_find_root_element
  ===========================================================================*/
  /*!
      @brief
      This function searches for the root element in the xmllib's top level
      sibling list

      @params
      xml_root [in] - Root of the xmllib's parse tree

      @return
      NULL if root element is not found
      Pointer to the root element
  */
/*=========================================================================*/

static const xmllib_parsetree_node_s_type *configdb_xml_find_root_element
(
   const xmllib_parsetree_node_s_type *xml_root
)
{
   const xmllib_parsetree_node_s_type *node = NULL;

   /* Find the root element node */
   for(node = xml_root; NULL != node; node = node->sibling)
   {
      if (XMLLIB_PARSETREE_NODE_ELEMENT == node->nodetype)
      {
         break;
      }
   }

   return node;
}


/*===========================================================================
  FUNCTION:  configdb_xml_dup_node
  ===========================================================================*/
  /*!
      @brief
      This function recursively duplicates and aggregates each xml node
      into a configdb node

      @params
      xml_node    [in]  - Node in the xmllib's parse tree
      parent_node [in]  - Parent node of the to be created configdb node
      ret_val     [out] - Return value

      @return
      NULL if no valid elements in the subtree
      Root of the subtree
  */
/*=========================================================================*/

static configdb_node_t *configdb_xml_dup_node
(
   const xmllib_parsetree_node_s_type *xml_node,
   configdb_node_t *parent_node,
   int32 *ret_val
)
{
   configdb_node_t *cfgdb_node = NULL;

   *ret_val = CFGDB_SUCCESS;

   if (XMLLIB_PARSETREE_NODE_ELEMENT == xml_node->nodetype)
   {
      cfgdb_node = malloc(sizeof(configdb_node_t));
      if (NULL == cfgdb_node)
      {
         CFGDB_LOG_ERR("configdb_xml_dup_node: failed to allocate "
                       "new configdb node\n");
         *ret_val = CFGDB_EMEMORY;
      }
      else
      {
         *ret_val = configdb_xml_init_aggregate_node(xml_node,
                                                     cfgdb_node,
                                                     parent_node);

         /* If this is not a "data" element node, explore its subtree */
         if (CFGDB_SUCCESS == *ret_val &&
             NULL != xml_node->payload.element.child &&
             0 != configdb_util_icmp_string(&xml_node->payload.element.name,
                                            CFGDB_ELE_DATA))
         {
            cfgdb_node->child = configdb_xml_dup_node(xml_node->payload.element.child,
                                                      cfgdb_node,
                                                      ret_val);

            /* Increment the count for the last child node */
            if (CFGDB_SUCCESS == *ret_val && NULL != cfgdb_node->child)
            {
               ++cfgdb_node->child_count;
            }
         }
      }
   }

   /* Once we are done with a node's children, explore its siblings */
   if (CFGDB_SUCCESS == *ret_val &&
       NULL != xml_node->sibling)
   {
      /* If this is a non-element node (e.g. comment), skip to the
         next sibling node */
      if (XMLLIB_PARSETREE_NODE_ELEMENT != xml_node->nodetype)
      {
         cfgdb_node = configdb_xml_dup_node(xml_node->sibling, parent_node, ret_val);
      }
      else
      {
         cfgdb_node->sibling = configdb_xml_dup_node(xml_node->sibling,
                                                     parent_node,
                                                     ret_val);
         /* Increment the parent node's child count on the addition of
            each sibling node */
         if (CFGDB_SUCCESS == *ret_val   &&
             NULL != cfgdb_node->sibling &&
             NULL != parent_node)
         {
            ++parent_node->child_count;
         }
      }
   }

   return cfgdb_node;
}


/*===========================================================================
  FUNCTION:  configdb_xml_init_aggregate_node
  ===========================================================================*/
  /*!
      @brief
      This function initializes the configdb node, aggregates data from the
      corresponding xml node and converts all stored data to lowercase.

      @params
      xml_node    [in] - Node in the xmllib's parse tree
      cfgdb_node  [in] - Node in the configdb tree
      parent_node [in] - Parent node of cfgdb_node

      @return
      CFGDB_EBADPARAM
      CFGDB_EMEMORY
      CFGDB_SUCCESS
  */
/*=========================================================================*/

static int32 configdb_xml_init_aggregate_node
(
   const xmllib_parsetree_node_s_type *xml_node,
   configdb_node_t *cfgdb_node,
   configdb_node_t *parent_node
)
{
   int32 ret_val = CFGDB_SUCCESS;
   xmllib_parsetree_attribute_s_type *attr = xml_node->payload.element.attribute_list;

   /* Initialize the cfgdb_node */
   memset(cfgdb_node, 0, sizeof(configdb_node_t));
   cfgdb_node->parent  = parent_node;

   /* Go through the attribute list to obtain the name and type (if present) */
   for (; attr != NULL; attr = attr->next)
   {
      if (0 == configdb_util_icmp_string(&attr->name, CFGDB_ATTR_NAME))
      {
         /* Copy the value */
         if ((int32)CFGDB_SUCCESS != (ret_val = configdb_util_dup_string(&cfgdb_node->name,
                                                                  &attr->value)))
         {
            break;
         }
         else
         {
            /* Get rid of any quote or extra space characters at either end */
            ret_val = configdb_util_clip_chars(CFGDB_XML_CLIP_SPACE_QUOTES,
                                               cfgdb_node->name);
         }
      }
      else if (0 == configdb_util_icmp_string(&attr->name, CFGDB_ATTR_TYPE))
      {
         /* Copy the value */
         if ((int32)CFGDB_SUCCESS != (ret_val = configdb_util_dup_string(&cfgdb_node->type,
                                                                  &attr->value)))
         {
            break;
         }
         else
         {
            /* Get rid of any quote or extra space characters at either end */
            ret_val = configdb_util_clip_chars(CFGDB_XML_CLIP_SPACE_QUOTES,
                                               cfgdb_node->type);
         }
      }
   }

   if (CFGDB_SUCCESS == ret_val)
   {
      /* If this is a "data" element, obtain the content from its child node */
      if (0 == configdb_util_icmp_string(&xml_node->payload.element.name,
                                         CFGDB_ELE_DATA))
      {
         /* Copy the child node's data in the value field */
         ret_val = configdb_util_dup_string(
                      &cfgdb_node->value,
                      &xml_node->payload.element.child->payload.content
                   );

         /* Get rid of any extra spaces at either end */
         if (CFGDB_SUCCESS == ret_val)
            ret_val = configdb_util_clip_chars(CFGDB_XML_CLIP_SPACE,
                                               cfgdb_node->value);
      }
      /* If this is a "list" or "listitem" element, copy the element's name as
         the configdb node's type */
      else if (0 == configdb_util_icmp_string(&xml_node->payload.element.name,
                                              CFGDB_ELE_LIST) ||
               0 == configdb_util_icmp_string(&xml_node->payload.element.name,
                                              CFGDB_ELE_LISTITEM))
      {
         /* Free any previously allocated type */
         CFGDB_FREE(cfgdb_node->type);

         /* Copy the xml_node's name as the cfgdb_node's type */
         ret_val = configdb_util_dup_string(&cfgdb_node->type,
                                            &xml_node->payload.element.name);
      }
   }

   return ret_val;
}

