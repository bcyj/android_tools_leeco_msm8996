/******************************************************************************

  @file    qmi_ril_peripheral_mng.c
  @brief   Provides interface to communicate with peripheral manager

  DESCRIPTION

  ---------------------------------------------------------------------------

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

  ---------------------------------------------------------------------------
******************************************************************************/

#include "qmi_ril_peripheral_mng.h"

#ifndef QMI_RIL_UTF
#include "pm-service.h"
#include "mdm_detect.h"
#endif

#define QMI_RIL_PM_QCRIL_CLIENT_NAME  "QCRIL"

typedef struct
{
    /* reference returned by register() */
    void           *id;

#ifndef QMI_RIL_UTF
    /* peripehral device name */
    const char      device_name[MAX_NAME_LEN];
#endif

} qmi_ril_peripheral_conn_info;

qmi_ril_peripheral_conn_info qmi_ril_peripheral_mng_conn_info = {0};

#ifndef QMI_RIL_UTF
/*===========================================================================

  FUNCTION  qmi_ril_pm_event_notifier

===========================================================================*/
/*!
    @brief
        peripheral manager event call back

    @return
        None
*/
/*=========================================================================*/
void qmi_ril_pm_event_notifier
(
    void *client_cookie,
    enum pm_event event
)
{

    qmi_ril_peripheral_conn_info *info = client_cookie;
    int                           ret;

    pm_client_event_acknowledge(info->id, event);

    switch (event) {

    case EVENT_PERIPH_GOING_OFFLINE:
        QCRIL_LOG_INFO("%s is going off-line\n", info->device_name);
        break;

    case EVENT_PERIPH_IS_OFFLINE:
        QCRIL_LOG_INFO("%s is off-line\n", info->device_name);
        break;

    case EVENT_PERIPH_GOING_ONLINE:
        QCRIL_LOG_INFO("%s going on-line\n", info->device_name);
        break;

    case EVENT_PERIPH_IS_ONLINE:
        QCRIL_LOG_INFO("%s is on-line\n", info->device_name);
        break;

    default:
        QCRIL_LOG_INFO("%s: invalid event\n", info->device_name);
        break;
    }

    return;
}
#endif

/*===========================================================================

  FUNCTION  qmi_ril_peripheral_mng_init

===========================================================================*/
/*!
    @brief
        Initialize peripheral manager client

    @return
        0 if function is successful.
*/
/*=========================================================================*/
int qmi_ril_peripheral_mng_init
(
    char *device_name
)
{
    int ret = RIL_E_SUCCESS;
    int ret_code;
#ifndef QMI_RIL_UTF
    enum pm_event event;

    QCRIL_LOG_FUNC_ENTRY();

    strlcpy(qmi_ril_peripheral_mng_conn_info.device_name,
            device_name, sizeof(qmi_ril_peripheral_mng_conn_info.device_name));

    ret_code = pm_client_register(qmi_ril_pm_event_notifier,
                               &qmi_ril_peripheral_mng_conn_info,
                               qmi_ril_peripheral_mng_conn_info.device_name,
                               QMI_RIL_PM_QCRIL_CLIENT_NAME,
                               &event,
                               &qmi_ril_peripheral_mng_conn_info.id);

    if (ret_code != PM_RET_SUCCESS)
    {
        QCRIL_LOG_ERROR("Failed to register for %s\n",
                        qmi_ril_peripheral_mng_conn_info.device_name);
        ret = RIL_E_GENERIC_FAILURE;
    }
    else
    {
        QCRIL_LOG_INFO("Successful registration\n");
    }

#else
    ret = RIL_E_GENERIC_FAILURE;
#endif
    QCRIL_LOG_FUNC_RETURN();
    return ret;
}


/*===========================================================================

  FUNCTION  qmi_ril_peripheral_mng_deinit

===========================================================================*/
/*!
    @brief
        de init peripheral manager client

    @return
        None
*/
/*=========================================================================*/
void qmi_ril_peripheral_mng_deinit
(
    void
)
{
    QCRIL_LOG_FUNC_ENTRY();

    if (qmi_ril_peripheral_mng_conn_info.id)
    {
#ifndef QMI_RIL_UTF

        pm_client_unregister(qmi_ril_peripheral_mng_conn_info.id);

#endif
    }

    QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION  qmi_ril_peripheral_mng_vote

===========================================================================*/
/*!
    @brief
        Vote for registered device with peripheral manager

    @return
        0 if function is successful.
*/
/*=========================================================================*/
int qmi_ril_peripheral_mng_vote
(
    void
)
{
    int ret = RIL_E_GENERIC_FAILURE;

    QCRIL_LOG_FUNC_ENTRY();

    if (qmi_ril_peripheral_mng_conn_info.id)
    {
#ifndef QMI_RIL_UTF

        ret = pm_client_connect(qmi_ril_peripheral_mng_conn_info.id);
        if (ret)
        {
            QCRIL_LOG_ERROR("%s connect failed %d", qmi_ril_peripheral_mng_conn_info.device_name,
                                                    ret);
        }

#endif
    }

    QCRIL_LOG_FUNC_RETURN();
    return ret;
}

/*===========================================================================

  FUNCTION  qmi_ril_peripheral_mng_release_vote

===========================================================================*/
/*!
    @brief
        Release vote from peripheral manager

    @return
        0 if function is successful.
*/
/*=========================================================================*/
void qmi_ril_peripheral_mng_release_vote
(
    void
)
{
    int ret;

    QCRIL_LOG_FUNC_ENTRY();
    if (qmi_ril_peripheral_mng_conn_info.id)
    {
#ifndef QMI_RIL_UTF

        ret = pm_client_disconnect(qmi_ril_peripheral_mng_conn_info.id);
        if (ret)
        {
            QCRIL_LOG_ERROR("%s disconnect failed %d", qmi_ril_peripheral_mng_conn_info.device_name,
                                                       ret);
        }

#endif
    }

    QCRIL_LOG_FUNC_RETURN();
    return;
}
