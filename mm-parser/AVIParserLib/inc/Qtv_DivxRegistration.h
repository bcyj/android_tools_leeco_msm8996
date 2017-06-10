#ifndef _QTVPLAYER_DIVX_DRM_REG_H_
#define _QTVPLAYER_DIVX_DRM_REG_H_

/* =======================================================================
                               Qtv_DivxRegistration.h
DESCRIPTION
 Retrieves the DRM registration code for DivX DRM.

Copyright 2011-2014 Qualcomm Technologies, Inc., All Rights Reserved
========================================================================== */

/* =======================================================================
                             Edit History
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/AVIParserLib/main/latest/inc/Qtv_DivxRegistration.h#10 $
$DateTime: 2014/01/31 02:45:41 $
$Change: 5198970 $
========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "parserdatadef.h"
#include "parserinternaldefs.h"

#include "AEEStdDef.h" 

#ifdef FEATURE_FILESOURCE_DIVX_DRM

#include "MMDebugMsg.h" 
#include "DrmTypes.h"
#include "avidatatypes.h"

class QtvDivXDrmClient
{
  public:
    QtvDivXDrmClient();
    ~QtvDivXDrmClient();
    bool GetRegistrationCode(char*,int*);    

  private:
    char m_regCode[DRM_REGISTRATION_CODE_BYTES];
    avi_uint8* m_drmContext;
    int m_nLength;
};

#endif//#ifdef FEATURE_FILESOURCE_DIVX_DRM

#endif//#define _QTVPLAYER_DIVX_DRM_REG_H_
