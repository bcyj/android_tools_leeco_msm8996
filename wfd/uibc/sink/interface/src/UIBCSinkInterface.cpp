/*==============================================================================
*  @file UIBCSinkInterface.cpp
*
*  @par  DESCRIPTION:
*        UIBC Sink Interface with Session Manager.
*        This is sink interface for UIBC with Session Manager
*
*
*  Copyright (c) 2012 - 2014 by Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================

  $Header:
==============================================================================*/

#include "UIBCSinkInterface.h"
#include "UIBCSinkManager.h"
#include "MMMalloc.h"
#include "MMDebugMsg.h"
#include "wfd_cfg_parser.h"
#include <stdio.h>


#define FEATURE_CONFIG_FROM_FILE

  /** @brief Get UIBC capabity parameters structure
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Sink instance handle (not used)
               WFD_uibc_capability_config_t* - pointer to UIBC capability structure
  *
  * @return  WFD_status_t - status
  */
UIBCSinkInterface::UIBCSinkInterface ()
{
  m_pUIBCHandle = NULL;
}

UIBCSinkInterface::~UIBCSinkInterface ()
{
  if (m_pUIBCHandle != NULL)
  {
    MM_Delete(m_pUIBCHandle);
  }
  m_pUIBCHandle = NULL;
}


bool UIBCSinkInterface::readUIBCCfgCapability(WFD_uibc_capability_t* pUibcCapability)
{
#ifdef FEATURE_CONFIG_FROM_FILE
      {
         readConfigFile getUIBCCap;
         memset (&getUIBCCap, 0,sizeof(readConfigFile));

        // read the uibc supported key value and then parse the cfg based on that
         getCfgItem(UIBC_VALID_KEY,&m_bSupportedFlag );

        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                    "UIBCSinkInterface::readUIBCCfgCapability UIBC support = %d",m_bSupportedFlag);

         if( !m_bSupportedFlag )
         {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"UIBCSinkInterface::"\
                                    "readUIBCCfgCapability UIBC NOT SUPPORTED");
            return false;
         }

         parseCfgforUIBC(WFD_CFG_FILE_SINK,&getUIBCCap);

         pUibcCapability->config.category =
                                      getUIBCCap.uibcCapability.config.category;
         pUibcCapability->config.generic_input_type =
                            getUIBCCap.uibcCapability.config.generic_input_type;
         for (int i= 0; i<UIBC_NUM_INPUT_PATHS;i++)
        {
            pUibcCapability->config.hid_input_type_path[i]= getUIBCCap.uibcCapability.config.hid_input_type_path[i];
        }
         pUibcCapability->port_id = 0;

         MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_ERROR,"UIBCSinkInterface::readUIBCCfgCapability "\
                                                "Category : %d "\
                                                "Generic Input : %d "\
                                                "Port ID : %d",
                                                pUibcCapability->config.category,
                                                pUibcCapability->config.generic_input_type,
                                                pUibcCapability->port_id);
         return true;
      }
#else
      {
         MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"UIBCSinkInterface::"\
                     "readUIBCCfgCapability FEATURE_CONFIG_FROM_FILE not defined");
         return false;
      }
#endif
      return false;
}


void UIBCSinkInterface::getUibcCapability(WFD_uibc_capability_t* pUibcCapability)
{
  bool bUIBCfgCap = false;
  if (pUibcCapability != NULL)
  {
    memset (pUibcCapability, 0,sizeof(WFD_uibc_capability_t));
    bUIBCfgCap = readUIBCCfgCapability(pUibcCapability);
    if(!bUIBCfgCap)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"UIBCSinkInterface : UIBC Not supported");
    }
  }
}


bool UIBCSinkInterface::createUIBCSession()
{
  if( !m_bSupportedFlag )
  {
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                "UIBCSinkInterface::createSession UIBC not supported = %d",m_bSupportedFlag);
   return false;
  }
  if(!m_pUIBCHandle)
  {
    m_pUIBCHandle = MM_New (UIBCSinkManager);
  }
  else
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                "UIBCSinkInterface::createSession m_pUIBCHandle already exists");
    return false;
  }
  if (m_pUIBCHandle == NULL)
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                "UIBCSinkInterface::createSession to create UIBC handle");
    return false;
  }
  return true;
}

bool UIBCSinkInterface::destroyUIBCSession()
{
  if (m_pUIBCHandle != NULL)
  {
    MM_Delete(m_pUIBCHandle);
    m_pUIBCHandle = NULL;
    return true;
  }
  return false;
}

/** @brief Enable the UIBC session
  *
  * @return  bool - status
  */
bool UIBCSinkInterface::Enable()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "UIBCSinkInterface::Enable");
  bool retStatus = false;
  if( m_pUIBCHandle )
  {
    retStatus = m_pUIBCHandle->enable();
  }
  return retStatus;
}

/** @brief Disable the UIBC session
  *
  * @return  bool - status
  */
bool UIBCSinkInterface::Disable()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "UIBCSinkInterface::Disable");
  bool retStatus = false;
  if( m_pUIBCHandle )
  {
    retStatus = m_pUIBCHandle->disable();
  }
  return retStatus;
}

/** @brief Send the captured UIBC event
  *
  * @param[in] WFD_uibc_event_t - UIBC event
  * @return  bol - status
  */
 bool UIBCSinkInterface::sendUIBCEvent(WFD_uibc_event_t* pEvent)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCSinkInterface::sendUIBCEvent");
  bool retStatus = false;
  if( m_pUIBCHandle )
  {
     if(UIBC_SINK_SUCCESS == m_pUIBCHandle->sendEvent(pEvent))
    {
      retStatus = true;
    }	
  }
   return retStatus;
}

/** @brief Set the UIBC capabilities
  *
  * @param[in] WFD_uibc_capability_t - UIBC capabilities
  * @return  bol - status
  */
 void UIBCSinkInterface::setUibcCapability(WFD_uibc_capability_t* pUibcCapability, 
                                                    wfd_uibc_capability_change_cb capChangeCB)
{
  UNUSED(capChangeCB);
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "UIBCSinkInterface::setUibcCapability");
  bool retStatus = false;
  if( m_pUIBCHandle )
  {
    if(UIBC_SINK_SUCCESS == m_pUIBCHandle->setNegotiatedCapability(pUibcCapability))
    {
      retStatus = true;
    }
  }
  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "UIBCSinkInterface::setUibcCapability status = %d",retStatus);
}

/** @brief get local UIBC capabilities
  *
  * @param[in] WFD_uibc_capability_t - UIBC capabilities
  * @return  bol - status
  */
 bool UIBCSinkInterface::getLocalCapability(WFD_uibc_capability_config_t* pUibcLocalCapability)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "UIBCSinkInterface::getLocalCapability");
  bool retStatus = false;
  if( m_pUIBCHandle )
  {
    if(UIBC_SINK_SUCCESS == m_pUIBCHandle->getLocalCapability(pUibcLocalCapability))
    {
      retStatus = true;
    }		
  }
   return retStatus;
}

