#ifndef SNS_DEBUG_STR_MDM_H
#define SNS_DEBUG_STR_MDM_H

/*============================================================================
  @file
  sns_debug_str_mdm.h

  @brief
  Contains macros and definitions required for printing
  debug strings.

  Copyright (c) 2010-2011 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/

/*=====================================================================
  INCLUDE FILES
  =======================================================================*/
#include "sensor1.h"
#include "string.h"
#include "msg.h"
#include "msgcfg.h"
#include "sns_debug_interface_v01.h"
#include "sns_debug_api.h"
#include <stddef.h>

/* Debug string priority defines */
#define SNS_MSG_LOW    0
#define SNS_MSG_MEDIUM 1
#define SNS_MSG_HIGH   2
#define SNS_MSG_FATAL  3
#define SNS_MSG_ERROR  4

/**  Debug string parameters structure
     Contains format specifiers and details on file and line number */
typedef struct 
{
  char filename[SNS_DEBUG_MAX_FILENAME_SIZE_V01];
  uint16_t line_num;
  uint8_t num_params_valid;
  int32_t param1; 
  int32_t param2;
  int32_t param3;
} debug_params_s;



/*===========================================================================

  FUNCTION:   sns_debug_printf_string

  ===========================================================================*/
/*!
  @brief
  Prints out the debug string.
   
  @param[i] module_id         : Module id assigned by Sensor Message Router
  @param[i] priority          : Priority of the message string 
  @param[i] debug_str         : Debug string
  @param[i] fmt_params_ptr*   : Pointer to format parameter structure

  @return
  No return value.
*/
/*=========================================================================*/
void sns_debug_printf_string(sns_debug_module_id_e module_id,
                             uint8_t priority,
                             const char *debug_str,
                             const debug_params_s *fmt_params_ptr); 
/*============================================================================

  FUNCTION:   sns_debug_printf_string_id

=============================================================================*/
/*===========================================================================*/
/*!
  @brief
  Prints out the debug string based on the string identifier.
   
  @param[i] module_id         : Module id assigned by Sensor Message Router
  @param[i] priority          : Priority of the message string 
  @param[i] debug_str_id      : Debug string identifier
  @param[i] fmt_params_ptr*   : Pointer to format parameter structure

  @return
  No return value.
*/
/*=========================================================================*/
void sns_debug_printf_string_id(sns_debug_module_id_e module_id,
                                uint8_t priority,
                                uint16_t string_id_param,
                                const debug_params_s *fmt_params_ptr);

/*===========================================================================

  FUNCTION:   sns_debug_set_dbg_str_mask

===========================================================================*/
/*===========================================================================*/
/*!
  @brief
  Sets the debug string mask. Essentially a copy of the mask bits to a global
  variable visible to only this file is made.
   
  @param[i] bit_mask         : Debug string bit mask

  @return
  No return value.
*/
/*=========================================================================*/
void sns_debug_set_dbg_str_mask(uint64_t bit_mask);

/*===========================================================================

  FUNCTION:   sns_debug_is_module_disabled

===========================================================================*/
/*===========================================================================*/
/*!
  @brief
  Returns 1 if the the module's debug strings need to be filtered.
   
  @param[i] module_id        : Debug module id

  @return
  uint8_t : 1 - Module's messages should be filtered
            0 - Module's messages should NOT be filtered
*/
/*=========================================================================*/
uint8_t sns_debug_is_module_disabled(sns_debug_module_id_e module_id);



/* These macros have to call the DIAG Macros as the macros take the format string as a constant
 * If we did not pass a const string literal we get compilation errors
 * If we malloced and passed them in the memory would not be freed by diag causing a memory leak
 */
//Lows
#define SNS_PRINTF_STRING_LOW_0(module_id,str)                            \
  {                                                                       \
    if (!sns_debug_is_module_disabled(module_id))                         \
    {                                                                     \
      MSG_3(MSG_SSID_SNS,MSG_LEGACY_LOW,str,0,0,0);                       \
    }                                                                     \
  }


