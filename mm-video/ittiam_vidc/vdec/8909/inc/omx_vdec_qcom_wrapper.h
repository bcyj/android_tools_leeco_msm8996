/******************************************************************************
*
*                                 OMX Ittiam
*
*                     ITTIAM SYSTEMS PVT LTD, BANGALORE
*                             COPYRIGHT(C) 2011
*
*  This program is proprietary to ittiam systems pvt. ltd.,and is protected
*  under indian copyright act as an unpublished work.its use and disclosure
*  is limited by the terms and conditions of a license agreement.it may not
*  be copied or otherwise  reproduced or disclosed  to persons  outside the
*  licensee's   organization  except  in  accordance   with  the  terms and
*  conditions  of such  an agreement. all copies and reproductions shall be
*  the property of ittiam systems pvt. ltd.  and  must bear this  notice in
*  its entirety.
*
******************************************************************************/

/****************************************************************************

   Copyright (c) 2006-2008, 2010-2012 Qualcomm Technologies Incorporated.
   All Rights Reserved. Qualcomm Proprietary and Confidential.

*******************************************************************************/

#ifndef __OMX_VDEC_QCOM_WRAPPER_H__
#define __OMX_VDEC_QCOM_WRAPPER_H__

/*============================================================================
                            O p e n M A X   Component
                                Video Decoder

*//** @file c omx_vdec_qcom_wrapper.h
  This module contains the class definition for openMAX decoder component.

*//*========================================================================*/

//////////////////////////////////////////////////////////////////////////////
//                             Include Files
//////////////////////////////////////////////////////////////////////////////


#include<stdlib.h>

#include <stdio.h>
#include <inttypes.h>
#include <OMX_QCOMExtns.h>

#include <binder/MemoryHeapBase.h>


extern "C"{
#include<utils/Log.h>
}
#include<utils/RefBase.h>

#include "OMX_Core.h"

#include "qc_omx_component.h"

using namespace android;

extern "C" {
  OMX_API void * get_omx_component_factory_fn(void);
}

// OMX video decoder class
class omx_vdec: public qc_omx_component
{

public:
    omx_vdec();  // constructor
    virtual ~omx_vdec();  // destructor

    OMX_ERRORTYPE allocate_buffer(
                                   OMX_HANDLETYPE hComp,
                                   OMX_BUFFERHEADERTYPE **bufferHdr,
                                   OMX_U32 port,
                                   OMX_PTR appData,
                                   OMX_U32 bytes
                                  );


    OMX_ERRORTYPE component_deinit(OMX_HANDLETYPE hComp);

    OMX_ERRORTYPE component_init(OMX_STRING role);

    OMX_ERRORTYPE component_role_enum(
                                       OMX_HANDLETYPE hComp,
                                       OMX_U8 *role,
                                       OMX_U32 index
                                      );

    OMX_ERRORTYPE component_tunnel_request(
                                            OMX_HANDLETYPE hComp,
                                            OMX_U32 port,
                                            OMX_HANDLETYPE  peerComponent,
                                            OMX_U32 peerPort,
                                            OMX_TUNNELSETUPTYPE *tunnelSetup
                                           );

    OMX_ERRORTYPE empty_this_buffer(
                                     OMX_HANDLETYPE hComp,
                                     OMX_BUFFERHEADERTYPE *buffer
                                    );



    OMX_ERRORTYPE fill_this_buffer(
                                    OMX_HANDLETYPE hComp,
                                    OMX_BUFFERHEADERTYPE *buffer
                                   );


    OMX_ERRORTYPE free_buffer(
                              OMX_HANDLETYPE hComp,
                              OMX_U32 port,
                              OMX_BUFFERHEADERTYPE *buffer
                              );

    OMX_ERRORTYPE get_component_version(
                                        OMX_HANDLETYPE hComp,
                                        OMX_STRING componentName,
                                        OMX_VERSIONTYPE *componentVersion,
                                        OMX_VERSIONTYPE *specVersion,
                                        OMX_UUIDTYPE *componentUUID
                                        );

    OMX_ERRORTYPE get_config(
                              OMX_HANDLETYPE hComp,
                              OMX_INDEXTYPE configIndex,
                              OMX_PTR configData
                             );

    OMX_ERRORTYPE get_extension_index(
                                      OMX_HANDLETYPE hComp,
                                      OMX_STRING paramName,
                                      OMX_INDEXTYPE *indexType
                                      );

    OMX_ERRORTYPE get_parameter(OMX_HANDLETYPE hComp,
                                OMX_INDEXTYPE  paramIndex,
                                OMX_PTR        paramData);

    OMX_ERRORTYPE get_state(OMX_HANDLETYPE hComp,
                            OMX_STATETYPE *state);



    OMX_ERRORTYPE send_command(OMX_HANDLETYPE  hComp,
                               OMX_COMMANDTYPE cmd,
                               OMX_U32         param1,
                               OMX_PTR         cmdData);


    OMX_ERRORTYPE set_callbacks(OMX_HANDLETYPE   hComp,
                                OMX_CALLBACKTYPE *callbacks,
                                OMX_PTR          appData);

    OMX_ERRORTYPE set_config(OMX_HANDLETYPE hComp,
                             OMX_INDEXTYPE  configIndex,
                             OMX_PTR        configData);

    OMX_ERRORTYPE set_parameter(OMX_HANDLETYPE hComp,
                                OMX_INDEXTYPE  paramIndex,
                                OMX_PTR        paramData);

    OMX_ERRORTYPE use_buffer(OMX_HANDLETYPE      hComp,
                             OMX_BUFFERHEADERTYPE **bufferHdr,
                             OMX_U32              port,
                             OMX_PTR              appData,
                             OMX_U32              bytes,
                             OMX_U8               *buffer);

    OMX_ERRORTYPE  use_input_heap_buffers(
                          OMX_HANDLETYPE            hComp,
                          OMX_BUFFERHEADERTYPE** bufferHdr,
                          OMX_U32                   port,
                          OMX_PTR                   appData,
                          OMX_U32                   bytes,
                          OMX_U8*                   buffer);

    OMX_ERRORTYPE use_EGL_image(OMX_HANDLETYPE     hComp,
                                OMX_BUFFERHEADERTYPE **bufferHdr,
                                OMX_U32              port,
                                OMX_PTR              appData,
                                void *               eglImage);

private:
    OMX_ERRORTYPE use_android_native_buffer(OMX_IN OMX_HANDLETYPE hComp, OMX_PTR data);
    OMX_COMPONENTTYPE *hComponent;
};

#endif // __OMX_ITTIAM_VDEC_H__
