/*============================================================================

   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

/*============================================================================
 *                      INCLUDE FILES
 *===========================================================================*/
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <dlfcn.h>

#include "camera_dbg.h"
#include "sensor.h"
#include "chromatix.h"

/* This section includes the two sensors
 * which are supported in LINUX */
#if VFE_40
  #include "ov2720_u.h"
  #include "s5k3l1yx_u.h"
  #include "mt9m114_u.h"
#endif
#if VFE_32
  #include "ov2720_u.h"
  #include "imx074_u.h"
  #include "mt9m114_u.h"
  #include "s5k3l1yx_u.h"
  #include "imx091_u.h"
  #include "imx135_u.h"
#endif

#if VFE_31
  #include "imx074_u.h"
  #include "vx6953_u.h"
  #include "mt9e013_u.h"
  #include "ov7692_u.h"
#endif
#if VFE_2X
  #include "s5k4e1_u.h"
  #include "mt9e013_u.h"
  #include "ov9726_u.h"
  #include "ov7692_u.h"
  #include "ov5647_u.h"
  #include "ov8825_u.h"
#endif
#ifdef SENSOR_DEBUG
#undef CDBG
#define CDBG LOGE
#endif
/*============================================================================
                        INTERNAL FEATURES
============================================================================*/

/*============================================================================
                        CONSTANTS
============================================================================*/
#define BUFF_SIZE_255 255

/*============================================================================
                        EXTERNAL DECLARATION
============================================================================*/
extern char *ov2720_load_chromatix[];
extern char *imx074_load_chromatix[];
extern char *mt9m114_load_chromatix[];
extern char *s5k3l1yx_load_chromatix[];
extern char *vx6953_load_chromatix[];
extern char *s5k4e1_load_chromatix[];
extern char *mt9e013_load_chromatix[];
extern char *ov9726_load_chromatix[];
extern char *ov7692_load_chromatix[];
extern char *ov5647_load_chromatix[];
extern char *ov8825_load_chromatix[];
extern char *imx135_load_chromatix[];

static sensor_proc_start_t sensor_start;
/*============================================================================
                        INTERNAL VARIABLES DEFINITIONS
============================================================================*/

#define SENSORS_PROCCESS_START(n) { .sname = ""#n, \
  .s_start = n##_process_start, \
  .sensor_load_chromatixfile = n##_load_chromatix,\
}

static sensor_proc_start_t sensors[] = {
#if VFE_40
  SENSORS_PROCCESS_START(ov2720),
  SENSORS_PROCCESS_START(s5k3l1yx),
  SENSORS_PROCCESS_START(mt9m114),
#endif
#if VFE_32
  SENSORS_PROCCESS_START(ov2720),
  SENSORS_PROCCESS_START(imx074),
  SENSORS_PROCCESS_START(mt9m114),
  SENSORS_PROCCESS_START(s5k3l1yx),
  SENSORS_PROCCESS_START(imx135),
#endif
#if VFE_31
  SENSORS_PROCCESS_START(imx074),
  SENSORS_PROCCESS_START(vx6953),
  SENSORS_PROCCESS_START(mt9e013),
  SENSORS_PROCCESS_START(ov7692),
#endif
#if VFE_2X
  SENSORS_PROCCESS_START(s5k4e1),
  SENSORS_PROCCESS_START(mt9e013),
  SENSORS_PROCCESS_START(ov9726),
  SENSORS_PROCCESS_START(ov7692),
  SENSORS_PROCCESS_START(ov5647),
  SENSORS_PROCCESS_START(ov8825),
#endif
};

#undef SENSORS_PROCCESS_START

