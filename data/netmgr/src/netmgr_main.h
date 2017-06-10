/******************************************************************************

                          N E T M G R _ M A I N . H

******************************************************************************/

/******************************************************************************

  @file    netmgr_main.h
  @brief   Network Manager main function header file

  DESCRIPTION
  Header file containing definition of NetMgr's main function.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2010-2015 Qualcomm Technologies, Inc. All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
02/08/10   ar         Initial version

******************************************************************************/

#ifndef __NETMGR_MAIN_H__
#define __NETMGR_MAIN_H__

#include "comdef.h"
#include "netmgr.h"
#include "netmgr_defs.h"
#include "netmgr_config.h"

/*===========================================================================
                            GLOBAL FUNCTION DECLARATIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  netmgr_main
===========================================================================*/
/*!
@brief
  Main entry point of the core program. Performs all program initialization.

@return
  int - NETMGR_SUCCESS always

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_main(int argc, char ** argv);


/*===========================================================================
  FUNCTION  netmgr_main_get_qos_enabled
===========================================================================*/
/*!
@brief
  Return value for QOS enabled configuration item

@return
  int - TRUE/FALSE

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_main_get_qos_enabled( void );

/*===========================================================================
  FUNCTION  netmgr_main_get_tcpackprio_enabled
===========================================================================*/
/*!
@brief
  Return value for TCP_ACK_PRIO enabled configuration item from the netmgr
  configuration property.

@return
  int - TRUE/FALSE

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_main_get_tcpackprio_enabled( void );

/*===========================================================================
  FUNCTION  netmgr_main_get_tc_ul_baserate
===========================================================================*/
/*!
@brief
  Return value for TC_BASE_DATARATE enabled configuration item from the netmgr
  configuration property.

@return
  int -

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
unsigned long netmgr_main_get_tc_ul_baserate( void );

/*===========================================================================
  FUNCTION  netmgr_main_get_tc_ul_burst
===========================================================================*/
/*!
@brief
  Return value for TC_UL_BURST enabled configuration item from the netmgr
  configuration property.

@return
  int -

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
unsigned long netmgr_main_get_tc_ul_burst( void );

/*===========================================================================
  FUNCTION  netmgr_main_get_ibfc_enabled
===========================================================================*/
/*!
@brief
  Return value for IBFC enabled configuration item from the netmgr
  configuration property.

@return
  int - TRUE/FALSE

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_main_get_ibfc_enabled( void );

/*===========================================================================
  FUNCTION  netmgr_main_get_iwlan_enabled
===========================================================================*/
/*!
@brief
  Return value for iWLAN enabled configuration item

@return
  int - TRUE/FALSE

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
iwlan_state_t netmgr_main_get_iwlan_enabled( void );


/*===========================================================================
  FUNCTION  netmgr_main_get_iwlan_ims_enabled
===========================================================================*/
/*!
@brief
  Return value for iWLAN IMS enabled configuration item

@return
  int - TRUE/FALSE

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_main_get_iwlan_ims_enabled( void );

/*=========================================================================
  FUNCTION  netmgr_main_get_rmnet_data_enabled
===========================================================================*/
/*!
@brief
  Return value for rmnet data driver enabled configuraition item

@return
  int - TRUE/FALSE

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_main_get_rmnet_data_enabled( void );

/*===========================================================================
  FUNCTION  netmgr_main_reset_links
===========================================================================*/
/*!
@brief
  selects all the links/interfaces for use by NetMgr. Typically,
  this is the default behavior unless another subsystem (e.g.
  USB rmnet) wanted to use one of the default SMD ports.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void netmgr_main_reset_links(void);

/*===========================================================================
  FUNCTION  netmgr_main_update_links
===========================================================================*/
/*!
@brief
  Update the link array to disable those for any SMD port used by
  external subsystem, and any over the number of links requested.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void netmgr_main_update_links(void);

/*===========================================================================
  FUNCTION  netmgr_load_data_format
===========================================================================*/
/*!
@brief
  Populates the runtime data format based on ADB properties or configuration.

@return
  0 always.
*/
/*=========================================================================*/
int netmgr_load_data_format(void);

/*===========================================================================
  FUNCTION  netmgr_main_get_phys_net_dev
===========================================================================*/
/*!
@brief
  Return value for physical network device if any.

@return
  char pointer to physical network device name
  NULL if none such device exists.
*/
/*=========================================================================*/
inline char *netmgr_main_get_phys_net_dev( void );

/*===========================================================================
  FUNCTION  netmgr_main_get_low_latency_filters_enabled
===========================================================================*/
/*!
@brief
  Return value for low latency filters through QMI DFS enablement

@return
  int - TRUE/FALSE
*/
/*=========================================================================*/
inline int netmgr_main_get_low_latency_filters_enabled(void);

#endif /* __NETMGR_MAIN_H__ */
