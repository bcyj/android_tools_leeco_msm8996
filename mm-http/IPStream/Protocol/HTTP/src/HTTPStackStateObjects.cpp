/************************************************************************* */
/**
 * HTTPStackStateObjects.cpp
 * @brief implementation of the HTTPStackStateObjects.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Protocol/HTTP/main/latest/src/HTTPStackStateObjects.cpp#6 $
$DateTime: 2012/03/20 07:46:30 $
$Change: 2284651 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "HTTPStackStateObjects.h"

namespace video {

HTTPStateIdle        HTTPStackStateObjects::HTTPStateIdleObj;
HTTPStateConnecting  HTTPStackStateObjects::HTTPStateConnectingObj;
HTTPStateConnected   HTTPStackStateObjects::HTTPStateConnectedObj;
HTTPStateClosed      HTTPStackStateObjects::HTTPStateClosedObj;
HTTPStateError       HTTPStackStateObjects::HTTPStateErrorObj;

} // end namespace video
