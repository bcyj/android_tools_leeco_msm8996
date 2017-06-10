/*==============================================================================
*  @file UIBCSourceInterface.cpp
*
*  @par  DESCRIPTION:
*        UIBC Source Interface with Session Manager.
*        This is source interface for UIBC with Session Manager
*
*
*  Copyright (c) 2012 - 2014 by Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================

  $Header:
==============================================================================*/

#include "UIBCSourceInterface.h"
#include "UIBCInputReceiver.h"
#include "MMMalloc.h"
#include "MMDebugMsg.h"
#include "wfd_cfg_parser.h"
#include <stdio.h>


#define FEATURE_CONFIG_FROM_FILE
#define UNUSED(x) ((void)x)

  /** @brief Get UIBC capabity parameters structure
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle (not used)
               WFD_uibc_capability_config_t* - pointer to UIBC capability structure
  *
  * @return  WFD_status_t - status
  */
UIBCSourceInterface::UIBCSourceInterface ()
{
  m_pUIBCHandle = NULL;
}

UIBCSourceInterface::~UIBCSourceInterface ()
{
  if (m_pUIBCHandle != NULL)
  {
    MM_Delete(m_pUIBCHandle);
  }
  m_pUIBCHandle = NULL;
}

bool UIBCSourceInterface::readUIBCCfgCapability(WFD_uibc_capability_t* pUibcCapability)
{
#ifdef FEATURE_CONFIG_FROM_FILE
      {
         readConfigFile getUIBCCap;
         memset (&getUIBCCap, 0,sizeof(readConfigFile));

        // read the uibc supported key value and then parse the cfg based on that
         getCfgItem(UIBC_VALID_KEY,&m_bSupportedFlag);
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                    "UIBCSourceInterface::readUIBCCfgCapability UIBC support = %d",m_bSupportedFlag);

         if( !m_bSupportedFlag )
         {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"UIBCSourceInterface::"\
                                    "readUIBCCfgCapability UIBC NOT SUPPORTED");
            return false;
         }

         parseCfgforUIBC(WFD_CFG_FILE,&getUIBCCap);

         pUibcCapability->config.category =
                                      getUIBCCap.uibcCapability.config.category;
         pUibcCapability->config.generic_input_type =
                            getUIBCCap.uibcCapability.config.generic_input_type;
         for (int i= 0; i<UIBC_NUM_INPUT_PATHS;i++)
        {
            pUibcCapability->config.hid_input_type_path[i]= getUIBCCap.uibcCapability.config.hid_input_type_path[i];
        }
         pUibcCapability->port_id = getUIBCCap.uibcCapability.port_id;

         MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_HIGH,"UIBCSourceInterface::readUIBCCfgCapability "\
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
         MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"UIBCSourceInterface::"\
                     "readUIBCCfgCapability FEATURE_CONFIG_FROM_FILE not defined");
         return false;
      }
#endif
      return false;
}

void UIBCSourceInterface::getUibcCapability(WFD_uibc_capability_t* pUibcCapability)
{
  bool bUIBCfgCap = false;
  if (pUibcCapability != NULL)
  {
    memset (pUibcCapability, 0,sizeof(WFD_uibc_capability_t));
    bUIBCfgCap = readUIBCCfgCapability(pUibcCapability);
    if(!bUIBCfgCap)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"UIBCSourceInterface : UIBC Not supported");
    }
  }
}

/** @brief Get UIBC capabity parameters structure
*
* @param[in] WFD_MM_HANDLE - WFD MM Source instance handle (not used)
           WFD_uibc_capability_config_t* - pointer to UIBC capability structure
*
* @return  WFD_status_t - status
*/
bool  UIBCSourceInterface::getNegotiatedCapability
                                          (WFD_uibc_capability_config_t *preferred_capability,
                                           WFD_uibc_capability_config_t *remote_capability,
                                           WFD_uibc_capability_config_t *negotiated_capability
                                          )
{
  bool retStatus = false;
  if (preferred_capability && remote_capability && negotiated_capability)
  {
    WFD_uibc_capability_t local_capability;
    getUibcCapability(&local_capability);

    memset(negotiated_capability, 0, sizeof(WFD_uibc_capability_config_t));
    negotiated_capability->category = preferred_capability->category
        & remote_capability->category & local_capability.config.category;
    negotiated_capability->generic_input_type
        = preferred_capability->generic_input_type
            & remote_capability->generic_input_type
            & local_capability.config.generic_input_type;
    for (int i = 0; i < UIBC_NUM_INPUT_PATHS; i++)
    {
      negotiated_capability->hid_input_type_path[i]
          = preferred_capability->hid_input_type_path[i]
              & remote_capability->hid_input_type_path[i]
              & local_capability.config.hid_input_type_path[i];
    }
    retStatus = true;
  }

  return retStatus;
}

/** @brief Set the UIBC capabilities
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
  * WFD_uibc_capability_t*   - Pointer to (Sink)UIBC Capability structure,
  * wfd_uibc_capability_change_cb - Capability change callback(unused as of now)
  *
  * @return  WFD_status_t - status
  */

void UIBCSourceInterface::setUibcCapability(WFD_uibc_capability_t* pUibcCapability,
                                            wfd_uibc_capability_change_cb capChangeCB)

{
  UNUSED(capChangeCB);
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "UIBCSourceInterface::setUibcCapability");
   //Will change whenever it is used.
  //(void)capChangeCB; //Unused as of now
  if(pUibcCapability && m_pUIBCHandle)
  {
    m_pUIBCHandle->SetUIBCCapability(pUibcCapability);
  }
}

void UIBCSourceInterface::registerUibcCallback(wfd_uibc_attach_cb Attach,
                                               wfd_uibc_send_event_cb Send,
                                               wfd_uibc_hid_event_cb sendHID)
{
  if(m_pUIBCHandle)
  {
    m_pUIBCHandle->RegisterCallback(Attach, Send, sendHID,NULL);
  }
}

bool UIBCSourceInterface::createUIBCSession()
{
  if( !m_bSupportedFlag )
  {
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                "UIBCSourceInterface::createSession UIBC not supported = %d",m_bSupportedFlag);
   return false;
  }
  if (!m_pUIBCHandle)
  {
    m_pUIBCHandle = MM_New (UIBCInputReceiver);
  }
  else
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"UIBCSourceInterface session already created");
    return false;
  }
  if (m_pUIBCHandle == NULL)
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                "UIBCSourceInterface::createSession to create UIBC handle");
    return false;
  }
  return true;
}

bool UIBCSourceInterface::destroyUIBCSession()
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
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
  *
  * @return  WFD_status_t - status
  */
bool UIBCSourceInterface::Enable()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "UIBCSourceInterface::Enable");
  bool retStatus = false;
  if( m_pUIBCHandle )
  {
    retStatus = m_pUIBCHandle->Start();
  }
  return retStatus;
}

/** @brief Disable the UIBC session
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
  * @return  WFD_status_t - status
  */
bool UIBCSourceInterface::Disable()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "UIBCSourceInterface::Disable");
  bool retStatus = false;
  if( m_pUIBCHandle )
  {
    retStatus = m_pUIBCHandle->Stop();
  }
  return retStatus;
}
