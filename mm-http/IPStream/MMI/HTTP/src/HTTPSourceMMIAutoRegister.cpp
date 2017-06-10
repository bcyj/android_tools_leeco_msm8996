/************************************************************************* */
/**
 * HTTPSourceMMIAutoRegister.cpp
 * @brief Implementation of HTTPSourceAutoRegister.
 *  HTTPSourceAutoRegister does the auto registration of the component in
 *  in qc omx core.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/MMI/HTTP/dev/DASH/src/HTTPSourceMMIAutoRegister.cpp#4 $
$DateTime: 2012/03/20 07:46:30 $
$Change: 2284651 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPSourceMMIAutoRegister.cpp
** ======================================================================= */
#include "qc_omx_core.h"
#include  "HTTPMMIComponent.h"

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

static OMX_U8* HTTPMMIStreamingRoles[] =
{
    (OMX_U8 *)"container_streaming.http",
    (OMX_U8 *)"*"
};

static OMXILCORE_COMPONENTINFOTYPE HTTPMMIComponent =
{
    MMI_HTTP_COMPONENT_NAME,
    QOMX_MMIV_HTTP_ComponentInit,
    sizeof(HTTPMMIStreamingRoles)/sizeof(OMX_U8*),
    HTTPMMIStreamingRoles
};

static OmxComponentRegister HTTPMMIRegister(&HTTPMMIComponent);

