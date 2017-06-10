/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

       ULP debug logic module

GENERAL DESCRIPTION
  This file contains ULP debug logic to generate and send the required debug
  info.

Copyright (c) 2011-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
=============================================================================*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013 Qualcomm Atheros, Inc.
  All Rights Reserved.
  Qualcomm Atheros Confidential and Proprietary.
=============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/stat.h>
#include <fcntl.h>
#include <linux/types.h>
#include <errno.h>
#include <sys/time.h>

// Internal include files
#include <ulp_engine.h>

#include "ulp_quipc.h"

#include "loc_cfg.h"

#include "ulp_data.h"
#include "ulp_internal.h"

#ifdef FEATURE_QUIPC_DEBUG_INFO

#define LOG_NDDEBUG 0
#define LOG_TAG "LocSvc_libulp"

// DEBUG Command strings
// Restart the location engine. Not implemented yet.
#define ULP_DEBUG_INFO_STR_COLD_START "quips_cold_start"
// Enable debug info for AP measurement
#define ULP_DEBUG_INFO_STR_AP_MEASUREMENT "quips_enable_debug_out:Measurement"
// Enable debug info for Engine modes
#define ULP_DEBUG_INFO_STR_ENGINE_MODE "quips_enable_debug_out:Modes"
// Enable debug info for engine modes and measurements both
#define ULP_DEBUG_INFO_STR_ALL "quips_enable_debug_out:ALL"
// Disable debug info
#define ULP_DEBUG_INFO_STR_DISABLE "quips_disable_debug_out"
// LCI selection mode
#define ULP_PIP_USER_LCI_DETERMINATION_MODE "pip_lci_determination_mode"

// DEBUG Info response strings
#define ULP_DEBUG_INFO_STR_QUIPS_STATE "QUIPS_STATE"
#define ULP_DEBUG_INFO_STR_GNSS_STATE "GNSS_STATE"
#define ULP_DEBUG_INFO_STR_QUIPC_MODE "quipc_mode"
#define ULP_DEBUG_INFO_STR_QUIPC_ENGINE_DATA "quipc_engine_data"

static quipc_string_s_type* ulp_debug_get_debug_info_state_json ();
static quipc_string_s_type* ulp_debug_bundle_debug_info (
    quipc_string_s_type* json_states, quipc_string_s_type* json_quipc_engine);

