/* =======================================================================
                               Qtv_DivxRegistration.cpp
DESCRIPTION
 Retrieves the DRM registration code for DivX DRM.

Copyright 2011-2014 Qualcomm Technologies Incorporated, All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

========================================================================== */

/* =======================================================================
                             Edit History
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/AVIParserLib/main/latest/src/Qtv_DivxRegistration.cpp#15 $
$DateTime: 2014/01/31 02:45:41 $
$Change: 5198970 $
========================================================================== */
/* ==========================================================================
                     INCLUDE FILES FOR MODULE
========================================================================== */
#include "Qtv_DivxRegistration.h"

/*==========================================================================*/

#ifdef FEATURE_FILESOURCE_DIVX_DRM

/* ==========================================================================
                     Include DRM library and AVI Parser Data Types
========================================================================== */
#include "DrmApi.h"
#include "MMMalloc.h"
/*==========================================================================*/

/* ======================================================================
FUNCTION
  QtvDivXDrmClient::QtvDivXDrmClient

DESCRIPTION
  Constructor

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

========================================================================== */
QtvDivXDrmClient::QtvDivXDrmClient()
{
  memset(m_regCode,0,DRM_REGISTRATION_CODE_BYTES);
  m_drmContext = NULL;
  m_nLength = 0;
}
/* ======================================================================
FUNCTION
  QtvDivXDrmClient::~QtvDivXDrmClient

DESCRIPTION
  Destructor

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

========================================================================== */
QtvDivXDrmClient::~QtvDivXDrmClient()
{
  if(m_drmContext)
  {
    MM_Free(m_drmContext);
    m_drmContext = NULL;
    m_nLength = 0;
  }
}
/* ======================================================================
FUNCTION
  QtvDivXDrmClient::GetRegistrationCode

DESCRIPTION
  Retrieves the DivX DRM registration code.

DEPENDENCIES
  None

RETURN VALUE
  True is successful otherwise returns false.

SIDE EFFECTS
  None

========================================================================== */
bool QtvDivXDrmClient::GetRegistrationCode(char* code,int* max_length)
{
  drmErrorCodes_t result;
  avi_uint32 drmContextLength;
  int max = 3;

  if( !max_length )
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_FATAL, "QtvDivXDrmClient::GetRegistrationCode INVALID PARAMETER..");
    return false;
  }

  if(!code)
  {
    *max_length = DRM_REGISTRATION_CODE_BYTES;
    return true;
  }
  if((*max_length) < DRM_REGISTRATION_CODE_BYTES)
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_FATAL, "QtvDivXDrmClient::GetRegistrationCode INVALID PARAMETER..");
    return false;
  }
  if(m_nLength > 0)
  {
    //No need to retrieve the code again.
    memcpy(code,m_regCode,DRM_REGISTRATION_CODE_BYTES);
    return true;
  }
  result = drmInitSystem( NULL,&drmContextLength );

  m_drmContext = (uint8_t*)MM_Malloc( drmContextLength  );

  if(!m_drmContext)
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_FATAL, "drmInitSystem memory allocation failed..");
    return false;
  }
  result = drmInitSystem( m_drmContext, &drmContextLength );
  if ( DRM_SUCCESS!= result )
  {
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_FATAL, "drmInitSystem failed result %d",result);
    return false;
  }
#ifdef FEATURE_FILESOURCE_DIVX_DRM_MOBILE_THEATER
  max = 4;
#endif

  for ( int i = 0; i < max; i++ )
  {
    drmSetRandomSample( m_drmContext );
  }

#ifdef FEATURE_FILESOURCE_DIVX_DRM_MOBILE_THEATER
  uint8_t szUserID[30];
  uint32_t nUserIDLen = 30;
  result = drmGetActivationStatus(szUserID,&nUserIDLen);
  int selection =0;
  DrmInfoT* drminfo = 0 ;

  HWND hOwnerWin = NULL;

  switch(result)
  {
    case DRM_NEVER_REGISTERED:
      result = drmGetRegistrationCodeString( m_drmContext,m_regCode );
      break;
    case DRM_SUCCESS:
      selection = CQCMediaDlg::ShowInfoDlg(hOwnerWin, DIVX_INFO_DEREGISTRATION_CODE, drminfo);
      if(selection ==IDYES)
      {
        result = drmGetDeactivationCodeString( m_drmContext,m_regCode );
        selection = CQCMediaDlg::ShowInfoDlg(hOwnerWin, DIVX_INFO_DEREGISTRATION_CODE_1,drminfo,m_regCode,MB_OK);
      }
      else if(selection ==IDNO)
      {
        /*result = drmGetRegistrationCodeString( m_drmContext,m_regCode );
        selection = CQCMediaDlg::ShowInfoDlg(hOwnerWin, DIVX_INFO_DEREGISTRATION_CODE_2,drminfo,m_regCode,MB_OK);*/
      }
      break;
    case DRM_NOT_REGISTERED:
      result = drmGetDeactivationCodeString( m_drmContext,m_regCode );
      selection = CQCMediaDlg::ShowInfoDlg(hOwnerWin, DIVX_INFO_DEREGISTRATION_CODE_2,drminfo,m_regCode,MB_OK);
      if(selection ==IDOK)
      {
        result = drmGetRegistrationCodeString( m_drmContext,m_regCode );
      }
      break;
  default:
    return false;
  }
#else
  result = drmGetRegistrationCodeString( m_drmContext,m_regCode );
#endif

  if ( DRM_SUCCESS!= result )
  {
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_FATAL, "drmGetRegistrationCodeString failed result %d",result);
    return false;
  }
  m_nLength = drmContextLength;
  *max_length = DRM_REGISTRATION_CODE_BYTES;
  memcpy(code,m_regCode,DRM_REGISTRATION_CODE_BYTES);
  return true;
}

#endif//#ifdef FEATURE_FILESOURCE_DIVX_DRM
