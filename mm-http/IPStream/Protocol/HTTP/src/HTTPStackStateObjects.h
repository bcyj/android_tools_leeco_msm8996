#ifndef HTTPSTACKSTATEOBJECTS_H
#define HTTPSTACKSTATEOBJECTS_H
/************************************************************************* */
/**
 * HTTPStackStateObjects.h
 * @brief Handle to derived classes of HTTPStackStateBase
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Protocol/HTTP/main/latest/src/HTTPStackStateObjects.h#6 $
$DateTime: 2012/03/20 07:46:30 $
$Change: 2284651 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */


/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "HTTPStackStateMachine.h"

namespace video {

class HTTPStackStateObjects
{
public:
  // State objects:
  static HTTPStateIdle        HTTPStateIdleObj;
  static HTTPStateConnecting  HTTPStateConnectingObj;
  static HTTPStateConnected   HTTPStateConnectedObj;
  static HTTPStateClosed      HTTPStateClosedObj;
  static HTTPStateError       HTTPStateErrorObj;
};

} // end namespace video

#endif /* HTTPSTACKSTATEOBJECTS_H */
