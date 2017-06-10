/*!
  @file
  configdb.c

  @brief
  This file implements the configuration parameters parser.

*/

/*===========================================================================
  Copyright (c) 2011-2014 Qualcomm Technologies, Inc. All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.
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
#include <pthread.h>
#include "configdb.h"
#include "configdbi.h"
#include "configdb_xml.h"
#include "configdb_util.h"

/* Character used for separating names in param_id string */
#define CFGDB_PARAM_ID_SEPARATOR '.'


/* Macros for locking and unlocking mutexes */
#define CFGDB_MUTEX_ACQUIRE(x) pthread_mutex_lock((x))
#define CFGDB_MUTEX_RELEASE(x) pthread_mutex_unlock((x))


/* Static global variables */

/* Root of the configdb tree */
static configdb_node_t *cfgdb_root = NULL;

/* Operating mode requested by client */
static configdb_opmode_e_t cfgdb_opmode = CFGDB_OPMODE_CACHED;

/* Head pointer to the list of registered type converters */
static configdb_converter_node_t *cfgdb_converter_head = NULL;

/* XML file to read from */
static const char *cfgdb_xmlfile = NULL;

/* Mutex for protecting the configdb tree */
static pthread_mutex_t cfgdb_tree_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Mutex for protecting the converter function list */
static pthread_mutex_t cfgdb_converter_mutex = PTHREAD_MUTEX_INITIALIZER;


/* Private function declarations */

static void configdb_reset_globals(
  void
);

static void configdb_release_tree(
   configdb_node_t **root
);

static void configdb_release_type_converters(
   configdb_converter_node_t **head
);

static const configdb_node_t *configdb_find_node(
   const char *param_id,
   const configdb_node_t *root
);

static const configdb_node_t *configdb_find_node_by_name(
   const char *name,
   const configdb_node_t *root
);

static const configdb_node_t *configdb_find_node_by_index(
   uint32 index,
   const configdb_node_t *root
);

static int32 configdb_type_convert(
   const configdb_node_t *param_node,
   void  *param_val,
   uint32 param_size
);

static int32 configdb_add_size_nodes(
   configdb_node_t *root
);

static int32 configdb_init_tree(
   const char *xml_file,
   configdb_node_t **root
);


/* External Function Definitions */

/*===========================================================================
  FUNCTION:  configdb_init
  ===========================================================================*/
  /*!
      @brief
      Initializes the parser and initiates parsing of the XML file in the given
      operating mode. Currently, only the CFGDB_OPMODE_CACHED is supported.

      This function must be called once per process. The entity calling this
      function is responsible for calling the corresponding configdb_release().
      If it is called more than once a CFGDB_EEXISTS error is returned.

      @params
      mode     [in] - Operating mode
      xml_file [in] - Input XML file

      @return
      CFGDB_EFAIL
      CFGDB_EEXISTS
      CFGDB_EFORMAT
      CFGDB_EMEMORY
      CFGDB_ENOSUPPORT
      CFGDB_SUCCESS
  */
/*=========================================================================*/

