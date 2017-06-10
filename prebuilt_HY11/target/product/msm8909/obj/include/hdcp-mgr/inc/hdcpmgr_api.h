/* Copyright (c) 2013 by Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef HDCPMGR_API_H
#define HDCPMGR_API_H

#ifdef __cplusplus
extern "C" {
#endif

/* total dev_count of ds receiver's BKSV assembled together */
struct HDCP_V2V1_DS_TOPOLOGY {
                uint8_t dev_count; /* start from first attached active node */
                uint8_t depth; /* how deep of repeating cascade */
                uint8_t ksv_list[5 * 128];
                uint32_t max_cascade_exceeded;
                uint32_t max_dev_exceeded;
};

enum ERROR_NO {
    ERROR_IO = -1,
    ERROR_NOT_AUTHED = -2,
};

enum EV_TYPE {
        EV_REQUEST_TOPOLOGY = 1,
        EV_SEND_TOPOLOGY = 2,
        EV_ERROR,
};

/**
  * @brief This API will provide information about hdmi transmitter core
  * connection to sync.
  *
  * @return true when connected otherwise false.
  */
bool HDCP1X_COMM_hdmi_status(void);

/**
  * @brief This API to receive HDCP V2 event and associated data from
  *      DX Lib module.
  *
  * @param[in]  evType             Defined by DX API.
  * @param[source]  param1         Defined by DX API, associated event.
  * @param[source]  param2         Defined by DX API, associated event.
  *
  * @return zero on success, negative on failure standing for error codes
  */
int HDCP1X_COMM_Send_hdcp2x_event(int evType,
                                  void *param1, void *param2);

/**
  * @brief This CALLBACK is passed from WFD module to receive HDCP V1 event and
  *            associated data from HDCP manager.
  *
  * @param[in]  evType         Defined by enum EV_TYPE.
  * @param[source]  param      Defined by HDCP_V2V1_DS_TOPOLOGY or error
  *                             messages.
  *
  * @return zero on success, negative on failure standing for error codes
  */
typedef  int (* CALLBACK)(int evType,  void *param);

/**
  * @brief This API to be invoked by WFD context to start HDCP manager and HDCP
  *     V2V1 engine. CALLBACK is expected for HDCP manager sending the data
  *     back to WFD module.
  *
  * @param[in]  arg         CALLBACK passed in HDCP manager.
  *
  * @return zero on success, negative on failure standing for error codes
  */
int HDCP1X_COMM_Init(CALLBACK arg);

/**
  * @brief This API to be invoked by WFD context to stop HDCP V2V1 engine.
    *     and clean up any resources.
  *
  * @param[in]  arg         CALLBACK passed in HDCP manager.
  *
  * @return zero on success, negative on failure standing for error codes
  */

int HDCP1X_COMM_Term(void);

#ifdef __cplusplus
}
#endif

#endif /* HDCPMGR_API_H */