/*===========================================================================
FUNCTION    ulp_debug_get_debug_info_state_json

DESCRIPTION
   This function is invoked to get the debug info for the quips and gnss states in json
   format.

NOTE
   IT IS THE RESPONSIBILITY OF THE CALLER TO DELETE THE MEMORY ON HEAP ALLOCATED
   BY THIS FUNCTION

DEPENDENCIES
   None

RETURN VALUE
   json: Json encoded string / NULL

SIDE EFFECTS
   N/A
===========================================================================*/
static quipc_string_s_type* ulp_debug_get_debug_info_state_json ()
{
  char buf [6];
  quipc_string_s_type* json = NULL;
  quipc_string_s_type* s1 = NULL;
  quipc_string_s_type* s2 = NULL;
  quipc_string_s_type* s3 = NULL;
  quipc_string_s_type* s4 = NULL;
  int mem_failure = 1;
  int     temp_size = 0;

  ENTRY_LOG_CALLFLOW();

  do
  {
    ////////////////
    // QUIPS States
    ////////////////
    int quipc_state = ulp_data.quipc_provider_info.state;
    snprintf(buf, sizeof(buf), "%d", quipc_state);

    // Key
    temp_size = sizeof (quipc_string_s_type) + strlen (ULP_DEBUG_INFO_STR_QUIPS_STATE) + 1;
    s1 = (quipc_string_s_type*) QUIPC_MALLOC (temp_size);
    if (s1 == NULL)
    {
      // Can not continue
      break;
    }
    strlcpy (s1->arr, ULP_DEBUG_INFO_STR_QUIPS_STATE, temp_size);
    LOC_LOGI ("%s, ULP_DEBUG_INFO_ENGINE_MODE s1 = %s, length = %d \n", __func__, s1->arr, temp_size);

    // Value
    temp_size = sizeof (quipc_string_s_type) + strlen (buf) + 1;
    s2 = (quipc_string_s_type*) QUIPC_MALLOC (temp_size);
    if (s2 == NULL)
    {
      // Can not continue
      break;
    }
    strlcpy (s2->arr, buf, temp_size);
    LOC_LOGI ("%s, ULP_DEBUG_INFO_ENGINE_MODE s2 = %s, length = %d \n", __func__, s2->arr, temp_size);

    ////////////////
    // GNSS States
    ////////////////
    int gnss_state = ulp_data.gnss_provider_info.state;
    snprintf(buf, sizeof(buf), "%d", gnss_state);

    // Key
    temp_size = sizeof (quipc_string_s_type) + strlen (ULP_DEBUG_INFO_STR_GNSS_STATE) + 1;
    s3 = (quipc_string_s_type*) QUIPC_MALLOC (temp_size);
    if (s3 == NULL)
    {
      // Can not continue
      break;
    }
    strlcpy (s3->arr, ULP_DEBUG_INFO_STR_GNSS_STATE, temp_size);
    LOC_LOGI ("%s, ULP_DEBUG_INFO_ENGINE_MODE s3 = %s, length = %d \n", __func__, s3->arr, temp_size);

    // Value
    temp_size = sizeof (quipc_string_s_type) + strlen (buf) + 1;
    s4 = (quipc_string_s_type*) QUIPC_MALLOC (temp_size);
    if (s4 == NULL)
    {
      // Can not continue
      break;
    }
    strlcpy (s4->arr, buf, temp_size);
    LOC_LOGI ("%s, ULP_DEBUG_INFO_ENGINE_MODE s4 = %s, length = %d \n", __func__, s4->arr, temp_size);

    // Start JSON encoding
    quipc_string_string_s_type temp;
    temp.num_of_allocations = 2;

    temp.key = (quipc_string_s_type**) QUIPC_MALLOC(temp.num_of_allocations*sizeof(quipc_string_s_type*));
    if (temp.key == NULL)
    {
      // Can not continue
      break;
    }
    temp.value = (quipc_string_s_type**) QUIPC_MALLOC(temp.num_of_allocations*sizeof(quipc_string_s_type*));
    if (temp.value == NULL)
    {
      // Can not continue
      QUIPC_FREE (temp.key);
      break;
    }

    // We can ontinue beyond this even with the mem allocation failure
    mem_failure = 0;

    // Initialize the strings
    temp.key [0] = s1; // QUIPS_STATE - KEY
    temp.key [1] = s3; // GNSS_STATE - KEY
    temp.value [0] = s2; // QUIPS_STATE - VALUE
    temp.value [1] = s4; // GNSS_STATE - VALUE

    json = quipc_convert_to_json_string (&temp);

    if (json != NULL)
    {
      LOC_LOGI ("%s, json = %s, length = %d \n", __func__, json->arr, strlen(json->arr));
    }

    // Free the allocated memory
    for (int ii = 0; ii < temp.num_of_allocations; ii++)
    {
      QUIPC_FREE (temp.key[ii]);
      QUIPC_FREE (temp.value[ii]);
    }

    QUIPC_FREE (temp.key);
    QUIPC_FREE (temp.value);

  } while (0);

  // Free the allocated memory
  if (mem_failure == 1)
  {
    // Free the allocated memory
    QUIPC_FREE (s1);
    QUIPC_FREE (s2);
    QUIPC_FREE (s3);
    QUIPC_FREE (s4);
  }

  EXIT_LOG(%s, json);
  return json;
}

