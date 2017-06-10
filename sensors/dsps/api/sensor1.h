#ifndef SENSOR1_H_
#define SENSOR1_H_

/*============================================================================*/
/**
  @file sensor1.h

  The sensor1 API is used to communicate with the sensors framework. It is
  designed to provide access for clients running in the same process context
  as the sensors framework.

  All clients communicate with the framework via function calls with a
  client handle, and receive responses and indications via registered callback
  functions.
*/

/*============================================================================
  Copyright (c) 2010, 2012-2014 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Confidential and Proprietary
  ============================================================================*/

/*============================================================================
  INCLUDE FILES
  ============================================================================*/

#include <stdint.h>

/*============================================================================
  Preprocessor Definitions and Constants
============================================================================*/

#define SENSOR1_Q16_FROM_FLOAT( q16_output, float_input )   \
  ( (q16_output) = (uint32_t)((float_input)*65536) )
#define SENSOR1_FLOAT_FROM_Q16( float_output, q16_input )   \
  ( (float_output) = ((float)(q16_input))/65536.0 )

#ifndef TRUE
  #define TRUE 1
#endif /* TRUE */
#ifndef FALSE
  #define FALSE 0
#endif /* FALSE */
#ifndef NULL
  #define NULL ((void*)0)
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*============================================================================
   Type Declarations
============================================================================*/

/**
 * Defines the errors used in the sensor1 API.
 */
typedef enum sensor1_error_e {

  SENSOR1_SUCCESS, /**< No error */

  SENSOR1_EBUFFER, /**< Invalid buffer */

  SENSOR1_ENOMEM, /**< Insufficient memory to process request */

  SENSOR1_EINVALID_CLIENT, /**< Invalid client handle */

  SENSOR1_EUNKNOWN, /**< Unknown error reason */

  SENSOR1_EFAILED, /**< Generic error */

  SENSOR1_ENOTALLOWED, /**< Operation not allowed at this time */

  SENSOR1_EBAD_PARAM, /**< Invalid parameter value */

  SENSOR1_EBAD_PTR, /**< Invalid pointer. May be due to NULL values, or
                       pointers to memory not allocated with
                       {@link sensor1_alloc_msg_buf}. */

  SENSOR1_EBAD_MSG_ID, /**< The service specified does not support this message
                          ID */

  SENSOR1_EBAD_MSG_SZ, /**< The message size does not match the size of the
                         message determined  by the service/message ID */

  SENSOR1_EWOULDBLOCK, /**< Function was not successful, and would need to
                          block to complete successfully. The client should
                          retry at a later time. */

  SENSOR1_EBAD_SVC_ID /**< Unknown service ID/service number */

} sensor1_error_e;

/**
 * Defines the types of response messages
 */
typedef enum sensor1_msg_type_e {
  SENSOR1_MSG_TYPE_REQ,  /**< Request */
  SENSOR1_MSG_TYPE_RESP, /**< Response to a request */
  SENSOR1_MSG_TYPE_IND,  /**< Asynchronous indication */
  SENSOR1_MSG_TYPE_RESP_INT_ERR, /**< Error response due to internal error.
                                    Request failed. The associated msg data is
                                    not a valid response. The msg header should
                                    be used to identify the failed REQ. */

  SENSOR1_MSG_TYPE_BROKEN_PIPE = 250,
  /**< This "message type" indicates that the
     message pipe to the sensors has been broken,
     and the associated client handle is no longer
     usable. The client should call sensor1_close()
     to free the client handle. */
  SENSOR1_MSG_TYPE_RETRY_OPEN = 251
  /**< This "message type" will be used if sensor1_open returns
     SENSOR1_WOULDBLOCK. This indicates that the sensor client may now retry
     calling sensor1_open to get a valid client handle. */
} sensor1_msg_type_e;


/**
 * An opaque handle provided by the sensor1 API.
 */
typedef struct sensor1_handle_s sensor1_handle_s;

