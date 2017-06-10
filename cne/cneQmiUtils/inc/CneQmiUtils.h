
#ifndef CNE_QMI_UTILS_H
#define CNE_QMI_UTILS_H
/**
 * @file CneQmiUtils.h
 *
 *
 * ============================================================================
 *             Copyright (c) 2011,2014 Qualcomm Technologies, Inc.
 *             All Rights Reserved.
 *             Qualcomm Technologies Confidential and Proprietary
 * ============================================================================
 */

#include <stdlib.h>
#include <qmi_platform_config.h>
extern "C" {
  #define QMI_DSD_SYNC_MSG_TIMEOUT  (10000)

  #define CNE_CLIENT_MSM_QMI_CONN_ID           QMI_PORT_RMNET_1
  #define CNE_CLIENT_DEF_MSM_QMI_CONN_ID       QMI_PORT_RMNET_0
  #define CNE_CLIENT_DEFAULT_QMI_CONN_ID       CNE_CLIENT_MSM_QMI_CONN_ID
  #define CNE_CLIENT_MDMUSB_QMI_CONN_ID        QMI_PORT_RMNET_USB_0
  #define CNE_CLIENT_MDMMHI_QMI_CONN_ID        QMI_PORT_RMNET_MHI_0
}

#define CNE_CLIENT_NUM_PORT_ENTRIES  \
  (sizeof(CneClientQmiConnId)/sizeof(CneClientQmiConnId[0]))

static const char *CneClientQmiConnId[] =
{
  CNE_CLIENT_MSM_QMI_CONN_ID,
  CNE_CLIENT_DEF_MSM_QMI_CONN_ID,
  CNE_CLIENT_MDMUSB_QMI_CONN_ID,
  CNE_CLIENT_MDMMHI_QMI_CONN_ID,
};

class CneQmiUtils {
    public:
        static bool isQmiPort (const char *device);
};

#endif /*CNE_QMI_UTILS_H*/
