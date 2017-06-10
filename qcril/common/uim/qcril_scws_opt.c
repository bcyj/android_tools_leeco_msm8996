
/*===========================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

when       who     what, where, why
--------   ---     ----------------------------------------------------------
05/05/14   ar      Fix critical KW errors
04/17/13   yt      Critical KW fixes
08/09/12   sc      Initial Version
===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include "qcril_scws_opt.h"


/*===========================================================================

                           INTERNAL DEFINITIONS

===========================================================================*/
#define QCRIL_SCWS_HTTP                                  "HTTP/"
#define QCRIL_SCWS_HTTP_LEN                              5
#define QCRIL_SCWS_SPACE_HTTP                            " HTTP/"
#define QCRIL_SCWS_SPACE_HTTP_LEN                        6
#define QCRIL_SCWS_GET                                   "GET "
#define QCRIL_SCWS_GET_LEN                               4
#define QCRIL_SCWS_HEAD                                  "HEAD "
#define QCRIL_SCWS_HEAD_LEN                              5
#define QCRIL_SCWS_POST                                  "POST "
#define QCRIL_SCWS_POST_LEN                              5
#define QCRIL_SCWS_CONTENT_LENGTH                        "Content-Length:"
#define QCRIL_SCWS_CONTENT_LENGTH_LEN                    15
#define QCRIL_SCWS_CRLF                                  "\r\n"
#define QCRIL_SCWS_CRLF_LEN                              2
#define QCRIL_SCWS_LF                                    "\n"
#define QCRIL_SCWS_LF_LEN                                1
#define QCRIL_SCWS_TRANSFER_ENCODING                     "Transfer-Encoding:"
#define QCRIL_SCWS_TRANSFER_ENCODING_LEN                 18
#define QCRIL_SCWS_CHUNKED                               "Chunked"
#define QCRIL_SCWS_CHUNKED_LEN                           7
#define QCRIL_SCWS_FINAL_CHUNK_CRLF                      "0\r\n\r\n"
#define QCRIL_SCWS_FINAL_CHUNK_CRLF_LEN                  5
#define QCRIL_SCWS_FINAL_CHUNK_LF                        "0\n\n"
#define QCRIL_SCWS_FINAL_CHUNK_LF_LEN                    3
#define QCRIL_SCWS_HTTP_100                              100
#define QCRIL_SCWS_HTTP_200                              200
#define QCRIL_SCWS_HTTP_204                              204
#define QCRIL_SCWS_HTTP_304                              304


/*=========================================================================

  FUNCTION:  qcril_scws_opt_isnum

===========================================================================*/
/*!
    @brief
    Returns TRUE if a character is a digit (0-9).

    @return
    TRUE if c is a pointer to a digit (0-9), FALSE otherwise
*/
/*=========================================================================*/
static boolean qcril_scws_opt_isnum(char c)
{
  if (c >= '0' && c <= '9')
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}/* isnum */


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
  const qcril_scws_opt_traffic_analyzer_type * analyzer_ptr)
{
  if(analyzer_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Invalid value, cannot process request");
    QCRIL_ASSERT(0);
    return;
  }

  QCRIL_LOG_DEBUG("%s, request_count: 0x%x, requests_processed: 0x%x, request_header_end_found: 0x%x, request_found: 0x%x\n",
         __FUNCTION__,
         analyzer_ptr->request_count,
         analyzer_ptr->requests_processed,
         analyzer_ptr->request_header_end_found,
         analyzer_ptr->request_found);

  QCRIL_LOG_DEBUG("%s, Processing POST request: 0x%x, request_content_length_found: 0x%x, request_content_length: 0x%x, request_buffer_len: 0x%x\n",
         __FUNCTION__,
         analyzer_ptr->request_processing_post_request,
         analyzer_ptr->request_content_length_found,
         analyzer_ptr->request_content_length,
         analyzer_ptr->request_buffer_len);

  QCRIL_LOG_DEBUG("%s, response_content_length_found: 0x%x, response_content_length: 0x%x, response_status_code_found: 0x%x, response_status_code: %d\n",
         __FUNCTION__,
         analyzer_ptr->response_content_length_found,
         analyzer_ptr->response_content_length,
         analyzer_ptr->response_status_code_found,
         analyzer_ptr->response_status_code);

  QCRIL_LOG_DEBUG("%s, response_header_end_found: 0x%x, response_transfer_encoding_chunked_found: 0x%x, response_final_chunk_found: 0x%x, response_buffer_len: 0x%x\n",
         __FUNCTION__,
         analyzer_ptr->response_header_end_found,
         analyzer_ptr->response_transfer_encoding_chunked_found,
         analyzer_ptr->response_final_chunk_found,
         analyzer_ptr->response_buffer_len);
} /* qcril_scws_opt_print_analyzer_state */


/*=========================================================================

  FUNCTION:  qcril_scws_opt_reset_requests_pending

===========================================================================*/
/*!
    @brief
    Resets only the members of the traffic analyzer necessary in the case
    that a request is processed and there are more left to process in the
    data packet. In this case, a full reset must not be performed; the
    request count and requests processed items must be left intact.

    @return
    none
*/
/*=========================================================================*/
static void qcril_scws_opt_reset_requests_pending(
  qcril_scws_opt_traffic_analyzer_type* analyzer_ptr)
{
  if(analyzer_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Invalid value, cannot process request");
    QCRIL_ASSERT(0);
    return;
  }

  analyzer_ptr->response_transfer_encoding_chunked_found   = FALSE;
  analyzer_ptr->response_final_chunk_found                 = FALSE;
  analyzer_ptr->response_content_length                    = 0;
  analyzer_ptr->response_content_length_found              = FALSE;
  analyzer_ptr->response_header_end_found                  = FALSE;
  analyzer_ptr->response_status_code                       = 0;
  analyzer_ptr->response_status_code_found                 = FALSE;

  /* response_buffer is not freed here because
     it is initialized in this function. Other functions
     handle allocating and freeing the dynamic memory. */
  analyzer_ptr->response_buffer                            = NULL;
  analyzer_ptr->response_buffer_len                        = 0;
} /* qcril_scws_opt_reset_requests_pending */


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
  qcril_scws_opt_traffic_analyzer_type  * analyzer_ptr)
{
  if(analyzer_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Invalid value, cannot process request");
    QCRIL_ASSERT(0);
    return;
  }

  analyzer_ptr->request_header_end_found          = FALSE;
  analyzer_ptr->request_content_length_found      = FALSE;
  analyzer_ptr->request_processing_post_request   = FALSE;
  analyzer_ptr->request_found                     = FALSE;
  analyzer_ptr->request_content_length            = 0;
  analyzer_ptr->request_count                     = 0;
  analyzer_ptr->requests_processed                = 0;

  /* request_buffer is not freed here because
     it is initialized in this function. Other functions
     handle allocating and freeing the dynamic memory. */
  analyzer_ptr->request_buffer                    = NULL;
  analyzer_ptr->request_buffer_len                = 0;

  analyzer_ptr->analyzer_state                    = QCRIL_SCWS_OPT_ANALYZER_STATE_IDLE;
  qcril_scws_opt_reset_requests_pending(analyzer_ptr);

  memset(analyzer_ptr->requests_arr,
         QCRIL_SCWS_HTTP_REQUEST_NONE,
         sizeof(analyzer_ptr->requests_arr));
}/* qcril_scws_opt_reset*/


