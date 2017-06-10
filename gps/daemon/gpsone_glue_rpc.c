/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
// #include <rpc/rpc.h>
#include "gpsone_daemon_dbg.h"
#include "gpsone_glue_rpc.h"
#ifndef DEBUG_X86
#include "oncrpc.h"
#else
#include "comdef.h"
#endif

#include "gpsone_bit_api.h"

#ifndef DEBUG_X86
#include "gpsone_bit_api_rpc.h"
#else
boolean gpsone_bit_api_null(void);
void gpsone_bit_apicb_app_init(void);
#endif

static void restart_cb( void *handle, void *data )
{
    GPSONE_DMN_PR_ERR("%s:%d] we exit...", __func__, __LINE__);

    exit(1);
}
static void exit_cb( void *handle, void *data )
{
    GPSONE_DMN_DBG("%s:%d] modem down", __func__, __LINE__);
}

/*===========================================================================
FUNCTION    gpsone_glue_rpc_init

DESCRIPTION
    This function initialize the RPC

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_glue_rpc_init(void)
{

#ifndef DEBUG_X86
    int ret;

    /* Initialize ONCRPC */
    GPSONE_DMN_DBG("%s:%d]", __func__, __LINE__);
    oncrpc_init();

    GPSONE_DMN_DBG("%s:%d]", __func__, __LINE__);
    oncrpc_task_start();

    GPSONE_DMN_DBG("%s:%d]", __func__, __LINE__);
    gpsone_bit_apicb_app_init();

    GPSONE_DMN_DBG("%s:%d]", __func__, __LINE__);
    ret = gpsone_bit_api_null();

    /*
    * Modem is not up.
    * Poll for modem to come up.
    */
    if (ret == 0) {
      GPSONE_DMN_PR_ERR("%s:%d] ERROR: gpsone_bit_api_null() failed! ret=%d. entering retry loop...", __func__, __LINE__, ret);
      int i;
      int count = 10;
      for (i = 0; i < count; i++) {
        sleep(5);
        ret = gpsone_bit_api_null();
        if (ret != 0) {
          GPSONE_DMN_DBG("%s:%d] modem is up on try number %d", __func__, __LINE__, i + 1);
          break;
        } else {
          GPSONE_DMN_PR_ERR("%s:%d] ERROR: gpsone_bit_api_null() failed! ret=%d. retrying...", __func__, __LINE__, ret);
        }
      }
    }

    GPSONE_DMN_DBG("%s:%d]", __func__, __LINE__);
    if (ret == 0) {
        GPSONE_DMN_PR_ERR("%s:%d] ERROR: gpsone_bit_api_null() failed! ret=%d", __func__, __LINE__, ret);
        return -1;
    }

    /* Register for future notifications */
    GPSONE_DMN_DBG("%s:%d]", __func__, __LINE__);
    oncrpc_register_server_exit_notification_cb( GPSONE_BIT_APIPROG, GPSONE_BIT_APIVERS, exit_cb, NULL);
    oncrpc_register_server_restart_notification_cb( GPSONE_BIT_APIPROG, GPSONE_BIT_APIVERS, restart_cb, NULL);
#endif

    return 0;
}