static void sensor_common_parm_init(sensor_ctrl_t * sctrl)
{

  /* ------------------------------------------- */
  /* Configure the extra Camera Layer Parameters */
  /* Point to output of Chromatix - for sensor specific params */
  sctrl->sensor.out_data.chromatix_ptr = NULL;
  sctrl->chromatixType = SENSOR_LOAD_CHROMATIX_PREVIEW;
  /* ------------------  Sensor-specific Config -------------- */

  /* BAYER or YCbCr */
  sctrl->sensor.out_data.camif_setting.format = SENSOR_BAYER;

  sctrl->sensor.out_data.sensor_output.connection_mode = SENSOR_MIPI_CSI;
  /* A pointer to the sensor name for EXIF tags */
  /* sctrl->sensor.sensor_name = NULL; */

  /* Raw output bit width */
  sctrl->sensor.out_data.sensor_output.raw_output = SENSOR_8_BIT_DIRECT;

  sctrl->sensor.out_data.aec_info.stored_digital_gain = 1.0;
  sctrl->sensor.cur_res = MSM_SENSOR_INVALID_RES;
}

/*===========================================================================
FUNCTION      sensor_init_ctrl

DESCRIPTION
===========================================================================*/
int8_t sensor_init(sensor_ctrl_t *sctrl)
{
  int8_t rc = 1;
  uint8_t cnt;
  void *sensor_lib_handle;
  char libName[BUFF_SIZE_255] = {0};
  char process_start[BUFF_SIZE_255] = {0};
  int8_t (*sensor_process_start)(void *sctrl);

  CDBG("%s: E", __func__);
  sensor_common_parm_init(sctrl);

  rc = ioctl(sctrl->sfd, MSM_CAM_IOCTL_GET_SENSOR_INFO, &sctrl->sinfo);
  if (rc < 0) {
    CDBG_ERROR("%s: MSM_CAM_IOCTL_GET_SENSOR_INFO(%d) failed %d!\n", __func__,
      sctrl->sfd, rc);
    return rc;
  }

  CDBG("kernel returned %s\n", sctrl->sinfo.name);
  for (cnt = 0; cnt < (sizeof(sensors) / sizeof(sensors[0])); cnt++) {
    CDBG("kernel retuned %s, compared to %s\n", sctrl->sinfo.name, sensors[cnt].sname);
    if (!strcmp(sctrl->sinfo.name, sensors[cnt].sname)) {

      strlcpy(sctrl->sensor.sensor_model_no, sctrl->sinfo.name,
        SENSOR_MODEL_NO);

      /* Load chromatix file from chromatixType variable */
      sctrl->start = &sensors[cnt];
      //sCtrl->sensor.cam_mode = cctrl->current_mode;
      rc = sctrl->start->s_start(sctrl);
      if (rc < 0) {
          CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
          return rc;
      }

      sctrl->sensor.out_data.pxlcode = sctrl->sinfo.pxlcode;

      if (sctrl->sensor.out_data.sensor_output.output_format == SENSOR_BAYER) {
        rc = sensor_load_chromatix(sctrl);
        if( rc != 0) {
          CDBG_ERROR("%s:%d:Error:Failed to load chromatix file\n",
          __func__, __LINE__);
          return rc;
        }
      }

      break;
    }
  }
  if (cnt >= (sizeof(sensors) / sizeof(sensors[0]))) {
    sprintf(libName, "libmmcamera_%s.so", sctrl->sinfo.name);
    CDBG("%s libname %s\n", __func__, libName);
    if (libName == NULL) {
      CDBG_ERROR("%s:Failed to find out chromatix dynamic load shared object\n",
        __func__);
      goto ERROR;
    }
    CDBG("%s:%d:chromatix library Name = %s\n", __func__, __LINE__, libName);
    sensor_lib_handle = dlopen(libName, RTLD_NOW);
    if (!sensor_lib_handle) {
      CDBG("%s:%d:sensor_lib_handle NULL\n", __func__, __LINE__);
    } else {
      sprintf(process_start, "%s_process_start", sctrl->sinfo.name);
      *(void **)&sensor_process_start = dlsym(sensor_lib_handle, process_start);
      if (sensor_process_start) {
        CDBG("%s:%d calling %s_process_start()\n", __func__, __LINE__,
          sctrl->sinfo.name);
        sensor_process_start(sctrl);
        sctrl->sensor.out_data.pxlcode = sctrl->sinfo.pxlcode;
      }
      if (sctrl->sensor.out_data.sensor_output.output_format == SENSOR_BAYER) {
        sctrl->start = &sensor_start;
        sctrl->start->sensor_load_chromatixfile =
          sctrl->sensor.sensor_load_chromatixfile;
        if (sctrl->start->sensor_load_chromatixfile) {
          rc = sensor_load_chromatix(sctrl);
          if( rc != 0) {
            CDBG_ERROR("%s:%d:Error:Failed to load chromatix file\n",
            __func__, __LINE__);
            return rc;
          }
        }
      }
    }
  }
ERROR:
  return 0;
}