int32 configdb_init
(
   configdb_opmode_e_t mode,
   const char *xml_file
)
{
   int32 ret_val = CFGDB_EFAIL;
   static int inited = FALSE;

   /* If library is already initialized, release it */
   if (inited)
   {
     CFGDB_LOG_ERR("configdb_init: releasing earlier instance\n");
     configdb_release();
   }

   inited = TRUE;

   /* Acquire the mutex */
   CFGDB_MUTEX_ACQUIRE(&cfgdb_tree_mutex);

   if (NULL != cfgdb_root)
   {
      CFGDB_LOG_ERR("configdb_init: database already initialized\n");

      CFGDB_MUTEX_RELEASE(&cfgdb_tree_mutex);
      return CFGDB_EEXISTS;
   }

   /* Initialize the logging subsystem */
   CFGDB_LOG_INIT(DS_DBG_LEVEL_DFLT, DS_LOG_MODE_DFLT);

#ifdef FEATURE_DATA_LOG_QXDM
   /* Initialize Diag services */
   if ( TRUE != Diag_LSM_Init(NULL) )
   {
      CFGDB_LOG_ERR("configdb_init: failed on Diag_LSM_Init\n" );
   }
#endif

   if (NULL == xml_file)
   {
      CFGDB_LOG_ERR("configdb_init: invalid input file\n");
      ret_val = CFGDB_EBADPARAM;
   }
   /* TODO: Add support for non-cached mode */
   else if (CFGDB_OPMODE_NONCACHED == mode)
   {
      CFGDB_LOG_ERR("configdb_init: noncached operating mode not supported\n");
      ret_val = CFGDB_ENOSUPPORT;
   }
   else
   {
      /* Save the operating mode */
      cfgdb_opmode = mode;

      /* Save the xml filename */
      cfgdb_xmlfile = strndup(xml_file, (size_t)CFGDB_MAX_STRING_SIZE);

      if (NULL == cfgdb_xmlfile)
      {
         CFGDB_LOG_ERR("configdb_init: failed to allocate xml filename\n");
         ret_val = CFGDB_EMEMORY;
      }
      else
      {
         /* If the operating mode is cached, obtain the configdb tree.
            Otherwise, it will be done on every configdb_get_parameter call*/
         if (CFGDB_OPMODE_CACHED == cfgdb_opmode)
         {
            ret_val = configdb_init_tree(cfgdb_xmlfile, &cfgdb_root);
         }

         if (CFGDB_SUCCESS == ret_val)
         {
            /* Register converters for the types handled by the library */
            (void)configdb_register_type_converter(
                     CFGDB_TYPE_STRING,
                     configdb_util_default_string_converter
                  );

            (void)configdb_register_type_converter(
                     CFGDB_TYPE_INT,
                     configdb_util_default_int_converter
                  );
         }
      }
   }

   /* If something went wrong, reset the global variables */
   if (CFGDB_SUCCESS != ret_val)
   {
      configdb_reset_globals();
   }

   /* Release the mutex */
   CFGDB_MUTEX_RELEASE(&cfgdb_tree_mutex);

   return ret_val;
}

/*===========================================================================
  FUNCTION:  configdb_release
  ===========================================================================*/
  /*!
      @brief
      Cleans up and releases memory allocated for any internal data

      @return
      None
  */
/*=========================================================================*/

void configdb_release
(
   void
)
{
   /* Acquire the mutex */
   CFGDB_MUTEX_ACQUIRE(&cfgdb_tree_mutex);

   CFGDB_FREE(cfgdb_xmlfile);
   configdb_release_tree(&cfgdb_root);

   /* Release the mutex */
   CFGDB_MUTEX_RELEASE(&cfgdb_tree_mutex);

   configdb_release_type_converters(&cfgdb_converter_head);
}


/*===========================================================================
  FUNCTION:  configdb_get_parameter
  ===========================================================================*/
  /*!
      @brief
      Get the data corresponding to the given param_id and validate that the
      expected type matches the actual type in the database. If the types don't
      match a CFGDB_ETYPEMISMATCH error is returned.

      The param_id has the form:
      "modems.size"          - Number of modems,
      "modems.0.name"        - Name of the first modem (index starts at 0)
      "modems.1.dataports.0" - Second modem's first dataport etc.

      For elements of type "list", names are auto generated and indices start
      from 0. (e.g. modems.0, modems.1.dataports.0 above)

      Also, elements of type "list" inherently support an attribute "size" of
      type "int" which provides the number of members of that "list".
      (e.g. modems.size above)

      Data is automatically converted to the proper type (as specified in the
      xml file) if a converter function for that type is found. Otherwise, a
      CFGDB_ENOCONVERT error is returned. If successful, the data corresponding
      to the param_id is stored in param_val.

      Note that adequate memory allocation for param_val argument is the
      responsibility of the client. If enough memory isn't allocated for
      param_val a CFGDB_EMEMORY error will be returned.

      @params
      param_id   [in]  - ID of the desired parameter
      param_type [in]  - Expected type of param_id
      param_val  [out] - Converted value corresponding to param_id
      param_size [in]  - Memory allocaed for param_val


      @return
      CFGDB_EFAIL
      CFGDB_ENOTFOUND
      CFGDB_EBADPARAM
      CFGDB_EMEMORY
      CFGDB_ENOCONVERT
      CFGDB_ENOSUPPORT
      CFGDB_ETYPEMISMATCH
      CFGDB_SUCCESS
  */
/*=========================================================================*/