#define SNS_PRINTF_STRING_LOW_1(module_id,str,parameter1)                 \
  {                                                                       \
    if (!sns_debug_is_module_disabled(module_id))                         \
    {                                                                     \
      debug_params_s format_params;                                       \
      format_params.param1 = (parameter1);                                \
      MSG_3(MSG_SSID_SNS,MSG_LEGACY_LOW,str,format_params.param1,0,0);              \
    }                                                                     \
  }


#define SNS_PRINTF_STRING_LOW_2(module_id,str,parameter1,parameter2)      \
  {                                                                       \
    if (!sns_debug_is_module_disabled(module_id))                         \
    {                                                                     \
      debug_params_s format_params;                                       \
      format_params.param1 = (parameter1);                                \
      format_params.param2 = (parameter2);                                \
      MSG_3(MSG_SSID_SNS,MSG_LEGACY_LOW,str,format_params.param1,         \
                                            format_params.param2,0);      \
    }                                                                     \
  }


#define SNS_PRINTF_STRING_LOW_3(module_id,str,parameter1,parameter2,parameter3) \
  {                                                                       \
    if (!sns_debug_is_module_disabled(module_id))                         \
    {                                                                     \
      debug_params_s format_params;                                       \
      format_params.param1 = (parameter1);                                \
      format_params.param2 = (parameter2);                                \
      format_params.param3 = (parameter3);                                \
      MSG_3(MSG_SSID_SNS,MSG_LEGACY_LOW,str,format_params.param1,         \
                                            format_params.param2,         \
                                            format_params.param3);        \
    }                                                                     \
  }

// Mediums
#define SNS_PRINTF_STRING_MEDIUM_0(module_id,str)                         \
  {                                                                       \
    if (!sns_debug_is_module_disabled(module_id))                         \
    {                                                                     \
      MSG_3(MSG_SSID_SNS,MSG_LEGACY_MED,str,0,0,0);                       \
    }                                                                     \
  }


#define SNS_PRINTF_STRING_MEDIUM_1(module_id,str,parameter1)              \
  {                                                                       \
    if (!sns_debug_is_module_disabled(module_id))                         \
    {                                                                     \
      debug_params_s format_params;                                       \
      format_params.param1 = (parameter1);                                \
      MSG_3(MSG_SSID_SNS,MSG_LEGACY_MED,str,format_params.param1,0,0);    \
    }                                                                     \
  }


#define SNS_PRINTF_STRING_MEDIUM_2(module_id,str,parameter1,parameter2)   \
  {                                                                       \
    if (!sns_debug_is_module_disabled(module_id))                         \
    {                                                                     \
      debug_params_s format_params;                                       \
      format_params.param1 = (parameter1);                                \
      format_params.param2 = (parameter2);                                \
      MSG_3(MSG_SSID_SNS,MSG_LEGACY_MED,str,format_params.param1,         \
                                            format_params.param2,         \
                                            0);                           \
    }                                                                     \
  }

#define SNS_PRINTF_STRING_MEDIUM_3(module_id,str,parameter1,parameter2,parameter3) \
  {                                                                       \
    if (!sns_debug_is_module_disabled(module_id))                         \
    {                                                                     \
      debug_params_s format_params;                                       \
      format_params.param1 = (parameter1);                                \
      format_params.param2 = (parameter2);                                \
      format_params.param3 = (parameter3);                                \
      MSG_3(MSG_SSID_SNS,MSG_LEGACY_MED,str,format_params.param1,         \
                                            format_params.param2,         \
                                            format_params.param3);        \
    }                                                                     \
  }

// Highs
#define SNS_PRINTF_STRING_HIGH_0(module_id,str)                           \
  {                                                                       \
    if (!sns_debug_is_module_disabled(module_id))                         \
    {                                                                     \
      MSG_3(MSG_SSID_SNS,MSG_LEGACY_HIGH,str,0,0,0);                      \
    }                                                                     \
  }


#define SNS_PRINTF_STRING_HIGH_1(module_id,str,parameter1)                \
  {                                                                       \
    if (!sns_debug_is_module_disabled(module_id))                         \
    {                                                                     \
      debug_params_s format_params;                                       \
      format_params.param1 = (parameter1);                                \
      MSG_3(MSG_SSID_SNS,MSG_LEGACY_HIGH,str,format_params.param1,0,0);   \
    }                                                                     \
  }


