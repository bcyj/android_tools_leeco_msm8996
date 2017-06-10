/*============================================================================
  @file sns_main.h

  @brief Header file for sns_main.h. Provides externalized functions for the
  Linux Android sensors library.

  <br><br>

  DEPENDENCIES:

  Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/

/*============================================================================
  Preprocessor Definitions and Constants
  ============================================================================*/

#define SNS_USERNAME "sensors"
#define SNS_GROUPNAME "sensors"

#define ROOT_GROUPNAME "root"
#define SYSTEM_GROUPNAME "system"
#define GPS_GROUPNAME "gps"
#define CAMERA_GROUPNAME "camera"

#define USERNAME_NOBODY "nobody"
#define GROUPNAME_NET_RAW "net_raw"
#define GROUPNAME_NOBODY "nobody"


/*============================================================================
  Function Declarations
  ============================================================================*/

/*===========================================================================

  FUNCTION:   sns_main_exit

  ===========================================================================*/
/*!
  @brief Causes the main thread to exit.
*/
/*=========================================================================*/
void sns_main_exit( void );
