#ifndef __DRMPROV_CLIENT_H_
#define __DRMPROV_CLIENT_H_
/*===========================================================================
  Copyright (c) 2012-14 QUALCOMM TECHONOLOGIES Incorporated.
  All Rights Reserved.
  Qualcomm Technologies Confidential and Proprietary
===========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE

  $Header:

when        who     what, where, why
--------   ---     ----------------------------------------------------------
04/03/14   tp      Formatted by running "atyle --style=allman" command.
10/31/13   wt      New drmprov API to support OEM wrapped keys
10/07/13   wt      New drmprov API to support IPC wrapped keys
12/03/12   cz      Initial version and move drmprov api to a seperate header file

===========================================================================*/

/*===========================================================================

                     INCLUDE FILES FOR MODULE

===========================================================================*/

#define FIRMWARE_PATH_LENGTH      128

/********************************************************************************
  Provision DRM keys using sfs.

  @return
  zero        - Success.
  non-zero    - Failure.

  @param[in]      feature_name        feature name
  @param[in]      feature_name_len    feature name length
  @param[in]      file_name           file name
  @param[in]      file_name_len       file name length
  @param[in]     *data                payload of the key.
  @param[in]      size                Size of the key.

  @dependencies
  None

  @sideeffects
  None
********************************************************************************/
long drm_save_keys
(
    uint8*  feature_name,
    uint32  feature_name_len,
    uint8*  file_name,
    uint32  file_name_len,
    uint8*  data,
    uint32  size
);

/********************************************************************************
  Verify provisioned DRM keys using sfs.

  @return
  zero        - Success.
  non-zero    - Failure.

  @param[in]      feature_name        feature name
  @param[in]      feature_name_len    feature name length
  @param[in]      file_name           file name
  @param[in]      file_name_len       file name length
  @param[in]     *data                payload of the key.
  @param[in]      size                Size of the key.

  @dependencies
  None

  @sideeffects
  None
********************************************************************************/
long drm_verify_keys
(
    uint8*  feature_name,
    uint32  feature_name_len,
    uint8*  file_name,
    uint32  file_name_len,
    uint8*  data,
    uint32  size
);

/********************************************************************************
  Finalized the provisioning keys. After this function is called, provisioning
  will not work anymore.

  @return
  zero        - Success.
  non-zero    - Failure.

  @dependencies
  None

  @sideeffects
  None
********************************************************************************/
int drm_prov_finalize();

/********************************************************************************
  Provision DRM keys using sfs.

  @return
  zero        - Success.
  non-zero    - Failure.

  @param[in]      feature_name        feature name
  @param[in]      feature_name_len    feature name length
  @param[in]      file_name           file name
  @param[in]      file_name_len       file name length
  @param[in]     *data                payload of the key.
  @param[in]      size                Size of the key.
  @param[in]      app_id              defined in \vendor\qcom\proprietary\securemsm\tzcommon\inc\app_main.h
  @param[in]     *app_dir             directory of tz applicaion path, e.g., /firmware/image/playready (No extension name)
  @param[in]      mode                reserved

  @dependencies
  None

  @sideeffects
  None
********************************************************************************/
long drm_app_save_keys
(
    uint8* feature_name,
    uint32 feature_name_len,
    uint8* file_name,
    uint32 file_name_len,
    uint8* data,
    uint32 size,
    uint32 app_id,
    uint8* app_dir,
    uint32 mode
);

/********************************************************************************
  Verify provisioned DRM keys using sfs.

  @return
  zero        - Success.
  non-zero    - Failure.

  @param[in]      feature_name        feature name
  @param[in]      feature_name_len    feature name length
  @param[in]      file_name           file name
  @param[in]      file_name_len       file name length
  @param[in]     *data                payload of the key.
  @param[in]      size                Size of the key.
  @param[in]      app_id              defined in \vendor\qcom\proprietary\securemsm\tzcommon\inc\app_main.h
  @param[in]     *app_dir             directory of tz applicaion path, e.g., /firmware/image/playready (No extension name)
  @param[in]      mode                reserved

  @dependencies
  None

  @sideeffects
  None
********************************************************************************/
long drm_app_verify_keys
(
    uint8* feature_name,
    uint32 feature_name_len,
    uint8* file_name,
    uint32 file_name_len,
    uint8* data,
    uint32 size,
    uint32 app_id,
    uint8* app_dir,
    uint32 mode
);

/********************************************************************************
  Finalized the provisioning keys. After this function is called, provisioning
  will not work anymore.

  @return
  zero        - Success.
  non-zero    - Failure.

  @param[in]      app_id              defined in \vendor\qcom\proprietary\securemsm\tzcommon\inc\app_main.h
  @param[in]     *app_dir             directory of tz applicaion path, e.g., /firmware/image/playready (No extension name)

  @dependencies
  None

  @sideeffects
  None
********************************************************************************/
int drm_app_prov_finalize(
    uint32  app_id,
    uint8*  app_dir
);

/********************************************************************************
  Provision DRM wrapped keys using sfs.

  @return
  zero        - Success.
  non-zero    - Failure.

  @param[in]      source_app_name     source app name
  @param[in]      source_app_name_len source app name length
  @param[in]      feature_name        feature name
  @param[in]      feature_name_len    feature name length
  @param[in]      file_name           file name
  @param[in]      file_name_len       file name length
  @param[in]     *data                payload of the key.
  @param[in]      size                Size of the key.
  @param[in]     *app_dir             directory of tz application path, e.g., /firmware/image/playread (No extension name)
  @param[in]     *decap_app_dir       directory of tz application path that will decapsulate the key, e.g., /firmware/image/widevine (No extension name)
  @param[in]      mode                reserved

  @dependencies
  The application instance whose keys were used to encapsulate the key MUST be the same
  instance, and still open, when using this function. The decapsulation is dependent on this.
  i.e., the application used to encapsulate the keys cannot be closed before this
  function is called.

  @sideeffects
  None
********************************************************************************/
long drm_app_save_qsee_ipc_wrapped_keys
(
    char* source_app_name,
    uint32 source_app_name_len,
    uint8* feature_name,
    uint32 feature_name_len,
    uint8* file_name,
    uint32 file_name_len,
    uint8* data,
    uint32 size,
    uint8* app_dir,
    uint8* encap_app_dir,
    uint32 mode
);

/********************************************************************************
  Provision OEM Wrapped DRM keys using sfs.

  @return
  zero        - Success.
  non-zero    - Failure.

  @param[in]      feature_name        feature name
  @param[in]      feature_name_len    feature name length
  @param[in]      file_name           file name
  @param[in]      file_name_len       file name length
  @param[in]     *data                payload of the key.
  @param[in]      size                Size of the key.
  @param[in]     *app_dir             directory of tz application path, e.g., /firmware/image/playread (No extension name)
  @param[in]      mode                reserved

  @dependencies
  None

  @sideeffects
  None
********************************************************************************/
long drm_app_save_wrapped_keys
(
    uint8* feature_name,
    uint32 feature_name_len,
    uint8* file_name,
    uint32 file_name_len,
    uint8* data,
    uint32 size,
    uint8* app_dir,
    uint32 mode
);

#endif /* __DRMPROV_CLIENT_H_ */
