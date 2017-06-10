#ifndef _XMLLIB_COMMON_H
#define _XMLLIB_COMMON_H
/*===========================================================================

              X M L   L I B R A R Y   C O M M O N   D E F S
                   
DESCRIPTION
 This file declares the XML library common definitions.
 
EXTERNALIZED FUNCTIONS

Copyright (c) 2005 by Qualcomm Technologies, Inc.  All Rights Reserved.
===========================================================================*/
/*===========================================================================

                      EDIT HISTORY FOR FILE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //depot/asic/sandbox/projects/cne/common/xmllib/inc/xmllib_common.h#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
06/29/06   ssingh  Added support for UTF-8 encoding - extended 
                   xmllib_error_e_type, xmllib_encoding_e_type
09/09/05   jsh     Added support for generator, moved common structures here.
09/08/05   jsh     Changed XMLLIB_ERROR enum values to aid testing.
10/30/03   ifk     Created module.

===========================================================================*/
/*===========================================================================

                      INCLUDE FILES FOR MODULE

===========================================================================*/

#ifdef FEATURE_XMLLIB
#include "comdef.h"
#include <stdio.h>
#include <assert.h>

#ifdef FEATURE_XMLLIB_LOG_QXDM
#include <msg.h>
#include <msgcfg.h>
#include <diag_lsm.h>
#endif

/*===========================================================================
                          VERSION NUMBER
===========================================================================*/
/*---------------------------------------------------------------------------
  When the xml content being parsed self-contains version information (
  such as xmldecl element), xml parser will check the version number against
  this constant value.
---------------------------------------------------------------------------*/
#define XMLLIB_XML_VER "1.0"

/*===========================================================================
                             MACRO DEFINITIONS
===========================================================================*/
/*---------------------------------------------------------------------------
  Define the values of SUCCESS and ERROR.  This is the return type from
  most functions to indicate status of function operations.
---------------------------------------------------------------------------*/
#define XMLLIB_SUCCESS  0 
#define XMLLIB_ERROR   -1

/*---------------------------------------------------------------------------
  Macros used in debugging.  Turn off in production code.
---------------------------------------------------------------------------*/
#define XMLLIBI_DEBUG

#ifdef XMLLIBI_DEBUG
  #define XMLLIBI_DEBUG_ASSERT(x)  assert(x)
#else
  #define XMLLIBI_DEBUG_ASSERT(x)
#endif /* #ifdef XMLLIBI_DEBUG */

/*---------------------------------------------------------------------------
  Macros used for logging
---------------------------------------------------------------------------*/
#if defined(FEATURE_XMLLIB_LOG_QXDM)
  /* Maximum length of log message */
  #define XMLLIB_MAX_DIAG_LOG_MSG_SIZE  512

  /* Log message to Diag */
  #define DS_LOG_MSG_DIAG( lvl, ... )                                               \
  do                                                                                \
  {                                                                                 \
    char buf[ XMLLIB_MAX_DIAG_LOG_MSG_SIZE ];                                       \
                                                                                    \
    /* Format message for logging */                                                \
    xmllib_common_format_log_msg( buf, XMLLIB_MAX_DIAG_LOG_MSG_SIZE, __VA_ARGS__ ); \
                                                                                    \
    /* Log message to Diag */                                                       \
    MSG_SPRINTF_1( MSG_SSID_QCNE, lvl, "%s", buf );                                 \
  }                                                                                 \
  while(0)

  #define XMLLIB_LOG_ERR(...) DS_LOG_MSG_DIAG(MSG_LEGACY_ERROR, __VA_ARGS__)

