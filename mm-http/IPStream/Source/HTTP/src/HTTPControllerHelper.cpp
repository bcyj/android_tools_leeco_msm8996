/************************************************************************* */
/**
 * HTTPControllerHelper.cpp
 * @brief implementation of HTTPControllerHelper.
 *  HTTPControllerHelper is the helper class for HTTPController. It primarily
 *  implements the input command queue for HTTPController.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/src/HTTPControllerHelper.cpp#4 $
$DateTime: 2012/03/20 07:46:30 $
$Change: 2284651 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPDownloader.cpp
** ======================================================================= */
#include "HTTPControllerHelper.h"
#include "HTTPController.h"

#include <SourceMemDebug.h>

namespace video {
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
/** @brief HTTPControllerCmdExecHelper Constructor.
  *
  * @param[in] cmd - HTTPController command
  * @param[in] pHTTPController - Reference to HTTP controller
  */
HTTPControllerCmdExecHelper::HTTPControllerCmdExecHelper
(
 const HTTPControllerCommand cmd,
 HTTPController* pHTTPController
)
{
  m_cmd = cmd;
  m_pHTTPController = pHTTPController;
}

/** @brief Notify HTTP event to MAPI source.
  *
  * @param[in] bNotify - Flag indicating whether or not to notify
  * @param[in] status - HTTP command status
  * @param[in] pUserData - Reference to MAPI source callback data
  */
void HTTPControllerCmdExecHelper::Notify
(
 const bool bNotify,
 const HTTPDownloadStatus status,
 void* pUserData
)
{
  if (bNotify && m_pHTTPController)
  {
    m_pHTTPController->NotifyHTTPEvent(m_cmd, status, pUserData);
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                  "HTTP MAPI notification suppressed" );
  }
}

}/* namespace video */