int32 configdb_get_parameter
(
   const char *param_id,
   const char *param_type,
   void       *param_val,
   uint32      param_size
)
{
   int32 ret_val  = CFGDB_EFAIL;

   /* Validate the arguments */
   if (NULL == param_id || NULL == param_type || NULL == param_val || 0 == param_size)
   {
      CFGDB_LOG_ERR("configdb_get_parameter: bad parameter(s)\n");
      return CFGDB_EBADPARAM;
   }

   /* TODO: Add support for non-cached mode with proper protection against
      concurrent access */
#ifdef CFGDB_NONCACHED_MODE
   /* Always re-initialize configdb tree for non-cached mode */
   if (CFGDB_OPMODE_NONCACHED == cfgdb_opmode)
   {
      /* TODO: Acquire the mutex */
      configdb_release_tree(&cfgdb_root);
      ret_val = configdb_init_tree(cfgdb_xmlfile, &cfgdb_root);
      /* TODO: Release the mutex */
   }
#endif

   /* Acquire the mutex */
   CFGDB_MUTEX_ACQUIRE(&cfgdb_tree_mutex);

   if (NULL != cfgdb_root)
   {
      const configdb_node_t *param_node = NULL;
      char *dup_param_id = strndup(param_id, (size_t)CFGDB_MAX_STRING_SIZE);

      if (NULL == dup_param_id)
      {
         CFGDB_LOG_ERR("configdb_get_parameter: failed to dup param_id\n");
         ret_val = CFGDB_EMEMORY;
      }
      else
      {
         if (NULL == (param_node = configdb_find_node(dup_param_id, cfgdb_root)))
         {
            CFGDB_LOG_ERR("configdb_get_parameter: failed to find node "
                          "for param_id: %s\n", param_id);
            ret_val = CFGDB_ENOTFOUND;
         }
         /* Iterating through the elements of a non-leaf node (i.e. list or listitem)
            is not currently supported */
         else if (NULL == param_node->type               ||
                  0 == std_strnicmp(param_node->type,
                                    CFGDB_ELE_LIST,
                                    CFGDB_ELE_LIST_LEN)  ||
                  0 == std_strnicmp(param_node->type,
                                    CFGDB_ELE_LISTITEM,
                                    CFGDB_ELE_LISTITEM_LEN))
         {
            CFGDB_LOG_ERR("configdb_get_parameter: param_id %s refers to an "
                          "internal node\n", param_id);
            ret_val = CFGDB_ENOSUPPORT;
         }
         /* If the expected and actual types don't match, return error */
         else if (0 != std_strnicmp(param_node->type, param_type, (size_t)CFGDB_MAX_STRING_SIZE))
         {
            CFGDB_LOG_ERR("configdb_get_parameter: type %s of param_id %s "
                          "doesn't match expected type %s\n",
                          param_node->type,
                          param_id,
                          param_type);
            ret_val = CFGDB_ETYPEMISMATCH;
         }
         else
         {
            ret_val = configdb_type_convert(param_node, param_val, param_size);
         }

         /* Free up the duplicate param id */
         CFGDB_FREE(dup_param_id);
      }
   }

   /* Release the mutex */
   CFGDB_MUTEX_RELEASE(&cfgdb_tree_mutex);

   return ret_val;
}


/*===========================================================================
  FUNCTION:  configdb_register_type_converter
  ===========================================================================*/
  /*!
      @brief
      Register a type converter for the given type. This function can be called
      before or after configdb_init() is called. Note that if it is called before
      configdb_init() any default converter functions overwritten by configdb_init()
      may need to be re-registered.

      If this function is called with a new converter for an already handled type,
      the old converter is replaced with the new one.

      @params
      type           [in] - Type handled by the converter function
      converter_func [in] - Converter function for the given type

      @return
      CFGDB_EMEMORY
      CFGDB_EBADPARAM
      CFGDB_SUCCESS
  */
/*=========================================================================*/