/**
 * The message header. Used in both incoming and outgoing messages
 */
typedef struct sensor1_msg_header_s {
  uint32_t service_number; /**< The QMI service number, as defined by the
                              service */

  int32_t msg_id; /**< Message ID, as defined by the service */

  uint16_t msg_size; /**< Size, in bytes, of the c-structure representing the
                        message */
  uint8_t txn_id; /**< A transaction ID defined by the client in the write
                     request in sensor1_write().  It may be used by client to
                     identify which Request caused this Response. In the notify
                     data callback, the txn_id field is only valid for msg_type
                     == SENSOR1_MSG_TYPE_RESPONSE and
                     SENSOR1_MSG_TYPE_RESP_INT_ERR. Indications use transaction
                     ID = 0. */

} sensor1_msg_header_s;


/**
   @brief This type defines a function pointer. A function of this type shall be
   implemented by the client to handle received data.

   A typical implementation will copy the received data into an IPC queue to
   pass to another process or thread.

   When the client has processed or copied the received data, it shall free the
   msg_ptr using {@link sensor1_free_msg_buf}.

   The msg_hdr need not be freed, and the pointer to the msg_hdr will not be
   valid after this function returns. If the client needs the msg_hdr
   information after returning from the callback, the client will need to copy
   the data.

   @note This callback can be called from a thread internal to the sensor
   framework. Any processing done in this thread can impact performance of the
   entire sensor subsystem. Thus, the implementation of the client callback
   should be as efficient as possible - ideally it will only signal a client
   thread using an OS specific mechanism that data has arrived.

   @see sensor1_open
   @see sensor1_free_msg_buf
   @see sensor1_write
   @see sensor1_msg_header_s

   @param cb_data: [i] The unmodified value passed in when the callback was
            registered in {@link sensor1_open}.
   @param msg_hdr: [i] Message header defining the message.
   @param msg_type: [i] Type of message
   @param msg_ptr: [i] A pointer to the QMI-based message. These messages are
            defined in their respective service header files. The client shall
            free this pointer via {@link sensor1_free_msg_buf}. Note that
            the msg_ptr will only be valid for msg_type SENSOR1_MSG_TYPE_RESP
            and SENSOR1_MSG_TYPE_IND.
*/
typedef void (*sensor1_notify_data_cb_t) (intptr_t cb_data,
                                          sensor1_msg_header_s *msg_hdr,
                                          sensor1_msg_type_e msg_type,
                                          void *msg_ptr);


/**
   @brief This type defines a function pointer. A function of this type shall be
   implemented by the client to handle a writable indication.

   A typical implementation will signal a client thread to handle this
   condition.

   @note This callback can be called from a thread internal to the sensor
   framework. Any processing done in this thread can impact performance of the
   entire sensor subsystem. Thus, the implementation of the client callback
   should be as efficient as possible - ideally it will only signal a client
   thread using an OS specific mechanism that data can be written.

   @see sensor1_writable

   @param cb_data: [i] The unmodified value passed in when the callback was
            registered in {@link sensor1_writable}.
   @param service_number: [i] The Service ID this callback refers to.
*/
typedef void (*sensor1_write_cb_t) (intptr_t cb_data,
                                    uint32_t service_number);

/*===========================================================================
  Function Declarations and Documentation
  ===========================================================================*/

/*===========================================================================

  FUNCTION:   sensor1_open

  ===========================================================================*/