/*===========================================================================
FUNCTION    ulp_debug_bundle_debug_info

DESCRIPTION
   This function is invoked to bundle the debug info for the quips and gnss states
   and QUIPC engine data (including scan results) in json format.

NOTE
   IT IS THE RESPONSIBILITY OF THE CALLER TO DELETE THE MEMORY ON HEAP ALLOCATED
   BY THIS FUNCTION

DEPENDENCIES
   None

RETURN VALUE
   json: Json encoded string / NULL i.e.

SIDE EFFECTS
   N/A
===========================================================================*/
static quipc_string_s_type* ulp_debug_bundle_debug_info (
    quipc_string_s_type* json_states, quipc_string_s_type* json_quipc_engine)
{
  quipc_string_s_type* json = NULL;
  quipc_string_s_type* s1 = NULL;
  quipc_string_s_type* s2 = NULL;
  int mem_failure = 1;
  int     temp_size = 0;

  ENTRY_LOG_CALLFLOW();

  do
  {
    quipc_string_string_s_type temp;
    temp.num_of_allocations = 0;

    ////////////////
    // States
    ////////////////
    if (json_states != NULL)
    {
      // We have the value (json_states), just create the Key
      temp_size = sizeof (quipc_string_s_type) + strlen (ULP_DEBUG_INFO_STR_QUIPC_MODE) + 1;
      s1 = (quipc_string_s_type*) QUIPC_MALLOC (temp_size);
      if (s1 == NULL)
      {
        // Can not continue
        break;
      }
      strlcpy (s1->arr, ULP_DEBUG_INFO_STR_QUIPC_MODE, temp_size);
      LOC_LOGI ("%s, ULP_DEBUG_INFO_ENGINE_MODE s1 = %s, length = %d \n", __func__, s1->arr, temp_size);
      ++temp.num_of_allocations;
    }

    ////////////////////////////////
    // QUIPC ENGINE DATA results ///
    ////////////////////////////////
    if (json_quipc_engine != NULL)
    {
      // We have the value (json_quipc_engine), just create the Key
      temp_size = sizeof (quipc_string_s_type) + strlen (ULP_DEBUG_INFO_STR_QUIPC_ENGINE_DATA) + 1;
      s2 = (quipc_string_s_type*) QUIPC_MALLOC (temp_size);
      if (s2 == NULL)
      {
        // Can not continue
        break;
      }
      strlcpy (s2->arr, ULP_DEBUG_INFO_STR_QUIPC_ENGINE_DATA, temp_size);
      LOC_LOGI ("%s, ULP_DEBUG_INFO_ENGINE_MODE s2 = %s, length = %d \n", __func__, s2->arr, temp_size);
      ++temp.num_of_allocations;
    }

    if (temp.num_of_allocations == 0)
    {
      break;
    }

    // Start JSON encoding
    temp.key = (quipc_string_s_type**) QUIPC_MALLOC(temp.num_of_allocations*sizeof(quipc_string_s_type*));
    if (temp.key == NULL)
    {
      // Can not continue
      break;
    }
    temp.value = (quipc_string_s_type**) QUIPC_MALLOC(temp.num_of_allocations*sizeof(quipc_string_s_type*));
    if (temp.value == NULL)
    {
      // Can not continue
      QUIPC_FREE (temp.key);
      break;
    }

    // We can continue beyond this even with the mem allocation failure
    mem_failure = 0;

    // Initialize the strings
    temp.key [0] = s1; // STATE - KEY
    temp.value [0] = json_states; // STATE - VALUE
    if (json_states == NULL)
    {
      temp.key [0] = s2; // QUIPC engine data - KEY
      temp.value [0] = json_quipc_engine; // QUIPC engine data - VALUE
    }
    else
    {
      temp.key [1] = s2; // QUIPC engine data - KEY
      temp.value [1] = json_quipc_engine; // QUIPC engine data - VALUE
    }

    json = quipc_convert_to_json_string (&temp);

    if (json != NULL)
    {
      LOC_LOGI ("%s, json = %s, length = %d \n", __func__, json->arr, strlen(json->arr));
    }

    // Free the allocated memory
    for (int ii = 0; ii < temp.num_of_allocations; ii++)
    {
      QUIPC_FREE (temp.key[ii]);
      // QUIPC_FREE (temp.value[ii]); // Let's not free the values as we have not allocated them
    }

    QUIPC_FREE (temp.key);
    QUIPC_FREE (temp.value);

  } while (0);

  // Free the allocated memory
  if (mem_failure == 1)
  {
    // Free the allocated memory
    QUIPC_FREE (s1);
    QUIPC_FREE (s2);
  }

  EXIT_LOG(%s, json);
  return json;
}
/*===========================================================================
FUNCTION    ulp_debug_get_debug_info

DESCRIPTION
   This function is invoked to check if debug info is to be provided and prepare
   debug info.

   This function checks the type of debug info required and does the memory management
   Also allocates the memory on the heap.

NOTE
   IT IS THE RESPONSIBILITY OF THE CALLER TO DELETE THE MEMORY ON HEAP ALLOCATED
   BY THIS FUNCTION


DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A
===========================================================================*/
int ulp_debug_get_debug_info (void** debug_info, int* debug_info_size)
{
  int     retVal = -1;
  int     debug_info_size_total = 0;
  char*   debug_info_total = NULL;
  int     temp_size = 0;
  quipc_string_s_type* json_quipc_engine = NULL;
  quipc_string_s_type* json = NULL;

  ENTRY_LOG_CALLFLOW();

  // Parameters passed might contain the memory allocated to hold the debug
  // info from Quipc
  char*   quipc_debug_info = (char*) (*debug_info);
  int     quipc_debug_info_size = (*debug_info_size);

  LOC_LOGI ("%s, debug_info = %s, length = %d, Debug TYPE = %d\n", __func__,
      quipc_debug_info, quipc_debug_info_size, ulp_data.ulp_debug_info_type);

  // Change the char* to quipc_string_s_type*
  if (quipc_debug_info != NULL && quipc_debug_info_size != 0)
  {
    int size = sizeof (quipc_string_s_type) + quipc_debug_info_size*sizeof(char) + 1;
    json_quipc_engine = (quipc_string_s_type*) QUIPC_MALLOC (size);
    strlcpy(json_quipc_engine->arr, quipc_debug_info, size);

    // Free the memory pointed to by rawData
    delete quipc_debug_info;
  }

  // Reset the rawData and rawDataSize as the quipc related memory is now
  // referenced by another pointer.
  (*debug_info) = NULL;
  (*debug_info_size) = 0;

  switch (ulp_data.ulp_debug_info_type)
  {
  case ULP_DEBUG_INFO_ENGINE_MODE:
  case ULP_DEBUG_INFO_ALL:
  {
    LOC_LOGI ("%s, ULP_DEBUG_INFO_ALL\n", __func__);

    // Get the JSON string for the states debug info
    quipc_string_s_type* json_states = ulp_debug_get_debug_info_state_json ();

    // quipc_info contains the JSON string for the measurements info

    // Bundle the states & QUIPC engine data into the final json string
    json = ulp_debug_bundle_debug_info (json_states, json_quipc_engine);
    QUIPC_FREE (json_states);
    break;
  }
  case ULP_DEBUG_INFO_MEASUREMENT:
  case ULP_DEBUG_INFO_NONE:
  default:
    LOC_LOGI ("%s, ULP_DEBUG_INFO_MEASUREMENT\n", __func__);
    // Bundle the states & QUIPC engine data into the final json string
    json = ulp_debug_bundle_debug_info (NULL, json_quipc_engine);
    break;
  }

  // Allocate memory
  if (json != NULL)
  {
    LOC_LOGI ("%s, Final JSON is = %s", __func__, json->arr);
    int size = strlen(json->arr)+1;
    debug_info_total = new char [size]();

    if (debug_info_total == NULL)
    {
      // Failed to create memory
      (*debug_info) = NULL;
      (*debug_info_size) = 0;
    }
    else
    {
      strlcpy (debug_info_total, json->arr, size);
      (*debug_info) = (void*)debug_info_total;
      (*debug_info_size) = size - 1; // Actual size
      retVal = 0;
    }
    QUIPC_FREE (json);
  }

  if (json_quipc_engine != NULL)
  {
    QUIPC_FREE (json_quipc_engine);
  }

  EXIT_LOG(%d, retVal);
  return retVal;
}