#define SNS_PRINTF_STRING_HIGH_2(module_id,str,parameter1,parameter2)     \
  {                                                                       \
    if (!sns_debug_is_module_disabled(module_id))                         \
    {                                                                     \
      debug_params_s format_params;                                       \
      format_params.param1 = (parameter1);                                \
      format_params.param2 = (parameter2);                                \
      MSG_3(MSG_SSID_SNS,MSG_LEGACY_HIGH,str,format_params.param1,         \
                                            format_params.param2,         \
                                            0);                           \
    }                                                                     \
  }

#define SNS_PRINTF_STRING_HIGH_3(module_id,str,parameter1,parameter2,parameter3) \
  {                                                                       \
    if (!sns_debug_is_module_disabled(module_id))                         \
    {                                                                     \
      debug_params_s format_params;                                       \
      format_params.param1 = (parameter1);                                \
      format_params.param2 = (parameter2);                                \
      format_params.param3 = (parameter3);                                \
      MSG_3(MSG_SSID_SNS,MSG_LEGACY_HIGH,str,format_params.param1,        \
                                            format_params.param2,         \
                                            format_params.param3);        \
    }                                                                     \
  }


// Fatals
#define SNS_PRINTF_STRING_FATAL_0(module_id,str)                          \
  {                                                                       \
    if (!sns_debug_is_module_disabled(module_id))                         \
    {                                                                     \
      MSG_3(MSG_SSID_SNS,MSG_LEGACY_FATAL,str,0,0,0);                     \
    }                                                                     \
  }


#define SNS_PRINTF_STRING_FATAL_1(module_id,str,parameter1)               \
  {                                                                       \
    if (!sns_debug_is_module_disabled(module_id))                         \
    {                                                                     \
      debug_params_s format_params;                                       \
      format_params.param1 = (parameter1);                                \
      MSG_3(MSG_SSID_SNS,MSG_LEGACY_FATAL,str,format_params.param1,0,0);  \
    }                                                                     \
  }


#define SNS_PRINTF_STRING_FATAL_2(module_id,str,parameter1,parameter2)    \
  {                                                                       \
    if (!sns_debug_is_module_disabled(module_id))                         \
    {                                                                     \
      debug_params_s format_params;                                       \
      format_params.param1 = (parameter1);                                \
      format_params.param2 = (parameter2);                                \
      MSG_3(MSG_SSID_SNS,MSG_LEGACY_FATAL,str,format_params.param1,       \
                                            format_params.param2,         \
                                            0);                           \
    }                                                                     \
  }

#define SNS_PRINTF_STRING_FATAL_3(module_id,str,parameter1,parameter2,parameter3) \
  {                                                                       \
    if (!sns_debug_is_module_disabled(module_id))                         \
    {                                                                     \
      debug_params_s format_params;                                       \
      format_params.param1 = (parameter1);                                \
      format_params.param2 = (parameter2);                                \
      format_params.param3 = (parameter3);                                \
      MSG_3(MSG_SSID_SNS,MSG_LEGACY_FATAL,str,format_params.param1,       \
                                            format_params.param2,         \
                                            format_params.param3);        \
    }                                                                     \
  }

// Errors
#define SNS_PRINTF_STRING_ERROR_0(module_id,str)                          \
  {                                                                       \
    if (!sns_debug_is_module_disabled(module_id))                         \
    {                                                                     \
      MSG_3(MSG_SSID_SNS,MSG_LEGACY_ERROR,str,0,0,0);                     \
    }                                                                     \
  }


#define SNS_PRINTF_STRING_ERROR_1(module_id,str,parameter1)               \
  {                                                                       \
    if (!sns_debug_is_module_disabled(module_id))                         \
    {                                                                     \
      debug_params_s format_params;                                       \
      format_params.param1 = (parameter1);                                \
      MSG_3(MSG_SSID_SNS,MSG_LEGACY_ERROR,str,format_params.param1,0,0);  \
    }                                                                     \
  }