/**
   @brief This function registers an external client with the sensors framework.
   It provides a Client Handle which will be used for all future communication
   with the sensor framework.

   This registers a callback with the sensor framework. The callback will be
   called when received data is available. All received data will come in the
   form of a message Response or Indication. Data will be pushed via callback
   parameters, and the data should be consumed before the callback returns. Only
   one callback per sensor1 connection can be registered.

   @note Callbacks will be called from a thread internal to the sensor
   framework. Any processing done in this thread can impact performance of other
   clients and the entire sensor subsystem. Thus, the implementation of the
   client callback should be as efficient as possible - ideally it will only
   copy the data and signal a client thread using an OS specific mechanism.


   @param hndl: [o] An opaque handle used to identify this client
   @param data_cbf: [i] A pointer to the client's callback function to process
            received data.
   @param cb_data: [i] This data will be passed unmodified to the callback
            function data_cbf. It may be used by the client for any purpose,
            and will not be used by the sensor1 API.

   @return
     - SENSOR1_SUCCESS: client successfully registered
     - SENSOR1_ENOMEM: Insufficient memory to process this request. The "hndl"
       parameter is not valid.
     - SENSOR1_EWOULDBLOCK: The connection to the sensor framework cannot be
       opened at this time. The "hndl" parameter is not valid. The registered
       callback will be called at a later time with message type
       SENSOR1_MSG_TYPE_RETRY_OPEN to indicate that sensor1_open may now
       succeed.
     - All other values indicate an error has occurred, and the "hndl"
       parameter is not valid.

*/
sensor1_error_e
sensor1_open( sensor1_handle_s **hndl,
              sensor1_notify_data_cb_t data_cbf,
              intptr_t cb_data );

/*===========================================================================

  FUNCTION:   sensor1_close

  ===========================================================================*/
/**
   @brief This function de-registers a client, and releases the client handle.
   The client handle shall not be used after calling {@link sensor1_close}.

   Before calling {@link sensor1_close} a client may deregister from any
   services it is using. Each service (and that service's messages) will be
   defined in a separate document dedicated to that service. If the client does
   not explicitly deregister from services, the sensor1 framework will do so.

   @note Registered callback functions may be called while the client is in the
   process of closing, and after this function has returned. Thus, clients should
   take care to insure their registered callback function will execute properly
   in these cases.

   @param hndl: An opaque handle used to identify this client

   @return
     - SENSOR1_SUCCESS: client successfully registered
     - SENSOR1_EINVALID_CLIENT: Invalid client handle
*/
sensor1_error_e
sensor1_close( sensor1_handle_s *hndl );

/*===========================================================================

  FUNCTION:   sensor1_write

  ===========================================================================*/
/**
   @brief This function writes a request message into the sensor framework. All
   requests come in the form of c-types request message structures generated by
   the QMI IDL. QMI request messages will be defined in separate files.

   @note Message buffers shall be allocated using
   {@link sensor1_alloc_msg_buf}. The sensor framework will free the data once
   it has been successfully processed. The client shall not free the message
   data if the return value is SENSOR1_SUCCESS. If there is an error with the
   write, the client shall free the message (or retransmit it at a later time).

   @see sensor1_writable
   @see sensor1_alloc_msg_buf
   @see sensor1_msg_header_s

   @param hndl: [i] An opaque handle to identify this client
   @param msg_hdr: [i] Header for defining the message
   @param msg_ptr: [i] A pointer to the QMI-based request message. These
            messages are defined in their respective service header files. The
            memory pointed to shall be allocated by
            {@link sensor1_alloc_msg_buf}.

   @return
     - SENSOR1_SUCCESS: the message has been sent and will be deallocated
         (freed) by the sensor framework. A reply will be generated later.
     - SENSOR1_EWOULDBLOCK: the message queue is full, and no more messages may
         be sent. Try again later.
     - SENSOR1_EBAD_PTR: Message memory was not allocated using
         {@link sensor1_alloc_msg_buf}.
     - SENSOR1_EBAD_MSG_SZ: The message size does not match the size required
         for that message ID.
     - SENSOR1_EBAD_MSG_ID: unknown message ID for this service.
     - SENSOR1_EBAD_SVC_ID: unknown or unsupported service number.
*/
sensor1_error_e
sensor1_write( sensor1_handle_s     *hndl,
               sensor1_msg_header_s *msg_hdr,
               void                 *msg_ptr );

