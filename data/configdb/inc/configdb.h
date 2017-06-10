/*!
  @file
  configdb.h

  @brief
  This file declares the configuration parser common definitions.

*/
/*===========================================================================

  Copyright (c) 2011, 2013 Qualcomm Technologies, Inc. All Rights Reserved
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

#ifndef _CONFIGDB_H
#define _CONFIGDB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "comdef.h"


/* Max string size supported by the library */
#define CFGDB_MAX_STRING_SIZE 1000

/* API return codes */

#define CFGDB_SUCCESS          0  /* Successful completion */
#define CFGDB_EFAIL           -1  /* Generic error */
#define CFGDB_EEXISTS         -2  /* Already exists */
#define CFGDB_EBADPARAM       -3  /* Invalid parameter(s) */
#define CFGDB_ENOTFOUND       -4  /* Item not found */
#define CFGDB_EFORMAT         -5  /* Invalid format */
#define CFGDB_EMEMORY         -6  /* Not enough memory */
#define CFGDB_ENOCONVERT      -7  /* No type converter found */
#define CFGDB_ENOSUPPORT      -8  /* Operation is not suppported */
#define CFGDB_ETYPEMISMATCH   -9  /* Mismatch in the actual and expected types */


/* Conversions of types handled by the configdb parser.
   The "int" type (32-bits) should be used for all numeric integer types.
   e.g. boolean, int16, uint8 etc */

#define CFGDB_TYPE_STRING  "string"
#define CFGDB_TYPE_INT     "int"


/* Strings in the xml file recognized by the configdb parser */

#define CFGDB_ELE_LIST      "list"      /* list element */
#define CFGDB_ELE_LISTITEM  "listitem"  /* listitem element */
#define CFGDB_ELE_DATA      "data"      /* data element */

#define CFGDB_ATTR_NAME     "name"      /* name attribute */
#define CFGDB_ATTR_TYPE     "type"      /* type attribute */

#define CFGDB_SPL_ATTR_SIZE "size"      /* special size attribute */


/* Lengths of the different strings recognized by the configdb parser */

#define CFGDB_ELE_LIST_LEN      (sizeof(CFGDB_ELE_LIST))
#define CFGDB_ELE_LISTITEM_LEN  (sizeof(CFGDB_ELE_LISTITEM))
#define CFGDB_ELE_DATA_LEN      (sizeof(CFGDG_ELE_DATA))
#define CFGDB_ATTR_NAME_LEN     (sizeof(CFGDB_ATTR_NAME))
#define CFGDB_ATTR_TYPE_LEN     (sizeof(CFGDB_ATTR_TYPE))
#define CFGDB_TYPE_INT_LEN      (sizeof(CFGDB_TYPE_INT))
#define CFGDB_SPL_ATTR_SIZE_LEN (sizeof(CFGDB_SPL_ATTR_SIZE))


/* Macros for freeing dynamically allocated memory */

#define CFGDB_FREE(x) { free((void *)(x)); (x) = NULL; }


/* Modes of operation supported by the configdb parser */
typedef enum
{
   CFGDB_OPMODE_CACHED,          /* Parse the XML file and cache data internally for
                                    subsequent access */

   CFGDB_OPMODE_NONCACHED,       /* Always parse the XML file on every access */

   CFGDB_OPMODE_MAX = 0x7FFFFFFF /* Should be the last element */

} configdb_opmode_e_t;


/* Type converter function pointer to convert a given string data
   to the desired type (if possible) and store it in data */
typedef int32 (*configdb_type_converter_t)(
   const char *str_data,         /* source string data */
   void  *data,                  /* storage for converted data */
   uint32 size                   /* size of memory allocated for data */
);


/* Externalized functions */


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
int32 configdb_init(
   configdb_opmode_e_t mode,
   const char *xml_file
);


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
void configdb_release(
   void
);


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
      from 0. (e.g. modems.0, modems.1 above)

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
int32 configdb_get_parameter(
   const char *param_id,
   const char *param_type,
   void       *param_val,
   uint32      param_size
);


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
      type      [in] - Type handled by the converter function
      converter [in] - Converter function for the given type

      @return
      CFGDB_EMEMORY
      CFGDB_EBADPARAM
      CFGDB_SUCCESS
  */
/*=========================================================================*/
int32 configdb_register_type_converter(
   const char *type,
   configdb_type_converter_t converter
);


#ifdef __cplusplus
}
#endif

#endif /* _CONFIGDB_H */

