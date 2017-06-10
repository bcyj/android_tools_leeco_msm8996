#ifndef __H_MMI_DEVICE_API_H__
#define __H_MMI_DEVICE_API_H__

/*========================================================================

*//** @file mmiDeviceApi.h

@par FILE SERVICES:
      API for Multimedia Interface in user mode.

      This file contains the description of Command Codes, Data Types and
      Other Constants which abstracts a device in user mode for Multimedia.

      In system where there is kernel and user separation, this can be a
      thin interface layer for kernel access and event waiting.

@par EXTERNALIZED FUNCTIONS:
      See below.

    Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
    Qualcomm Technologies Proprietary. Export of this technology or software is regulated
    by the U.S. Government, Diversion contrary to U.S. law prohibited.

*//*====================================================================== */

/*========================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/OpenMAX/base/rel/1.0/inc/mmiDeviceApi.h#6 $

when       who    what, where, why
--------   ---    -------------------------------------------------------
11/22/10   zm     Add typedefs for function pointers of heap memory management
                  APIs.
10/28/10   zm     Command codes reservation for base class.
09/24/10   zm     Add informational MMI command for stopping device
                  referring a buffer.
09/13/10   dm     Extension indexes reservation for base class.
08/24/10   dm     Add platform private data for allocated buffers.
06/22/10   dm     Add support for stream corruption error.
03/17/10   dm     Allow ETB / FTB asynchronous only.
03/11/10   dm     Add not ready error status.
02/11/10   dm     Add async fatal error event.
11/25/09   dm     Corrected auto port detection behavior.
11/24/09   dm     Added support for device extension indexes.
10/13/09   dm     Removed MMI_S_ENOTSUPP error code.
10/13/09   dm     Added error codes for unsupported setting and index.
08/28/09   dm     Added support for auto pause.
08/06/09   dm     Added extension specific generic event.
07/31/09   dm     Added resource acquired event.
07/28/09   dm     Removed bPreempted flag from resource lost message.
07/22/09   dm     Device should queue the buffers while paused or starting.
07/22/09   dm     Changed file descriptor handle.
07/22/09   dm     Added error NoMore.
07/14/09   dm     Reverted back some changes.
07/13/09   dm     Changed UseBuffer command parameters.
06/25/09   dm     Cleanup.
06/09/09   srk    Cleanup and out pointer in command function.
06/05/09   dm     Added release on wait command. 05/28/09 dm Removed redundant
                  port ids.
05/26/09   dm     Removed dependency from comdef.h. Used OMX defines instead.
05/20/09   dm     Removed compilation errors.
05/19/09   srk    Additional updates after more review comments.
05/07/09   srk    Updates after 05/06/09 meeting comments from Alwyn/Tom
                  others.
05/01/09   srk    Updates after Tom/Alwyn/Scott/Dileep reviewed this file
                  during the OMX workshop conducted in San Diego.
04/17/09   srk    Updates after initial set of review comments.
03/26/09   srk    Created file based on the VEN interface created by Rohit,
                  Bobby and Yogesh.

========================================================================== */

/*========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include <OMX_Types.h>
#include <OMX_Core.h>
#include <OMX_Component.h>

/*========================================================================

                      DEFINITIONS AND DECLARATIONS

========================================================================== */