/*===========================================================================

  FUNCTION:   sensor1_writable

  ===========================================================================*/
/**
   @brief This function registers a callback with the sensor framework. The
   callback will be called when the framework may have buffer space available to
   process a call to {@link sensor1_write} to the specified service ID.

   Only one callback per client per service ID can be registered. Additional
   calls to {@link sensor1_writable} will overwrite any previously registered
   callbacks with the same service ID.

   The callback function will only be called once, and then the sensor framework
   will de-register the callback. If the client wishes to be notified again for
   the same service ID, it must call {@link sensor1_writable} again to
   re-register the callback.

   Typically, this function will be called (and the callback registered) when
   {@link sensor1_write} returns with the error code SENSOR1_EWOULDBLOCK.

   If the specified service ID is already writable, it is possible that the
   callback will be called directly from the implementation of {@link
   sensor1_writable}.


   @note The callback can be called from a thread internal to the sensor
   framework. Any processing done in this thread can impact performance of the
   entire sensor subsystem. Thus, the implementation of the client callback
   should be as efficient as possible - ideally it will only signal a client
   thread using an OS specific mechanism that writing is now possible.

   @param hndl: [i] An opaque handle to identify this client.
   @param cbf: [i] A pointer to the client's callback function.
   @param cb_data: [i]This data is set by the client, and will be passed
            unmodified as a parameter to the callback function.
   @param service_number: [i] The client callback function will be called when
            it is possible that this Service ID can accept new Requests via
            {@link sensor1_write}.

   @return
     - SENSOR1_SUCCESS
     - SENSOR1_EINVALID_CLIENT
*/
sensor1_error_e
sensor1_writable( sensor1_handle_s  *hndl,
                  sensor1_write_cb_t cbf,
                  intptr_t           cb_data,
                  uint32_t           service_number );

/*===========================================================================

  FUNCTION:   sensor1_alloc_msg_buf

  ===========================================================================*/
/**
   @brief This function uses the sensor framework to allocate a message buffer.
   All messages written using {@link sensor1_write} shall be allocated using
   this function.

   @param hndl: [i] An opaque handle to identify this client.
   @param size: [i] The size of the message structure
   @param buffer: [o] Address of a pointer to the memory address where the
            message should be placed.

   @return
     - SENSOR1_SUCCESS: a buffer has been allocated, and buffer points to the
         allocated data region.
     - SENSOR1_EINVALID_CLIENT: Invalid client handle.
     - SENSOR1_ENOMEM: Insufficient memory to process this request.
*/
sensor1_error_e
sensor1_alloc_msg_buf(sensor1_handle_s *hndl,
                      uint16_t          size,
                      void            **buffer );


/*===========================================================================

  FUNCTION:   sensor1_free_msg_buf

  ===========================================================================*/
/**
   @brief This function uses the sensor framework to free a message buffer. All
   messages received via the registered notification callback shall free the
   message using this function.

   @param hndl: [i] An opaque handle to identify this client.
   @param msg_buf: [i] Buffer to free.

   @return
     - SENSOR1_SUCCESS: the specified buffer has been de-allocated.

   Any other error code indicates that the buffer has NOT been de-allocated.
     - SENSOR1_EINVALID_CLIENT: Invalid client handle.
     - SENSOR1_EBAD_PTR: Invalid memory pointer.
*/
sensor1_error_e
sensor1_free_msg_buf(sensor1_handle_s *hndl,
                     void             *msg_buf );

/*===========================================================================

  FUNCTION:   sensor1_init

  ===========================================================================*/
/**
   @brief Initialize the sensor framework. This shall be called only once by
   one thread. Each client should not call this function.

   @warn This function should only be called once!

   @return
     - SENSOR1_SUCCESS if successfull.
     - Otherwise an error
*/
sensor1_error_e
sensor1_init( void );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SENSOR1_H_ */
