/*******************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/
#ifndef __OMX_BSENC_H__
#define __OMX_BSENC_H__
#include "OMX_Encoder.h"

class omx_bsencoder : public QOMXImageCodec{


protected:

  QOMX_BUFFER_INFO *m_thumbBufferInfo;
  qomx_intermediate_port_state_t m_thumbPortTransState;
  OMX_PARAM_PORTDEFINITIONTYPE * m_thumbnail_port;

  virtual OMX_ERRORTYPE omx_component_use_buffer(OMX_IN OMX_HANDLETYPE hComp,
    OMX_INOUT OMX_BUFFERHEADERTYPE * *bufferHdr, OMX_IN OMX_U32 port,
    OMX_IN OMX_PTR appData, OMX_IN OMX_U32 bytes, OMX_IN OMX_U8 *buffer);

};

#endif