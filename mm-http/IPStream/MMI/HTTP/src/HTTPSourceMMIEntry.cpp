/************************************************************************* */
/**
 * HTTPSourceMMIEntry.cpp
 * @brief Implementation of the static routine to create HTTPSourceMMI.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/MMI/HTTP/dev/DASH/src/HTTPSourceMMIEntry.cpp#6 $
$DateTime: 2012/07/13 00:08:15 $
$Change: 2588515 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPSourceMMIEntry.cpp
** ======================================================================= */
#include "httpInternalDefs.h"
#include <OMX_Core.h>
#include <OMX_Types.h>
#include "mmiDeviceApi.h"
#ifdef FEATURE_HTTP_USE_OMX_COMPONENT_API
#include "qomxCmpntApi.h"
#endif
#include "HTTPSourceMMIEntry.h"
#include "HTTPSourceMMI.h"

/* =======================================================================
**                      Data Declarations
** ======================================================================= */

/* -----------------------------------------------------------------------
** Constant / Macro Definitions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Definitions
** ======================================================================= */
/** @brief HTTP Source MMI component initiation
  *
  * @param[in] hComponent
  *
  * @return init status
  */
extern "C" {

OMX_ERRORTYPE QOMX_MMIV_HTTP_ComponentInit(OMX_IN OMX_HANDLETYPE /*hComponent*/)
{
   OMX_ERRORTYPE ret = OMX_ErrorBadParameter;
#ifdef FEATURE_HTTP_USE_OMX_COMPONENT_API
   QOMX_CmpntConfig  httpConfig;
   MMI_DeviceType    httpDevice;
   OMX_UUIDTYPE      UUID = MMI_HTTP_COMPONENT_UUID;

   if(NULL == hComponent)
   {
     return ret;
   }

   QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                "QOMX_HTTP_ComponentInit called" );

   OMX_COMPONENTTYPE *hCmp = (OMX_COMPONENTTYPE *)hComponent;

   httpDevice.pfnOpen    = video::HTTPSourceMMI::HTTPSourceMMIOpen;
   httpDevice.pfnClose   = video::HTTPSourceMMI::HTTPSourceMMIClose;
   httpDevice.pfnCmd     = video::HTTPSourceMMI::HTTPSourceMMICmd;
   httpDevice.pfnEvtHdlr = video::HTTPSourceMMI::HTTPSourceMMIRegisterEventHandler;

   //Audio port initialization
   httpConfig.audioDomain.bConfigSet        = OMX_TRUE;
   httpConfig.audioDomain.nPortStart        = MMI_HTTP_AUDIO_PORT_INDEX;
   httpConfig.audioDomain.nNumInputPorts    = 0;
   httpConfig.audioDomain.nNumOutputPorts   = MMI_HTTP_NUM_AUDIO_PORTS;

   //Video Port Initialization
   httpConfig.videoDomain.bConfigSet        = OMX_TRUE;
   httpConfig.videoDomain.nPortStart        = MMI_HTTP_VIDEO_PORT_INDEX;
   httpConfig.videoDomain.nNumInputPorts    = 0;
   httpConfig.videoDomain.nNumOutputPorts   = MMI_HTTP_NUM_VIDEO_PORTS;

   //Image Port Initialization
   httpConfig.imageDomain.bConfigSet        = OMX_FALSE;
   httpConfig.imageDomain.nPortStart        = MMI_HTTP_IMAGE_PORT_INDEX;
   httpConfig.imageDomain.nNumInputPorts    = 0;
   httpConfig.imageDomain.nNumOutputPorts   = MMI_HTTP_NUM_IMAGE_PORTS;

   //Other Port initialization
   httpConfig.otherDomain.bConfigSet        = OMX_TRUE;
   httpConfig.otherDomain.nPortStart        = MMI_HTTP_OTHER_PORT_INDEX;
   httpConfig.otherDomain.nNumInputPorts    = 0;
   httpConfig.otherDomain.nNumOutputPorts   = MMI_HTTP_NUM_OTHER_PORTS;

   // component Name
   httpConfig.pCmpnntName                   = MMI_HTTP_COMPONENT_NAME ;
   httpConfig.bSupportPortBasedResources    = OMX_TRUE;

   //component UUID
   httpConfig.pCmpnntUUID                   = &UUID;

   ret = qomx_CmpntInit(&httpConfig, &httpDevice, hCmp);
#endif
   return ret;
}

/*
 * This function will be called during the Component init, the function will
 * open the device
 *
 * @param[out] pHandle  -       device descripter returned by the device
 *
 * @return MMI_S_EFAIL on failure MMI_S_COMPLETE on success.
 */

OMX_U32 HTTPMMIDeviceOpen(OMX_HANDLETYPE* pHandle)
{
  /* Call the underlying device Open */
  uint32 nReturn = MMI_S_EFAIL;
  if (pHandle)
  {
    nReturn = video::HTTPSourceMMI::HTTPSourceMMIOpen(pHandle);
  }
  return nReturn;
}


/*
 * This function will be called during the Component deinit, the function will
 * close the device
 *
 * @param[in] handle -  descripter of the device to close
 *
 * @return MMI_S_EFAIL on failure MMI_S_COMPLETE on success.
 */
OMX_U32 HTTPMMIDeviceClose(OMX_HANDLETYPE handle)
{
  /* Call the underlying device Close */
  uint32 nReturn = MMI_S_EFAIL;

  nReturn = video::HTTPSourceMMI::HTTPSourceMMIClose(handle);

  return nReturn;
}

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
OMX_U32 HTTPMMIDeviceCommand( OMX_HANDLETYPE handle, OMX_U32 nCode, void *pData)
{
  /* Call the underlying device command processing */
  OMX_U32 nReturn = MMI_S_EFAIL;
  nReturn = video::HTTPSourceMMI::HTTPSourceMMICmd(handle, nCode, pData);
  return nReturn;
}

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
                                    void *pClient)
{
  /* Call the underlying device register handler */
  uint32 nReturn = MMI_S_EFAIL;
  nReturn = video::HTTPSourceMMI::HTTPSourceMMIRegisterEventHandler(handle,
                                                                    pfEvHandler,
                                                                    pClient);
  return nReturn;
}

}
