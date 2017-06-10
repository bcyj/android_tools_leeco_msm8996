/*******************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/
#ifndef __QOMX_BUFFER_H__
#define __QOMX_BUFFER_H__

#include "QIBase.h"
#include "OMX_Component.h"
#include "qomx_common.h"

/*===========================================================================
 * Class: QOMX_Buffer
 *
 * Description: This class represents OMX buffer class
 *
 * Notes: none
 *==========================================================================*/
class QOMX_Buffer : public QIBase {

public:

  /** QOMX_Buffer
   *
   * constructor
   **/
  QOMX_Buffer();

  /** QOMX_Buffer
   *  @a_omxBufferheader: OMX buffer header
   *
   *  overloaded constructor
   **/
  QOMX_Buffer(OMX_BUFFERHEADERTYPE *a_omxBufferheader);

  /** ~QOMX_Buffer
   *
   *  destructor
   **/
  ~QOMX_Buffer();

  /** getBuffer
   *
   * get the buffer header from class
   **/
  OMX_BUFFERHEADERTYPE *getBuffer();

private:

  /** m_omxBufferheader:
   *
   *  buffer header structure
   **/
  OMX_BUFFERHEADERTYPE *m_omxBufferheader;
};


#endif
