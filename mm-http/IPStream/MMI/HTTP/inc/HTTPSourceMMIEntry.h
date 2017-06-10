#ifndef __HTTPSOURCEMMIENTRY_H__
#define __HTTPSOURCEMMIENTRY_H__
/************************************************************************* */
/**
 * HTTPSourceMMIEntry.h
 * @brief Header file for HTTPSourceMMIEntry.
 *
 COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/MMI/HTTP/dev/DASH/inc/HTTPSourceMMIEntry.h#5 $
$DateTime: 2012/03/20 07:46:30 $
$Change: 2284651 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPSourceMMIEntry.h
** ======================================================================= */
#include "HTTPMMIComponent.h"
/* -----------------------------------------------------------------------
** Constant / Macro Declarations
** ----------------------------------------------------------------------- */

#define QOMX_STRUCT_INIT(_s_, _name_)\
   memset(&(_s_), 0x0, sizeof(_name_));\
   (_s_).nSize = (OMX_U32)sizeof(_name_);\
   (_s_).nVersion.s.nVersionMajor = 1;\
   (_s_).nVersion.s.nVersionMinor = 1;\
   (_s_).nVersion.s.nRevision = 2;\
   (_s_).nVersion.s.nStep = 1

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */
#ifdef __cplusplus
extern "C" {
#endif
/*
 * This function will be called during the Component init, the function will
 * open the device
 *
 * @param[out] pHandle  -       device descripter returned by the device
 *
 * @return MMI_S_EFAIL on failure MMI_S_COMPLETE on success.
 */
OMX_U32 HTTPMMIDeviceOpen(OMX_HANDLETYPE* pHandle);

/*
 * This function will be called during the Component deinit, the function will
 * close the device
 *
 * @param[in] handle -  descripter of the device to close
 *
 * @return MMI_S_EFAIL on failure MMI_S_COMPLETE on success.
 */
OMX_U32 HTTPMMIDeviceClose(OMX_HANDLETYPE handle);

/*
 * The function implements all the commands supported by the device
 *
 * @param[in]   handle descripter of the device
 * @param[in]   nCode MMI command code
 * @param[in]   pData data associated with the command
 *
 * @return MMI_S_EFAIL on failure MMI_S_SUCCESS_COMPLETE on success and other
 * codes as appropriate.
 */
OMX_U32 HTTPMMIDeviceCommand( OMX_HANDLETYPE handle, OMX_U32 nCode, void *pData);

/*
 *  Event callback registration function to notify asynchronous events from the
 *  device to the upper layers
 *
 * @param[in]   handle descripter of the device
 * @param[in]   pfEvHandler Event handler
 * @param[in]   pClient client specific data to be along with events
 *
 * @return MMI_S_EFAIL on failure MMI_S_SUCCESS_COMPLETE on success
 */
OMX_U32 HTTPMMIRegisterEventHandler(OMX_HANDLETYPE handle,
                                    MMI_CmpntEvtHandlerType pfEvHandler,
                                    void *pClient);

#ifdef __cplusplus
}
#endif

#endif /* __HTTPSOURCEMMIENTRY_H__ */
