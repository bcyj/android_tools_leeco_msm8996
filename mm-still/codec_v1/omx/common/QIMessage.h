/*******************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

#ifndef __QIMESSAGE_H__
#define __QIMESSAGE_H__

#include "QIBase.h"
#include "qomx_common.h"

/*===========================================================================
 * Class: QIMessage
 *
 * Description: This class represents the OMX message objects
 *
 * Notes: none
 *==========================================================================*/
class QIMessage: public QIBase
{
public:

  /** QIMessage:
   *
   * constructor
   **/
  QIMessage();

  /** ~QIMessage:
   *
   * destructor
   **/
  virtual ~QIMessage();

  /** m_qMessage:
   *
   * message type
   **/
  qomx_message_t m_qMessage;

  /**
   *  @pData: pointer to the data
   *  @iData: integer data
   *
   *  message data
   **/
  union {
    void *pData;
    int iData;
  };
};

#endif


