/**********************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#ifndef __IMG_COMP_FACTORY_PRIV_H__
#define __IMG_COMP_FACTORY_PRIV_H__

#include "img_comp_factory.h"

/** img_comp_reg_t
 *   @role: role of the component
 *   @name: name of the component
 *   @ops: function table for the operation
 *
 *   Registry for the imaging components
 *
 **/
typedef struct {
  img_comp_role_t role;
  char *name;
  img_core_ops_t ops;
} img_comp_reg_t;


/* Since all the components are compiled as part of the
 * same library, the function symbols needs to be exported
 */
/** wd_comp_create
 *   @handle: library handle
 *   @p_ops: pointer to the image component ops
 *
 *   Create wavelet denoise component
 **/
extern int wd_comp_create(void* handle, img_component_ops_t *p_ops);

/** wd_comp_load
 *   @name: library name
 *   @handle: library handle
 *
 *   Load wavelet denoise component
 **/
extern int wd_comp_load(const char* name, void** handle);

/** wd_comp_load
 *   @handle: library handle
 *
 *   UnLoad wavelet denoise component
 **/
extern int wd_comp_unload(void* handle);

/** hdr_comp_create
 *   @handle: library handle
 *   @p_ops: pointer to the image component ops
 *
 *   Create HDR component
 **/
extern int hdr_comp_create(void* handle, img_component_ops_t *p_ops);

/** hdr_comp_load
 *   @name: library name
 *   @handle: library handle
 *
 *   Load HDR component
 **/
extern int hdr_comp_load(const char* name, void** handle);

/** hdr_comp_unload
 *   @handle: library handle
 *
 *   UnLoad HDR component
 **/
extern int hdr_comp_unload(void* handle);

/** faceproc_comp_create
 *   @p_ops: pointer to the image component ops
 *
 *   Create faceproc component
 **/
extern int faceproc_comp_create(void* handle, img_component_ops_t *p_ops);

/** faceproc_comp_load
 *   @name: library name
 *   @handle: library handle
 *
 *   Load faceproc component
 **/
extern int faceproc_comp_load(const char* name, void** handle);

/** faceproc_comp_unload
 *   @handle: library handle
 *
 *   UnLoad faceproc component
 **/
extern int faceproc_comp_unload(void* handle);

/** cac_comp_create
 *   @handle: library handle
 *   @p_ops: pointer to the image component ops
 *
 *   Create cac component
 **/
extern int cac_comp_create(void* handle, img_component_ops_t *p_ops);

/** cac_comp_load
 *   @name: library name
 *   @handle: library handle
 *
 *   Load CAC component
 **/
extern int cac_comp_load(const char* name, void** handle);

/** cac_comp_unload
 *   @handle: library handle
 *
 *   UnLoad CAC component
 **/
extern int cac_comp_unload(void* handle);

/** frameproc_comp_create
 *   @handle: library handle
 *   @p_ops: pointer to the image component ops
 *
 *   Create faameproc component
 **/
extern int frameproc_comp_create(void* handle, img_component_ops_t *p_ops);

/** frameproc_comp_load
 *   @name: library name
 *   @handle: library handle
 *
 *   Load faameproc component
 **/
extern int frameproc_comp_load(const char* name, void** handle);

/** frameproc_comp_unload
 *   @handle: library handle
 *
 *   UnLoad faameproc component
 **/
extern int frameproc_comp_unload(void* handle);

#endif //__IMG_COMP_FACTORY_PRIV_H__
