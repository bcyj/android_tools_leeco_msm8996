/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

       ULP XTWIFI logic module

GENERAL DESCRIPTION
  This file contains the module that ULP interacts with XTWIFI.

 Copyright (c) 2012-2013 Qualcomm Atheros, Inc.
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
#include <dlfcn.h>

// Internal include files
#include <ulp_engine.h>
#include "loc_cfg.h"

#include "ulp_quipc.h"
#include "ulp_data.h"
#include "ulp_internal.h"

typedef int (ulp_xtwifi_send_pos_func)
(
   const char*                provider_name,
   const UlpLocation*         location_ptr,
   const GpsLocationExtended* gpsLocationExtendedPtr
);

// Only attempt to load the library and function once
static bool ulp_xtwifi_init = false;
static void*   xtwifi_adaptor_handle = NULL;
static ulp_xtwifi_send_pos_func* xtwifi_send_pos_func = NULL;

/*=============================================================================================
 * Function: ulp_xtwifi_send_position
 *
 * Description
 *   This function initializes sends GNSS position to ULP module.
 *   position report.
 *
 * Parameters:
 *   arg: None
 *
 * Return value:
 *   Error code
 =============================================================================================*/
int ulp_xtwifi_send_position (const char*                provider_name,
                              enum loc_sess_status       status,
                              LocPosTechMask             tech_mask,
                              const UlpLocation*         locationPtr,
                              const GpsLocationExtended* gpsLocationExtendedPtr)
{
   const char *error = NULL;
   int ret_val = 0;

   ENTRY_LOG_CALLFLOW();
   do
   {
      if ((ulp_data.gtp_wifi_enabled == false) &&
          (ulp_data.gtp_ap_cell_enabled == false))
      {
         break;
      }

      // Qualcomm enhanced service is disabled, no need to forward position to GTP AP CS module
      if ((ulp_data.phoneSetting.context_type & ULP_PHONE_CONTEXT_ENH_LOCATION_SERVICES_SETTING) &&
          (ulp_data.phoneSetting.is_enh_location_services_enabled == false))
      {
         break;
      }

      // XT-WIFI only uses successful GNSS fixes,
      // ignore intermediate and failed GNSS fixes
      if (status != LOC_SESS_SUCCESS)
      {
        break;
      }

      // For GPS fixes, GTP CS only uses GPS or sensor assisted
      if (strncmp (provider_name, GPS_PROVIDER_NAME, sizeof (GPS_PROVIDER_NAME)) == 0)
      {
        if ((tech_mask & LOC_POS_TECH_MASK_SATELLITE) == 0 &&
            (tech_mask & LOC_POS_TECH_MASK_SENSORS) == 0)
        {
           break;
        }
      }

      if (ulp_xtwifi_init == false)
      {
         // If the .so or function can not be found, we will deem this as GTP AP CS
         // module not present.
         // No attempt will be made again to locate the library and the function
         ulp_xtwifi_init = true;

         xtwifi_adaptor_handle = dlopen ("libxtwifi_ulp_adaptor.so", RTLD_NOW);
         if (xtwifi_adaptor_handle == NULL)
         {
            if ((error = dlerror()) != NULL)
            {
               LOC_LOGE ("%s, dlopen for libxtwifi_ulp_adaptor.so failed, error = %s\n",
                         __func__, error);
            }
            break;
         }

         dlerror();    /* Clear any existing error */
         xtwifi_send_pos_func = (ulp_xtwifi_send_pos_func*) dlsym(xtwifi_adaptor_handle,
                                                                  "xtwifi_ulp_send_position");

         if (xtwifi_send_pos_func == NULL)
         {
            LOC_LOGE ("%s, dlsym for ulp_xtwifi_send_position failed, error = %s\n",
                      __func__, error);
            ret_val = -1;
         }
      }

      // Come here for both first time adaptor loading or subsequent calls
      if (xtwifi_send_pos_func == NULL)
      {
         // If we can not find the function to send callback, break out
         break;
      }

      ret_val = xtwifi_send_pos_func (provider_name,
                                      locationPtr,
                                      gpsLocationExtendedPtr);
   } while (0);

   EXIT_LOG(%d, ret_val);

   return ret_val;
}
