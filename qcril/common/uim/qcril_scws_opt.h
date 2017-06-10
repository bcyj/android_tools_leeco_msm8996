#ifndef QCRIL_SCWS_OPT_H
#define QCRIL_SCWS_OPT_H

/*===========================================================================

  Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

when      who   what, where, why
--------  ---   ----------------------------------------------------------
08/09/12  sc    Initial version
===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include <sys/types.h>
#include <unistd.h>
#include "qcril_log.h"

/*===========================================================================

                           DEFINES

===========================================================================*/
#define QCRIL_SCWS_OPT_MAX_PIPELINED_REQUESTS          20
#define TRUE                                            1
#define FALSE                                           0

/*===========================================================================

                           TYPES

===========================================================================*/
/* -----------------------------------------------------------------------------
   TYPEDEF:     QCRIL_SCWS_SOCKET_STATE_ENUM_TYPE

   DESCRIPTION:
     This is the type that indicates the traffic analyzer state.

     QCRIL_SCWS_OPT_ANALYZER_STATE_IDLE     - The IDLE state is the state of an
                                              analyzer when it is first created.
                                              Analyzer returns to this state after
                                              it has finished sending all data and
                                              is reset.

     QCRIL_SCWS_OPT_ANALZYER_STATE_SENDRECV - The SENDRECV state is the state an
                                              an analyzer is in while it is
                                              processing (sending and receiving)
                                              data.

     QCRIL_SCWS_OPT_ANALYZER_STATE_ERROR    - If at any point an analyzer
                                              encounters an error, it enters
                                              the ERROR state and stays in this
                                              state until it is closed by the
                                              browser or the SCWS module. While
                                              in this state, no optimizations
                                              take effect.
-------------------------------------------------------------------------------*/
typedef enum
{
  QCRIL_SCWS_OPT_ANALYZER_STATE_IDLE,
  QCRIL_SCWS_OPT_ANALYZER_STATE_SENDRECV,
  QCRIL_SCWS_OPT_ANALYZER_STATE_ERROR
} qcril_scws_analyzer_state_enum_type;

/* -----------------------------------------------------------------------------
   TYPEDEF:      QCRIL_SCWS_HTTP_REQUEST_METHOD_TYPE

   DESCRIPTION:
     Used as a means of representing the request type of an HTTP message.
     QCRIL_SCWS_HTTP_REQUEST_NONE - Does not represent any kind of request;
                                    used as a default value to initialize the
                                    analyzer's request array.
     QCRIL_SCWS_HTTP_REQUEST_GET  - Represents a GET request.
     QCRIL_SCWS_HTTP_REQUEST_HEAD - Represents a HEAD request.
     QCRIL_SCWS_HTTP_REQUEST_POST - Represents a POST request.

-------------------------------------------------------------------------------*/
typedef enum
{
  QCRIL_SCWS_HTTP_REQUEST_NONE,
  QCRIL_SCWS_HTTP_REQUEST_GET,
  QCRIL_SCWS_HTTP_REQUEST_HEAD,
  QCRIL_SCWS_HTTP_REQUEST_POST
} qcril_scws_http_request_method_type;

/* -----------------------------------------------------------------------------
   STRUCT:      QCRIL_SCWS_OPT_TRAFFIC_ANALYZER_TYPE

   DESCRIPTION:
     This structure holds the state of the traffic analyzer for each
     connected socket. It is a member of the connected_socket structure in scws.h
-------------------------------------------------------------------------------*/
typedef struct
{
  qcril_scws_analyzer_state_enum_type     analyzer_state;

  int                                     response_content_length;
  int                                     response_status_code;
  boolean                                 response_status_code_found;
  boolean                                 response_content_length_found;
  boolean                                 response_transfer_encoding_chunked_found;
  boolean                                 response_final_chunk_found;
  boolean                                 response_header_end_found;
  uint8                                 * response_buffer;
  size_t                                  response_buffer_len;

  int                                     request_content_length;
  int                                     request_count;
  int                                     requests_processed;
  uint8                                 * request_buffer;
  size_t                                  request_buffer_len;
  boolean                                 request_found;
  boolean                                 request_header_end_found;
  boolean                                 request_processing_post_request;
  boolean                                 request_content_length_found;
  qcril_scws_http_request_method_type     requests_arr[QCRIL_SCWS_OPT_MAX_PIPELINED_REQUESTS];
} qcril_scws_opt_traffic_analyzer_type;


/*=========================================================================

  FUNCTION:  qcril_scws_opt_process_rx

===========================================================================*/
/*!
    @brief
    Processes RX data. Ensures the data is an HTTP request, and parses
    the request method. If a valid HTTP request is parsed, updates
    the requests_remaining counter.

    @return
    None
*/
/*=========================================================================*/
void qcril_scws_opt_process_rx(
  qcril_scws_opt_traffic_analyzer_type  * analyzer_ptr,
  const uint8                           * data_ptr,
  uint16                                  data_len);


/*=========================================================================

  FUNCTION:  qcril_scws_opt_process_tx

===========================================================================*/
/*!
    @brief
    Processes TX data. Ensures the data is an HTTP response, and parses
    the content length and status code, and uses these to determine when
    all data has been sent.

    @return
    True if all data has been sent successfully
    False otherwise
*/
/*=========================================================================*/
boolean qcril_scws_opt_process_tx(
  qcril_scws_opt_traffic_analyzer_type  * analyzer_ptr,
  const uint8                           * data_ptr,
  uint16                                  data_len);


/*=========================================================================

  FUNCTION:  qcril_scws_opt_reset

===========================================================================*/
/*!
    @brief
    Refreshes the state of the traffic analyzer to an initial state.

    @return
    none
*/
/*=========================================================================*/
void qcril_scws_opt_reset(
  qcril_scws_opt_traffic_analyzer_type  * analyzer_ptr);


/*=========================================================================

  FUNCTION:  qcril_scws_opt_print_analyzer_state

===========================================================================*/
/*!
    @brief
    Prints the state of the traffic analyzer.

    @return
    none
*/
/*=========================================================================*/
void qcril_scws_opt_print_analyzer_state(
  const qcril_scws_opt_traffic_analyzer_type * analyzer_ptr);

#endif /* QCRIL_SCWS_OPT_H */