#define SNS_PRINTF_STRING_ERROR_2(module_id,str,parameter1,parameter2)    \
  {                                                                       \
    if (!sns_debug_is_module_disabled(module_id))                         \
    {                                                                     \
      debug_params_s format_params;                                       \
      format_params.param1 = (parameter1);                                \
      format_params.param2 = (parameter2);                                \
      MSG_3(MSG_SSID_SNS,MSG_LEGACY_ERROR,str,format_params.param1,       \
                                            format_params.param2,         \
                                            0);                           \
    }                                                                     \
  }

#define SNS_PRINTF_STRING_ERROR_3(module_id,str,parameter1,parameter2,parameter3) \
  {                                                                       \
    if (!sns_debug_is_module_disabled(module_id))                         \
    {                                                                     \
      debug_params_s format_params;                                       \
      format_params.param1 = (parameter1);                                \
      format_params.param2 = (parameter2);                                \
      format_params.param3 = (parameter3);                                \
      MSG_3(MSG_SSID_SNS,MSG_LEGACY_ERROR,str,format_params.param1,       \
                                            format_params.param2,         \
                                            format_params.param3);        \
    }                                                                     \
  }

/* The debug printf macros for DSPS and Apps processor where only the string ID is provided */

#define SNS_PRINTF_STRING_ID_LOW_0(module_id,str_id)                                                \
             {                                                                                      \
               if (!sns_debug_is_module_disabled(module_id))                                        \
               {                                                                                    \
                 debug_params_s format_params;                                                      \
                 format_params.line_num = __LINE__;                                                 \
                 strlcpy(format_params.filename,__FILE__,sizeof(format_params.filename));           \
                 format_params.num_params_valid = 0;                                                \
                 format_params.param1 = 0;                                                          \
                 format_params.param2 = 0;                                                          \
                 format_params.param3 = 0;                                                          \
                 sns_debug_printf_string_id(module_id,SNS_MSG_LOW,str_id,&format_params);           \
               }                                                                                    \
             }


#define SNS_PRINTF_STRING_ID_LOW_1(module_id,str_id,parameter1)                                     \
            {                                                                                       \
               if (!sns_debug_is_module_disabled(module_id))                                        \
               {                                                                                    \
                 debug_params_s format_params;                                                      \
                 format_params.line_num = __LINE__;                                                 \
                 strlcpy(format_params.filename,__FILE__,sizeof(format_params.filename));           \
                 format_params.num_params_valid = 1;                                                \
                 format_params.param1 = (parameter1);                                               \
                 format_params.param2 = 0;                                                          \
                 format_params.param3 = 0;                                                          \
                 sns_debug_printf_string_id(module_id,SNS_MSG_LOW,str_id,&format_params);           \
               }                                                                                    \
             }
                       

#define SNS_PRINTF_STRING_ID_LOW_2(module_id,str_id,parameter1,parameter2)                          \
             {                                                                                      \
               if (!sns_debug_is_module_disabled(module_id))                                        \
               {                                                                                    \
                 debug_params_s format_params;                                                      \
                 format_params.line_num = __LINE__;                                                 \
                 strlcpy(format_params.filename,__FILE__,sizeof(format_params.filename));           \
                 format_params.num_params_valid = 2;                                                \
                 format_params.param1 = (parameter1);                                               \
                 format_params.param2 = (parameter2);                                               \
                 format_params.param3 = 0;                                                          \
                 sns_debug_printf_string_id(module_id,SNS_MSG_LOW,str_id,&format_params);           \
               }                                                                                    \
             }


#define SNS_PRINTF_STRING_ID_LOW_3(module_id,str_id,parameter1,parameter2,parameter3)               \
             {                                                                                      \
               if (!sns_debug_is_module_disabled(module_id))                                        \
               {                                                                                    \
                 debug_params_s format_params;                                                      \
                 format_params.line_num = __LINE__;                                                 \
                 strlcpy(format_params.filename,__FILE__,sizeof(format_params.filename));           \
                 format_params.num_params_valid = 3;                                                \
                 format_params.param1 = (parameter1);                                               \
                 format_params.param2 = (parameter2);                                               \
                 format_params.param3 = (parameter3);                                               \
                 sns_debug_printf_string_id(module_id,SNS_MSG_LOW,str_id,&format_params);           \
               }                                                                                    \
             }

