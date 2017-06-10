/* Copyright (c) 2010-2014, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
 /*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013 Qualcomm Atheros, Inc.
  All Rights Reserved.
  Qualcomm Atheros Confidential and Proprietary.
=============================================================================*/
#ifndef ULP_QUIPC_H
#define ULP_QUIPC_H

#ifdef __cplusplus
extern "C"
{
#endif


// QUIPC status used by ULP
#define ULP_QUIPC_STATUS_NO_ERROR                       0
#define ULP_QUIPC_STATUS_POSITION_OUTSIDE_LCI_BOUNDARY  1
#define ULP_QUIPC_STATUS_NO_WIFI_MEAS                   2
#define ULP_QUIPC_STATUS_NO_LCI_WITHIN_VICINITY         3
#define ULP_QUIPC_STATUS_LCI_TRANSITION                 4
   // More status in between
#define ULP_QUIPC_STATUS_GENERAL_FAILURE                100

// QUIPC debug info
#define ULP_QUIPC_DEBUG_INFO_STOP                      0
#define ULP_QUIPC_DEBUG_INFO_PF_STATE                  1
#define ULP_QUIPC_DEBUG_INFO_AP_MEAS                   2
#define ULP_QUIPC_DEBUG_INFO_ALL                       3

typedef int (ulp_quipc_process_position_cb)
(
   int                 quipc_error_code,
   const UlpLocation*  quipc_location_ptr,
   int                 quipc_debug_info_len,
   char*               quipc_debug_info
);

typedef int (ulp_quipc_request_coarse_position_cb) ();

/** Represents the adapter interface between ulp and quipc
 *  module interface. */
typedef struct {
    /** set to sizeof(ulpInterface) */
    size_t   size;

    /**
     * Starts the ulp module. 0: success
     */
    int   (*init)( ulp_quipc_process_position_cb*         process_pos_cb,
                   ulp_quipc_request_coarse_position_cb*  req_cb);
    /** Starts the ulp engine. 0: success, interval is in seconds */
    int   (*start_fix)( int interval,
                        int user_lci_specified,
                        unsigned char* user_lci_id );

    /** Stops the ulp engine. 0: success */
    int   (*stop_fix)( void );

    /** Send back requsted info */
    int   (*send_coarse_position) (const UlpLocation* pos_ptr );

    /** Request debug info */
    int   (*request_debug_info) (int debug_type);

    /** Closes the interface */
    int   (*destroy)( void );

} ulpQuipcInterface;


typedef const ulpQuipcInterface* (get_ulp_quipc_interface) (void);

#ifdef __cplusplus
}
#endif
#endif /* ULP_QUIPC_H */

