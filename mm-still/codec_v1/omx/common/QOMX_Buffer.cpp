/*******************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/
#include "QOMX_Buffer.h"

/*==============================================================================
* Function : QOMX_Buffer
* Parameters: None
* Return Value : None
* Description: Default Constructor
==============================================================================*/
QOMX_Buffer::QOMX_Buffer(){
  m_omxBufferheader = NULL;
}

/*==============================================================================
* Function : QOMX_Buffer
* Parameters: OMX_BUFFERHEADERTYPE *a_omxBufferheader, QImage *a_qImage
* Return Value : None
* Description: Constructor
==============================================================================*/
QOMX_Buffer::QOMX_Buffer(OMX_BUFFERHEADERTYPE *a_omxBufferheader)
{
  m_omxBufferheader = a_omxBufferheader;
}

/*==============================================================================
* Function : getBuffer
* Parameters: None
* Return Value : OMX_BUFFERHEADERTYPE *
* Description: Get the buffer information
==============================================================================*/
OMX_BUFFERHEADERTYPE * QOMX_Buffer::getBuffer(){
  return m_omxBufferheader;
}
/*==============================================================================
* Function : QOMX_Buffer
* Parameters: None
* Return Value : None
* Description: Destructor
==============================================================================*/
QOMX_Buffer::~QOMX_Buffer(){

}
