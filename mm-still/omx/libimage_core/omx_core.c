/*******************************************************************************
*   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
*   Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/
#include <dlfcn.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "omx_core.h"

#define BUFF_SIZE 255
component_libname_mapping libNameMapping[] =
{
  { "OMX.qcom.image.jpeg.encoder", "libimage-jpeg-enc-omx-comp.so"},
  { "OMX.qcom.image.jpeg.decoder", "libimage-jpeg-dec-omx-comp.so"}
};

static OMX_COMPONENTTYPE *local_fns;
static void *local_libHandle;
/*==============================================================================
 * FUNCTION    - getLibName -
 *
 * DESCRIPTION:
 *============================================================================*/
uint8_t getLibName(char *componentName, char *libName)
{
  uint8_t cnt;

  for(cnt=0; cnt < (sizeof(libNameMapping)/sizeof(libNameMapping[0]));
                cnt++) {
   OMX_DBG_ERROR("%s: cnt =%d componentName=%s, libName=%s\n", __func__,
                cnt, libNameMapping[cnt].componentName,
                libNameMapping[cnt].libName);

    if(!strcmp(libNameMapping[cnt].componentName, componentName)) {
      strlcpy(libName, libNameMapping[cnt].libName,
        BUFF_SIZE);
      OMX_DBG_ERROR("%s: libName=%s\n", __func__, libName);
      break;
    }
  }
  return 1;
}

/*==============================================================================
 * FUNCTION    - OMX_Init -
 *
 * DESCRIPTION:
 *============================================================================*/
OMX_API OMX_ERRORTYPE OMX_APIENTRY
OMX_Init()
{
  OMX_ERRORTYPE rc = OMX_ErrorNone;
  OMX_DBG_ERROR("%s:\n", __func__);
  local_fns->AllocateBuffer(local_fns, NULL, 0, 0, 0);
  return rc;
}

OMX_API OMX_ERRORTYPE OMX_APIENTRY
OMX_Deinit()
{
  OMX_ERRORTYPE rc = OMX_ErrorNone;
  OMX_DBG_INFO("%s:E\n", __func__);
  local_fns->ComponentDeInit(local_fns);
  OMX_DBG_INFO("%s:X\n", __func__);
  return rc;
}

/*==============================================================================
 * FUNCTION    - OMX_GetHandle -
 *
 * DESCRIPTION:
 *============================================================================*/
OMX_API OMX_ERRORTYPE OMX_APIENTRY
OMX_GetHandle(OMX_OUT OMX_HANDLETYPE *handle,
             OMX_IN OMX_STRING componentName,
             OMX_IN OMX_PTR appData, OMX_IN OMX_CALLBACKTYPE *callBacks)
{
  OMX_ERRORTYPE rc = OMX_ErrorNone;
  OMX_COMPONENTTYPE *component_fns = NULL;
  char libName[BUFF_SIZE] = {0};
  void *libHandle;
  void (*LINK_get_component_fns)(OMX_COMPONENTTYPE *component_fns);

  OMX_DBG_ERROR("%s: E\n", __func__);

  component_fns = (OMX_COMPONENTTYPE *)malloc(sizeof(OMX_COMPONENTTYPE));
  if(!component_fns)
        return  OMX_ErrorInsufficientResources;

  getLibName(componentName, libName);
  if (libName == NULL) {
    OMX_DBG_ERROR("%s:L#%d: Failed to get libName\n", __func__, __LINE__);
    return OMX_ErrorInvalidComponent;
  }
  libHandle = dlopen(libName, RTLD_NOW);
  if (!libHandle) {
    OMX_DBG_ERROR("%s:L#%d: Failed to dlopen %s: %s\n",
      __func__, __LINE__, libName, dlerror());
    free(component_fns);
    return OMX_ErrorInvalidComponent;
  }
  OMX_DBG_ERROR("%s: dlopen for %s is successful.\n", __func__, libName);

  char *symbolName = "get_component_fns";
  *(void **)&LINK_get_component_fns = dlsym(libHandle, symbolName);

  if (LINK_get_component_fns != NULL) {
    LINK_get_component_fns((OMX_COMPONENTTYPE *)component_fns);
    *handle = (OMX_HANDLETYPE *)component_fns;
    OMX_DBG_ERROR("here address is %p ", component_fns->GetParameter);
  } else {
    OMX_DBG_ERROR("%s:L#%d: Failed to find symbol %s dlerror=%s\n",
      __func__, __LINE__, symbolName, dlerror());
    free(component_fns);
    return OMX_ErrorInvalidComponent;
  }
  local_fns = component_fns;
  local_libHandle = libHandle;
  component_fns->SetCallbacks(component_fns, callBacks, appData);
  OMX_DBG_ERROR("%s: dlopen for %s is successful.\n", __func__, libName);
  return rc;
}

/*==============================================================================
 * FUNCTION    - OMX_FreeHandle -
 *
 * DESCRIPTION:
 *============================================================================*/
OMX_API OMX_ERRORTYPE OMX_APIENTRY
OMX_FreeHandle(OMX_IN OMX_HANDLETYPE hComp)
{
  OMX_ERRORTYPE rc = OMX_ErrorNone;

  OMX_DBG_ERROR("%s:comp %p\n", __func__,hComp);
  if (hComp) {
    unsigned ref = dlclose(local_libHandle);
    OMX_DBG_ERROR("%s:L#%d: dlclose refcount %d\n",
      __func__, __LINE__,ref);
    free(local_fns);
  } else {
    OMX_DBG_ERROR("%s:L#%d: Handler is NULL\n", __func__, __LINE__);
    rc = OMX_ErrorComponentNotFound; /* TBD: check return value */
  }
  return rc;
}