int32 configdb_register_type_converter
(
   const char *type,
   configdb_type_converter_t converter_func
)
{
   int32 ret_val = CFGDB_EFAIL;
   configdb_converter_node_t *node = NULL;

   if (NULL == type || NULL == converter_func)
   {
      CFGDB_LOG_ERR("configdb_register_type_converter: bad parameter(s)\n");
      return CFGDB_EBADPARAM;
   }

   /* Acquire the mutex */
   CFGDB_MUTEX_ACQUIRE(&cfgdb_converter_mutex);

   /* Find if the type converter already exists */
   for (node = cfgdb_converter_head; node != NULL; node = node->next)
   {
      if (0 == std_strnicmp(node->type, type, CFGDB_MAX_STRING_SIZE))
      {
         break;
      }
   }

   /* If there is an existing entry for the given type, update the converter function */
   if (NULL != node)
   {
      node->converter = converter_func;
      ret_val = CFGDB_SUCCESS;
   }
   /* If there is no existing entry for the given type, create a new one */
   else if (NULL == (node = malloc(sizeof(configdb_converter_node_t))) ||
            NULL == (node->type = strndup(type, (size_t)CFGDB_MAX_STRING_SIZE)))
   {
      CFGDB_LOG_ERR("configdb_register_type_converter: allocation failed\n");
      if (NULL != node) {
          CFGDB_FREE(node);
      }
      ret_val = CFGDB_EMEMORY;
   }
   else
   {
      /* Store the information and insert the node into the list */
      node->converter = converter_func;
      node->next = cfgdb_converter_head;
      cfgdb_converter_head = node;

      ret_val = CFGDB_SUCCESS;
   }

   /* Release the mutex */
   CFGDB_MUTEX_RELEASE(&cfgdb_converter_mutex);

   return ret_val;
}


/* Private function definitions */

/*===========================================================================
  FUNCTION:  configdb_init_tree
  ===========================================================================*/
  /*!
      @brief
      Initializes the configdb tree and adds "size" child nodes for all
      nodes of type "list" in the configdb tree

      @params
      xml_file [in]  - XML file to construct the tree from
      root     [out] - Root of the configdb tree

      @return
      CFGDG_EMEMORY
      CFGDG_EFORMAT
      CFGDG_SUCCESS

      @pre
      The cfgdb_tree_mutex must be locked before calling this function
  */
/*=========================================================================*/

static int32 configdb_init_tree
(
   const char *xml_file,
   configdb_node_t **root
)
{
   int32 ret_val = configdb_xml_init(xml_file, root);

   if (CFGDB_SUCCESS == ret_val)
   {
      /* Add "size" nodes to all "list" nodes in the tree */
      ret_val = configdb_add_size_nodes(*root);
   }

   return ret_val;
}


/*===========================================================================
  FUNCTION:  configdb_add_size_nodes
  ===========================================================================*/
  /*!
      @brief
      This function traverses the configdb tree and adds "size" child nodes
      to all "list" nodes in the tree

      @params
      node [in] - Pointer to a node in the configdb tree

      @return
      CFGDB_EMEMORY
      CFGDB_SUCCESS

      @pre
      The cfgdb_tree_mutex must be locked before calling this function and
      must only be called once per a configdb tree
  */
/*=========================================================================*/

static int32 configdb_add_size_nodes
(
   configdb_node_t *node
)
{
   int32 ret_val = CFGDB_SUCCESS;

   if (NULL == node)
      return ret_val;

   /* If this is a "list" element, append the "size" node */
   if (0 == std_strnicmp(node->type, CFGDB_ELE_LIST, CFGDB_ELE_LIST_LEN))
   {
      configdb_node_t *size = malloc(sizeof(configdb_node_t));

      if (NULL == size)
      {
         CFGDB_LOG_ERR("configdb_add_size_nodes: "
                       "failed to allocate new size node\n");
         ret_val = CFGDB_EMEMORY;
      }
      else
      {
         int asprintf_ret = -1;

         /* Initialize the "size" node */
         size->child_count = 0;
         size->child = size->sibling = NULL;
         size->name = size->type = size->value = NULL;

         /* If memory allocation failed */
         if (NULL == (size->name = strndup(CFGDB_SPL_ATTR_SIZE, CFGDB_SPL_ATTR_SIZE_LEN)) ||
             NULL == (size->type = strndup(CFGDB_TYPE_INT, CFGDB_TYPE_INT_LEN))           ||
             -1   == (asprintf_ret = asprintf(&size->value, "%d", node->child_count)))
         {
            if (NULL != size->name) {
                CFGDB_FREE(size->name);
            }
            if (NULL != size->type) {
                CFGDB_FREE(size->type);
            }
            if (-1 != asprintf_ret)
            {
               CFGDB_FREE(size->value);
            }

            CFGDB_FREE(size);

            CFGDB_LOG_ERR("configdb_add_size_nodes: "
                          "failed to allocate members of new size node\n");
            ret_val = CFGDB_EMEMORY;
         }
         else
         {
            /* Add the "size" node to the end of the "list" node's children */
            ret_val = configdb_util_append_child(node, size);
         }
      }
   }

   /* Traverse the children */
   if (CFGDB_SUCCESS == ret_val)
      ret_val = configdb_add_size_nodes(node->child);

   /* Traverse the siblings */
   if (CFGDB_SUCCESS == ret_val)
      ret_val = configdb_add_size_nodes(node->sibling);

   return ret_val;
}


