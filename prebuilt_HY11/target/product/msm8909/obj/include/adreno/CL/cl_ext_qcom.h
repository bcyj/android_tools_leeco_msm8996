/* Copyright (c) 2009-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __OPENCL_CL_EXT_QCOM_H
#define __OPENCL_CL_EXT_QCOM_H

// Needed by cl_khr_egl_event extension 
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <CL/cl_ext.h>

#ifdef __cplusplus
extern "C" {
#endif


/************************************
 * cl_qcom_create_buffer_from_image *
 ************************************/

#define CL_BUFFER_FROM_IMAGE_ROW_PITCH_QCOM         0x40C0
#define CL_BUFFER_FROM_IMAGE_SLICE_PITCH_QCOM       0x40C1

extern CL_API_ENTRY cl_mem CL_API_CALL
clCreateBufferFromImageQCOM(cl_mem       image,
                            cl_mem_flags flags,
                            cl_int      *errcode_ret);


/************************************
 * cl_qcom_limited_printf extension *
 ************************************/

/* Builtin printf function buffer size in bytes. */
#define CL_DEVICE_PRINTF_BUFFER_SIZE_QCOM           0x1049


/*************************************
 * cl_qcom_extended_images extension *
 *************************************/

#define CL_CONTEXT_ENABLE_EXTENDED_IMAGES_QCOM      0x40AA
#define CL_DEVICE_EXTENDED_IMAGE2D_MAX_WIDTH_QCOM   0x40AB
#define CL_DEVICE_EXTENDED_IMAGE2D_MAX_HEIGHT_QCOM  0x40AC
#define CL_DEVICE_EXTENDED_IMAGE3D_MAX_WIDTH_QCOM   0x40AD
#define CL_DEVICE_EXTENDED_IMAGE3D_MAX_HEIGHT_QCOM  0x40AE
#define CL_DEVICE_EXTENDED_IMAGE3D_MAX_DEPTH_QCOM   0x40AF

/*************************************
 * cl_qcom_perf_hint extension *
 *************************************/

typedef cl_uint                                     cl_perf_hint;

#define CL_CONTEXT_PERF_HINT_QCOM                   0x40C2

/*cl_perf_hint*/
#define CL_PERF_HINT_HIGH_QCOM                      0x40C3
#define CL_PERF_HINT_NORMAL_QCOM                    0x40C4
#define CL_PERF_HINT_LOW_QCOM                       0x40C5

extern CL_API_ENTRY cl_int CL_API_CALL
clSetPerfHintQCOM(cl_context    context,
                  cl_perf_hint  perf_hint);

/*********************************
* cl_qcom_android_native_buffer_host_ptr extension
*********************************/

#define CL_MEM_ANDROID_NATIVE_BUFFER_HOST_PTR_QCOM                  0x40C6


typedef struct _cl_mem_android_native_buffer_host_ptr
{
    // Type of external memory allocation.
    // Must be CL_MEM_ANDROID_NATIVE_BUFFER_HOST_PTR_QCOM for Android native buffers.
    cl_mem_ext_host_ptr  ext_host_ptr;

    // Virtual pointer to the android native buffer
    void*                anb_ptr;

} cl_mem_android_native_buffer_host_ptr;
#ifdef __cplusplus
}
#endif

#endif /* __OPENCL_CL_EXT_QCOM_H */
