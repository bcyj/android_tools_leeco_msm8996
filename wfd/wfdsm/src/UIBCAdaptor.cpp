/*==============================================================================
*  @file UIBCAdaptor.cpp
*
*  @par  DESCRIPTION:
*        Interface with Session Manager and UIBC service
*
*
*  Copyright (c) 2012 - 2014 by Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/
#include "UIBCAdaptor.h"

uibcDeviceType convSMtoUIBCDevtype(DeviceType dType)
{
  switch (dType)
  {
  case SOURCE:
    return UIBC_SOURCE;
  case PRIMARY_SINK:
    return UIBC_PRIMARY_SINK;
  case SECONDARY_SINK:
    return UIBC_SECONDARY_SINK;
  case SOURCE_PRIMARY_SINK:
    return UIBC_UNKNOWN;
  default:
    return UIBC_UNKNOWN;
  }
}

DeviceType                UIBCAdaptor::myDeviceType;
UIBCInterface*            UIBCAdaptor::pUibcInterface;
wfd_uibc_attach_cb        UIBCAdaptor::AttachVM;
wfd_uibc_send_event_cb    UIBCAdaptor::SendEvent;
wfd_uibc_hid_event_cb     UIBCAdaptor::sendHIDEvent;
bool                      UIBCAdaptor::bUIBCSessionValid;
bool                      UIBCAdaptor::bUIBCenableSendEvent = true;
wfd_uibc_capability_change_cb UIBCAdaptor::capChangeCB;


UIBCAdaptor::UIBCAdaptor()
{
   bUIBCSessionValid = false;
   bUIBCenableSendEvent = true;
  //Do Nothing for now
}

UIBCAdaptor::~UIBCAdaptor()
{
  //Do Nothing for now
}

void UIBCAdaptor::setDeviceType (DeviceType devType)
{
  myDeviceType = devType;
}

void UIBCAdaptor::registerUIBCCallbacks (wfd_uibc_attach_cb Attach ,
                                         wfd_uibc_send_event_cb Send,
                                         wfd_uibc_hid_event_cb sendHID)
{
  AttachVM  = Attach;
  SendEvent = Send;
  sendHIDEvent = sendHID;
}

void UIBCAdaptor::createUibcInterface()
{
  pUibcInterface = UIBCInterface::createInstance(convSMtoUIBCDevtype(myDeviceType));
}

void UIBCAdaptor::destroyUibcInterface()
{
  if (pUibcInterface != NULL)
  {
    delete pUibcInterface;
    pUibcInterface = NULL;
  }
}

bool UIBCAdaptor::updateLocalUIBCCapability(WFD_uibc_capability_t* pUibcCapability)
{
  if (pUibcInterface)
  {
    pUibcInterface->getUibcCapability(pUibcCapability);
    return true;
  }

  return false;
}

bool UIBCAdaptor::getNegotiatedCapability(WFD_uibc_capability_t* pLocalUIBCCapability,
                                                 WFD_uibc_capability_t* pPeerUIBCCapability,
                                                 WFD_uibc_capability_t* pNegotiatedUIBCCapability)
{
  if (pUibcInterface && pLocalUIBCCapability && pPeerUIBCCapability && pNegotiatedUIBCCapability)
  {
    if (pUibcInterface->getNegotiatedCapability(&(pLocalUIBCCapability->config),
                                                &(pPeerUIBCCapability->config),
                                                &(pNegotiatedUIBCCapability->config))
                                               )
    return true;
  }
  return false;
}

bool UIBCAdaptor::createSession(WFD_uibc_capability_t* pUibcCapability)
{
  if (pUibcInterface)
  {
    if (pUibcInterface->createUIBCSession())
    {
      pUibcInterface->setUibcCapability(pUibcCapability, capChangeCB);
      pUibcInterface->registerUibcCallback(AttachVM,SendEvent,sendHIDEvent);
      bUIBCSessionValid = true;
      return true;
    }
  }
  return false;
}

bool UIBCAdaptor::destroySession()
{
  bUIBCenableSendEvent = true;
  if (pUibcInterface && bUIBCSessionValid)
  {
    bUIBCSessionValid = false;
    if (pUibcInterface->destroyUIBCSession())
    {
      return true;
    }
  }
  return false;
}

/**
 * Function to start UIBC data path
 *
 * @return bool
 */
bool UIBCAdaptor::startUIBC()
{
  //LOGD("UIBCAdaptor::startUIBC");

  bool ret = false;
  if (pUibcInterface)
  {
    ret = pUibcInterface->Enable();
    return ret;
  }
  return ret;
}


/**
 * Function to stop UIBC
 *
 * @return bool
 */
bool UIBCAdaptor::stopUIBC()
{
  bool ret = false;
  if (pUibcInterface)
  {
    ret = pUibcInterface->Disable();
    return ret;
  }
  return ret;
}

bool UIBCAdaptor::sendUIBCEvent (WFD_uibc_event_t* event)
{
  bool ret = false;
  if(!bUIBCenableSendEvent)
  {
    return true;
  }
  if (pUibcInterface)
  {
    ret = pUibcInterface->sendUIBCEvent(event);
  }
  return ret;
}

void UIBCAdaptor::handleUIBCEvent (bool bUIBCEvent)
{
    bUIBCenableSendEvent = bUIBCEvent;
}