/*===========================================================================
  FUNCTION:  configdb_release_tree
  ===========================================================================*/
  /*!
      @brief
      Releases memory allocated for the configdb tree

      @params
      root [inout] - Root of the configdb tree

      @return
      None

      @pre
      cfdb_tree_mutex should be acquired before calling this function
  */
/*=========================================================================*/

static void configdb_release_tree
(
   configdb_node_t **root
)
{
   if (NULL == root)
   {
      return;
   }

   configdb_util_free_tree(*root);
   *root = NULL;
}


/*===========================================================================
  FUNCTION:  configdb_release_type_converters
  ===========================================================================*/
  /*!
      @brief
      Releases memory allocated for the type converters

      @params
      head [inout] - Head of the type converter list

      @return
      None
  */
/*=========================================================================*/

static void configdb_release_type_converters
(
   configdb_converter_node_t **head
)
{
   configdb_converter_node_t *node = NULL;
   configdb_converter_node_t *next = NULL;

   if (NULL == head)
   {
      return;
   }

   /* Acquire the mutex */
   CFGDB_MUTEX_ACQUIRE(&cfgdb_converter_mutex);

   /* Go through the list and release each node's data and finally the
      node itself */
   for (node = *head; NULL != node; node = next)
   {
      next = node->next;
      CFGDB_FREE(node->type);
      CFGDB_FREE(node);
   }

   *head = NULL;

   /* Release the mutex */
   CFGDB_MUTEX_RELEASE(&cfgdb_converter_mutex);
}


/*===========================================================================
  FUNCTION:  configdb_find_node
  ===========================================================================*/
  /*!
      @brief
      Return a pointer to the node corresponding to param_id in the configdb
      tree if valid or return NULL otherwise

      @params
      param_id [in] - ID of the parameter being searched for
      root     [in] - Root of the configdb tree

      @return
      NULL if a node corresponding to param_id is not found
      Pointer to the node corresponding to param_id
  */
/*=========================================================================*/

static const configdb_node_t *configdb_find_node
(
   const char *param_id,
   const configdb_node_t *root
)
{
   const configdb_node_t *node = root;
   const configdb_node_t *node_found = NULL;
   const char *begin = param_id;
   const char *end = NULL;
   int end_reached = FALSE;

   /* Have we reached the end of param_id string? */
   while (FALSE == end_reached)
   {
      char *name = NULL;
      uint32 name_len;

      end = strchr(begin, (int)CFGDB_PARAM_ID_SEPARATOR);

      if (NULL == end)
      {
         end = param_id + strlen(param_id);
         end_reached = TRUE;
      }

      node_found = NULL;

      /* Allocate memory to store the next name substring */
      name_len = (uint32)(end - begin);
      name = malloc((size_t)(name_len + 1));

      if (NULL == name)
      {
         break;
      }
      else
      {
         strlcpy(name, begin, (size_t)(name_len + 1));
      }

      /* If the substring is numeric, find the next node by index */
      if (TRUE == configdb_util_isnumeric(name))
      {
         node_found = configdb_find_node_by_index((uint32)atol(name), node);
      }
      /* Otherwise, find the next node by name */
      else
      {
         node_found = configdb_find_node_by_name(name, node);
      }

      /* free the previously allocated name substring */
      CFGDB_FREE(name);

      if (NULL == node_found)
      {
         break;
      }
      else
      {
         /* For the next iteration, continue search in the node's children */
         node = node_found->child;
         /* Start at the next substring */
         begin = end + 1;
      }
   }

   return node_found;
}


