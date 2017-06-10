/******************************************************************************

                        D S _ S T R I N G . H

******************************************************************************/

/******************************************************************************

  @file    ds_string.h
  @brief   Data Services string Header File

  DESCRIPTION
  Header file for DS string functions.

  ---------------------------------------------------------------------------
  Copyright (c) 2010 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------

******************************************************************************/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
04/27/10   ar         Initial version

******************************************************************************/

#ifndef __DS_STRING_H__
#define __DS_STRING_H__

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// TODO Need to get the proper implmentation for these functions
#define std_strlen         strlen

#ifdef USE_GLIB
#define std_strlcpy        g_strlcpy
#define std_strlcat        g_strlcat
#else
#define std_strlcpy        strlcpy
#define std_strlcat        strlcat
#endif

#define std_strlprintf     snprintf
#define std_stricmp        strcasecmp
#define std_strnicmp       strncasecmp
  
#ifdef __cplusplus
}
#endif

#endif /* __DS_STRING_H__ */