// Mediums
#define SNS_PRINTF_STRING_ID_MEDIUM_0(module_id,str_id)                                             \
             {                                                                                      \
               if (!sns_debug_is_module_disabled(module_id))                                        \
               {                                                                                    \
                 debug_params_s format_params;                                                      \
                 format_params.line_num = __LINE__;                                                 \
                 strlcpy(format_params.filename,__FILE__,sizeof(format_params.filename));           \
                 format_params.num_params_valid = 0;                                                \
                 format_params.param1 = 0;                                                          \
                 format_params.param2 = 0;                                                          \
                 format_params.param3 = 0;                                                          \
                 sns_debug_printf_string_id(module_id,SNS_MSG_MEDIUM,str_id,&format_params);        \
               }                                                                                    \
             }


#define SNS_PRINTF_STRING_ID_MEDIUM_1(module_id,str_id,parameter1)                                  \
             {                                                                                      \
               if (!sns_debug_is_module_disabled(module_id))                                        \
               {                                                                                    \
                 debug_params_s format_params;                                                      \
                 format_params.line_num = __LINE__;                                                 \
                 strlcpy(format_params.filename,__FILE__,sizeof(format_params.filename));           \
                 format_params.num_params_valid = 1;                                                \
                 format_params.param1 = (parameter1);                                               \
                 format_params.param2 = 0;                                                          \
                 format_params.param3 = 0;                                                          \
                 sns_debug_printf_string_id(module_id,SNS_MSG_MEDIUM,str_id,&format_params);        \
               }                                                                                    \
             }


#define SNS_PRINTF_STRING_ID_MEDIUM_2(module_id,str_id,parameter1,parameter2)                       \
             {                                                                                      \
               if (!sns_debug_is_module_disabled(module_id))                                        \
               {                                                                                    \
                 debug_params_s format_params;                                                      \
                 format_params.line_num = __LINE__;                                                 \
                 strlcpy(format_params.filename,__FILE__,sizeof(format_params.filename));           \
                 format_params.num_params_valid = 2;                                                \
                 format_params.param1 = (parameter1);                                               \
                 format_params.param2 = (parameter2);                                               \
                 format_params.param3 = 0;                                                          \
                 sns_debug_printf_string_id(module_id,SNS_MSG_MEDIUM,str_id,&format_params);        \
               }                                                                                    \
             }

#define SNS_PRINTF_STRING_ID_MEDIUM_3(module_id,str_id,parameter1,parameter2,parameter3)            \
             {                                                                                      \
               if (!sns_debug_is_module_disabled(module_id))                                        \
               {                                                                                    \
                 debug_params_s format_params;                                                      \
                 format_params.line_num = __LINE__;                                                 \
                 strlcpy(format_params.filename,__FILE__,sizeof(format_params.filename));           \
                 format_params.num_params_valid = 3;                                                \
                 format_params.param1 = (parameter1);                                               \
                 format_params.param2 = (parameter2);                                               \
                 format_params.param3 = (parameter3);                                               \
                 sns_debug_printf_string_id(module_id,SNS_MSG_MEDIUM,str_id,&format_params);        \
               }                                                                                    \
             }

// Highs
#define SNS_PRINTF_STRING_ID_HIGH_0(module_id,str_id)                                               \
             {                                                                                      \
               if (!sns_debug_is_module_disabled(module_id))                                        \
               {                                                                                    \
                 debug_params_s format_params;                                                      \
                 format_params.line_num = __LINE__;                                                 \
                 strlcpy(format_params.filename,__FILE__,sizeof(format_params.filename));           \
                 format_params.num_params_valid = 0;                                                \
                 format_params.param1 = 0;                                                          \
                 format_params.param2 = 0;                                                          \
                 format_params.param3 = 0;                                                          \
                 sns_debug_printf_string_id(module_id,SNS_MSG_HIGH,str_id,&format_params);          \
               }                                                                                    \
             }


