/************************************************************************* */
/**
 * HTTPStackInterface.cpp
 *
 * @brief Implements the HTTP Stack protocol interface
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Protocol/HTTP/main/latest/src/HTTPStackInterface.cpp#9 $
$DateTime: 2012/07/24 22:31:09 $
$Change: 2626836 $

========================================================================== */

#include "HTTPStackInterface.h"
#include "HTTPStack.h"
#include "SourceMemDebug.h"

namespace video
{

/**
 * @brief
 *  Create and return an instance of CmHTTPStack which
 *  implements HTTPStackInterface
 *
 * @param result
 * @param pEnv
 * @param ownerInterface
 *
 * @return HTTPStackInterface*
 */
HTTPReturnCode
HTTPStackInterface::CreateInstance(
  HTTPStackInterface **ppIface,
  HTTPStatusHandlerInterface *ownerInterface,
  HTTPCookieMgr& cookieMgr)
{
  HTTPReturnCode result = HTTP_FAILURE;

  if (NULL == ppIface)
  {
    result = HTTP_BADPARAM;
  }
  else
  {
    *ppIface = QTV_New_Args(HTTPStack, (ownerInterface, cookieMgr));

    if (NULL != *ppIface)
    {
      result = HTTP_SUCCESS;
    }
  }

  return result;
}

/**
 * c'tor
 */
HTTPStackInterface::HTTPStackInterface()
{
}

/**
 * d'tor
 */
HTTPStackInterface::~HTTPStackInterface()
{

}

} // end namespace video