/*===========================================================================
FUNCTION      sensor_load_chromatix

DESCRIPTION
===========================================================================*/
int8_t sensor_load_chromatix(sensor_ctrl_t *sctrl)
{
  void *libchromatix_handle;
  void (*LINK_load_chromatix)(void *sctrl);
  char libName[BUFF_SIZE_255] = {0};

  CDBG("%s: chromatixType=%d\n", __func__, sctrl->chromatixType);

  if ((sctrl->chromatixType >= SENSOR_LOAD_CHROMATIX_MAX) ||
      (!sctrl->start->sensor_load_chromatixfile[sctrl->chromatixType])) {
    CDBG_ERROR("%s failed: %d\n", __func__, __LINE__);
    goto ERROR;
  }

  CDBG_HIGH("%s: %s: %d\n", __func__,
    sctrl->start->sensor_load_chromatixfile[sctrl->chromatixType],
    strlen(sctrl->start->sensor_load_chromatixfile[sctrl->chromatixType]));

  strlcpy(libName,
    sctrl->start->sensor_load_chromatixfile[sctrl->chromatixType],
    BUFF_SIZE_255);

  if (libName == NULL) {
    CDBG_ERROR("%s:Failed to find out chromatix dynamic load shared object\n",
      __func__);
    goto ERROR;
  }
  CDBG("%s:%d:chromatix library Name = %s\n", __func__, __LINE__, libName);
  libchromatix_handle = dlopen(libName, RTLD_NOW);
  if (!libchromatix_handle) {
    CDBG_ERROR("Failed to dlopen %s: %s", libName, dlerror());
    goto ERROR;
  }
  *(void **)&LINK_load_chromatix = dlsym(libchromatix_handle,
    "load_chromatix");

  if (LINK_load_chromatix != NULL)
    LINK_load_chromatix(sctrl);
  else {
    CDBG_ERROR("Failed to find symbol: %s :%s\n",	libName, dlerror());
    goto ERROR;
  }
  sctrl->sensor.out_data.chromatix_ptr = &(sctrl->chromatixData);
  CDBG("%s:%d: chromatix_version=%d\n", __func__, __LINE__,
    (int)sctrl->sensor.out_data.chromatix_ptr->chromatix_version);

  if (libchromatix_handle) {
    unsigned ref = dlclose(libchromatix_handle);
    CDBG("%s:%d: dlclose(libchromatix_handle) refcount %d\n",
      __func__, __LINE__,ref);
  }
  return 0;
ERROR:
  return -EINVAL;
}

/*===========================================================================
 * FUNCTION    - sensor_re_load_chromatix -
 *
 * DESCRIPTION: re-load chromatix and re_set 3A chroma parameters
 *==========================================================================*/
int8_t sensor_re_load_chromatix(sensor_ctrl_t *sctrl,
  sensor_load_chromatix_t chromatix_type)
{
  int rc = 0;
  sensor_load_chromatix_t old_chromatixType;

  if(sctrl->chromatixType != chromatix_type) {
    old_chromatixType = sctrl->chromatixType;
    sctrl->chromatixType = chromatix_type;
    rc = sensor_load_chromatix(sctrl);
    if (rc < 0) {
      sctrl->chromatixType = old_chromatixType;
    }
  }
  return rc;
} /* sensor_re_load_chromatix */