#define SNS_PRINTF_STRING_ID_HIGH_1(module_id,str_id,parameter1)                                    \
             {                                                                                      \
               if (!sns_debug_is_module_disabled(module_id))                                        \
               {                                                                                    \
                 debug_params_s format_params;                                                      \
                 format_params.line_num = __LINE__;                                                 \
                 strlcpy(format_params.filename,__FILE__,sizeof(format_params.filename));           \
                 format_params.num_params_valid = 1;                                                \
                 format_params.param1 = (parameter1);                                               \
                 format_params.param2 = 0;                                                          \
                 format_params.param3 = 0;                                                          \
                 sns_debug_printf_string_id(module_id,SNS_MSG_HIGH,str_id,&format_params);          \
               }                                                                                    \
             }


#define SNS_PRINTF_STRING_ID_HIGH_2(module_id,str_id,parameter1,parameter2)                         \
             {                                                                                      \
               if (!sns_debug_is_module_disabled(module_id))                                        \
               {                                                                                    \
                 debug_params_s format_params;                                                      \
                 format_params.line_num = __LINE__;                                                 \
                 strlcpy(format_params.filename,__FILE__,sizeof(format_params.filename));           \
                 format_params.num_params_valid = 2;                                                \
                 format_params.param1 = (parameter1);                                               \
                 format_params.param2 = (parameter2);                                               \
                 format_params.param3 = 0;                                                          \
                 sns_debug_printf_string_id(module_id,SNS_MSG_HIGH,str_id,&format_params);          \
               }                                                                                    \
             }

#define SNS_PRINTF_STRING_ID_HIGH_3(module_id,str_id,parameter1,parameter2,parameter3)              \
             {                                                                                      \
               if (!sns_debug_is_module_disabled(module_id))                                        \
               {                                                                                    \
                 debug_params_s format_params;                                                      \
                 format_params.line_num = __LINE__;                                                 \
                 strlcpy(format_params.filename,__FILE__,sizeof(format_params.filename));           \
                 format_params.num_params_valid = 3;                                                \
                 format_params.param1 = (parameter1);                                               \
                 format_params.param2 = (parameter2);                                               \
                 format_params.param3 = (parameter3);                                               \
                 sns_debug_printf_string_id(module_id,SNS_MSG_HIGH,str_id,&format_params);          \
               }                                                                                    \
             }


// Fatals
#define SNS_PRINTF_STRING_ID_FATAL_0(module_id,str_id)                                              \
             {                                                                                      \
               if (!sns_debug_is_module_disabled(module_id))                                        \
               {                                                                                    \
                 debug_params_s format_params;                                                      \
                 format_params.line_num = __LINE__;                                                 \
                 strlcpy(format_params.filename,__FILE__,sizeof(format_params.filename));           \
                 format_params.num_params_valid = 0;                                                \
                 format_params.param1 = 0;                                                          \
                 format_params.param2 = 0;                                                          \
                 format_params.param3 = 0;                                                          \
                 sns_debug_printf_string_id(module_id,SNS_MSG_FATAL,str_id,&format_params);         \
               }                                                                                    \
             }


#define SNS_PRINTF_STRING_ID_FATAL_1(module_id,str_id,parameter1)                                   \
             {                                                                                      \
               if (!sns_debug_is_module_disabled(module_id))                                        \
               {                                                                                    \
                 debug_params_s format_params;                                                      \
                 format_params.line_num = __LINE__;                                                 \
                 strlcpy(format_params.filename,__FILE__,sizeof(format_params.filename));           \
                 format_params.num_params_valid = 1;                                                \
                 format_params.param1 = (parameter1);                                               \
                 format_params.param2 = 0;                                                          \
                 format_params.param3 = 0;                                                          \
                 sns_debug_printf_string_id(module_id,SNS_MSG_FATAL,str_id,&format_params);         \
               }                                                                                    \
             }


#define SNS_PRINTF_STRING_ID_FATAL_2(module_id,str_id,parameter1,parameter2)                        \
             {                                                                                      \
                if (!sns_debug_is_module_disabled(module_id))                                       \
               {                                                                                    \
                 debug_params_s format_params;                                                      \
                 format_params.line_num = __LINE__;                                                 \
                 strlcpy(format_params.filename,__FILE__,sizeof(format_params.filename));           \
                 format_params.num_params_valid = 2;                                                \
                 format_params.param1 = (parameter1);                                               \
                 format_params.param2 = (parameter2);                                               \
                 format_params.param3 = 0;                                                          \
                 sns_debug_printf_string_id(module_id,SNS_MSG_FATAL,str_id,&format_params);         \
               }                                                                                    \
             }

