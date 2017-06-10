// Local QTV Feature Definitions

#ifndef _HTTPINTERNALDEF_
#define _HTTPINTERNALDEF_

/* =======================================================================
                              httpInternalDefs.h
DESCRIPTION
 This file should include #defines (aka FEATURES) which are not customer
 configurable and meant for internal use to http. For example during develoment
 we can use some #define to guard a piece of code which will be main lined
 after development.

COPYRIGHT 2011-2012 Qualcomm Technologies, Inc.
All rights reserved. Qualcomm Technologies proprietary and confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/httpInternalDefs.h#5 $
$DateTime: 2012/03/20 07:46:30 $
$Change: 2284651 $


========================================================================== */
/* use omx component api*/
//#define FEATURE_HTTP_USE_OMX_COMPONENT_API

/* Turning on evrc*/
//#define FEATURE_HTTP_EVRC

/* Turning on wm*/
//#define FEATURE_HTTP_WM

/* Turning on wm*/
//#define FEATURE_HTTP_AMR

//Feature for collecting DASH stats
//#define DASH_STATS

#endif /* _HTTPINTERNALDEF_ */