/*===========================================================================
FUNCTION    ulp_debug_process_raw_command

DESCRIPTION
   This function is invoked when we need to provide / stop the debug info along with
   the Location Fix.

   Can be any of the following commands
   ULP_DEBUG_INFO_STR_COLD_START - Restart the location engine. Not implemented yet.
   ULP_DEBUG_INFO_STR_AP_MEASUREMENT - Enable debug info for AP measurement
   ULP_DEBUG_INFO_STR_ENGINE_MODE - Enable debug info for Engine modes
   ULP_DEBUG_INFO_STR_ALL - Enable debug info for engine modes and measurements both
   ULP_DEBUG_INFO_STR_DISABLE - Disable debug info

   This function finds out the following pieces of info:
   Type of command.
       Info will be stored at: ulp_data.ulp_debug_info_type

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A
===========================================================================*/
int ulp_debug_process_raw_command (const char* rawCmd, int rawCmdLength)
{
   int  ret_val = 0;
   int  i;
   int  pip_cmd_length = 0;

   ENTRY_LOG_CALLFLOW();
   LOC_LOGD ("%s: ulp_started = %d, length = %d\n",
         __func__,
         ulp_data.ulp_started,
         rawCmd, rawCmdLength);

   if (strncmp (rawCmd, ULP_DEBUG_INFO_STR_AP_MEASUREMENT, rawCmdLength) == 0)
   {
     ulp_data.ulp_debug_info_type = ULP_DEBUG_INFO_MEASUREMENT;
     // Send debug info request to QUIPC
     ulp_quipc_request_debug_info (ULP_QUIPC_DEBUG_INFO_AP_MEAS);
   }
   else if (strncmp (rawCmd, ULP_DEBUG_INFO_STR_ENGINE_MODE, rawCmdLength) == 0)
   {
     ulp_data.ulp_debug_info_type = ULP_DEBUG_INFO_ENGINE_MODE;
     // Send debug info request to QUIPC
     ulp_quipc_request_debug_info (ULP_QUIPC_DEBUG_INFO_PF_STATE);
   }
   else if (strncmp (rawCmd, ULP_DEBUG_INFO_STR_ALL, rawCmdLength) == 0)
   {
     ulp_data.ulp_debug_info_type = ULP_DEBUG_INFO_ALL;
     // Send debug info request to QUIPC
     ulp_quipc_request_debug_info (ULP_QUIPC_DEBUG_INFO_ALL);
   }
   else if (strncmp (rawCmd, ULP_DEBUG_INFO_STR_DISABLE, rawCmdLength) == 0)
   {
     ulp_data.ulp_debug_info_type = ULP_DEBUG_INFO_NONE;
     // Send stop debug info request to QUIPC
     ulp_quipc_request_debug_info (ULP_QUIPC_DEBUG_INFO_STOP);
   }
   else if (strncmp (rawCmd, ULP_PIP_USER_LCI_DETERMINATION_MODE,
                     (pip_cmd_length = strlen (ULP_PIP_USER_LCI_DETERMINATION_MODE))) == 0)
   {
      if (rawCmdLength > pip_cmd_length)
      {
         if ((*(rawCmd + pip_cmd_length) == '1') &&
             (rawCmdLength == (pip_cmd_length + 1 + LCI_ID_STRING_LENGTH)))
         {
            ulp_quipc_set_lci_selection_mode (1, (unsigned char*) rawCmd + pip_cmd_length + 1);
         }
         else
         {
            ulp_quipc_set_lci_selection_mode (0, NULL);
         }
      }
   }
   else
   {
     //TODO: Handle the additional command here
     ret_val = -1;
   }

   LOC_LOGD ("%s: Debug Cmd Type = %d,\n",
         __func__,
         ulp_data.ulp_debug_info_type);

   EXIT_LOG(%d, ret_val);

   return ret_val;
}
#else
// FEATURE_QUIPC_DEBUG_INFO not supported.
// Stubbed functions for compilation
int ulp_debug_get_debug_info (void** debug_info, int* debug_info_size)
{
  return 0;
}
int ulp_debug_process_raw_command (const char* rawCmd, int rawCmdLength)
{
  return 0;
}
#endif