#define SNS_PRINTF_STRING_ID_FATAL_3(module_id,str_id,parameter1,parameter2,parameter3)             \
             {                                                                                      \
               if (!sns_debug_is_module_disabled(module_id))                                        \
               {                                                                                    \
                 debug_params_s format_params;                                                      \
                 format_params.line_num = __LINE__;                                                 \
                 strlcpy(format_params.filename,__FILE__,sizeof(format_params.filename));           \
                 format_params.num_params_valid = 3;                                                \
                 format_params.param1 = (parameter1);                                               \
                 format_params.param2 = (parameter2);                                               \
                 format_params.param3 = (parameter3);                                               \
                 sns_debug_printf_string_id(module_id,SNS_MSG_FATAL,str_id,&format_params);         \
               }                                                                                    \
             }

// Errors
#define SNS_PRINTF_STRING_ID_ERROR_0(module_id,str_id)                                              \
             {                                                                                      \
               if (!sns_debug_is_module_disabled(module_id))                                        \
               {                                                                                    \
                 debug_params_s format_params;                                                      \
                 format_params.line_num = __LINE__;                                                 \
                 strlcpy(format_params.filename,__FILE__,sizeof(format_params.filename));           \
                 format_params.num_params_valid = 0;                                                \
                 format_params.param1 = 0;                                                          \
                 format_params.param2 = 0;                                                          \
                 format_params.param3 = 0;                                                          \
                 sns_debug_printf_string_id(module_id,SNS_MSG_ERROR,str_id,&format_params);         \
               }                                                                                    \
             }


#define SNS_PRINTF_STRING_ID_ERROR_1(module_id,str_id,parameter1)                                   \
             {                                                                                      \
               if (!sns_debug_is_module_disabled(module_id))                                        \
               {                                                                                    \
                 debug_params_s format_params;                                                      \
                 format_params.line_num = __LINE__;                                                 \
                 strlcpy(format_params.filename,__FILE__,sizeof(format_params.filename));           \
                 format_params.num_params_valid = 1;                                                \
                 format_params.param1 = (parameter1);                                               \
                 format_params.param2 = 0;                                                          \
                 format_params.param3 = 0;                                                          \
                 sns_debug_printf_string_id(module_id,SNS_MSG_ERROR,str_id,&format_params);         \
               }                                                                                    \
             }


#define SNS_PRINTF_STRING_ID_ERROR_2(module_id,str_id,parameter1,parameter2)                        \
             {                                                                                      \
               if (!sns_debug_is_module_disabled(module_id))                                        \
               {                                                                                    \
                 debug_params_s format_params;                                                      \
                 format_params.line_num = __LINE__;                                                 \
                 strlcpy(format_params.filename,__FILE__,sizeof(format_params.filename));           \
                 format_params.num_params_valid = 2;                                                \
                 format_params.param1 = (parameter1);                                               \
                 format_params.param2 = (parameter2);                                               \
                 format_params.param3 = 0;                                                          \
                 sns_debug_printf_string_id(module_id,SNS_MSG_ERROR,str_id,&format_params);         \
               }                                                                                    \
             }

#define SNS_PRINTF_STRING_ID_ERROR_3(module_id,str_id,parameter1,parameter2,parameter3)             \
             {                                                                                      \
               if (!sns_debug_is_module_disabled(module_id))                                        \
               {                                                                                    \
                 debug_params_s format_params;                                                      \
                 format_params.line_num = __LINE__;                                                 \
                 strlcpy(format_params.filename,__FILE__,sizeof(format_params.filename));           \
                 format_params.num_params_valid = 3;                                                \
                 format_params.param1 = (parameter1);                                               \
                 format_params.param2 = (parameter2);                                               \
                 format_params.param3 = (parameter3);                                               \
                 sns_debug_printf_string_id(module_id,SNS_MSG_ERROR,str_id,&format_params);         \
               }                                                                                    \
             }
#endif /* SNS_DEBUG_STR_H */