/*===========================================================================
  FUNCTION:  configdb_find_node_by_name
  ===========================================================================*/
  /*!
      @brief
      Return a pointer to the node corresponding to param_id in the configdb
      tree if valid or return NULL otherwise

      @params
      name [in] - Name of the node being searched for
      node [in] - Head of the sibling list

      @return
      NULL if a node with the given name is not found
      Pointer to the node corresponding to name

      @pre
      The cfgdb_tree_mutex must be locked before calling this function
  */
/*=========================================================================*/

static const configdb_node_t *configdb_find_node_by_name
(
   const char *name,
   const configdb_node_t *node
)
{
   /* Go through the list of siblings and try to find a node with the
      matching name */
   for (; NULL != node; node = node->sibling)
   {
      if (NULL != node->name &&
          0 == std_strnicmp(name, node->name, (size_t)CFGDB_MAX_STRING_SIZE))
      {
         break;
      }
   }

   return node;
}


/*===========================================================================
  FUNCTION:  configdb_find_node_by_index
  ===========================================================================*/
  /*!
      @brief
      If the index is valid, return a pointer to the node at the
      corresponding index. Otherwise, return NULL

      @params
      index [in] - Index of the sibling being searched for
      node  [in] - Head of the sibling list

      @return
      NULL if a node at the given index is not found
      Pointer to the node at the corresponding index

      @pre
      The cfgdb_tree_mutex must be locked before calling this function
  */
/*=========================================================================*/

static const configdb_node_t *configdb_find_node_by_index
(
   uint32 index,
   const configdb_node_t *node
)
{
   const configdb_node_t *node_found = NULL;

   /* Make sure that the parent node is a "list" node and the index requested
      is also valid */
   if (NULL != node &&
       NULL != node->parent &&
       0 == std_strnicmp(node->parent->type, CFGDB_ELE_LIST, CFGDB_ELE_LIST_LEN) &&
       index < node->parent->child_count)
   {
      uint32 i;
      for (i = 0; i < index && NULL != node; ++i)
      {
         node = node->sibling;
      }
      node_found = node;
   }

   return node_found;
}


/*===========================================================================
  FUNCTION:  configdb_type_convert
  ===========================================================================*/
  /*!
      @brief
      Go through the list of converters and find one which can handle the
      param_node's type. If found, call the corresponding converter.

      @params
      param_node [in]  - Pointer to a node in the configdb tree
      param_val  [out] - Memory to store the converted value
      param_size [in]  - Size of memory allocated for param_val

      @return
      CFGDB_ENOCONVERT
      CFGDB_EMEMORY
      CFGDB_SUCCESS

      @dependency
      The client provided converter function must never call back into the
      library as it might result in a dead lock
  */
/*=========================================================================*/

int32 configdb_type_convert
(
   const configdb_node_t *param_node,
   void  *param_val,
   uint32 param_size
)
{
   int32 ret_val = CFGDB_ENOCONVERT;
   configdb_converter_node_t *node;

   /* Acquire the mutex */
   CFGDB_MUTEX_ACQUIRE(&cfgdb_converter_mutex);

   /* Find and invoke the converter that can handle the param_node's type */
   for (node = cfgdb_converter_head; NULL != node; node = node->next)
   {
      if (0 == std_strnicmp(node->type, param_node->type, (size_t)CFGDB_MAX_STRING_SIZE))
      {
         ret_val = node->converter(param_node->value, param_val, param_size);
         break;
      }
   }

   /* Release the mutex */
   CFGDB_MUTEX_RELEASE(&cfgdb_converter_mutex);

   return ret_val;
}


/*===========================================================================
  FUNCTION:  configdb_reset_globals
  ===========================================================================*/
  /*!
      @brief
      This function resets the global variables used by the library

      @return
      None

      @pre
      cfgdb_tree_mutex must be locked before calling this function
  */
/*=========================================================================*/

void configdb_reset_globals
(
   void
)
{
   configdb_release_tree(&cfgdb_root);
   cfgdb_opmode = CFGDB_OPMODE_CACHED;
   CFGDB_FREE(cfgdb_xmlfile);
}

