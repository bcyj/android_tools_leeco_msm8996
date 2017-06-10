#ifndef _IMS_CAMERA_INTERFACE_H_
#define _IMS_CAMERA_INTERFACE_H_
/*****************************************************************************
 Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.

 File: ImsCameraInterface.h
 Description: Interface to communicate to IMS camera Interface Library

 Revision History
 ===============================================================================
 Date    |   Author's Name    |  BugID  |        Change Description
 ===============================================================================
 03-Jun-2013   Rakesh K M                 -    First version
 10-Jun-2013   Sandeep P R				  -    Updated version
  *****************************************************************************/
#include <jni.h>
#include <utils/String16.h>
using namespace android;


/*--------------------------------------------------------------------------
	Camera Id's
--------------------------------------------------------------------------*/
#define IMS_CAMERA_FACING_BACK 0		/* Back facing camera id */
#define IMS_CAMERA_FACING_FRONT 1		/* Front facing camera id */



/*--------------------------------------------------------------------------
    TYPEDEF STRUCT Resolution

    Structure holds the width and height corresponding to the resoltuion.
--------------------------------------------------------------------------*/
typedef struct {
	/** width **/
	int width;
	/** height **/
	int height;
}Resolution;

/*--------------------------------------------------------------------------
    TYPEDEF ENUM eParamType

    Enumeration defines the parameter keys to be configured at Camera.
--------------------------------------------------------------------------*/
typedef enum {
	/** INVALID_PARAM : Invalid Param **/
	INVALID_PARAM = 0,
	/** SET_FPS : Param to configure the Fps negotiated **/
	SET_FPS,
	/** SET_RESOLUTION : Param to configure the resolution **/
	SET_RESOLUTION
}eParamType;
/*--------------------------------------------------------------------------
    TYPEDEF UNION CameraParams

    Union defines the parameters unions to be configured at Camera.
--------------------------------------------------------------------------*/
typedef union {
	/** fps negotiated **/
	int fps;
	/** Resolution for Preview and Recording Buffer **/
	Resolution cameraResolution;
}CameraParams;
/*--------------------------------------------------------------------------
    TYPEDEF STRUCT CameraParamContainer

    Structure defines the camera parameter structure to be configured at Camera.
--------------------------------------------------------------------------*/
typedef struct {
	/** Parameter type **/
	eParamType type;
	/** Union of Params to be configured **/
	CameraParams params;
}CameraParamContainer;
/*--------------------------------------------------------------------------
    TYPEDEF VOID (*ims_notify_callback)

    Defines the function pointer to be registered by Application layer
	for events from Camera.
--------------------------------------------------------------------------*/
typedef void (*ims_notify_callback)(int msgType,
                            int ext1,
                            int ext2,
                            void* user);

/*--------------------------------------------------------------------------
				IMS Camera API interfaces for App layer
--------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

//interfaces exposed for camera handling by ims
/*===========================================================================

FUNCTION cameraOpen


DESCRIPTION

  This function opens the camera instance specified by the
  input camera id.

DEPENDENCIES
  None

ARGUMENTS IN
     cameraid         - camera id


RETURN VALUE
  0  - operation succesfully
 (-1) - Duplicate camera opens
 (-2) - unknown error

SIDE EFFECTS
  None.

===========================================================================*/

short cameraOpen(unsigned int cameraid);


/*===========================================================================

FUNCTION cameraOpen2


DESCRIPTION

  This function opens the camera instance specified by the
  input camera id.

DEPENDENCIES
  None

ARGUMENTS IN
     cameraid         - camera id
	 package name     - package name


RETURN VALUE
  0  - operation succesfully
 (-1) - Duplicate camera opens
 (-2) - unknown error

SIDE EFFECTS
  None.

===========================================================================*/

short cameraOpen2(unsigned int cameraid,String16* apppackagename);



/*===========================================================================

FUNCTION cameraRelease


DESCRIPTION

  This function releases the camera instance already opened.

DEPENDENCIES
  None

ARGUMENTS IN
  None.


RETURN VALUE
  0  - operation succesfully
 (-1) - Camera not open
 (-2) - unknown error

SIDE EFFECTS
  None.

===========================================================================*/
short cameraRelease();
/*===========================================================================

FUNCTION setNearEndSurface


DESCRIPTION

  This function sets the near surface for displaying the camera Preview. This
  function should be called after cameraOpen() and before we perform startPreview()

DEPENDENCIES
  None

ARGUMENTS IN
  env - JNI environement pointer
  jSurfacenear - surface object


RETURN VALUE
  0  - operation succesfully
 (-1) - camera not opened
 (-2) - unknown error

SIDE EFFECTS
  None.

===========================================================================*/
short setNearEndSurface(JNIEnv * env, jobject jSurfacenear);
/*===========================================================================

FUNCTION setPreviewDisplayOrientation


DESCRIPTION

  This function configures the display orientation of Camera preview display.
  This function should be called before startCameraPreview() in invoked.

DEPENDENCIES
  None

ARGUMENTS IN
  rotation - Set the clockwise rotation of preview display in degrees

RETURN VALUE
  0  - operation succesfully
 (-1) - camera not opened
 (-2) - preview has started
 (-3) - unknown error

SIDE EFFECTS
  None.

===========================================================================*/
short setPreviewDisplayOrientation(unsigned int rotation);
/*===========================================================================

FUNCTION startCameraPreview


DESCRIPTION

  This function starts the camera Preview.

DEPENDENCIES
  None

ARGUMENTS IN
  None

RETURN VALUE
  0  - operation succesfully
 (-1) - camera not opened
 (-2) - Duplicate preview start
 (-3) - no Surface set for preview
 (-4) - unknown error

SIDE EFFECTS
  None.

===========================================================================*/
short startCameraPreview();


