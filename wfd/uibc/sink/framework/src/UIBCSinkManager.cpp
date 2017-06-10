/*==============================================================================
  *       UIBCSinkManager.h
  *
  *  DESCRIPTION:
  *       Manager class to UIBC sink
  *
  *
  *  Copyright (c) 2011 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
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

#include "UIBCSinkManager.h"
#include "MMDebugMsg.h"

/* ==========================================================================

                              DATA DECLARATIONS

      ========================================================================== */
/* -----------------------------------------------------------------------
      ** Constant / Define Declarations
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
 **                            Function Definitions
 ** ======================================================================= */

UIBCSinkManager::UIBCSinkManager()
  :m_pUIBCTransmitter(NULL)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "UIBCSinkManager:constructor");

  /*Hardcoding local capability values*/
  std_memset(&m_localCapability, 0, sizeof(WFD_uibc_capability_t));
  m_localCapability.category = GENERIC;
  m_localCapability.generic_input_type = 0xFF;//enabling all generic events

  std_memset(&m_negotiatedCapability, 0, sizeof(WFD_uibc_capability_t));
}

UIBCSinkManager::~UIBCSinkManager()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "UIBCSinkManager:destructor");

  if(m_pUIBCTransmitter != NULL)
  {
    MM_Delete(m_pUIBCTransmitter);
    m_pUIBCTransmitter = NULL;
  }
}

/*==========================================================================
   FUNCTION     : sendEvent

   DESCRIPTION:Send the event to source

   PARAMETERS :WFD_uibc_event_t* - pointer to uibc event that need to be sent

   Return Value  : return UIBC_SINK_SUCCESS for success else error code
  ===========================================================================*/

UIBC_sink_status_t UIBCSinkManager::sendEvent(WFD_uibc_event_t* pEvent)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCSinkManager:sendEvent");

  UIBC_sink_status_t nStatus = UIBC_SINK_ERROR_UNKNOWN;

  if(m_pUIBCTransmitter != NULL && pEvent != NULL)
  {
    nStatus = m_pUIBCTransmitter->sendEvent(pEvent);
  }
  else
  {
    MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR, "UIBCSinkManager:Invalid parm transmitter %p  event %p",m_pUIBCTransmitter,pEvent);
    nStatus = UIBC_SINK_ERROR_INVALID;
  }


  return nStatus;
}



/*==========================================================================
   FUNCTION     : getLocalCapability

   DESCRIPTION: Provides the local capability of UIBC
         
   PARAMETERS :  WFD_uibc_capability_t*[out]
   
   Return Value  : return UIBC_SINK_SUCCESS for success else error code
  ===========================================================================*/

UIBC_sink_status_t UIBCSinkManager::getLocalCapability(WFD_uibc_capability_config_t* pLocalCapability)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "UIBCSinkManager:getLocalCapability");

  UIBC_sink_status_t nStatus = UIBC_SINK_ERROR_UNKNOWN;
  if(pLocalCapability != NULL)
  {
    memcpy(pLocalCapability, &m_localCapability, sizeof(WFD_uibc_capability_config_t));
    nStatus = UIBC_SINK_SUCCESS;
  }

  return nStatus;
}

/*==========================================================================
   FUNCTION     : setNegotiatedCapability

   DESCRIPTION: Set the negotiated capability
         
   PARAMETERS :  WFD_uibc_capability_t*[in]
   
   Return Value  : return UIBC_SINK_SUCCESS for success else error code
  ===========================================================================*/

UIBC_sink_status_t UIBCSinkManager::setNegotiatedCapability(WFD_uibc_capability_t*  pNegotiatedCapability)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "UIBCSinkManager:setNegotiatedCapability");

  UIBC_sink_status_t nStatus = UIBC_SINK_ERROR_UNKNOWN;
  if(pNegotiatedCapability != NULL)
  {
    memcpy(&m_negotiatedCapability, pNegotiatedCapability, sizeof(WFD_uibc_capability_t));
    nStatus = UIBC_SINK_SUCCESS;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "UIBCSinkManager:setNegotiatedCapability UIBC_SINK_SUCCESS");
    MM_MSG_PRIO4(MM_GENERAL, MM_PRIO_DEBUG,
      "UIBCSinkManager:setNegotiatedCapability %d %u %d %u",
      pNegotiatedCapability->port_id,pNegotiatedCapability->ipv4_addr,
      m_negotiatedCapability.port_id,m_negotiatedCapability.ipv4_addr);
  }

  return nStatus;

}



/*==========================================================================
   FUNCTION     : enable

   DESCRIPTION:Creates the UIBC Transmitter to establish connection

   PARAMETERS :None

   Return Value  : return UIBC_SINK_SUCCESS for success else error code
  ===========================================================================*/

UIBC_sink_status_t UIBCSinkManager::enable()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "UIBCSinkManager:enable");

  UIBC_sink_status_t nStatus = UIBC_SINK_ERROR_UNKNOWN;

  if(m_negotiatedCapability.port_id != 0 && m_negotiatedCapability.ipv4_addr != 0)
  {
    //note:compilation is failing with MM_New_Args, hence using standard new operator
    if (m_pUIBCTransmitter == NULL)
    {
    m_pUIBCTransmitter = new UIBCPacketTransmitter(m_negotiatedCapability.port_id,m_negotiatedCapability.ipv4_addr,
                            m_negotiatedCapability.negotiated_height ,
                            m_negotiatedCapability.negotiated_width);
    }
    if(m_pUIBCTransmitter != NULL)
    {
     nStatus = m_pUIBCTransmitter->start();
    }
  }
  else
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCSinkManager:Invalid capability");
    nStatus = UIBC_SINK_ERROR_INVALID;
  }
  return nStatus;
}

/*==========================================================================
   FUNCTION     : Disable

   DESCRIPTION:Disable the current uibc session

   PARAMETERS :None

   Return Value  : return UIBC_SINK_SUCCESS for success else error code
  ===========================================================================*/

UIBC_sink_status_t UIBCSinkManager::disable()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "UIBCSinkManager:disable");

  UIBC_sink_status_t nStatus = UIBC_SINK_ERROR_UNKNOWN;

  if(m_pUIBCTransmitter != NULL)
  {
    m_pUIBCTransmitter->stop();

    nStatus = UIBC_SINK_SUCCESS;
  }
  return nStatus;
}
