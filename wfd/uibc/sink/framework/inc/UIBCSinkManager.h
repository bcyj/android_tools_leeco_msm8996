#ifndef __UIBC_SINK_MANAGER_H__
#define __UIBC_SINK_MANAGER_H__
/*==============================================================================
  *       UIBCSinkManager.h
  *
  *  DESCRIPTION: 
  *       Manager class to UIBC sink
  *
  *
  *  Copyright (c) 2011 - 2012  by Qualcomm Technologies, Inc. All Rights Reserved. 
  *  Qualcomm Technologies Proprietary and Confidential.
  *==============================================================================*/
/* =======================================================================
                             Edit History
========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "UIBCPacketTransmitter.h"

/* ==========================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
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

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */


/* =======================================================================
**                        Class Declarations
** ======================================================================= */
class UIBCSinkManager
{
public:

  UIBCSinkManager();

  virtual ~UIBCSinkManager();

  UIBC_sink_status_t sendEvent(WFD_uibc_event_t*);

  UIBC_sink_status_t setNegotiatedCapability(WFD_uibc_capability_t* );

  UIBC_sink_status_t getLocalCapability(WFD_uibc_capability_config_t* );

  UIBC_sink_status_t enable();

  UIBC_sink_status_t disable();

private:

  //local capability
  WFD_uibc_capability_config_t m_localCapability;

  WFD_uibc_capability_t m_negotiatedCapability;

  UIBCPacketTransmitter* m_pUIBCTransmitter;

};
#endif/*__UIBC_SINK_MANAGER_H__ */