/*===========================================================================

FUNCTION stopCameraPreview


DESCRIPTION

  This function stops the Camera Preview.

DEPENDENCIES
  None

ARGUMENTS IN
  None

RETURN VALUE
  0  - operation succesfully
 (-1) - camera not opened
 (-2) - preview not started
 (-3) - unknown error

SIDE EFFECTS
  None.

===========================================================================*/
short stopCameraPreview();

/*===========================================================================

FUNCTION startCameraRecording


DESCRIPTION

  This function starts the camera's recording mode of operation. It commits
  all the parameters  configured by the Application to the camera like
  video size, frame rate and also calls a custom callback to VT library
  for any VT specific custom parameter to be configured. Once the parameters
  are configured; its registers  the required set of events/callbacks and
  starts the Recording.

DEPENDENCIES
  None

ARGUMENTS IN
  None

RETURN VALUE
  0  - operation succesfully
 (-1) - camera not opened
 (-2) - unknown error

SIDE EFFECTS
  None.

===========================================================================*/
short startCameraRecording();
/*===========================================================================

FUNCTION stopCameraRecording


DESCRIPTION

  This function stops the Camera Recording.

DEPENDENCIES
  None

ARGUMENTS IN
  None

RETURN VALUE
  0  - operation succesfully
 (-1) - camera not opened
 (-2) - recording not started
 (-3) - unknown error

SIDE EFFECTS
  None.

===========================================================================*/
short stopCameraRecording();

/*===========================================================================

FUNCTION setCameraParameter


DESCRIPTION

  This function sets the Camera parameters. This API should be called when the
  camera is in initialised state i.e cameraOpen is called. For few configurations
  like resolution change, the camera has to be in preview stopped and recorder stopped
  state before configuring the parameters.

  The arguments are in a (key,paramater) pair where the key specifies what to be
  configured and parameter includes its associated arguments.

DEPENDENCIES
  None

ARGUMENTS IN
  setParams - structure holding the (key,paramater) pair

RETURN VALUE
  0  - operation succesfully
 (-1) - camera not opened
 (-2) - unknown error

SIDE EFFECTS
  None.

===========================================================================*/
short setCameraParameter(CameraParamContainer setParams);
/*===========================================================================

FUNCTION getCameraParameter


DESCRIPTION

  This function returns the parameter queried in the paramKey.

  It returns a CameraParams union which needs to be interpreted
  as per the key mapping.

DEPENDENCIES
  None

ARGUMENTS IN
  paramKey - parameter key

RETURN VALUE
  Valid camera params if key is in supported set.

SIDE EFFECTS
  None.

===========================================================================*/
CameraParams getCameraParameter(eParamType paramKey);


/*===========================================================================

FUNCTION getMaxZoom


DESCRIPTION

  This function returns the max zoom value from camera.

DEPENDENCIES
  None

ARGUMENTS IN
  None

RETURN VALUE
  max zoom level

SIDE EFFECTS
  None.

===========================================================================*/
short getMaxZoom();


/*===========================================================================

FUNCTION isSmoothZoomSupported


DESCRIPTION

  This function returns whether the Camera Hardware Interface supports Smooth
  Zoom feature.

DEPENDENCIES
  None

ARGUMENTS IN
  None

RETURN VALUE
  1 - If supported
  0 - if not supported

SIDE EFFECTS
  None.

===========================================================================*/
short isSmoothZoomSupported();
/*===========================================================================

FUNCTION startSmoothZoom


DESCRIPTION

  This function starts the Smooth zoom with current zoom level to the set zoom level in stages of 1.
  Callback is given to the ims_notify_callback function pointer at each stage. If the zoom level has
  to stopped before it reaches that level, stopSmoothZoom has to be called. If a new level has to be set,
  function call has to be made after the previous set zoom level is reached or after calling stopSmoothZoom.
  Note: This is only supported if the isSmoothZoomSupported() is true

DEPENDENCIES
  None

ARGUMENTS IN
  value - Zoom value, range is 0 to getMaxZoom()

RETURN VALUE
  None

SIDE EFFECTS
  None.

===========================================================================*/
void startSmoothZoom(unsigned int value);


/*===========================================================================

FUNCTION stopSmoothZoom


DESCRIPTION

  This function stops the Smooth zoom.

DEPENDENCIES
  None

ARGUMENTS IN
  None

RETURN VALUE
  None

SIDE EFFECTS
  None.

===========================================================================*/
void stopSmoothZoom();


/*===========================================================================

FUNCTION registerCameraCallbacks


DESCRIPTION

  This function registers the callback to be triggered to Application layer
  for camera events. This is necessary for Smooth zoom feature.

DEPENDENCIES
  None

ARGUMENTS IN
  None

RETURN VALUE
  None

SIDE EFFECTS
  None.

===========================================================================*/
void registerCameraCallbacks(ims_notify_callback notifyCb);

/*===========================================================================

FUNCTION isZoomSupported


DESCRIPTION

  This function returns whether the Camera Hardware Interface supports
  Zoom feature.

DEPENDENCIES
  None

ARGUMENTS IN
  None

RETURN VALUE
  1 - If supported
  0 - if not supported

SIDE EFFECTS
  None.

===========================================================================*/
short isZoomSupported();


/*===========================================================================

FUNCTION setZoom


DESCRIPTION

  This function Sets current zoom value.

DEPENDENCIES
  None

ARGUMENTS IN
  value - Zoom value, range is 0 to getMaxZoom()

RETURN VALUE
  None

SIDE EFFECTS
  None.

===========================================================================*/
void setZoom(unsigned int value);




#ifdef __cplusplus
}
#endif
#endif
