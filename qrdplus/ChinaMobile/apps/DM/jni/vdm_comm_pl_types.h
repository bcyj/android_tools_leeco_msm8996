#ifdef _VDM_COMM_PL_TYPES_H_
#error "vdm_comm_pl_types.h doubly included"
#endif
#define _VDM_COMM_PL_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *******************************************************************************
 * Notify that a communication open request has finished successfully.
 *
 * \param   inContext   Observer's context.
 * \param   inConnId    ID of new connection.
 *
 * \return  VDM_ERR_OK on success, or an error code on failure.
 *******************************************************************************
 */
typedef VDM_Error (*VDM_Comm_notifyCommOpenCB)(VDM_Handle_t inContext,
        IS32 inConnId);

/*!
 *******************************************************************************
 * Notify that a communication transport event has occurred.
 *
 * \param   inContext   Observer's context.
 * \param   inConnId    ID of new connection where transport has occurred.
 *
 * \return  VDM_ERR_OK on success, or an error code on failure.
 *******************************************************************************
 */
typedef VDM_Error (*VDM_Comm_notifyTransportCB)(VDM_Handle_t inContext,
        IS32 inConnId);

/*!
 *******************************************************************************
 * Notify that a communication open request has failed or that an open
 * communication has been broken.
 *
 * \param   inContext           Observer's context.
 * \param   inConnId            ID of the broken connection.
 * \param   inIsFatalError      Whether this is a fatal or a recoverable error.
 *
 * \return  VDM_ERR_OK on success, or an error code on failure.
 *******************************************************************************
 */
typedef VDM_Error (*VDM_Comm_notifyCommBrokenCB)(VDM_Handle_t inContext,
        IS32 inConnId, IBOOL inIsFatalError);

typedef VDM_Error (*VDM_Comm_notifyCommPauseCB)(VDM_Handle_t inContext,
        IS32 inConnId, IBOOL inIsFatalError);

/*!
 *******************************************************************************
 * The Comm Observer object should be notified when an asynchronous
 * communication event has occurred. This is done by calling one of the
 * callbacks of the comm observer. The Comm observer's context must be passed
 * in every notification callback.
 *
 *******************************************************************************
 */
typedef struct {
    VDM_Handle_t context; /**< Comm observer's context. */
    VDM_Comm_notifyCommOpenCB notifyCommOpen; /**< Open Request callback. */
    VDM_Comm_notifyTransportCB notifyTransport; /**< Transport notification. */
    VDM_Comm_notifyCommBrokenCB notifyCommBroken; /**< Communication break notification. */
    VDM_Comm_notifyCommPauseCB notifyCommPause; /**< Communication pause notification. */
} VDM_CommObserver_t;

/*! HMAC Authentication data. */
typedef struct {
    UTF8CStr algorithm; /**< optional. if missing MD5 is assumed */
    UTF8CStr username; /**< identity of the sender of the message */
    UTF8CStr mac; /**< digest values */
} VDM_Comm_HMAC_t;

#ifdef __cplusplus
}
#endif

