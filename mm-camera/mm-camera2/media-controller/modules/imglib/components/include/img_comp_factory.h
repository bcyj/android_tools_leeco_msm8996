/**********************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#ifndef __IMG_COMP_FACTORY_H__
#define __IMG_COMP_FACTORY_H__

#include "img_comp.h"

/** img_comp_role_t
 *   IMG_COMP_DENOISE: Component role is denoising
 *   IMG_COMP_HDR: component role is HDR
 *   IMG_COMP_FACE_PROC: component role is faceproc
 *   IMG_COMP_CAC - component role is CAC
 *   IMG_COMP_GEN_FRAME_PROC - component role is generic frame
 *                          processing
 *
 *   Role of the component
 **/
typedef enum {
  IMG_COMP_DENOISE,
  IMG_COMP_HDR,
  IMG_COMP_FACE_PROC,
  IMG_COMP_CAC,
  IMG_COMP_GEN_FRAME_PROC,
  IMG_COMP_ROLE_MAX,
} img_comp_role_t;

/** img_core_ops_t
 *   @create: function pointer for the component create function
 *   @load: function pointer for the component load
 *   @unload: function pointer for the component unload
 *   @handle: client handle
 *
 *   This structure represent the core ops for the component
 **/
typedef struct {
  //
  int (*create)(void*, img_component_ops_t *);
  //
  int (*load)(const char*, void**);
  //
  int (*unload)(void*);
  //
  void *handle;
} img_core_ops_t;

/** img_core_get_comp
 *   @role: role of the component
 *   @name: name of the component
 *   @p_ops: pointer to the core ops structure
 *
 *   This function is used to get the component given the name
 *   and the role
 **/
int img_core_get_comp(img_comp_role_t role, char *name,
  img_core_ops_t *p_ops);


/** IMG_COMP_CREATE
 *   @p_ops: pointer to the core ops structure
 *   @p_comp: pointer to the component structure
 *
 *   This function is used to create the component
 **/
#define IMG_COMP_CREATE(p_ops, p_comp) \
  (p_ops)->create((p_ops)->handle, p_comp)

/** IMG_COMP_LOAD
 *   @p_ops: pointer to the core ops structure
 *   @name: Library name (Optional), if not used pass NULL
 *
 *   This function is used to load the component
 **/
#define IMG_COMP_LOAD(p_ops, name) ({\
  int rc = IMG_ERR_GENERAL; \
  if (p_ops) { \
    (p_ops)->handle = NULL; \
    rc = (p_ops)->load(name, &((p_ops)->handle)); \
  } \
  rc; \
})

/** IMG_COMP_UNLOAD
 *   @p_ops: pointer to the core ops structure
 *
 *   This function is used to unload the component
 **/
#define IMG_COMP_UNLOAD(p_ops) \
  (p_ops)->unload((p_ops)->handle)

/** IMG_COMP_INIT
 *   @p_comp: pointer to the component
 *   @p_userdata: pointer to the userdata
 *   @p_data: pointer to the data
 *
 *   This function is used to initialize the component
 **/
#define IMG_COMP_INIT(p_comp, p_userdata, p_data) \
  (p_comp)->init((p_comp)->handle, (p_userdata), (p_data))

/** IMG_COMP_DEINIT
 *   @p_comp: pointer to the component
 *
 *   This function is used to uninitialize the component
 **/
#define IMG_COMP_DEINIT(p_comp) \
  (p_comp)->deinit((p_comp)->handle);

/** IMG_COMP_SET_PARAM
 *   @p_comp: pointer to the component
 *   @param: pointer to the image parameter type
 *   @p_data: pointer to the parameter data
 *
 *   This function is used to set the component parameters
 **/
#define IMG_COMP_SET_PARAM(p_comp, param, p_data) \
  (p_comp)->set_parm((p_comp)->handle, (param), (p_data));

/** IMG_COMP_GET_PARAM
 *   @p_comp: pointer to the component
 *   @param: pointer to the image parameter type
 *   @p_data: pointer to the parameter data
 *
 *   This function is used to get the component parameters
 **/
#define IMG_COMP_GET_PARAM(p_comp, param, p_data) \
  (p_comp)->get_parm((p_comp)->handle, (param), (p_data));

/** IMG_COMP_SET_CB
 *   @p_comp: pointer to the component
 *   @notify: pointer to notify callback function
 *
 *   This function is used to set the callback
 **/
#define IMG_COMP_SET_CB(p_comp, notify) \
  (p_comp)->set_callback((p_comp)->handle, (notify));

/** IMG_COMP_START
 *   @p_comp: pointer to the component
 *   @p_data: pointer to data
 *
 *   This function is used to start the component
 **/
#define IMG_COMP_START(p_comp, p_data) \
  (p_comp)->start((p_comp)->handle, (p_data));

/** IMG_COMP_ABORT
 *   @p_comp: pointer to the component
 *   @p_data: pointer to data
 *
 *   This function is used to abort the component
 **/
#define IMG_COMP_ABORT(p_comp, p_data) \
  (p_comp)->abort((p_comp)->handle, (p_data));

/** IMG_COMP_PROCESS
 *   @p_comp: pointer to the component
 *   @cmd: command type
 *   @p_data: pointer to data
 *
 *   This function is used to send command to the component
 **/
#define IMG_COMP_PROCESS(p_comp, cmd, p_data) \
  (p_comp)->process((p_comp)->handle, (cmd), (p_data));

/** IMG_COMP_Q_BUF
 *   @p_comp: pointer to the component
 *   @p_frame: frame to be queued to the component
 *   @type: image type (main image or thumbnail image)
 *
 *   This function is used to queue the buffer to the component
 **/
#define IMG_COMP_Q_BUF(p_comp, p_frame, type) \
  (p_comp)->queue_buffer((p_comp)->handle, (p_frame), (type));

/** IMG_COMP_DQ_BUF
 *   @p_comp: pointer to the component
 *   @pp_frame: double pointer for the frame to be dequeued from
 *            the component
 *
 *   This function is used to dequeue the buffer from the
 *   component
 **/
#define IMG_COMP_DQ_BUF(p_comp, pp_frame) \
  (p_comp)->deque_buffer((p_comp)->handle, (pp_frame));

/** IMG_COMP_GET_STATE
 *   @p_comp: pointer to the component
 *
 *   This function is used to get the state of the component
 **/
#define IMG_COMP_GET_STATE(p_comp) \
  (p_comp)->get_state((p_comp)->handle);

/** IMG_COMP_Q_META_BUF
 *   @p_comp: pointer to the component
 *   @p_metaframe: meta frame to be queued to the component
 *   @type: image type (main image or thumbnail image)
 *
 *   This function is used to queue the buffer to the component
 **/
#define IMG_COMP_Q_META_BUF(p_comp, p_metaframe) \
  (p_comp)->queue_metabuffer((p_comp)->handle, (p_metaframe));

#endif //__IMG_COMP_FACTORY_H__