#elif defined(FEATURE_XMLLIB_LOG_STDERR)
  #define XMLLIB_LOG_ERR(fmt, ...)  \
    fprintf( stderr, "\n%s: %d: " fmt,__FILE__, __LINE__, ##__VA_ARGS__) 
#else
  #define XMLLIB_LOG_ERR(...)
#endif /* #ifdef FEATURE_XMLLIB_LOG_QXDM */

/*===========================================================================
                            FORWARD DECLARATIONS
===========================================================================*/
/*---------------------------------------------------------------------------
  XML meta info structure declared here so it can be used in function
  declarations.  Actual definition later in the file
---------------------------------------------------------------------------*/
struct xmllib_metainfo_s_type;
struct xmllib_gen_metainfo_s_type;

/*===========================================================================
                     FUNCTION POINTER TYPE DEFINITIONS
===========================================================================*/
/*===========================================================================
FUNCTION POINTER TYPEDEF XMLLIB_MEMALLOC_FPTR_TYPE

DESCRIPTION
  This is the prototype of the function pointer registered by an 
  application which is called to allocate size bytes of memory.
  
DEPENDENCIES
  None.

RETURN VALUE
  NULL if memory couldn't be allocated 
  pointer to block of memory of size bytes

SIDE EFFECTS
  Allocates size bytes of memory.
===========================================================================*/
typedef void * (*xmllib_memalloc_fptr_type)
(
  int32 size
);


/*===========================================================================
FUNCTION POINTER TYPEDEF XMLLIB_MEMFREE_FPTR_TYPE

DESCRIPTION
  This is the function pointer used to call an application registered
  function to free a block of memory allocated by memalloc.
  
DEPENDENCIES
  This funtion must be able to handle NULL pointers.

RETURN VALUE
  None.

SIDE EFFECTS
  Frees the memory in the pointer passed in.
===========================================================================*/
typedef void (*xmllib_memfree_fptr_type)
(
  void *membuffer
);


/*===========================================================================
FUNCTION POINTER TYPEDEF XMLLIB_PEEKBYTES_FPTR_TYPE

DESCRIPTION
  This function is used to peek at bytecount bytes in the XML text without
  consumig them.  The bytes are returned in the passed buffer.
  
DEPENDENCIES
  The buffer passed must have sufficient memory allocated to return the 
  asked bytecount number of bytes.

RETURN VALUE
  XMLLIB_ERROR     in case of error
  XMLLIB_SUCCESS   if successful with the bytes returned in the buffer
                   argument.

SIDE EFFECTS
  None
===========================================================================*/
typedef int32 (*xmllib_peekbytes_fptr_type)
(
  struct xmllib_metainfo_s_type *metainfo,  /* XML meta information struct */
  int32                          offset,    /* Offset in the text          */
  int32                          bytecount, /* Number of bytes to return   */
  uint8                         *buffer     /* Buffer to return byte in    */
);


/*===========================================================================
FUNCTION POINTER TYPEDEF XMLLIB_GETBYTES_FPTR_TYPE

DESCRIPTION
  This function is used to consume bytecount bytes from the XML text.  If a
  non-NULL buffer is passed the bytes are copied to it.
  
DEPENDENCIES
  Enough memory should be allocated to buffer to copy bytecount bytes.

RETURN VALUE
  XMLLIB_ERROR    If stream doesn't have bytecount bytes in it.
  XMLLIB_SUCCESS  Otherwise with the bytes returned in the buffer argument
                  if non-NULL.

SIDE EFFECTS
  The bytes are consumed.
===========================================================================*/
typedef int32 (*xmllib_getbytes_fptr_type)
(
  struct xmllib_metainfo_s_type *metainfo,  /* XML meta information struct */
  int32                          bytecount, /* Number of bytes to consume  */
  char                          *buffer     /* Buffer to copy bytes to     */
);

/*===========================================================================
FUNCTION POINTER TYPEDEF XMLLIB_PUTBYTES_FPTR_TYPE

DESCRIPTION
  This function is used to put bytes in the specified location.
  
DEPENDENCIES
  None

RETURN VALUE
  XMLLIB_ERROR     in case of error
  XMLLIB_SUCCESS   if successful with the bytes put in the desired location

SIDE EFFECTS
  None
===========================================================================*/
typedef int32 (*xmllib_putbytes_fptr_type)
(
  struct xmllib_gen_metainfo_s_type *metainfo,    /* XML meta info struct  */
  uint32                             numbytes,    /* num of bytes to put   */
  char                              *buffer,      /* contains bytes to put */
  uint32                            *bytesput     /* actual bytes put      */
);

/*===========================================================================
                     DATA STRUCTURE TYPE DEFINITIONS
===========================================================================*/
/*---------------------------------------------------------------------------
  XML Library error codes
---------------------------------------------------------------------------*/
typedef enum xmllib_error_e_type
{
  /* 0 is assigned to XMLLIB_SUCCESS, we start from 1 here */
  XMLLIB_ERROR_INVALID_ARGUMENTS = 1,  /* Invalid arguments to function    */
  XMLLIB_ERROR_NOMEM             = 2,  /* Out of memory                    */
  XMLLIB_ERROR_INVALID_TEXT      = 3,  /* XML text malformed               */
  XMLLIB_ERROR_UNSUP_ENCODING    = 4,  /* Encoding is unsupported          */
  XMLLIB_ERROR_UNEXPECTED_TEXT   = 5,  /* Unexpected XML text encountered  */
  XMLLIB_ERROR_INVALID_TOKEN     = 6,  /* invalid token                    */
  XMLLIB_ERROR_MALFORMED_TREE    = 7,  /* malformed parse tree             */
  XMLLIB_ERROR_INVALID_ENCODING  = 8,  /* Encoding is incorrect            */
  XMLLIB_ERROR_MAX
} xmllib_error_e_type;

/*---------------------------------------------------------------------------
  XML library charachter encoding
---------------------------------------------------------------------------*/
typedef enum xmllib_encoding_e_type
{
  XMLLIB_ENCODING_ASCII   = 0,         /* ASCII text                       */
  XMLLIB_ENCODING_UTF8    = 1,         /* UTF-8 text                       */
  XMLLIB_ENCODING_MAX
} xmllib_encoding_e_type;

/*---------------------------------------------------------------------------
  XML string type
---------------------------------------------------------------------------*/
typedef struct xmllib_string_s_type
{
  char   *string;
  uint32  len;
} xmllib_string_s_type;


/*---------------------------------------------------------------------------
  XML meta information structure.  This is filled by the user of the XML
  parser libraries and provides functions to allocate/deallocate memory,
  functions to peek at and get XML text bytes and the XML text itself which
  should only be accessed by the peekbytes and getbytes functions.
---------------------------------------------------------------------------*/
typedef struct xmllib_metainfo_s_type
{
  void                       *xmltext;      /* XML text                    */
  xmllib_memalloc_fptr_type   memalloc;     /* Memory allocator            */
  xmllib_memfree_fptr_type    memfree;      /* Memory free                 */
  xmllib_peekbytes_fptr_type  peekbytes;    /* Function to peek text with  */
  xmllib_getbytes_fptr_type   getbytes;     /* Function to get text with   */
} xmllib_metainfo_s_type;

/*---------------------------------------------------------------------------
  XML meta information structure.  This is filled by the user of the XML
  generator libraries and provides function to put XML text bytes in the XML
  text.
---------------------------------------------------------------------------*/
typedef struct xmllib_gen_metainfo_s_type
{
  void                       *xmltext;      /* XML text                    */
  xmllib_putbytes_fptr_type   putbytes;     /* Function used to put bytes  */
} xmllib_gen_metainfo_s_type ;

/*===========================================================================
		                PARSE TREE TYPE DECLARATIONS
===========================================================================*/
/*---------------------------------------------------------------------------
  Type of nodes making up the parse tree
---------------------------------------------------------------------------*/
typedef enum xmllib_parsetree_node_e_type
{
  XMLLIB_PARSETREE_NODE_ELEMENT      = 0,
  XMLLIB_PARSETREE_NODE_EMPTYELEMENT = 1,
  XMLLIB_PARSETREE_NODE_CONTENT      = 2,
  XMLLIB_PARSETREE_NODE_PI           = 3,
  XMLLIB_PARSETREE_NODE_COMMENT      = 4,
  XMLLIB_PARSETREE_NODE_XMLDECL      = 5,
  XMLLIB_PARSETREE_NODE_MAX
} xmllib_parsetree_node_e_type;

/*---------------------------------------------------------------------------
  Type of an element in the attribute list associated with an element
---------------------------------------------------------------------------*/
typedef struct xmllib_parsetree_attribute_s_type
{
  xmllib_string_s_type                      name;
  xmllib_string_s_type                      value;
  struct xmllib_parsetree_attribute_s_type *next;
} xmllib_parsetree_attribute_s_type;

/*---------------------------------------------------------------------------
  Type of a processing instruction node (XMLLIB_PARSETREE_NODE_PI)
---------------------------------------------------------------------------*/
typedef struct xmllib_parsetree_pi_s_type
{
  xmllib_string_s_type target;
  xmllib_string_s_type instruction;
} xmllib_parsetree_pi_s_type;

/*---------------------------------------------------------------------------
  Type of an element node (XMLLIB_PARSETREE_NODE_ELEMENT)
---------------------------------------------------------------------------*/
struct xmllib_parsetree_node_s_type;
typedef struct xmllib_parsetree_element_s_type
{
  xmllib_string_s_type                 name;
  xmllib_parsetree_attribute_s_type   *attribute_list;
  struct xmllib_parsetree_node_s_type *child;
} xmllib_parsetree_element_s_type;

/*---------------------------------------------------------------------------
  Type of a node in the parse tree
---------------------------------------------------------------------------*/
typedef struct xmllib_parsetree_node_s_type
{
  xmllib_parsetree_node_e_type         nodetype;
  union 
  {
    xmllib_parsetree_element_s_type element;
    xmllib_string_s_type            content;
    xmllib_parsetree_pi_s_type      pi;
    xmllib_string_s_type            comment;
  } payload;
  struct xmllib_parsetree_node_s_type *sibling;
} xmllib_parsetree_node_s_type;

/*===========================================================================
                           FUNCTION DECLARATIONS
===========================================================================*/
/*=========================================================================
  FUNCTION:  xmllib_common_format_log_msg
===========================================================================*/
/*!
    @brief
    Format debug message for logging.

    @return
    None
*/
/*=========================================================================*/
extern void xmllib_common_format_log_msg
(
  char *buf_ptr,
  int buf_size,
  char *fmt,
  ...
);

#endif  /* FEATURE_XMLLIB */
#endif /* #ifndef _XMLLIB_COMMON_H */