/*=========================================================================

  FUNCTION:  qcril_scws_opt_handle_error

===========================================================================*/
/*!
    @brief
    Handles error cases by turning off optimizations until the offending
    socket is closed. During this time, no optimization will occur.

    @return
    none
*/
/*=========================================================================*/
static void qcril_scws_opt_handle_error(
  qcril_scws_opt_traffic_analyzer_type * analyzer_ptr)
{
  /* QCRIL_ASSERT(analyzer_ptr != NULL); */
  QCRIL_LOG_DEBUG("%s, Inside handle_error, printing state\n", __FUNCTION__);

  qcril_scws_opt_print_analyzer_state(analyzer_ptr);
  qcril_scws_opt_reset(analyzer_ptr);
  analyzer_ptr->analyzer_state = QCRIL_SCWS_OPT_ANALYZER_STATE_ERROR;
} /* qcril_scws_opt_handle_error */


/*=========================================================================

  FUNCTION:  qcril_scws_opt_get_line

===========================================================================*/
/*!
    @brief
    Gets the next line of data.

    @return
    uint8* pointer to line of data. The line includes the newline sequence
    ("\r\n"). Likewise, the output parameter line_len also includes the
    newline.
*/
/*=========================================================================*/
static char* qcril_scws_opt_get_line(
  uint8                               ** begin_ptr,
  size_t                                 data_len,
  size_t                               * line_len)
{
  int      i                 = 0;
  char   * line_ptr          = NULL;
  uint8  * eol_ptr           = NULL;
  int      eol_len           = 0;
  size_t   line_len_temp     = 0;

  if (begin_ptr == NULL ||
      line_len == NULL ||
      data_len <= 0)
  {
    QCRIL_LOG_DEBUG("Error in qcril_scws_opt_get_line - bad parameters. \n");
    return NULL;
  }

  /* Obtain pointer to CRLF, if present */
  if (data_len >= QCRIL_SCWS_CRLF_LEN)
  {
    eol_ptr = memmem(*begin_ptr,
                           data_len,
                           QCRIL_SCWS_CRLF,
                           QCRIL_SCWS_CRLF_LEN);

    /* eol_len is the length of the newline character;
    some clients may use LF (eol_len should be 1)
    instead of the standard CRLF (eol_len should be 2) */
    eol_len = QCRIL_SCWS_CRLF_LEN;
  }

  if (eol_ptr == NULL)
  {
    /* If CRLF not found, obtain pointer to LF,
       if present */
    eol_ptr = memmem(*begin_ptr, data_len, QCRIL_SCWS_LF, QCRIL_SCWS_LF_LEN);
    eol_len = QCRIL_SCWS_LF_LEN;
  }

  if (eol_ptr == NULL)
  {
    return NULL;
  }

  /* Allocate line_ptr and set *line_len. Add one extra
     space for null termination, as line_ptr is a char* */
  *line_len = (eol_ptr - *begin_ptr)+eol_len;
  line_ptr = (char *) qcril_malloc((*line_len + 1)*sizeof(uint8));

  if(line_ptr == NULL)
  {
    QCRIL_LOG_DEBUG("%s, Memory allocation failed. Returning NULL.\n",
                    __FUNCTION__);
    *begin_ptr += data_len;
    return NULL;
  }
  memset(line_ptr, '\0', *line_len+1);

  if (*line_len > 0)
  {
    memcpy(line_ptr, *begin_ptr, *line_len);
    *begin_ptr += *line_len;
  }

  return line_ptr;
} /* qcril_scws_opt_get_line */