#if defined( __cplusplus )
extern "C"
{
#endif /* end of macro __cplusplus */


/* -----------------------------------------------------------------------
** Constants & Macros
** ----------------------------------------------------------------------- */


/**
  *
  * These macro define the Status Codes which should be used in following
  * situations to return status of the device operations.
  * a) As device function call return type.
  * b) As status message while invoking the callback function registered by
  *    the base class.
  *
  */

#define MMI_S_BASE         ( 0x20000000 )       /**<
                                                * Base value which need to be
                                                * added in all the status
                                                * codes defined below.
                                                * NOTE: Not to be referred
                                                * directly by the client.
                                                */

#define MMI_S_PENDING      ( MMI_S_BASE )       /**<
                                                * This status code indicates
                                                * successful queueing of the
                                                * command. Command is being
                                                * processed asynchronously by
                                                * the device. Operation
                                                * success or failure upon
                                                * execution would be notified
                                                * in the client callback
                                                * function.
                                                */

#define MMI_S_COMPLETE     ( MMI_S_BASE + 1 )   /**<
                                                * This status code indicates
                                                * successful completion of the
                                                * command.
                                                * A function returns this
                                                * status code to notify client
                                                * about synchronous execution
                                                * and completion of the
                                                * command.
                                                * An asynchronous operation
                                                * returns this status code in
                                                * the callback function to
                                                * notify client about
                                                * successful completion of the
                                                * command.
                                                */

#define MMI_S_EFAIL        ( MMI_S_BASE + 2 )   /**<
                                                * This status code indicates a
                                                * general failure.
                                                */

#define MMI_S_EFATAL       ( MMI_S_BASE + 3 )   /**<
                                                * This status code indicates a
                                                * fatal irrecoverable failure.
                                                * The session need to be tore
                                                * down upon this error.
                                                */

#define MMI_S_EBADPARAM    ( MMI_S_BASE + 4 )   /**<
                                                * This status code is returned
                                                * when bad parameters are
                                                * detected in a function call.
                                                */

#define MMI_S_EINVALSTATE  ( MMI_S_BASE + 5 )   /**<
                                                * This status code is returned
                                                * when a command is called in
                                                * invalid device state.
                                                */

#define MMI_S_ENOSWRES     ( MMI_S_BASE + 6 )   /**<
                                                * This status code indicates
                                                * insufficient OS resources
                                                * e.g. thread, memory etc.
                                                */

#define MMI_S_ENOHWRES     ( MMI_S_BASE + 7 )   /**<
                                                * This status code indicates
                                                * insufficient HW resources
                                                * e.g. core capacity maxed
                                                * out.
                                                */

#define MMI_S_EBUFFREQ     ( MMI_S_BASE + 8 )   /**<
                                                * This status code is used to
                                                * indicate that the buffer
                                                * requirements were not met.
                                                */

#define MMI_S_EINVALCMD    ( MMI_S_BASE + 9 )   /**<
                                                * This status code is used to
                                                * indicate that an invalid
                                                * command is called.
                                                */

#define MMI_S_ETIMEOUT     ( MMI_S_BASE + 10 )  /**<
                                                * This status code indicates
                                                * command timeout.
                                                */

#define MMI_S_ENOREATMPT   ( MMI_S_BASE + 11 )  /**<
                                                * This status code is used to
                                                * indicate that a re-attempt
                                                * was made when multiple
                                                * multiple attempts are not
                                                * supported for the API.
                                                */

#define MMI_S_ENOPREREQ    ( MMI_S_BASE + 12 )  /**<
                                                * This status code is used to
                                                * indicate that the
                                                * pre-requirement is not met
                                                * for the API
                                                */

#define MMI_S_ECMDQFULL    ( MMI_S_BASE + 13 )  /**<
                                                * This status code indicates
                                                * that the command queue is
                                                * full.
                                                */

#define MMI_S_ENOTSUPIDX   ( MMI_S_BASE + 15 )  /**<
                                                * This status code indicates
                                                * that the parameter or config
                                                * indicated by the given index
                                                * is not supported by this
                                                * driver.
                                                */

#define MMI_S_ENOTSUPSET   ( MMI_S_BASE + 16 )  /**<
                                                * This status code indicates
                                                * that the values encapsulated
                                                * in the parameter or config
                                                * structure are not supported
                                                * by this driver.
                                                */

#define MMI_S_ENOTIMPL     ( MMI_S_BASE + 17 )  /**<
                                                * This status code indicates
                                                * that the command is not
                                                * implemented by the driver.
                                                */

#define MMI_S_ENOMORE      ( MMI_S_BASE + 18 )  /**<
                                                * This status code indicates
                                                * that indices are exhausted
                                                * and no more indices can be
                                                * enumerated.
                                                */

#define MMI_S_ENOTREADY    ( MMI_S_BASE + 19 )  /**<
                                                * This status code indicates
                                                * that device is not ready to
                                                * return data this time.
                                                */

#define MMI_S_ESTRMCORRUPT ( MMI_S_BASE + 20 )  /**<
                                                * This status code indicates
                                                * that the stream is found to
                                                * be corrupt.
                                                */

/*--------------------------------------------------------------------------*/



/**
  *
  * These macro define the Message Codes which are used to identify
  * asynchronous message responses and events that device wants to communicate
  * to the component.
  *
  * NOTE:-
  *
  *   _RESP_ - This prefix is added when the message being conveyed is purely
  *            the result of a command invoked by the IL Base class.
  *
  *   _EVT_  - This prefix is added when the message being conveyed is purely
  *            an unsolicited event. No command has resulted in the event
  *            notification. In certain cases event can be a side effect of
  *            a command, but then it will not directly convey the result of a
  *            command.
  *
  */

#define MMI_MSG_BASE       ( 0x30000000 )       /**<
                                                * Base value which need to be
                                                * added in all the message
                                                * codes defined below.
                                                * NOTE: Not to be referred
                                                * directly by the client.
                                                */

#define MMI_MSG_INVALID    ( MMI_MSG_BASE )     /**<
                                                * This message code indicates
                                                * an invalid message.
                                                */

/**
 *
 * This message is sent in response to MMI_CMD_ENABLE_PORT command which was
 * queued by the device and processed asynchronously.
 * The message is posted when a port is enabled at HW level. In HW blocks
 * where there is no additional enablement, this message should still be
 * posted with success status so that base class state machine can move port
 * state.
 *
 * Status Codes:-
 *    MMI_S_COMPLETE - Means that the port is enabled successfully. At the
 *                     reception of this message app will assume all the
 *                     resources ( additionaly ) required for the port are
 *                     allocated.
 *
 *    Following two status codes will be used when the resources are not met
 *    for starting the port. Typically possible when the component is in a
 *    suspended state.
 *
 *    MMI_S_ENOSWRES - Means that the SW resources are not available for
 *                     allocating the resources for this component.
 *    MMI_S_ENOHWRES - Means that the HW resources are not available for
 *                     allocating the resources for this component.
 *
 *    Other status codes would mean enable failure and there is no recovery
 *    from this.
 *
 * Associated message data type:-
 *    MMI_PortMsgType
 *
 */
#define MMI_RESP_ENABLE_PORT                 ( MMI_MSG_BASE + 1 )

/**
 *
 * This message is sent in response to MMI_CMD_DISABLE_PORT command which was
 * queued by the device and processed asynchronously.
 * The message is posted when a port is disabled at HW level. In HW blocks
 * where there is no additional disablement, this message should still be
 * posted with success status so that base class state machine can move port
 * state.
 *
 * Status Codes:-
 *    MMI_S_COMPLETE - Means that the port is disabled successfully. At the
 *                     reception of this message app will assume all the
 *                     resources ( additionaly ) allocated for the port are
 *                     removed from the system.
 *
 *    Other status codes would mean disable failure and there is no recovery
 *    from this.
 *
 * Associated message data type:-
 *    MMI_PortMsgType
 *
 */
#define MMI_RESP_DISABLE_PORT                ( MMI_MSG_BASE + 2 )

/**
 *
 * This message is sent in response to MMI_CMD_START command which was queued
 * by the device and processed asynchronously.
 * The message is posted when the device is ready for buffer processing,
 * resources are activated and thus component can move to Executing state.
 *
 * Status Codes:-
 *    MMI_S_COMPLETE - Means that the device started successfully and it is
 *                     ready for buffer processing.
 *
 *    Following two status codes will be used when the resources are not met
 *    for starting the device. Typically possible when the component is in a
 *    suspended state.
 *
 *    MMI_S_ENOSWRES - Means that the SW resources are not available for
 *                     allocating the resources for this component.
 *    MMI_S_ENOHWRES - Means that the HW resources are not available for
 *                     allocating the resources for this component.
 *
 *    Other status codes would mean start failure and there is no recovery
 *    from this.
 *
 * Associated message data type:-
 *    None
 *
 */
#define MMI_RESP_START                       ( MMI_MSG_BASE + 3 )

/**
 *
 * This message is sent in response to MMI_CMD_STOP command which was queued
 * by the device and processed asynchronously.
 * The message is posted when resources are deactivated, all the buffers are
 * returned to the application and thus component can move to Idle state.
 *
 * Status Codes:-
 *    MMI_S_COMPLETE - Means that the device stopped successfully and has
 *                     returned all the buffers.
 *
 *    Other status codes would mean stop failure and there is no recovery
 *    from this.
 *
 * Associated message data type:-
 *    None
 *
 */
#define MMI_RESP_STOP                        ( MMI_MSG_BASE + 4 )

/**
 *
 * This message is sent in response to MMI_CMD_PAUSE command which was queued
 * by the device and processed asynchronously.
 * The message is posted when the session is paused by the device and the
 * resources are deactivated and and thus component can move to Pause state.
 *
 * Status Codes:-
 *    MMI_S_COMPLETE - Means that device paused successfully.
 *
 *    Other status codes would mean pause failure and there is no recovery
 *    from this.
 *
 * Associated message data type:-
 *    None
 *
 */
#define MMI_RESP_PAUSE                       ( MMI_MSG_BASE + 5 )

/**
 *
 * This message is sent in response to MMI_CMD_RESUME command which was queued
 * by the device and processed asynchronously.
 * The message is posted when the session is resumed by the device and the
 * resources are activated and and thus component can move to Executing state.
 *
 * Status Codes:-
 *    MMI_S_COMPLETE - Means that device resumed successfully and ready to
 *                     start buffer processing.
 *
 *    Following two status codes will be used when the resources are not met
 *    for starting the device. Typically possible when the component is in a
 *    suspended state.
 *
 *    MMI_S_ENOSWRES - Means that the SW resources are not available for
 *                     allocating the resources for this component.
 *    MMI_S_ENOHWRES - Means that the HW resources are not available for
 *                     allocating the resources for this component.
 *
 *    Other status codes would mean resume failure and there is no recovery
 *    from this.
 *
 * Associated message data type:-
 *    None
 *
 */
#define MMI_RESP_RESUME                      ( MMI_MSG_BASE + 6 )

/**
 *
 * This message is sent in response to MMI_CMD_EMPTY_THIS_BUFFER command which
 * was queued by the device and processed asynchronously.
 * The message is posted when the input buffer has been consumed by the device
 * and it is available for the client to reuse for further processing. The
 * buffer flags would carry additional information.
 *
 * Status Codes:-
 *    MMI_S_COMPLETE - Means that input buffer is processed successfully and
 *                     can be reused.
 *
 *    Other status codes would mean empty this buffer command failure.
 *
 * Associated message data type:-
 *    MMI_BufferDoneMsgType
 *
 */
#define MMI_RESP_EMPTY_THIS_BUFFER           ( MMI_MSG_BASE + 7 )

/**
 *
 * This message is sent in response to MMI_CMD_FILL_THIS_BUFFER command which
 * was queued by the device and processed asynchronously.
 * The message is posted when a buffer filled with data is returned on output
 * port. The buffer flags would carry additional information.
 *
 * Status Codes:-
 *    MMI_S_COMPLETE - Means that output buffer is processed successfully and
 *                     has been filled with appropriate data.
 *
 *    Other status codes would mean fill this buffer command failure.
 *
 * Associated message data type:-
 *    MMI_BufferDoneMsgType
 *
 */
#define MMI_RESP_FILL_THIS_BUFFER            ( MMI_MSG_BASE + 8 )

/**
 *
 * This message is sent in response to MMI_CMD_FLUSH command which was queued
 * by the device and processed asynchronously.
 * The message is posted when all the buffers which were queued or being
 * processed are flushed and returned.
 *
 * Status Codes:-
 *    MMI_S_COMPLETE - Means that flush was processed successfully and all the
 *                     buffers are returned.
 *
 *    Other status codes would mean flush command failure.
 *
 * Associated message data type:-
 *    MMI_PortMsgType
 *
 */
#define MMI_RESP_FLUSH                       ( MMI_MSG_BASE + 9 )

/**
 *
 * This message is sent in response to MMI_CMD_LOAD_RESOURCES command which
 * was queued by the device and processed asynchronously.
 * The message is posted when the resources are reserved for the media session
 * configured and thus component can move to Idle state.
 *
 * Status Codes:-
 *    MMI_S_COMPLETE - Means that resources are loaded successfully. At this
 *                     time the device and associated resources for this
 *                     session are fully reserved.
 *
 *    Following two status codes will be used when the resources are not
 *    loaded. Typically possible when the resources required are being used by
 *    other processeses.
 *
 *    MMI_S_ENOSWRES - Means that the SW resources are not available for
 *                     allocating the resources for this component.
 *    MMI_S_ENOHWRES - Means that the HW resources are not available for
 *                     allocating the resources for this component.
 *
 *    Other status codes would mean load resources failure and thus client can
 *    elect to put component into WaitForResources state.
 *
 * Associated message data type:-
 *    None
 *
 */
#define MMI_RESP_LOAD_RESOURCES              ( MMI_MSG_BASE + 10 )

/**
 *
 * This message is sent in response to MMI_CMD_RELEASE_RESOURCES command which
 * was queued by the device and processed asynchronously.
 * The message is posted when the reserved resources are released for the
 * media session and thus component can move to Loaded state.
 *
 * Status Codes:-
 *    MMI_S_COMPLETE - Means that resources are released successfully.
 *
 *    Other status codes would mean release resources failure and there is no
 *    recovery from this.
 *
 * Associated message data type:-
 *    None
 *
 */
#define MMI_RESP_RELEASE_RESOURCES           ( MMI_MSG_BASE + 11 )

/**
 *
 * This message is sent in response to MMI_CMD_WAIT_FOR_RESOURCES command
 * which was queued by the device and processed asynchronously.
 * The message is posted when the required resources are available for use,
 * can be loaded and thus component can move out of WaitForResources state.
 * Component need to request MMI_CMD_LOAD_RESOURCES for making reservation
 * of the resources.
 *
 * Status Codes:-
 *    MMI_S_COMPLETE - Means that resources are available.
 *
 *    Other status codes would mean wait for resources failure and there is no
 *    recovery from this.
 *
 * Associated message data type:-
 *    None
 *
 */
#define MMI_RESP_WAIT_FOR_RESOURCES          ( MMI_MSG_BASE + 12 )




/**
 *
 * This event is posted when some of the resources currently being allocated
 * are lost or preempted by the high priority process. Thus component can be
 * put in WaitForResources state and wait there till the resources become
 * available again.
 *
 * Status Codes:-
 *    None
 *
 * Associated message data type:-
 *    MMI_ResourceLostMsgType
 *
 */
#define MMI_EVT_RESOURCES_LOST               ( MMI_MSG_BASE + 1000 )

/**
 *
 * This event is posted when the lost resources are available and have been
 * acquired by the device for further processing. Component would need to
 * send resume command to continue buffer processing further.
 *
 * Status Codes:-
 *    None
 *
 * Associated message data type:-
 *    None
 *
 */
#define MMI_EVT_RESOURCES_ACQUIRED           ( MMI_MSG_BASE + 1001 )

/**
 *
 * This event is posted when a port is auto detected. This event is the result
 * of parameter setting CodingAutoDetect and must be sent only when the auto
 * detection mode is set.
 * On successful port detection, this event need to be followed by sequence of
 * port config change events on output ports.
 *
 * Status Codes:-
 *    MMI_S_COMPLETE - Means that port autodetect is successful.
 *    MMI_S_EFAIL    - Means that port autodetect is failed.
 *
 * Associated message data type:-
 *    NA
 *
 */
#define MMI_EVT_PORT_DETECTION               ( MMI_MSG_BASE + 1002 )

/**
 *
 * This event is posted when buffer requirements related to a port is changed.
 * Thus client can reconfigure the port.
 *
 * Status Codes:-
 *    None
 *
 * Associated message data type:-
 *    MMI_PortMsgType
 *
 */
#define MMI_EVT_PORT_CONFIG_CHANGED          ( MMI_MSG_BASE + 1003 )

/**
 *
 * This event is posted when device needs buffer when a frame is partially
 * processed, driver does not have any input/output buffers and client should
 * provide a buffer to complete remaining processing of the frame.
 *
 * Status Codes:-
 *    None
 *
 * Associated message data type:-
 *    MMI_PortMsgType
 *
 */
#define MMI_EVT_NEED_BUFFER                  ( MMI_MSG_BASE + 1004 )

/**
 *
 * This event is posted when device sends an event that is defined in the
 * extensions and is not a part of OMX standard events. Base class would just
 * propagate this event to the IL client.
 *
 * Status Codes:-
 *    None
 *
 * Associated message data type:-
 *    MMI_ExtSpecificMsgType
 *
 */
#define MMI_EVT_QOMX_EXT_SPECIFIC            ( MMI_MSG_BASE + 1005 )

/**
 *
 * This event is generated when a tunneled device's input port reaches the
 * end of the data stream.  This event should be generated only by devices
 * configured for vendor tunneling.  See MMI_CMD_VENDOR_TUNNEL_REQUEST for
 * more details on vendor tunneling.
 *
 * Status Codes:-
 *    None
 *
 * Associated message data type:-
 *    MMI_PortMsgType
 *
 */
#define MMI_EVT_PORT_EOS                     ( MMI_MSG_BASE + 1006 )

/**
 *
 * This event is generated when a device automatically pauses itself and thus
 * wants component to transition itself to pause state e.g. device would pause
 * itself after completing capture when it receives configuration index
 * Omx_IndexAutoPauseAfterCapture from IL client.
 *
 * Status Codes:-
 *    None
 *
 * Associated message data type:-
 *    None
 *
 */
#define MMI_EVT_AUTO_PAUSE                   ( MMI_MSG_BASE + 1007 )

/**
 *
 * This event is generated when a device detects an unrecoverable error and
 * the component need to be moved to invalid state. Component would be
 * deinitialized after this event is reported.
 *
 * Status Codes:-
 *    None
 *
 * Associated message data type:-
 *    None
 *
 */
#define MMI_EVT_FATAL_ERROR                  ( MMI_MSG_BASE + 1008 )

/*--------------------------------------------------------------------------*/



/**
  *
  * These macro define the Command Codes.
  *
  * Conventions:-
  *
  * 1. There are 3 types of Commands: Set, Get, Operational Commands (Cmd)
  *
  * 2. All Set commands are even numbered ( = 2N ) to reseve ( = 2N + 1 ) for
  *    corresponding Get command.
  *
  *    All Operational commands are also even numbered ( = 2N ). Odd numbers
  *   ( = 2N + 1 ) are unused.
  *
  * 3. Command type tells whether the command processing is synchronous or
  *    asynchronous.
  *
  * 4. Associated data type gives the data type to be passed while sending
  *    the command. Refer to the CMD data type section for its definition and
  *    additonal details.
  *
  * 5. Command codes within the range of 0x40000000 -- 0x4000FFFE are reserved
  *    for base class usage. MMIs must not use these command codes;
  *    MMIs can use range 0x4000FFFF -- 0x4FFFFFFF for extension commands.
  */

#define MMI_CMD_BASE       ( 0x40000000 )       /**<
                                                * Base value which need to be
                                                * added in all the command
                                                * codes defined below.
                                                * NOTE: Not to be referred
                                                * directly by the client.
                                                */

/**
 *
 * Purpose:-
 *       This command is used to Set / Get OMX parameter & configurations
 *       into / out of the device.
 *
 *       Many OMX parameter and config settings / queries cannot be handled by
 *       the base class and base class will ask the driver to respond. This
 *       command provides opportunity to the device layer to handle the
 *       SetParameter / GetParameter and SetConfig / GetConfig APIs.
 *
 *       Most technology or media related parameters will be handed over to
 *       device layer.
 *
 * Command type:-
 *       Synchronous
 *
 * Associated command input data type:-
 *       MMI_OmxParamCmdType
 *
 * Command return type:-
 *       MMI_S_COMPLETE on success.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       NA
 *
 */
#define MMI_CMD_SET_STD_OMX_PARAM            ( MMI_CMD_BASE + 2 )
#define MMI_CMD_GET_STD_OMX_PARAM            ( MMI_CMD_BASE + 3 )

/**
 *
 * Commands for buffer requirements negotiation:
 *
 * CASE A: When external allocator allocates the input buffers:-
 *
 *         Negotiation should be carried out with following commands.
 *        1) MMI_CMD_GET_CUSTOM_PARAM  - To get driver's buffer requirements.
 *        2) MMI_CMD_SET_CUSTOM_PARAM  - To set final external allocator's
 *                                       requirement.
 *        3) MMI_CMD_USE_BUFFER        - To informs the driver which buffers
 *                                       are being used.
 *
 * CASE B: When driver allocates the input buffers:-
 *         Negotiation should be carried out with following commands.
 *        1) MMI_CMD_GET_CUSTOM_PARAM  - To get driver's buffer requirements.
 *        2) MMI_CMD_SET_CUSTOM_PARAM  - To set the collective requirements.
 *        3) MMI_CMD_ALLOC_BUFFER      - To let driver allocate memory.
 *
 * By default driver considers that an external allocator will be used.
 * MMI_CMD_ALLOC_BUFFER will instruct the driver to be the allocator.
 *
 */

/**
 *
 * Purpose:-
 *       This command is used to Set / Get parameters & configurations defined
 *       in this header file which are not identified by a unique OMX param
 *       index but are domain or technology specific and are required by the
 *       component to set into or query from the device. These parameters are
 *       identified by the customized indexes and structures defined in this
 *       header file.
 *       e.g. OMX port definition format specific parameters can not be
 *       handled by the base class and should be passed to device for action
 *       on these.
 *
 * Command type:-
 *       Synchronous
 *
 * Associated command input data type:-
 *       MMI_CustomParamCmdType
 *
 * Command return type:-
 *       MMI_S_COMPLETE on success.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       NA
 *
 */
#define MMI_CMD_SET_CUSTOM_PARAM             ( MMI_CMD_BASE + 4 )
#define MMI_CMD_GET_CUSTOM_PARAM             ( MMI_CMD_BASE + 5 )

/**
 *
 * Purpose:-
 *       This command is used to get buffer allocation. This command requests
 *       a single buffer allocation from the driver memory.
 *
 * Command type:-
 *       Synchronous
 *
 * Associated command input data type:-
 *       MMI_AllocBufferCmdType
 *
 * Command return type:-
 *       MMI_S_COMPLETE on success.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       NA
 *
 */
#define MMI_CMD_ALLOC_BUFFER                 ( MMI_CMD_BASE + 8 )

/**
 *
 * Purpose:-
 *       This command is used to free allocated memory. This command is the
 *       counterpart of MMI_CMD_ALLOC_BUFFER and must be used only on a buffer
 *       that is allocated using this command.
 *
 * Command type:-
 *       Synchronous
 *
 * Associated command input data type:-
 *       MMI_FreeBufferCmdType
 *
 * Command return type:-
 *       MMI_S_COMPLETE on success.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       NA
 *
 */
#define MMI_CMD_FREE_BUFFER                  ( MMI_CMD_BASE + 10 )

/**
 *
 * Purpose:-
 *       This command is used to pass buffers allocated by the client which
 *       would be used in the data path during the session.
 *
 * Command type:-
 *       Synchronous
 *
 * Associated command input data type:-
 *       MMI_UseBufferCmdType
 *
 * Command return type:-
 *       MMI_S_COMPLETE on success.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       NA
 *
 */
#define MMI_CMD_USE_BUFFER                   ( MMI_CMD_BASE + 12 )

/**
 *
 * Purpose:-
 *       This command is used as a notification to driver to be prepared to
 *       enable and populating the ports with buffers. Resources may be
 *       reserved but need to be committed only when an LOAD command is
 *       issued. If LOAD command is already done then the additional
 *       allocations should be done at the end of the command.
 *
 * Command type:-
 *       Asynchronous / Synchronous
 *
 * Associated command input data type:-
 *       MMI_PortCmdType
 *
 * Command return type:-
 *       MMI_S_PENDING on asynchronous execution.
 *       MMI_S_COMPLETE on successful synchronous execution.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       MMI_RESP_ENABLE_PORT
 *
 */
#define MMI_CMD_ENABLE_PORT                  ( MMI_CMD_BASE + 14 )

/**
 *
 * Purpose:-
 *       This command is used as a notification to driver to be prepared to
 *       disable and depopulating the ports. At the end of this command any
 *       additional resources allocated for this port should be deallocated.
 *
 * Command type:-
 *       Asynchronous / Synchronous
 *
 * Associated command input data type:-
 *       MMI_PortCmdType
 *
 * Command return type:-
 *       MMI_S_PENDING on asynchronous execution.
 *       MMI_S_COMPLETE on successful synchronous execution.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       MMI_RESP_DISABLE_PORT
 *
 */
#define MMI_CMD_DISABLE_PORT                 ( MMI_CMD_BASE + 16 )

/**
 *
 * Purpose:-
 *       This command is used to initialize and start the module with the
 *       currently set configuration. On successful exection of this command
 *       the module device would start processing the buffers. Device would
 *       queue any buffer processing command while start command is being
 *       processed.
 *
 * Command type:-
 *       Asynchronous / Synchronous
 *
 * Associated command input data type:-
 *       None
 *
 * Command return type:-
 *       MMI_S_PENDING on asynchronous execution.
 *       MMI_S_COMPLETE on successful synchronous execution.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       MMI_RESP_START
 *
 */
#define MMI_CMD_START                        ( MMI_CMD_BASE + 18 )

/**
 *
 * Purpose:-
 *       This command is used to stop the current module. If there are pending
 *       buffers in the pipeline, those would be ignored and not processed
 *       before stopping the module.
 *       This command would not clear the properties set currently. If the
 *       client wants to restart the session it should call MMI_CMD_START to
 *       reinitialize the module before submitting any buffer for processing.
 *
 * Command type:-
 *       Asynchronous / Synchronous
 *
 * Associated command input data type:-
 *       None
 *
 * Command return type:-
 *       MMI_S_PENDING on asynchronous execution.
 *       MMI_S_COMPLETE on successful synchronous execution.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       MMI_RESP_STOP
 *
 */
#define MMI_CMD_STOP                         ( MMI_CMD_BASE + 20 )

/**
 *
 * Purpose:-
 *       This command is used to pause the current session. If there are
 *       pending buffers in the pipeline, those would be put on hold and
 *       be processed once the session is resumed. Device would queue any
 *       buffer processing commands while paused.
 *
 * Command type:-
 *       Asynchronous / Synchronous
 *
 * Associated command input data type:-
 *       None
 *
 * Command return type:-
 *       MMI_S_PENDING on asynchronous execution.
 *       MMI_S_COMPLETE on successful synchronous execution.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       MMI_RESP_PAUSE
 *
 */
#define MMI_CMD_PAUSE                        ( MMI_CMD_BASE + 22 )

/**
 *
 * Purpose:-
 *       This command is used to resume a paused session.
 *
 * Command type:-
 *       Asynchronous / Synchronous
 *
 * Associated command input data type:-
 *       None
 *
 * Command return type:-
 *       MMI_S_PENDING on asynchronous execution.
 *       MMI_S_COMPLETE on successful synchronous execution.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       MMI_RESP_RESUME
 *
 */
#define MMI_CMD_RESUME                       ( MMI_CMD_BASE + 24 )

/**
 *
 * Purpose:-
 *       This command is used to pass an input buffer which need to processed
 *       and consumed by the device. If external allocator is being used it
 *       should allocate the pool identifier.
 *
 * Command type:-
 *       Asynchronous
 *
 * Associated command input data type:-
 *       MMI_BufferCmdType
 *
 * Command return type:-
 *       MMI_S_PENDING on asynchronous execution.
 *       MMI_S_COMPLETE is not applicable.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       MMI_RESP_EMPTY_THIS_BUFFER
 *
 */
#define MMI_CMD_EMPTY_THIS_BUFFER            ( MMI_CMD_BASE + 26 )

/**
 *
 * Purpose:-
 *       This command is used to pass an output buffer which need to processed
 *       and filled with data by the device. Once this buffer is passed, the
 *       device takes over the buffer management till the buffer is returned
 *       back by via fill buffer done response.
 *
 * Command type:-
 *       Asynchronous
 *
 * Associated command input data type:-
 *       MMI_BufferCmdType
 *
 * Command return type:-
 *       MMI_S_PENDING on asynchronous execution.
 *       MMI_S_COMPLETE is not applicable.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       MMI_RESP_FILL_THIS_BUFFER
 *
 */
#define MMI_CMD_FILL_THIS_BUFFER             ( MMI_CMD_BASE + 28 )

/**
 *
 * Purpose:-
 *       This command is used to clear the queued buffers. All outstanding
 *       buffers in the queue should be cleared and returned back via
 *       appropriate callbacks.
 *
 * Command type:-
 *       Asynchronous / Synchronous
 *
 * Associated command input data type:-
 *       MMI_PortCmdType
 *
 * Command return type:-
 *       MMI_S_PENDING on asynchronous execution.
 *       MMI_S_COMPLETE on successful synchronous execution.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       MMI_RESP_FLUSH
 *
 */
#define MMI_CMD_FLUSH                        ( MMI_CMD_BASE + 30 )

/**
 *
 * Purpose:-
 *       This command is used to send request to load the resources to support
 *       the initial configuration provided. This is the initial reservation
 *       driver would make for the component to operate. If the underlying
 *       device has any notion of reservation for the session, this command
 *       has to be invoked. After successful invocation of this command the
 *       device should not fail on invocation of the start. The device
 *       implementation should be careful to frugal on power even after
 *       successful execution of this command. Base class must call this
 *       after all the buffers are aquired.
 *
 * Command type:-
 *       Asynchronous / Synchronous
 *
 * Associated command input data type:-
 *       None
 *
 * Command return type:-
 *       MMI_S_PENDING on asynchronous execution.
 *       MMI_S_COMPLETE on successful synchronous execution.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       MMI_RESP_LOAD_RESOURCES
 *
 */
#define MMI_CMD_LOAD_RESOURCES               ( MMI_CMD_BASE + 32 )

/**
 *
 * Purpose:-
 *       This command is used to release all the reserved resources relating
 *       to this device. At the completion of this command there would be no
 *       resources or scratch buffers relating to the device allocated/or
 *       reserved. Base class must call this only after all the buffers are
 *       released.
 *
 * Command type:-
 *       Asynchronous / Synchronous
 *
 * Associated command input data type:-
 *       None
 *
 * Command return type:-
 *       MMI_S_PENDING on asynchronous execution.
 *       MMI_S_COMPLETE on successful synchronous execution.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       MMI_RESP_RELEASE_RESOURCES
 *
 */
#define MMI_CMD_RELEASE_RESOURCES            ( MMI_CMD_BASE + 34 )

/**
 *
 * Purpose:-
 *       This command is used to request device to respond when the resources
 *       are available for reservation. Usually this command would be called
 *       when resources are preempted or an attempt to allocate the resources
 *       is failed. Device should notify to the client when the resources are
 *       available through wait for resources response. If the resources are
 *       available when the command is called or there is no associated
 *       resource, device still need to return success synchronously or
 *       through asynchronous response.
 *
 * Command type:-
 *       Asynchronous / Synchronous
 *
 * Associated command input data type:-
 *       None
 *
 * Command return type:-
 *       MMI_S_PENDING on asynchronous execution.
 *       MMI_S_COMPLETE on successful synchronous execution.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       MMI_RESP_WAIT_FOR_RESOURCES
 *
 */
#define MMI_CMD_WAIT_FOR_RESOURCES           ( MMI_CMD_BASE + 36 )

/**
 *
 * Purpose:-
 *       This command is used to request device to release wait on resources
 *       which was put through call to command MMI_CMD_WAIT_FOR_RESOURCES.
 *       After completion of this command, device would not respond to the
 *       wait command through callback.
 *
 * Command type:-
 *       Synchronous
 *
 * Associated command input data type:-
 *       None
 *
 * Command return type:-
 *       MMI_S_COMPLETE on success.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       NA
 *
 */
#define MMI_CMD_RELEASE_WAIT_ON_RESOURCES    ( MMI_CMD_BASE + 38 )

/**
 *
 * Purpose:-
 *       This command is used to request the device to setup a proprietary
 *       vendor a port tunnel to another device.  This command will be called
 *       on the device that supplies the output port.  The device should query
 *       the device that supplies the input port to determine if a vendor
 *       tunnel can be setup between the devices, and return the appropriate
 *       response.
 *
 * Command type:-
 *       Synchronous
 *
 * Associated command input data type:-
 *       MMI_VendorTunnelCmdType
 *
 * Command return type:-
 *       MMI_S_COMPLETE if tunnel has been established with the peer device.
 *       MMI_S_EFAIL if tunnel cannot be established with the peer device.
 *
 * Asynchronous respone message code:-
 *       NA
 *
 */
#define MMI_CMD_VENDOR_TUNNEL_REQUEST        ( MMI_CMD_BASE + 39 )

/**
 *
 * Purpose:-
 *       This command is used to request the device to return extension index
 *       corresponding to the given extension parameter string.
 *
 *       The value of this index need to be in the following specific range.
 *          Video extensions: 0x7F100000 -- 0x7F1FFFFF
 *          Audio extensions: 0x7F200000 -- 0x7F2FFFFF
 *          Image extensions: 0x7F300000 -- 0x7F3FFFFF
 *
 *       Indexes within the range of 0x7FFFF000 -- 0x7FFFFFFE are reserved for
 *       Base class usage. MMIs must not use these indexes.
 *
 * Command type:-
 *       Synchronous
 *
 * Associated command input data type:-
 *       MMI_GetExtensionCmdType
 *
 * Command return type:-
 *       MMI_S_COMPLETE on success.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       NA
 *
 */
#define MMI_CMD_GET_EXTENSION_INDEX          ( MMI_CMD_BASE + 40 )

/**
 *
 * Purpose:-
 *       This command is used to instruct MMI device to stop referring a
 *       buffer. This command is the counterpart of MMI_CMD_USE_BUFFER and
 *       must be used only on a buffer that is passed to MMI device using
 *       this command. This is informational only, so MMI device can stop
 *       referring this buffer.
 *
 * Command type:-
 *       Synchronous
 *
 * Associated command input data type:-
 *       MMI_ClearBufferCmdType
 *
 * Command return type:-
 *       MMI_S_COMPLETE on success.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       NA
 *
 */
#define MMI_CMD_CLEAR_BUFFER                  ( MMI_CMD_BASE + 42 )

/*--------------------------------------------------------------------------*/



/* -----------------------------------------------------------------------
** Typedefs
** ----------------------------------------------------------------------- */


/**
 *
 * This data type declares a function prototype which is used to open a new
 * driver instance with the underlying HW or SW codec ( in case of all SW
 * implementation ). Other initialization and kernel mode loading is not
 * assumed part of this function call. This API is the common call to access
 * any driver. Typically this should map to a one line function in the adaptor
 * to open the device.
 *
 * Function parameters:-
 *    @param[out] pHFd  - Handle to the device driver. Upon successful open
 *                        a unique handle to the file descriptor would be
 *                        returned via this parameter.
 *
 * Function return type:-
 *    MMI_S_COMPLETE on success.
 *    Other error codes as appropriate on failure.
 *
 */
typedef  OMX_U32  ( *MMI_OpenType )
(
   OMX_OUT  OMX_HANDLETYPE      *pHFd
);


/**
 *
 * This data type declares a function prototype which is used to close an
 * already opened driver instance.
 *
 * Typically client should ensure that all the buffers are released and the
 * component is properly moved to Loaded state before calling this function,
 * otherwise this API may result in unexpected behaviors.
 *
 * Function parameters:-
 *    @param[in]  hFd   - Handle to the device driver. This handle would
 *                        become invalid after this function call and should
 *                        not be used by the client any further.
 *
 * Function return type:-
 *    MMI_S_COMPLETE on success.
 *    Other error codes as appropriate on failure.
 *
 */
typedef  OMX_U32  ( *MMI_CloseType )
(
   OMX_IN   OMX_HANDLETYPE    hFd
);


/**
 *
 * This data type declares a function prototype which is used to send a
 * command to the device. Commands can be a direct mapping to a kernel mode
 * driver if one exists in the device layer. If the underlying device is a SW
 * entity then the CMD can be mapped to a local invocation depending on the
 * implementation.
 *
 * Typically client should ensure that all the buffers are released and the
 * component is properly moved to Loaded state before calling this function,
 * otherwise this API may result in unexpected behaviors.
 *
 * Function parameters:-
 *    @param[in]  hFd      - Handle to the device driver.
 *    @param[in]  nCmdCode - Command code which are documented above.
 *    @param[in]  pCmdData - Data associated with the command. Device driver
 *                           would typecast this data to the appropriate
 *                           structure type.
 *
 * Function return type:-
 *    MMI_S_PENDING on asynchronous execution.
 *    MMI_S_COMPLETE on successful synchronous execution.
 *    Other error codes as appropriate on failure.
 *
 */
typedef  OMX_U32  ( *MMI_CmdType )
(
   OMX_IN   OMX_HANDLETYPE    hFd,
   OMX_IN   OMX_U32           nCmdCode,
   OMX_IN   OMX_PTR           pCmdData
);


/**
 *
 * This data type declares a function prototype which is invoked by the device
 * layer when the device want to report an event back to the component. The
 * component would process the event and may take one or more OMX events or
 * actions.
 *
 * Function parameters:-
 *    @param[in]  nEvtCode    - Uniqueue response / event code documented
 *                              above.
 *    @param[in]  nEvtStatus  - Event status code which is the result of the
 *                              command. This is valid when the event is a
 *                              response to a command. In general, if this
 *                              event code is not valid, the device should
 *                              still set it to MMI_S_COMPLETE. This avoids
 *                              unnecessary confusions and graph tier down
 *                              as a result from the base class.
 *    @param[in]  nPayloadLen - Length of the payload data associated with
 *                              this event.
 *    @param[in]  pEvtData    - Pointer to the payload data associated with
 *                              the event. IL base class should type cast
 *                              this with appropriate message data
 *                              structure.
 *    @param[in]  pClientData - Client Data which was given as part of event
 *                              handler registration. IL component internal
 *                              object is typically passed through the
 *                              registration.
 *
 * Function return type:-
 *    None
 *
 */
typedef  void  ( *MMI_CmpntEvtHandlerType )
(
   OMX_IN   OMX_U32     nEvtCode,
   OMX_IN   OMX_U32     nEvtStatus,
   OMX_IN   OMX_U32     nPayloadLen,
   OMX_IN   OMX_PTR     pEvtData,
   OMX_IN   OMX_PTR     pClientData
);


/**
 *
 * This data type declares a function prototype which is invoked by the IL
 * base class to register an event handler. This is called only once during
 * the component initialization. Multiple calls to this API are not allowed.
 *
 * Warning:-
 *    IL COMPONENT EXPECT THIS CALLBACK BE INVOKED FROM A THREAD DIFFERENT
 *    FROM IL CLIENT'S CALLING THREAD. THIS FORCES ONE THREAD IN THE DEVICE
 *    LAYER.
 *
 * Function parameters:-
 *    @param[in]  hFd         - Handle to the device driver.
 *    @param[in]  pfnEvtHdlr  - Pointer to the function pointer which is the
 *                              event callback into the component.
 *    @param[in]  pClientData - Client Data which should be passed as part of
 *                              the event handler call. IL component internal
 *                              object is typically passed through the it.
 *
 * Function return type:-
 *    MMI_S_COMPLETE on success.
 *    Other error codes as appropriate on failure.
 *
 */
typedef  OMX_U32  ( *MMI_RegisterEvtHandlerType )
(
   OMX_IN   OMX_HANDLETYPE             hFd,
   OMX_IN   MMI_CmpntEvtHandlerType    pfnEvtHdlr,
   OMX_IN   OMX_PTR                    pClientData
);


/**
 *
 * This structure encapsulates the entire device object. Each component object
 * initialization should have a valid table with all function the pointers
 * populated for proper component operation.
 *
 */
typedef  struct
{
   MMI_OpenType                  pfnOpen;    /**<
                                             * Pointer to Open function.
                                             * Component would call this
                                             * function to get handle to
                                             * file descriptor. Usually this
                                             * function maps directly to a
                                             * device driver's Open call.
                                             */

   MMI_RegisterEvtHandlerType    pfnEvtHdlr; /**<
                                             * Pointer to Register event
                                             * handler function. Device
                                             * would use this function
                                             * for sending periodic events
                                             * to the component.
                                             */

   MMI_CmdType                   pfnCmd;     /**<
                                             * Pointer to Command function.
                                             * Component would use this
                                             * function to send commands
                                             * identified by a unique code
                                             * into the device.
                                             */

   MMI_CloseType                 pfnClose;   /**<
                                             * Pointer to Close function.
                                             * Component would use this
                                             * function to terminate the
                                             * current device session.
                                             * File descriptor handle
                                             * becomes invalid after this
                                             * function call.
                                             */

} MMI_DeviceType;


/**
 *
 * This data type declares a function prototype which is used to instantiate
 * the specific memory module which is used for memory management for the
 * current component.
 *
 * Function parameters:-
 *    @param[out] hHHeap  - Handle to the heap area. Upon successful
 *                        GetHeapHandle, a unique handle to the heap would be
 *                        returned via this parameter.
 *
 * Function return type:-
 *    MMI_S_COMPLETE on success.
 *    Other error codes as appropriate on failure.
 *
 */
typedef  OMX_U32  ( *MMI_GetHeapHandleType )
(
   OMX_OUT  OMX_HANDLETYPE      *hHHeap
);


/**
 *
 * This data type declares a function prototype which is used to call
 * appropriate deinit function to release specific memory module
 * which is used for memory management for the current component.
 *
 * Function parameters:-
 *    @param[in] hHeap  - Handle to the heap area.
 *
 * Function return type:-
 *    None
 *
 */
typedef  void  ( *MMI_ReleaseHeapHandleType )
(
   OMX_IN   OMX_HANDLETYPE       hHeap
);


/**
 *
 * This data type declares a function prototype which is used to call
 * appropriate memory allocation routine from the given heap area to
 * allocate requested number of bytes.
 *
 * Function parameters:-
 *    @param[in] hHeap  - Handle to the heap area.
 *    @param[in] nSize  - Number of bytes to be allocated.
 *
 * Function return type:-
 *    Pointer to the allocated memory area.
 *
 */
typedef  void*  ( *MMI_MallocType )
(
   OMX_IN   OMX_HANDLETYPE       hHeap,
   OMX_IN   OMX_U32              nSize
);


/**
 *
 * This data type declares a function prototype which is used to call
 * appropriate free allocation routine from the given heap area to release
 * previously allocated memory.
 *
 * Function parameters:-
 *    @param[in] hHeap  - Handle to the heap area.
 *    @param[in] pMem   - Pointer to the memory location to be freed.
 *
 * Function return type:-
 *    None
 *
 */
typedef  void  ( *MMI_FreeType )
(
   OMX_IN   OMX_HANDLETYPE       hHeap,
   OMX_IN   void                *pMem
);


/**
 *
 * This enumeration is used to select a structure when either getting or
 * setting customized parameters or configuration data. Each entry in this
 * enumeration maps to an MMI specified parameter structure.
 *
 *  For example, if the client is setting port domain definition parameter,
 *  the MMI_CMD_SET_CUSTOM_PARAM command would have
 *    MMI_CustomParamType.nParamIndex  = MMI_IndexDomainDef, and
 *    MMI_CustomParamType.pParamStruct = Pointer to an object of
 *                                       MMI_ParamDomainDefType
 *
 * Commands association:
 *    MMI_CMD_SET_CUSTOM_PARAM
 *    MMI_CMD_GET_CUSTOM_PARAM
 *
 */
typedef enum
{
   MMI_IndexInvalid = 0,                     /**<
                                             * This index is used to define
                                             * the start for enums.
                                             * This is an invalid index and
                                             * must not be referred by base
                                             * class directly.
                                             */

   MMI_IndexDomainDef,                       /**<
                                             * Associated structure:-
                                             *  MMI_ParamDomainDefType
                                             */

   MMI_IndexBuffersReq,                      /**<
                                             * Associated structure:-
                                             *  MMI_ParamBuffersReqType
                                             */

   MMI_IndexClockComponent,                  /**<
                                             * Associated structure:-
                                             *  MMI_ParamClockComponentType
                                             */

   MMI_IndexMax = 0xFFFF                     /**<
                                             * This index is used to define
                                             * maximum value for the
                                             * enumeration and forcing
                                             * compiler to treat these enums
                                             * as 32-bit and avoid any
                                             * optimization.
                                             * This is an invalid index and
                                             * must not be referred by base
                                             * class directly.
                                             */

} MMI_IndexType;


/**
 *
 * This data type relates to the OMX parameters and configurations which need
 * to be handled by the device. Most of the OMX parameters and configurations
 * would be handled by the device. This information is pretty technology and
 * device specific and might not be possible for base component to realize.
 *
 * Commands association:-
 *    MMI_CMD_SET_STD_OMX_PARAM
 *    MMI_CMD_GET_STD_OMX_PARAM
 *
 */
typedef struct
{
   OMX_INDEXTYPE           nParamIndex;      /**<
                                             * OMX Parameter / Configuration
                                             * index associated with the
                                             * parameter / configuration
                                             * being Set / Get.
                                             */

   OMX_PTR                 pParamStruct;     /**<
                                             * OMX Parameter / Configuration
                                             * structure associated with the
                                             * parameter / configuration
                                             * being Set / Get.
                                             */

} MMI_OmxParamCmdType;


/**
 *
 * This data type relates to the custom parameters and configurations which
 * need to be handled by the device. These custom parameters do not have
 * associated standard OMX indexes.
 *
 * Commands association:-
 *    MMI_CMD_SET_CUSTOM_PARAM
 *    MMI_CMD_GET_CUSTOM_PARAM
 *
 */
typedef struct
{
   MMI_IndexType           nParamIndex;      /**<
                                             * Custom Parameter index
                                             * associated with the parameter
                                             * being Set / Get.
                                             */

   OMX_PTR                 pParamStruct;     /**<
                                             * Custome Parameter structure
                                             * associated with the parameter
                                             * being Set/ Get.
                                             */

} MMI_CustomParamCmdType;


/**
 *
 * This data type relates to the parameter which need to be handled by the
 * port defintion retrieval and setting. Base class can handle most of the
 * port definition part, but can not handle domain specific settings.
 *
 * Commands association as a substructure:-
 *    MMI_CMD_SET_CUSTOM_PARAM
 *    MMI_CMD_GET_CUSTOM_PARAM
 *
 */
typedef struct
{
   OMX_U32                 nPortIndex;       /**<
                                             * Port number on which this
                                             * parameter should be applied.
                                             */

   union
   {
      OMX_AUDIO_PORTDEFINITIONTYPE  audio;
      OMX_VIDEO_PORTDEFINITIONTYPE  video;
      OMX_IMAGE_PORTDEFINITIONTYPE  image;
      OMX_OTHER_PORTDEFINITIONTYPE  other;

   } format;                                 /**<
                                             * Domain specific port definition
                                             * which is being set / queuried.
                                             */

} MMI_ParamDomainDefType;


/**
 *
 * This data type relates to the parameter which need to be handled by the
 * port definition retrieval and setting. Base class can handle most of the
 * port definition part, but can not handle buffers management.
 *
 * Commands association as a substructure:-
 *    MMI_CMD_SET_CUSTOM_PARAM
 *    MMI_CMD_GET_CUSTOM_PARAM
 *
 */
typedef struct
{
   OMX_U32                 nPortIndex;       /**<
                                             * Port number on which this
                                             * parameter should be applied.
                                             */

   OMX_U32                 nMinCount;        /**<
                                             * Minimum number of buffers
                                             * required.
                                             */

   OMX_U32                 nCount;           /**<
                                             * When used with Get, if there
                                             * has not been a Set yet, it
                                             * would be the recommended number
                                             * of buffers to be allocated for
                                             * optimal performance. If there
                                             * has been a Set done, the value
                                             * provided in the previous set is
                                             * returned.
                                             * When used with Set, it should
                                             * be the actual number of buffers
                                             * that are allocated or to be
                                             * allocated.
                                             */

   OMX_U32                 nDataSize;        /**<
                                             * Data size of each buffer in
                                             * bytes ( i.e size excluding
                                             * suffix bytes )
                                             * This is the region that will
                                             * contain the actual payload.
                                             */

   OMX_U32                 nSuffixSize;      /**<
                                             * Number of bytes as suffix.
                                             * A suffix of these many bytes
                                             * follows the buffer data. This
                                             * region could contain metadata.
                                             */

   OMX_U32                 nAlignment;       /**<
                                             * Buffer's physical address
                                             * (start of prefix region )
                                             * would be aligned to multiple of
                                             * these many bytes. Start of Data
                                             * and Suffix region would be
                                             * aligned to this as well.
                                             */

   OMX_BOOL                bBuffersContiguous;
                                             /**<
                                             * To set or get buffers type
                                             * parameter. This tells if the
                                             * buffers should be contiguous.
                                             */

   OMX_U32                 nBufferPoolId;    /**<
                                             * Unique pool identifier which
                                             * can be set for all buffer
                                             * allocations. This can come from
                                             * driver or come from some local
                                             * policy component might have.
                                             * An Id of 0xffffff is used when
                                             * the device does not care about
                                             * a pariticular pool. In this
                                             * case a regular heap pointer is
                                             * used for populating buffers.
                                             */

} MMI_ParamBuffersReqType;


typedef struct
{
   OMX_U32                 nPortIndex;       /**<
                                             * Port number on which this
                                             * parameter should be applied.
                                             */

   OMX_HANDLETYPE          hClock;           /**<
                                             * OpenMAX Handle for the clock component.
                                             * If NULL, then the clock component has
                                             * been disabled.
                                             */

   OMX_U32                 nClockPort;       /**<
                                             * Port number for the clock component.
                                             */

} MMI_ParamClockComponentType;

/**
 *
 * This data type relates to the commands which are specific to the ports.
 *
 * Commands association:-
 *    MMI_CMD_ENABLE_PORT
 *    MMI_CMD_DISABLE_PORT
 *    MMI_CMD_FLUSH
 *
 */
typedef struct
{
   OMX_U32                 nPortIndex;       /**<
                                             * Port number with which this
                                             * command is related to.
                                             */

} MMI_PortCmdType;


/**
 *
 * This data type relates to the command for sending client allocated buffers
 * to the device for further use.
 *
 * Commands association:-
 *    MMI_CMD_USE_BUFFER
 *
 */
typedef struct
{
   OMX_U32                 nPortIndex;       /**<
                                             * Port number on which this
                                             * command should be applied.
                                             */

   OMX_U8                 *pBuffer;          /**<
                                             * This is the buffer allocated
                                             * by the IL client which need
                                             * to be used by device for
                                             * buffer processing later.
                                             */

   OMX_U32                 nSize;            /**<
                                             * Size of the buffer to be
                                             * allocated in bytes.
                                             */

   OMX_PTR                 pPlatformPvt;     /**<
                                             * This is pointer to any data or
                                             * context that platform wants to
                                             * associate with this buffer.
                                             */

} MMI_UseBufferCmdType;


/**
 *
 * This data type relates to the command for allocating buffers from the
 * device memory.
 *
 * Commands association:-
 *    MMI_CMD_ALLOC_BUFFER
 *
 */
typedef struct
{
   OMX_U32                 nPortIndex;       /**<
                                             * Port number on which this
                                             * command should be applied.
                                             */

   OMX_U32                 nSize;            /**<
                                             * Size of the buffer to be
                                             * allocated in bytes.
                                             */

   OMX_U8                 *pBuffer;          /**<
                                             * This is an out parameter and
                                             * will point to the buffer
                                             * allocated by the device on
                                             * successful execution of this
                                             * command.
                                             */

   OMX_PTR                 pPlatformPvt;     /**<
                                             * This is pointer to any data or
                                             * context that platform wants to
                                             * associate with this buffer.
                                             */

} MMI_AllocBufferCmdType;


/**
 *
 * This data type relates to the command for freeing buffers from the
 * device memory.
 *
 * Commands association:-
 *    MMI_CMD_FREE_BUFFER
 *
 */
typedef struct
{
   OMX_U32                 nPortIndex;       /**<
                                             * Port number on which this
                                             * command should be applied.
                                             */

   OMX_U8                 *pBuffer;          /**<
                                             * This is the buffer allocated
                                             * by device earlier, and should
                                             * be freed now.
                                             */

   OMX_PTR                 pPlatformPvt;     /**<
                                             * This is pointer to any data or
                                             * context that the given platform
                                             * has associated with use/alloc
                                             * buffer command.
                                             */

} MMI_FreeBufferCmdType;


/**
 *
 * This data type relates to the command for stopping device referring buffers
 * that were passed to device.
 *
 * Commands association:-
 *    MMI_CMD_CLEAR_BUFFER
 *
 */
typedef  MMI_FreeBufferCmdType      MMI_ClearBufferCmdType;


/**
 *
 * This data type relates to the command for processing buffers identified
 * by the associated OMX buffer headers.
 *
 * Commands association:-
 *    MMI_CMD_EMPTY_THIS_BUFFER
 *    MMI_CMD_FILL_THIS_BUFFER
 *
 */
typedef struct
{
   OMX_U32                 nPortIndex;       /**<
                                             * Port number on which this
                                             * command should be applied.
                                             */

   OMX_BUFFERHEADERTYPE   *pBufferHdr;       /**<
                                             * Buffer header encapsulating
                                             * the buffer which need to be
                                             * processed and the port with
                                             * which this buffer is
                                             * associated.
                                             */

} MMI_BufferCmdType;


/**
 *
 * This data type relates to the command for setting up a vendor tunnel
 * between two devices.
 *
 * Command association:-
 *       MMI_CMD_VENDOR_TUNNEL_REQUEST
 *
 */
typedef struct
{
   OMX_U32                 nOutputPortIndex; /**<
                                             * This is output port index.
                                             */

   MMI_DeviceType         *pInputDeviceApi;  /**<
                                             * This is pointer to the input
                                             * device table.
                                             */

   OMX_HANDLETYPE          hInputDevice;     /**<
                                             * This is input device handle.
                                             */

   OMX_U32                 nInputPortIndex;  /**<
                                             * This is input port index.
                                             */

} MMI_VendorTunnelCmdType;


/**
 *
 * This data type relates to the command for getting extension index
 * corresponding extension parameter string.
 *
 * Command association:-
 *       MMI_CMD_GET_EXTENSION_INDEX
 *
 */
typedef struct
{
   OMX_STRING              cParamName;       /**<
                                             * This is the string that should
                                             * get translated by the device
                                             * into an index.
                                             */

   OMX_INDEXTYPE          *pIndex;           /**<
                                             * This is an out parameter and it
                                             * is a pointer to a OMX_INDEXTYPE
                                             * to receive the index value.
                                             */

} MMI_GetExtensionCmdType;


/**
 *
 * This data type relates to the event message which is posted when some of
 * the resources are lost.
 *
 * Responses association:-
 *    MMI_EVT_RESOURCES_LOST
 *
 */
typedef struct
{
   OMX_BOOL                bSuspendable;     /**<
                                             * True when current session can
                                             * be suspended and resumed at a
                                             * later point upon resources
                                             * re-acquisition. otherwise, the
                                             * session must be closed.
                                             */

} MMI_ResourceLostMsgType;


/**
 *
 * This data type relates to the event message which is specific to QOMX
 * extensions.
 *
 * Responses association:-
 *    MMI_EVT_QOMX_EXT_SPECIFIC
 *
 */
typedef struct
{
   OMX_EVENTTYPE           eEvent;           /**<
                                             * Event that the device wants
                                             * to notify the application
                                             * about.
                                             * Refer 'EventHandler' prototype
                                             * in IL specification or
                                             * OMX_Core.h for details.
                                             */

   OMX_U32                 nData1;           /**<
                                             * Data associated with the event.
                                             * Refer 'EventHandler' prototype
                                             * in IL specification or
                                             * OMX_Core.h for details.
                                             */

   OMX_U32                 nData2;           /**<
                                             * Data associated with the event.
                                             * Refer 'EventHandler' prototype
                                             * in IL specification or
                                             * OMX_Core.h for details.
                                             */

   OMX_PTR                 pEventData;       /**<
                                             * Data associated with the event.
                                             * Refer 'EventHandler' prototype
                                             * in IL specification or
                                             * OMX_Core.h for details.
                                             */

} MMI_ExtSpecificMsgType;


/**
 *
 * This data type relates to the response messages associated with the buffer
 * processing commands. Buffer headers would contain the processed buffers
 * information.
 *
 * Responses association:-
 *    MMI_RESP_EMPTY_THIS_BUFFER
 *    MMI_RESP_FILL_THIS_BUFFER
 *
 */
typedef  MMI_BufferCmdType    MMI_BufferDoneMsgType;


/**
 *
 * This data type relates to the response or event messages which are related
 * to the ports.
 *
 * Responses association:-
 *    MMI_RESP_ENABLE_PORT
 *    MMI_RESP_DISABLE_PORT
 *    MMI_RESP_FLUSH
 *    MMI_EVT_PORT_CONFIG_CHANGED
 *    MMI_EVT_NEED_BUFFER
 *
 */
typedef  MMI_PortCmdType      MMI_PortMsgType;



#if defined( __cplusplus )
}
#endif /* end of macro __cplusplus */


#endif /* end of macro __H_MMI_DEVICE_API_H__ */
