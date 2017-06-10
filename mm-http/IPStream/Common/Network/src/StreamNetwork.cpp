/*
 * StreamNetwork.cpp
 *
 * @brief Implements the Stream Services Network interface.
 *
 * Define the interface to create an instance of the Stream
 * Source network interface.
 *
 * COPYRIGHT 2011-2012 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/Network/BSD/main/latest/src/StreamNetwork.cpp#8 $
$DateTime: 2012/03/20 08:56:06 $
$Change: 2284745 $

========================================================================== */

/*===========================================================================

                           INCLUDE FILES FOR MODULE

===========================================================================*/
/* BSD includes */
#include "StreamNetwork.h"
#include "StreamNetworkBSD.h"
/* other utils */
#include "SourceMemDebug.h"

/*
 * @brief: Creates an instance of the Stream Services Nekwork Object
 *
 * @return. Returns an instance of the Stream Network Object
 *
 */
CStreamNetwork *CStreamNetwork::CreateInstance
(
 bool /*bPersistentConnection*/
)
{
  CStreamNetwork *pSTREAMNet = NULL;
  int result = 1;
  pSTREAMNet = QTV_New_Args(CStreamNetworkBSD, (result));

  if ( pSTREAMNet && result != 0 )
  {
    QTV_Delete(pSTREAMNet);
    pSTREAMNet = NULL;
  }

  return pSTREAMNet;
}