/*=========================================================================

  FUNCTION:  qcril_scws_opt_process_rx

===========================================================================*/
/*!
    @brief
    Processes RX data. Ensures the data is an HTTP request, and parses
    the request method. If a valid HTTP request is parsed, adds the request
    type to an array in the analyzer and updates the requests_processed
    counter.

    @return
    None
*/
/*=========================================================================*/
void qcril_scws_opt_process_rx(
  qcril_scws_opt_traffic_analyzer_type * analyzer_ptr,
  const uint8                          * data_ptr,
  uint16                                 data_len)
{
  uint8 * merged_data_buffer      = NULL;
  uint16  merged_data_buffer_len  = 0;
  uint8 * begin_ptr               = NULL;
  char  * line_ptr                = NULL;
  size_t  line_len                = 0;
  boolean header_end_found        = 0;

  /* Check parameters */
  if (analyzer_ptr == NULL ||
      data_ptr == NULL     ||
      data_len <= 0)
  {
    /* Free request buffer in case it has
       been allocated in a previous call to
       process_tx. request_buffer_len is reset
       in the handle_error function. */
    if (analyzer_ptr != NULL &&
        analyzer_ptr->request_buffer != NULL)
    {
      qcril_free(analyzer_ptr->request_buffer);
      analyzer_ptr->request_buffer = NULL;
    }
    QCRIL_LOG_DEBUG("%s, Invalid parameters to process_rx, entering error state.\n",
           __FUNCTION__);
    qcril_scws_opt_handle_error(analyzer_ptr);
    return;
  }

  /* Check analyzer state. */
  if (analyzer_ptr->analyzer_state == QCRIL_SCWS_OPT_ANALYZER_STATE_IDLE ||
      analyzer_ptr->analyzer_state == QCRIL_SCWS_OPT_ANALYZER_STATE_SENDRECV)
  {
    analyzer_ptr->analyzer_state = QCRIL_SCWS_OPT_ANALYZER_STATE_SENDRECV;
  }
  else if (analyzer_ptr->analyzer_state == QCRIL_SCWS_OPT_ANALYZER_STATE_ERROR)
  {
    QCRIL_LOG_DEBUG("%s, Analyzer is in error state, returning.\n",
                    __FUNCTION__);

    if (analyzer_ptr->request_buffer != NULL)
    {
      qcril_free(analyzer_ptr->request_buffer);
      analyzer_ptr->request_buffer = NULL;
    }

    return;
  }

  /* If there is any leftover data from the last call to process_rx,
     concatenate to the beginning of the data for this call. */
  if (analyzer_ptr->request_buffer != NULL &&
      analyzer_ptr->request_buffer_len > 0)
  {
    merged_data_buffer_len =
      (uint16)((analyzer_ptr->request_buffer_len+data_len)*sizeof(uint8));
    merged_data_buffer =
      qcril_malloc(merged_data_buffer_len);

    if (merged_data_buffer == NULL)
    {
      QCRIL_LOG_DEBUG("%s, Memory allocation failed for merged_data_buffer.\n",
                      __FUNCTION__);

      qcril_free(analyzer_ptr->request_buffer);
      analyzer_ptr->request_buffer = NULL;

      qcril_scws_opt_handle_error(analyzer_ptr);
      return;
    }

    memcpy(merged_data_buffer,
           analyzer_ptr->request_buffer,
           analyzer_ptr->request_buffer_len);

    memcpy(merged_data_buffer + analyzer_ptr->request_buffer_len,
           data_ptr,
           data_len);

    data_ptr = merged_data_buffer;
    data_len = (int)analyzer_ptr->request_buffer_len + data_len;

    /* Free temporary request buffer data */
    qcril_free(analyzer_ptr->request_buffer);
    analyzer_ptr->request_buffer = NULL;
    analyzer_ptr->request_buffer_len = 0;
  }

  /* Process body of POST request, if needed */
  if (analyzer_ptr->request_processing_post_request &&
        analyzer_ptr->request_header_end_found)
  {
    if (analyzer_ptr->request_content_length_found)
    {
      analyzer_ptr->request_content_length -= data_len;

      if (analyzer_ptr->request_content_length == 0)
      {
        QCRIL_LOG_DEBUG("Content-Length = 0. Done processing POST request.\n");
        analyzer_ptr->request_processing_post_request = FALSE;
      }
      else if (analyzer_ptr->request_content_length > 0)
      {
        QCRIL_LOG_DEBUG("Content-Length = %d, which is > 0. Continuing.\n",
                analyzer_ptr->request_content_length);
      }
      else if (analyzer_ptr->request_content_length < 0)
      {
        QCRIL_LOG_DEBUG("Content-Length < 0 while processing post request. Going to error state.\n");

        /* Free merged_data_buffer and request_data_buffer before returning */
        if (merged_data_buffer != NULL)
        {
          qcril_free(merged_data_buffer);
          merged_data_buffer = NULL;
        }

        if (analyzer_ptr->request_buffer != NULL)
        {
          qcril_free(analyzer_ptr->request_buffer);
          analyzer_ptr->request_buffer = NULL;
        }

        qcril_scws_opt_handle_error(analyzer_ptr);
        return;
      }
    }
    else
    {
      QCRIL_LOG_DEBUG("Error: Content-Length not found in POST request. Going to error state.\n");

      /* Free merged_data_buffer and request_data_buffer before returning */
      if (merged_data_buffer != NULL)
      {
        qcril_free(merged_data_buffer);
        merged_data_buffer = NULL;
      }

      if (analyzer_ptr->request_buffer != NULL)
      {
        qcril_free(analyzer_ptr->request_buffer);
        analyzer_ptr->request_buffer = NULL;
      }

      qcril_scws_opt_handle_error(analyzer_ptr);
      return;
    }
  }

  /* Initialize pointer to request data. We will
     go through the request data via a while loop, incrementing
     begin_ptr. */
  begin_ptr = (uint8*)data_ptr;

  /* Loop through the request data and find the request method(s). */
  while(begin_ptr < data_ptr + data_len)
  {
    /* declare variables to hold search results */
    char* http_ptr = NULL;
    char* get_ptr  = NULL;
    char* head_ptr = NULL;
    char* post_ptr = NULL;

    /* Grab first line of request data (RX data). line_len will
       be initialized to the length of this line as a result of
       the call to qcril_scws_opt_get_line(). */
    line_ptr =
      qcril_scws_opt_get_line(&begin_ptr,
                              ((size_t)data_ptr+(size_t)data_len)-(size_t)begin_ptr,
                              &line_len);

    if (line_ptr == NULL)
    {
      /* Line is NULL - could not find newline. This happens during header
         parsing when a HTTP header is split across two packets (or more
         if it is a very large header). Copy remaining data from the line
         into temporary buffer to be prepended to the next line. The prepend
         operation occurs during the next call to process_rx. */
      analyzer_ptr->request_buffer_len =
          (data_ptr+data_len)-begin_ptr;
      analyzer_ptr->request_buffer =
          qcril_malloc(analyzer_ptr->request_buffer_len * sizeof(uint8));

      if (analyzer_ptr->request_buffer == NULL)
      {
        QCRIL_LOG_DEBUG("Memory allocation failed, entering error state.\n");

        /* Free merged_data_buffer if it is NULL */
        if (merged_data_buffer != NULL)
        {
          qcril_free(merged_data_buffer);
          merged_data_buffer = NULL;
        }

        qcril_scws_opt_handle_error(analyzer_ptr);
        return;

      }

      memcpy(analyzer_ptr->request_buffer,
             begin_ptr,
             analyzer_ptr->request_buffer_len);

      if (merged_data_buffer != NULL)
      {
        qcril_free(merged_data_buffer);
        merged_data_buffer = NULL;
      }
      return;
    }

    /* If processing the header of a post request, find
       its Content-Length. */
    if (analyzer_ptr->request_processing_post_request &&
        !analyzer_ptr->request_header_end_found)
    {
      /* Search for the content-length of the POST request */
      if (!analyzer_ptr->request_content_length_found)
      {
        int   i                  = 0;
        char* content_length_ptr = NULL;

        /* If the Content-Length header field is found, store it. */
        content_length_ptr = strcasestr(line_ptr, QCRIL_SCWS_CONTENT_LENGTH);

        if (content_length_ptr != NULL &&
            strncmp(line_ptr, content_length_ptr, QCRIL_SCWS_CONTENT_LENGTH_LEN) == 0)
        {
          uint16 i = 0;
          for (i = QCRIL_SCWS_CONTENT_LENGTH_LEN; i < line_len; i++)
          {
            if (qcril_scws_opt_isnum(content_length_ptr[i]))
            {
              analyzer_ptr->request_content_length =
                (analyzer_ptr->request_content_length*10) + (content_length_ptr[i] - '0');
            }
          }

          if (analyzer_ptr->request_content_length < 0)
          {
            QCRIL_LOG_DEBUG("Invalid Request Content-Length, entering error state.\n");

            /* Check merged_data_buffer, line_ptr, and request_buffer
               to avoid memory leaks; they are no
               longer needed once error state is entered. */
            if (merged_data_buffer != NULL)
            {
              qcril_free(merged_data_buffer);
              merged_data_buffer = NULL;
            }

            if (line_ptr != NULL)
            {
              qcril_free(line_ptr);
              line_ptr = NULL;
            }

            /* Length of request buffer is reset in handle_error */
            if (analyzer_ptr->request_buffer != NULL)
            {
              qcril_free(analyzer_ptr->request_buffer);
              analyzer_ptr->request_buffer = NULL;
            }

            qcril_scws_opt_handle_error(analyzer_ptr);
            return;
          }

          analyzer_ptr->request_content_length_found = TRUE;
          QCRIL_LOG_DEBUG("Response Content-Length: 0x%x\n",
                 analyzer_ptr->request_content_length);
        }
      }
    }/* end processing of POST request content-length */

    /* search for the string " HTTP/" in the line. */
    if (!analyzer_ptr->request_found &&
        !analyzer_ptr->request_processing_post_request)
    {
      http_ptr = strstr(line_ptr, QCRIL_SCWS_SPACE_HTTP);
    }

    if (http_ptr != NULL)
    {
      /* search for request method. Check to make sure
         first character of the line matches the first
         character of the request method name. strncmp is
         used to make sure that the found string is at
         the beginning of the line, as mandated in RFC 2616,
         section 3.1. */
      if (strncmp(line_ptr, QCRIL_SCWS_GET, QCRIL_SCWS_GET_LEN) == 0)
      {
        /* request method is get, add it to request array
           and increase request count */
        analyzer_ptr->request_found = TRUE;
        analyzer_ptr->requests_arr[analyzer_ptr->request_count] =
          QCRIL_SCWS_HTTP_REQUEST_GET;
        analyzer_ptr->request_count++;
        QCRIL_LOG_DEBUG("request method is GET\n");

        qcril_free(line_ptr);
        line_ptr = NULL;
        continue;
      }
      else if (strncmp(line_ptr, QCRIL_SCWS_HEAD, QCRIL_SCWS_HEAD_LEN) == 0)
      {
        analyzer_ptr->request_found = TRUE;
        analyzer_ptr->requests_arr[analyzer_ptr->request_count] =
          QCRIL_SCWS_HTTP_REQUEST_HEAD;
        analyzer_ptr->request_count++;
        QCRIL_LOG_DEBUG("request method is HEAD\n");

        qcril_free(line_ptr);
        line_ptr = NULL;
        continue;
      }
      else if (strncmp(line_ptr, QCRIL_SCWS_POST, QCRIL_SCWS_POST_LEN) == 0)
      {
        analyzer_ptr->request_found = TRUE;
        analyzer_ptr->request_processing_post_request = TRUE;
        analyzer_ptr->requests_arr[analyzer_ptr->request_count] =
          QCRIL_SCWS_HTTP_REQUEST_POST;
        analyzer_ptr->request_count++;
        QCRIL_LOG_DEBUG("request method is POST\n");

        qcril_free(line_ptr);
        line_ptr = NULL;
        continue;
      }
      else
      {
        QCRIL_LOG_DEBUG("Error: Invalid HTTP request. Entering error state.\n");

        qcril_free(line_ptr);
        line_ptr = NULL;

        if (merged_data_buffer != NULL)
        {
          qcril_free(merged_data_buffer);
          merged_data_buffer = NULL;
        }

        if (analyzer_ptr->request_buffer != NULL)
        {
          qcril_free(analyzer_ptr->request_buffer);
          analyzer_ptr->request_buffer = NULL;
        }

        qcril_scws_opt_handle_error(analyzer_ptr);
        return;
      }
    }
    else if (strncmp(line_ptr, QCRIL_SCWS_CRLF, QCRIL_SCWS_CRLF_LEN) != 0)
    {
      if (strncmp(line_ptr, QCRIL_SCWS_LF, QCRIL_SCWS_LF_LEN) == 0)
      {
        QCRIL_LOG_DEBUG("Request header end found.\n");
        analyzer_ptr->request_header_end_found = TRUE;
        analyzer_ptr->request_found = FALSE;

        if (analyzer_ptr->request_processing_post_request &&
            analyzer_ptr->request_content_length_found)
        {
          data_len = (uint16)((data_ptr+data_len)-begin_ptr);
        }
      }
    }
    else
    {
      QCRIL_LOG_DEBUG("Request header end found.\n");
      analyzer_ptr->request_header_end_found = TRUE;
      analyzer_ptr->request_found = FALSE;

      if (analyzer_ptr->request_processing_post_request &&
          analyzer_ptr->request_content_length_found)
      {
        data_len = (uint16)((data_ptr+data_len)-begin_ptr);
      }
    }

    /* Free line_ptr as it is no longer needed */
    qcril_free(line_ptr);
    line_ptr = NULL;
  }/*end while*/

  /* Free request_buffer and merged_data_buffer before we return, if they
     haven't already been freed */
  if (analyzer_ptr->request_buffer != NULL)
  {
    qcril_free(analyzer_ptr->request_buffer);
    analyzer_ptr->request_buffer = NULL;
  }

  if (merged_data_buffer != NULL)
  {
    qcril_free(merged_data_buffer);
    merged_data_buffer = NULL;
  }

  QCRIL_LOG_DEBUG("At end of process_rx. Printing analyzer state: \n");
  qcril_scws_opt_print_analyzer_state(analyzer_ptr);
}/* end qcril_scws_opt_process_rx */


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
  qcril_scws_opt_traffic_analyzer_type * analyzer_ptr,
  const uint8                          * data_ptr,
  uint16                                 data_len)
{
  uint16   i                          = 0;
  uint8  * begin_ptr                  = NULL;
  uint8  * merged_data_buffer         = NULL;
  uint16   merged_data_buffer_len     = 0;

  /* Check parameters */
  if (analyzer_ptr == NULL ||
      data_ptr == NULL     ||
      data_len <= 0)
  {
    /* Free response buffer in case it has
       been allocated in a previous call to
       process_tx. response_buffer_len is reset
       in the handle_error function. */
    if (analyzer_ptr != NULL &&
        analyzer_ptr->response_buffer != NULL)
    {
      qcril_free(analyzer_ptr->response_buffer);
      analyzer_ptr->response_buffer = NULL;
    }
    QCRIL_LOG_DEBUG("%s, Invalid parameters to process_tx, entering error state.\n",
           __FUNCTION__);
    qcril_scws_opt_handle_error(analyzer_ptr);
    return FALSE;
  }

  /* Check analyzer state. */
  if (analyzer_ptr->analyzer_state == QCRIL_SCWS_OPT_ANALYZER_STATE_ERROR)
  {
    /* If analyzer has entered error state, return false. No
       need to free response_buffer; has already been freed
       if the state of the analyzer is error. */
    QCRIL_LOG_DEBUG("%s, Analyzer state: ERROR. Returning false.\n",
                    __FUNCTION__);
    return FALSE;
  }

  if (analyzer_ptr->analyzer_state == QCRIL_SCWS_OPT_ANALYZER_STATE_IDLE)
  {
    /* If the analyzer state is in idle, set analyzer state to
       ERROR and proceed. No need to check for or free response
       buffer here since it is initialized to NULL if analyzer is
       in IDLE state. */
    QCRIL_LOG_DEBUG("%s, Analyzer state: IDLE. Entering error state, returning false.\n",
                    __FUNCTION__);
    analyzer_ptr->analyzer_state =
        QCRIL_SCWS_OPT_ANALYZER_STATE_ERROR;
    return FALSE;
  }

  /* If there is any leftover data from the last call to process_tx,
     concatenate to the beginning of the data for this call. */
  if (!analyzer_ptr->response_header_end_found &&
      analyzer_ptr->response_buffer_len > 0 &&
      analyzer_ptr->response_buffer != NULL)
  {
    merged_data_buffer_len =
      (uint16)((analyzer_ptr->response_buffer_len+data_len)*sizeof(uint8));
    merged_data_buffer =
      qcril_malloc(merged_data_buffer_len);

    if (merged_data_buffer == NULL)
    {
      QCRIL_LOG_DEBUG("Memory allocation failed, entering error state.\n");

      /* Free response_buffer in case it was allocated
         in a previous call to process_tx; response_buffer_len
         is reset in the handle_error function. */
      qcril_free(analyzer_ptr->response_buffer);
      analyzer_ptr->response_buffer = NULL;

      qcril_scws_opt_handle_error(analyzer_ptr);
      return FALSE;
    }

    memcpy(merged_data_buffer,
           analyzer_ptr->response_buffer,
           analyzer_ptr->response_buffer_len);

    memcpy(merged_data_buffer + analyzer_ptr->response_buffer_len,
           data_ptr,
           data_len);

    data_ptr = merged_data_buffer;
    data_len = (int)analyzer_ptr->response_buffer_len + data_len;

    /* Free response buffer and set it to NULL
       because it is no longer needed after concatenation */
    qcril_free(analyzer_ptr->response_buffer);
    analyzer_ptr->response_buffer = NULL;
    analyzer_ptr->response_buffer_len = 0;
  }

  /* Loop through the response data to find necessary header
     information and determine when all data has finished sending. */
  begin_ptr = (uint8*)data_ptr;
  while(begin_ptr <= data_ptr + data_len)
  {
    /*=====================================================
       HEADER END NOT FOUND.
       Mine information from the response header,
       if it hasn't been done already.
    =======================================================*/
    if (!analyzer_ptr->response_header_end_found)
    {
      /* get the next line */
      char* line_ptr = NULL;
      size_t line_len = 0;
      line_ptr = qcril_scws_opt_get_line(&begin_ptr,
                                     (size_t)((data_ptr+data_len)-begin_ptr),
                                     &line_len);
      if (line_ptr != NULL)
      {
        /* Search the current line for the response status code and verify
           that this is an HTTP message response, if this hasn't been
           done already for the current message. If the message is not
           an HTTP message, enter error state. */
        if (!analyzer_ptr->response_status_code_found)
        {
          char* status_code_ptr = NULL;
          status_code_ptr = strcasestr(line_ptr, QCRIL_SCWS_HTTP);

          /* Parse response status code. Distance from HTTP
             pointer is hard coded as this is mandated in the
             protocol (see RFC 2616, section 6) */
          if (status_code_ptr == NULL ||
              line_len < 12           ||
              !qcril_scws_opt_isnum(status_code_ptr[9])  ||
              !qcril_scws_opt_isnum(status_code_ptr[10]) ||
              !qcril_scws_opt_isnum(status_code_ptr[11]) ||
              strncmp(line_ptr, status_code_ptr, QCRIL_SCWS_HTTP_LEN) != 0)
          {

            /* Status code not found - message is not an HTTP message, and
               so cannot be optimized. Enter error state. */
            QCRIL_LOG_DEBUG("ERROR: Not an HTTP message. Enter error state.\n");

            /* Check merged_data_buffer, response_buffer,
               and line_ptr to avoid memory leaks; they are no
               longer needed once error state is entered. */
            if (merged_data_buffer != NULL)
            {
              qcril_free(merged_data_buffer);
              merged_data_buffer = NULL;
            }

            if (line_ptr != NULL)
            {
              qcril_free(line_ptr);
              line_ptr = NULL;
            }

            /* Length of buffer is reset in handle_error */
            if (analyzer_ptr->response_buffer != NULL)
            {
              qcril_free(analyzer_ptr->response_buffer);
              analyzer_ptr->response_buffer = NULL;
            }

            qcril_scws_opt_handle_error(analyzer_ptr);
            return FALSE;
          }

          /* Status code found; store it and continue */
          analyzer_ptr->response_status_code =
            (status_code_ptr[9] - '0') * 100 +
            (status_code_ptr[10] - '0') * 10 +
            (status_code_ptr[11] - '0');

          analyzer_ptr->response_status_code_found = TRUE;
          QCRIL_LOG_DEBUG("Response Status Code: %d \n",
                 analyzer_ptr->response_status_code);

          /* Since we've found the information we want from this line,
             free line_ptr and continue to the next line */
          qcril_free(line_ptr);
          line_ptr = NULL;
          continue;
        }/* end status code search */

        /* Search the current line for the Content-Length of this response and
           store it, if these things haven't been done already. Account for cases
           with no body and cases where the entire message can be processed in a
           single call to process_tx.*/
        if (!analyzer_ptr->response_content_length_found &&
            !analyzer_ptr->response_transfer_encoding_chunked_found)
        {
          char* content_length_ptr = NULL;
          content_length_ptr = strcasestr(line_ptr, QCRIL_SCWS_CONTENT_LENGTH);

          /* If the Content-Length header field is found, parse the content_length.*/
          if (content_length_ptr != NULL &&
              strncmp(line_ptr, content_length_ptr, QCRIL_SCWS_CONTENT_LENGTH_LEN) == 0)
          {
            uint16 i = 0;
            for (i = QCRIL_SCWS_CONTENT_LENGTH_LEN; i < line_len; i++)
            {
              if (qcril_scws_opt_isnum(content_length_ptr[i]))
              {
                analyzer_ptr->response_content_length =
                  (analyzer_ptr->response_content_length*10) + (content_length_ptr[i] - '0');
              }
            }

            if (analyzer_ptr->response_content_length < 0)
            {
              QCRIL_LOG_DEBUG("Invalid Content-Length, entering error state.\n");

              /* Check merged_data_buffer, line_ptr, and response_buffer
                 to avoid memory leaks; they are no
                 longer needed once error state is entered. */
              if (merged_data_buffer != NULL)
              {
                qcril_free(merged_data_buffer);
                merged_data_buffer = NULL;
              }

              if (line_ptr != NULL)
              {
                qcril_free(line_ptr);
                line_ptr = NULL;
              }

              /* Length of response buffer is reset in handle_error */
              if (analyzer_ptr->response_buffer != NULL)
              {
                qcril_free(analyzer_ptr->response_buffer);
                analyzer_ptr->response_buffer = NULL;
              }

              qcril_scws_opt_handle_error(analyzer_ptr);
              return FALSE;
            }

            analyzer_ptr->response_content_length_found = TRUE;
            QCRIL_LOG_DEBUG("Response Content-Length: 0x%x\n",
                   analyzer_ptr->response_content_length);

            /* Since we've found the information we want from this line,
               free line_ptr and continue to the next line */
            qcril_free(line_ptr);
            line_ptr = NULL;
            continue;
          }
        }/* end content-length search */

        /* Search for the header field Transfer-Encoding: Chunked.
           First obtain a pointer to the occurence of Transfer-Encoding, using
           a case-insensitive comparison, then compare the line and the string
           from that pointer to ensure "Transfer-Encoding:" appears at the
           beginning of the line (RFC 2616 sections 4.1 and 4.2). After doing this,
           find a pointer to "chunked" in the line using case-insensitive comparison. */
        if (!analyzer_ptr->response_transfer_encoding_chunked_found)
        {
          char* transfer_encoding_ptr = NULL;
          transfer_encoding_ptr = strcasestr(line_ptr, QCRIL_SCWS_TRANSFER_ENCODING);

          if (transfer_encoding_ptr != NULL &&
              strncmp(line_ptr, transfer_encoding_ptr, QCRIL_SCWS_TRANSFER_ENCODING_LEN) == 0)
          {
             char* chunked_ptr = NULL;
             chunked_ptr = strcasestr(transfer_encoding_ptr, QCRIL_SCWS_CHUNKED);

             if (chunked_ptr != NULL)
             {
               analyzer_ptr->response_transfer_encoding_chunked_found = TRUE;

               qcril_free(line_ptr);
               line_ptr = NULL;
               continue;
               QCRIL_LOG_DEBUG("Transfer-Encoding: Chunked \n");
             }
          }
        }/* end search for Transfer-Encoding: Chunked */

        /* If we find both a Content-Length field and a
           Transfer-Encoding, we must ignore the Content-Length.
           (see RFC 2616, section 4.4) */
        if (analyzer_ptr->response_transfer_encoding_chunked_found &&
            analyzer_ptr->response_content_length_found)
        {
          analyzer_ptr->response_content_length = 0;
          analyzer_ptr->response_content_length_found = FALSE;
        }

        /* Search for the end of the response header, if
           this hasn't been found already for this message. The
           header ends when the current line contains either
           CRLF or LF.*/
        if (!analyzer_ptr->response_header_end_found)
        {
          if (strncmp(line_ptr, QCRIL_SCWS_LF, QCRIL_SCWS_LF_LEN)   == 0 ||
              strncmp(line_ptr, QCRIL_SCWS_CRLF, QCRIL_SCWS_CRLF_LEN) == 0)
          {
            analyzer_ptr->response_header_end_found = TRUE;

            /* Subtract the length of the header from the data_len
               if the Content-Length field is present and has been found.
               From here, we can decrement the Content-Length by the data_len
               in each call to process_tx until it equals 0, signifying the
               end of the message. */
            if (analyzer_ptr->response_content_length_found)
            {
              data_len = (uint16)((data_ptr+data_len)-begin_ptr);
            }
            QCRIL_LOG_DEBUG("Found response header end. \n");
          }
        }/* End search for end of header */

        /* Free line_ptr as it is no longer needed after this point. */
        if (line_ptr != NULL)
        {
          qcril_free(line_ptr);
          line_ptr = NULL;
        }
      }/*end check for if line_ptr is null*/
      else
      {
        /* The line_ptr is null. Either HTTP message is malformed, or split across
           packets. check where begin_ptr is in relation to data_ptr+data_len
           and store that many bytes in a buffer, then concatenate this to the
           next packet that comes through. */
        analyzer_ptr->response_buffer_len = (data_ptr+data_len)-begin_ptr;
        analyzer_ptr->response_buffer =
          qcril_malloc(analyzer_ptr->response_buffer_len * sizeof(uint8));

        /* If the memory allocation is successful, copy remaining data
           into temporary buffer and return false, so the data is processed
           in the next call to process_tx. If the memory allocation fails,
           enter error state. */
        if (analyzer_ptr->response_buffer == NULL)
        {
          QCRIL_LOG_DEBUG("Memory allocation failed, entering error state.\n");

          /* Free merged_data_buffer */
          if (merged_data_buffer != NULL)
          {
            qcril_free(merged_data_buffer);
            merged_data_buffer = NULL;
          }

          qcril_scws_opt_handle_error(analyzer_ptr);
          return FALSE;
        }

        QCRIL_LOG_DEBUG("%s, Allocated memory for response_buffer\n", __FUNCTION__);
        memcpy(analyzer_ptr->response_buffer, begin_ptr, analyzer_ptr->response_buffer_len);

        /* Free merged_data_buffer */
        if (merged_data_buffer != NULL)
        {
          qcril_free(merged_data_buffer);
          merged_data_buffer = NULL;
        }

        return FALSE;
      }
    }/*endif (!header_end_found)*/

    /*=====================================================
       HEADER END FOUND.
       Is a separate if statement; this typically needs
       to be executed in addition to the case where the header
       end is not found, so cannot be placed in an else.
    =======================================================*/
    if(analyzer_ptr->response_header_end_found)
    {
      /* Check merged_data_buffer and response_buffer and free them
         if necessary, as all paths in this case lead to a return and
         these buffers are no longer needed.
         Length of response buffer is reset in the handle_error function */
      if (merged_data_buffer != NULL)
      {
        qcril_free(merged_data_buffer);
        merged_data_buffer = NULL;
      }

      if (analyzer_ptr->response_buffer != NULL)
      {
        qcril_free(analyzer_ptr->response_buffer);
        analyzer_ptr->response_buffer = NULL;
      }

      /* All response headers should have a
         status code. If not found, go to error state. */
      if (!analyzer_ptr->response_status_code_found)
      {
        QCRIL_LOG_DEBUG("ERROR: Malformed data.(HTTP message must have status code.)\n");
        QCRIL_LOG_DEBUG("Entering error state.\n");

        qcril_scws_opt_handle_error(analyzer_ptr);
        return FALSE;
      }

      /* If the message does not have a body, we have processed all data and
         may switch to the next available socket. Handle request tracking
         and return TRUE. */
      if ((QCRIL_SCWS_HTTP_100 <= analyzer_ptr->response_status_code &&
            analyzer_ptr->response_status_code < QCRIL_SCWS_HTTP_200) ||
          (analyzer_ptr->response_status_code == QCRIL_SCWS_HTTP_204) ||
          (analyzer_ptr->response_status_code == QCRIL_SCWS_HTTP_304) ||
          (analyzer_ptr->requests_arr[analyzer_ptr->requests_processed]
             == QCRIL_SCWS_HTTP_REQUEST_HEAD))
      {
        analyzer_ptr->requests_processed++;
        if (analyzer_ptr->requests_processed == analyzer_ptr->request_count)
        {
          QCRIL_LOG_DEBUG("%s, Inside no body case, returning TRUE \n",
                  __FUNCTION__);
          qcril_scws_opt_print_analyzer_state(analyzer_ptr);

          qcril_scws_opt_reset(analyzer_ptr);
          return TRUE;
        }
        else if (analyzer_ptr->requests_processed < analyzer_ptr->request_count)
        {
          QCRIL_LOG_DEBUG("%s, Inside no body case, but more requests left - returning FALSE \n",
                 __FUNCTION__);

          qcril_scws_opt_print_analyzer_state(analyzer_ptr);
          qcril_scws_opt_reset_requests_pending(analyzer_ptr);
          return FALSE;
        }
        else
        {
          QCRIL_LOG_DEBUG("%s, Inside no body case, but requests_processed > request_count.\n",
                 __FUNCTION__);
          QCRIL_LOG_DEBUG("%s, Enterting error state, returning FALSE \n", __FUNCTION__);

          qcril_scws_opt_handle_error(analyzer_ptr);
          return FALSE;
        }
      }/* End if: handling of case for responses without message bodies. */
      else
      {
        if (analyzer_ptr->response_content_length_found)
        {
          QCRIL_LOG_DEBUG("%s, content_length before subtraction = %d, data_len = %d \n",
                 __FUNCTION__, analyzer_ptr->response_content_length, data_len);

          analyzer_ptr->response_content_length -= data_len;
          if (analyzer_ptr->response_content_length > 0)
          {

            QCRIL_LOG_DEBUG("%s, Inside >0 case, returning FALSE \n",
                      __FUNCTION__, analyzer_ptr->response_content_length);
            QCRIL_LOG_DEBUG("content length after subtraction: %d \n", analyzer_ptr->response_content_length);
            return FALSE;
          }
          else if(analyzer_ptr->response_content_length < 0)
          {
            QCRIL_LOG_DEBUG("Error: Content-Length is less than zero. Entering error state.\n");
            qcril_scws_opt_handle_error(analyzer_ptr);
            return FALSE;
          }
          else
          {
            /* End of content reached - check request count to see
               if we can switch to the next socket or if we have
               more requests on the current socket */
            analyzer_ptr->requests_processed++;
            if (analyzer_ptr->requests_processed == analyzer_ptr->request_count)
            {
              QCRIL_LOG_DEBUG("content length after subtraction: %d \n", analyzer_ptr->response_content_length);
              QCRIL_LOG_DEBUG("%s, Inside == 0 case, returning TRUE \n",
                      __FUNCTION__);
              qcril_scws_opt_print_analyzer_state(analyzer_ptr);
              qcril_scws_opt_reset(analyzer_ptr);
              return TRUE;
            }
            else if (analyzer_ptr->requests_processed < analyzer_ptr->request_count)
            {
              QCRIL_LOG_DEBUG("content length after subtraction: %d \n", analyzer_ptr->response_content_length);
              QCRIL_LOG_DEBUG("Inside == 0 case, but %d requests left to process. Returning FALSE.\n",
                    analyzer_ptr->request_count - analyzer_ptr->requests_processed);

              qcril_scws_opt_reset_requests_pending(analyzer_ptr);
              return FALSE;
            }
            else
            {
              /* requests processed should never be greater than request count */
              QCRIL_LOG_DEBUG("ERROR - requests_processed > request_count. Entering ERROR state.\n");
              qcril_scws_opt_handle_error(analyzer_ptr);
              return FALSE;
            }
          }
        }/* end if: content_length_found */
        else if (analyzer_ptr->response_transfer_encoding_chunked_found)
        {
          /* Search for final chunk of transfer encoding. NOTE: This
             implementation is not optimal - it will not catch
             every case, because we don't find the size of each chunk
             and use it to tell when we've reached the end. Additonally,
             this implementation will fail if the FINAL_CHUNK_CRLF string
             or the FINAL_CHUNK_LF string is split across two packets - no
             support is provided for this case. In the case of a false
             negative (final chunk is missed), the optimization is simply
             not performed. In case of a false positive (perceived final
             chunk that is not really a final chunk) the socket switch
             will occur too soon and the page will not load properly. */
          uint8* final_ptr = NULL;
          final_ptr = (uint8*) memmem(begin_ptr,
                                           (data_ptr+data_len)-begin_ptr,
                                           QCRIL_SCWS_FINAL_CHUNK_CRLF,
                                           QCRIL_SCWS_FINAL_CHUNK_CRLF_LEN);
          if (final_ptr == NULL)
          {
            final_ptr = (uint8*) memmem(begin_ptr,
                                             (data_ptr+data_len)-begin_ptr,
                                             QCRIL_SCWS_FINAL_CHUNK_LF,
                                             QCRIL_SCWS_FINAL_CHUNK_LF_LEN);

            /* Check that final_ptr is not null and ensure that
               it occurs at the end of the current data packet */
            if (final_ptr != NULL &&
                (final_ptr+QCRIL_SCWS_FINAL_CHUNK_LF_LEN) == (data_ptr+data_len))
            {
              analyzer_ptr->response_final_chunk_found = TRUE;
            }
            else
            {
              /* analyzer_ptr->response_final_chunk_found == FALSE, return*/
              QCRIL_LOG_DEBUG("%s, This chunk is not the final chunk, returning FALSE.\n",
                              __FUNCTION__);
              return FALSE;
            }
          }
          else if ((final_ptr+QCRIL_SCWS_FINAL_CHUNK_CRLF_LEN) == (data_ptr+data_len))
          {
            analyzer_ptr->response_final_chunk_found = TRUE;
          }
          else
          {
            /* analyzer_ptr->response_final_chunk_found == FALSE, return*/
            QCRIL_LOG_DEBUG("%s, This chunk is not the final chunk, returning FALSE.\n",
                              __FUNCTION__);
            return FALSE;
          }
          /* end search for final chunk */

         if (analyzer_ptr->response_final_chunk_found)
         {
            analyzer_ptr->requests_processed++;
            if (analyzer_ptr->requests_processed == analyzer_ptr->request_count)
            {
              QCRIL_LOG_DEBUG("%s, Final Chunk found, returning TRUE \n",
                      __FUNCTION__);
              qcril_scws_opt_print_analyzer_state(analyzer_ptr);
              qcril_scws_opt_reset(analyzer_ptr);
              return TRUE;
            }
            else if (analyzer_ptr->requests_processed < analyzer_ptr->request_count)
            {
              QCRIL_LOG_DEBUG("Final Chunk found, but %d requests left to process. Returning FALSE.\n",
                    analyzer_ptr->request_count - analyzer_ptr->requests_processed);

              qcril_scws_opt_reset_requests_pending(analyzer_ptr);
              return FALSE;
            }
            else
            {
              /* requests processed should never be greater than request count */
              QCRIL_LOG_DEBUG("ERROR - requests_processed > request_count. Entering error state.\n");
              qcril_scws_opt_handle_error(analyzer_ptr);
              return FALSE;
            }
          }
        } /* end handling of transfer-encoding: chunked cases */
        else
        {
          /* Message has body, but neither content length nor
             transfer encoding found. Cannot optimize -
             enter error state. */

          QCRIL_LOG_DEBUG("ERROR - Message has body, but no content length or transfer-encoding.\n");
          QCRIL_LOG_DEBUG("Cannot optmize, entering error state.\n");
          qcril_scws_opt_handle_error(analyzer_ptr);
          return FALSE;
        }
      }
    }
  } /*end while*/

  /* Free merged_data_buffer */
  if(merged_data_buffer != NULL)
  {
    qcril_free(merged_data_buffer);
    merged_data_buffer = NULL;
  }

  /* Free response buffer if necessary */
  if (analyzer_ptr->response_buffer != NULL)
  {
    qcril_free(analyzer_ptr->response_buffer);
    analyzer_ptr->response_buffer = NULL;
  }

  QCRIL_LOG_DEBUG("%s, Error - reading past end of packet. Entering error state.\n",
                  __FUNCTION__);
  qcril_scws_opt_handle_error(analyzer_ptr);

  return FALSE;
}/* end qcril_scws_opt_process_tx */

