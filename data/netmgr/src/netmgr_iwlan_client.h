#ifndef _NETMGR_IWLAN_CLIENT_H_
#define _NETMGR_IWLAN_CLIENT_H_

/******************************************************************************

                        N E T M G R _ I W L A N _C L I E N T . H

******************************************************************************/

/******************************************************************************

  @file    netmgr_iwlan_client.h
  @brief   Network manager iWLAN client

  DESCRIPTION
  Header file for iwlan client module

******************************************************************************/
/*===========================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/*===========================================================================
                     GLOBAL DEFINITIONS AND DECLARATIONS
===========================================================================*/
#ifdef FEATURE_DATA_IWLAN
typedef enum
{
  NETMGR_IWLAN_CLIENT_INVALID_EV = -1,
  NETMGR_IWLAN_CLIENT_OOS_EV,
  NETMGR_IWLAN_CLIENT_IS_EV,
  NETMGR_IWLAN_CLIENT_MAX_EV
} netmgr_client_event_t;

/*===========================================================================
  FUNCTION  netmgrIwlanClientInit
===========================================================================*/
/*!
@brief
  Initializes the netmgr iwlan client state

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgrIwlanClientInit(void);

/*===========================================================================
  FUNCTION  netmgrIwlanClientRelease
===========================================================================*/
/*!
@brief
  Cleans up the netmgr iwlan client state

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgrIwlanClientRelease(void);

/*===========================================================================
  FUNCTION  netmgrIwlanClientProcessEvent
===========================================================================*/
/*!
@brief
  Cleans up the netmgr iwlan client state during SSR

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgrIwlanClientProcessEvent(netmgr_client_event_t evt);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* FEATURE_DATA_IWLAN */
#endif /* _NETMGR_IWLAN_CLIENT_H_ */

