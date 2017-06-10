/*==============================================================================
*  @file UIBCSinkInterface.h
*
*  @par  DESCRIPTION:
*        UIBC Sink Interface with Session Manager.
*        This is Sink interface for UIBC with Session Manager
*
*
*  Copyright (c) 2012 by Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================

  $Header:
==============================================================================*/


#ifndef _UIBCSINKINTERFACE_H_
#define _UIBCSINKINTERFACE_H_

#include "UIBCDefs.h"
#include "UIBCInterface.h"

class UIBCSinkManager;

class UIBCSinkInterface : public UIBCInterface
{
public:
  UIBCSinkInterface();
  ~UIBCSinkInterface();

  virtual void getUibcCapability(WFD_uibc_capability_t*);
  virtual void setUibcCapability(WFD_uibc_capability_t*, wfd_uibc_capability_change_cb );
  virtual bool readUIBCCfgCapability(WFD_uibc_capability_t* );
  virtual bool createUIBCSession();
  virtual bool destroyUIBCSession();
  virtual bool sendUIBCEvent(WFD_uibc_event_t*);
  virtual bool getLocalCapability(WFD_uibc_capability_config_t* );
  virtual bool Enable();
  virtual bool Disable();

  private:
  UIBCSinkManager* m_pUIBCHandle;
};

#endif
