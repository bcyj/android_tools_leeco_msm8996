#ifndef __HTTPAPI_H__
#define __HTTPAPI_H__
/************************************************************************* */
/**
 * HTTPAPI.h
 * @brief Header file for iHTTPMAPI interface definition.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/HTTPAPI.h#5 $
$DateTime: 2012/08/01 18:57:11 $
$Change: 2653906 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPMAPI.h
** ======================================================================= */
#include <HTTPBase.h>

namespace HTTP_API {
/* -----------------------------------------------------------------------
** Constant / Macro Declarations
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
/**
iHTTPMAPI
The iHTTPMAPI interface is a HTTP specific media interface used internally
by the MAPI Source Media Module to get information/events and configure
properties specific to HTTP streaming source.
*/
class iHTTPAPI : public video::iHTTPBase
{
public:
  /**
  Dtor
  */
  virtual ~iHTTPAPI() {}

  /**
  Initial preroll (in msec).
  */
  static const video::HTTPSourceProperty HTTP_PROPERTY_INIT_PREROLL_MSEC      = 0x010732DC;

  /**
  Rebuffering preroll for data underrun scenarios (in msec).
  */
  static const video::HTTPSourceProperty HTTP_PROPERTY_REBUFF_PREROLL_MSEC    = 0x010732DD;
};

} /* namespace HTTP_API */
#endif /* __HTTPAPI_H__ */
