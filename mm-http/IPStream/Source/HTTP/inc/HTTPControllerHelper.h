#ifndef __HTTPCONTROLLERHELPER_H__
#define __HTTPCONTROLLERHELPER_H__
/************************************************************************* */
/**
 * HTTPControllerHelper.h
 * @brief Header file for HTTPControllerHelper.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/HTTPControllerHelper.h#4 $
$DateTime: 2012/03/20 07:46:30 $
$Change: 2284651 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPControllerHelper.h
** ======================================================================= */
#include "HTTPCommon.h"

#include <Url.h>

namespace video {
/* -----------------------------------------------------------------------
** Constant / Macro Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
class HTTPControllerBaseCmd;
class HTTPController;

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
//HTTP controller callback structure - used for HTTPDownloader-->HTTPController
//callback
struct HTTPControllerCbData
{
  //HTTPController reference
  HTTPController* pSelf;

  //Command timeout - used by HTTPDownloader sub tasks
  uint32 nTimeout;

  //iResultRecipient iface used for MAPI callback
  void* pCbData;
};

typedef void (*HTTPControllerCb)(HTTPDownloadStatus HTTPStatus,
                                 const HTTPControllerCbData& callbackData);

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */

class HTTPControllerCmdExecHelper
{
public:
  HTTPControllerCmdExecHelper(const HTTPControllerCommand cmd,
                              HTTPController* pHTTPController = NULL);
  virtual ~HTTPControllerCmdExecHelper(){ };

  HTTPControllerCommand GetCmd() const
  {
    return m_cmd;
  }

  void Notify(const bool bNotify,
              const HTTPDownloadStatus status,
              void* pUserData = NULL);

private:
  HTTPControllerCommand m_cmd;
  HTTPController* m_pHTTPController;
};

}/* namespace video */
#endif /* __HTTPCONTROLLERHELPER_H__ */
