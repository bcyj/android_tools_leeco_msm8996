/*****************************************************************************
 Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.

 File: ImsCameraImplementation.cpp
 Description: IMS implementation of managing the Camera resources at
 HAL Layer

 Revision History
 ===============================================================================
 Date    |   Author's Name    |  BugID  |        Change Description
 ===============================================================================
 03-Jun-2013   Rakesh K M                 -    First version
 *****************************************************************************/
#include "ImsCameraImplementation.h"
#include <gui/Surface.h>

#ifndef FAREND_WITH_SURFACETEXTURE
#include <gui/BufferQueue.h>
#include <gui/GLConsumer.h>
#endif


#include <binder/IMemory.h>
#include <cutils/log.h>
#include <semaphore.h>
#ifndef ANDROID_MEDIA_JB
#include <android/native_window_jni.h>
#include <android_runtime/android_graphics_SurfaceTexture.h>
#endif

#include <camera/CameraBase.h>
#include <camera/ICameraService.h>
#include <binder/IServiceManager.h>
#include <camera/CameraParameters.h>
#include <string.h>



using namespace android;

//#define DEBUG_ENABLE 1

#ifdef DEBUG_ENABLE
#define DLOG  ALOGE
#else
#define DLOG(...)
#endif
CameraHardwareInterface *camera;
sp<Surface> surfaceNear = NULL;
#define CAMERA_OPEN_CMD 1
#define CAMERA_CLOSE_CMD 2
#define SET_NEAR_END_SURFACE_CMD 3
#define SET_PREVIEW_DISPLAY_ORIENTATION_CMD 4
#define START_PREVIEW_CMD 5
#define STOP_PREVIEW_CMD 6
#define START_RECORDING_CMD 7
#define STOP_RECORDING_CMD 8
#define SET_CAMERA_PARAM_CMD 9
#define GET_CAMERA_PARAM_CMD 10
#define SET_ZOOM_CMD 11
#define GET_MAX_ZOOM_CMD 12
#define RELEASE_CAMERA_FRAME 13
#define IS_ZOOM_SUPPORTED 14


#define TEMP_CAMERA_BUFFER_MAX 5

sp<ANativeWindow> mPreviewNativeWindow;
sp<IGraphicBufferProducer> gproducer;

static const String16* packagename = NULL;
static ImsCameraClient* cameraclient = NULL;

char device_name[10];
int cameraId = 1;
int facing = 1;
int mNumberOfCameras = 0;
camera_module_t *hw;
int previewTransform = 0;
int previewRotationAngle = 0;
int previewRotationOffset = 0;


int gotPreviewSurface = 0;
int startPreviewCalled = 0;
int startRecordingCalled = 0;
int fps;
int height = 240;
int width = 320;


pthread_t cameraLoopThread;
sem_t cameraEventSem;
sem_t cameraEventIntSem;
sem_t cameraEventLoopSem;
sem_t cameraEventReleaseSem;


int camera_cmd = 0;
void* cam_arg1;
void* cam_arg2;
void* cam_arg3;
void* camera_ret;

int processingCameraFrameCount = 0;
int processFrameIndex = 0;
int processFrameCount = 0;
void* framePointer[TEMP_CAMERA_BUFFER_MAX];
int stopping = 0;

sp < ICameraService > camera_service;

sp <ICameraClient> client;

/* Global Context of maintaining all the camera callbacks */
struct global_imscamera
{
	ims_notify_callback notify_cb;
	ims_data_callback data_cb;
	ims_data_callback_timestamp data_cb_timestamp;
	ims_custom_params_callback custom_cb;
        void *user;
};
/* Assigning all the fields to null */
struct global_imscamera global_imscamera_cb={NULL,NULL,NULL,NULL,NULL};

struct FPSRanges {
int lo;
int hi;
FPSRanges() {
      lo = 0;
      hi = 0;
   }
FPSRanges(int low, int high) {
      lo = low;
      hi = high;
   }
};

static int sem_timedwait(sem_t* sem_var){
	struct timespec ts;
	int result;

	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += 5;
	result = sem_timedwait(sem_var, &ts);
	if (result) {
		ALOGE("Failure in [%s:%d] result = %d", __func__, __LINE__, result);
		return -1;
	}

	return 0;
}



inline void acquire_lock_for_cmd()
{
	sem_wait(&cameraEventIntSem);
}

inline void release_lock_for_cmd()
{
	sem_post(&cameraEventIntSem);
}

inline void post_to_camera_loop()
{
	sem_post(&cameraEventLoopSem);
}

inline void wait_till_completion()
{
	sem_timedwait(&cameraEventSem);

}

inline void wait_for_command()
{
	sem_wait(&cameraEventLoopSem);

}

inline void signal_cmd_executed()
{
	sem_post(&cameraEventSem);
}

void* executeCommand(int command, void* arg1, void* arg2,void* arg3)
{

	ALOGE("executeCommand %d",command);

	acquire_lock_for_cmd();

	camera_cmd = command;
	cam_arg1 = arg1;
	cam_arg2 = arg2;
	cam_arg3 = arg3;

	post_to_camera_loop();

	wait_till_completion();

	release_lock_for_cmd();

	return camera_ret;
}


void setCameraFPS(CameraParameters *ptr)
{
	char * fpsRanges = (char *)ptr->get("preview-fps-range-values");
		Vector<FPSRanges> fpsRangeList;

		ALOGE("imscamera::supportedFPS2 = %s",fpsRanges);

		if(fpsRanges == NULL)
			return;

		int i = 0;
		while(fpsRanges[i]!='\0')
		{
			if(fpsRanges[i] == '(')
			{
				i++;
			}

			char* start = &fpsRanges[i];

			while(fpsRanges[i] != ',')
			{
				i++;
			}

			char* end = &fpsRanges[i-1];

			int minFps = (int)strtol(start, &end, 10);

			if(fpsRanges[i] == ',')
			{
				i++;
			}

			start = &fpsRanges[i];

			while(fpsRanges[i] != ')')
			{
				i++;
			}

			end = &fpsRanges[i-1];

			int maxFps = (int)strtol(start, &end, 10);

			fpsRangeList.push(FPSRanges(minFps,maxFps));

			if(fpsRanges[i] == ')')
			{
				i++;
			}

			if(fpsRanges[i] == ',')
			{
				i++;
			}

		}


		for(i = fpsRangeList.size()-1;i >= 0;i--)
		{

			ALOGE("fps hi %d lo %d",fpsRangeList[i].hi, fpsRangeList[i].lo);
			if(fpsRangeList[i].hi <= fps*1000)
				{
				char str[20];
				snprintf(str, sizeof(str), "%d,%d",fpsRangeList[i].lo,fpsRangeList[i].hi);
				ptr->set("preview-fps-range",str);
				break;
				}
		}

}



void setPreviewOrientationInternal(int rotation)
{
	int transform;			//Match the right


	if(facing != CAMERA_FACING_FRONT)
	{
		switch (rotation) {
			case 0: transform = 0; break;
			case 90: transform = HAL_TRANSFORM_ROT_90; break;
			case 180: transform = HAL_TRANSFORM_ROT_180; break;
			case 270: transform = HAL_TRANSFORM_ROT_270; break;
			default: transform = 0; break;
			}
	}
	else
	{
		switch (rotation) {
			case 0: transform = HAL_TRANSFORM_FLIP_H; break;
			case 90: transform = HAL_TRANSFORM_FLIP_H | HAL_TRANSFORM_ROT_90; break;
			case 180: transform = HAL_TRANSFORM_FLIP_V; break;
			case 270: transform = HAL_TRANSFORM_FLIP_V | HAL_TRANSFORM_ROT_90; break;
			default: transform = 0; break;
			}
	}


	if(mPreviewNativeWindow.get()!=NULL)
	{
	previewTransform = transform;
	//native_window_set_buffers_transform(mPreviewNativeWindow.get(),previewTransform);
	ImsCameraClient *ptr = ImsCameraClient::getInstance();
	ptr->iCamera->sendCommand(CAMERA_CMD_SET_DISPLAY_ORIENTATION, rotation, 0);
	ALOGE("cameraHandleLoop::Transform set %d",transform);
	}
	else
	{
	ALOGE("cameraHandleLoop::Transform set %d error",transform);
	previewTransform = transform;
	}
	camera_ret = (void *)0;
}


void startPreviewInternal(ImsCameraClient *ptr)
{
		ALOGE("cameraHandleLoop::startPreviewInternal >");

		if(ptr->iCamera.get() == NULL){
			camera_ret = (void *)-1;
			return;
		}

		String8 params = ptr->iCamera->getParameters();
		CameraParameters* cParams = new CameraParameters();

		cParams->unflatten((const String8 &)params);


		if(ptr->iCamera.get()&&gproducer.get())
			{
			ALOGE("cameraHandleLoop::ImsSetNearEndSurface Camera Set preview window : %d",ptr->iCamera->setPreviewTarget(gproducer));
			}



		CameraInfo info;
		camera_service->getCameraInfo(cameraId,&info);


		CustomCallbackParam callbackParam;

		callbackParam.type = CAMERA_ROTATION_CROP_PARAM;
		callbackParam.cropRotationParam.cameraFacing = info.facing;
		callbackParam.cropRotationParam.cameraMount = info.orientation;
		callbackParam.cropRotationParam.cameraParam = cParams;
		callbackParam.cropRotationParam.rotationOffset = &previewRotationOffset;
		callbackParam.cropRotationParam.height = height;
		callbackParam.cropRotationParam.width = width;





		if(global_imscamera_cb.custom_cb!=NULL)
			{
		global_imscamera_cb.custom_cb(&callbackParam);
			}
		else
			{
			ALOGE("cameraHandleLoop::custom callback is null");
			camera_ret = (void *)-1;
			return;
			}

		if(width>height)
		cParams->setPreviewSize(width,height);
		else
		cParams->setPreviewSize(height,width);

		/*if(width>height)
		cParams->setPictureSize(width,height);
		else
		cParams->setPictureSize(height,width);*/ //only needed for A family

		if(width>height)
		cParams->setVideoSize(width,height);
		else
		cParams->setVideoSize(height,width);

		cParams->set("recording-hint","true");

		const char * focusModes = cParams->get(CameraParameters::KEY_SUPPORTED_FOCUS_MODES);

		ALOGE("focus mode values %s",focusModes);

		if(focusModes && strstr(focusModes,CameraParameters::FOCUS_MODE_CONTINUOUS_VIDEO))
		{
			cParams->set(CameraParameters::KEY_FOCUS_MODE,CameraParameters::FOCUS_MODE_CONTINUOUS_VIDEO);
		}
		else
		{
			ALOGE("FOCUS_MODE_CONTINUOUS_VIDEO mode not supported");
		}

		setCameraFPS(cParams);

		params = cParams->flatten();

		ptr->iCamera->setParameters(params);


		delete cParams;

		ALOGE("previewRotationOffset %d previewRotationAngle %d",previewRotationOffset,previewRotationAngle);

		setPreviewOrientationInternal((previewRotationOffset + previewRotationAngle)%360);


		if(gotPreviewSurface ==1)
			{

		ALOGE("cameraHandleLoop::startCameraPreview  Start Preview : %d",ptr->iCamera->startPreview());
			}

		startPreviewCalled = 1;
		camera_ret = (void *)0;
		ALOGE("cameraHandleLoop::startPreviewInternal <");

}


void startRecordingInternal(ImsCameraClient *ptr)
{
		ALOGE("cameraHandleLoop::startRecordingInternal >");

		if(ptr->iCamera.get() == NULL){
			camera_ret = (void *)-1;
			return;
		}


			if(gotPreviewSurface ==1)
			{
			//ptr->iCamera->setPreviewCallbackFlag(CAMERA_MSG_ERROR | CAMERA_MSG_ZOOM | CAMERA_MSG_FOCUS | CAMERA_MSG_FOCUS_MOVE | CAMERA_MSG_VIDEO_FRAME);
			ALOGE("cameraHandleLoop::startCameraRecording	Storemetadatainbuffers : %d",ptr->iCamera->storeMetaDataInBuffers(1));
			ALOGE("cameraHandleLoop::startCameraRecording startRecording : %d",ptr->iCamera->startRecording());
			ALOGE("cameraHandleLoop::startCameraRecording recordingEnabled : %d",ptr->iCamera->recordingEnabled());
			}

			startRecordingCalled = 1;
			camera_ret = (void *)0;
			ALOGE("cameraHandleLoop::startRecordingInternal <");

}


void cleanUpEncoderNotification()
{
	CustomCallbackParam callbackParam;

	callbackParam.type = CAMERA_STOP_RECORDING_PARAM;
	memset(&callbackParam.cropRotationParam,0,sizeof(callbackParam.cropRotationParam));

	if(global_imscamera_cb.custom_cb!=NULL)
		{
		global_imscamera_cb.custom_cb(&callbackParam);
		}
	else
		{
		ALOGE("cameraHandleLoop::custom callback is null");
		}
}



/*===========================================================================

 FUNCTION cameraHandleLoop

 DESCRIPTION
 This is a thread loop to handle camera hals in a threadsafe manner.

 DEPENDENCIES
 None

 ARGUEMENTS IN
 params for thread looop creation

 RETURN VALUE
 return value for thread loop creation.

 SIDE EFFECTS
 None

 ===========================================================================*/
void* cameraHandleLoop(void* param)
{

	ImsCameraClient *ptr = ImsCameraClient::getInstance();

	/* IMS_ARCH_64 */
    VT_UNUSED_PTR(param);

	ALOGE("cameraHandleLoop thread created");
	while(1)
	{

		wait_for_command();

		switch(camera_cmd)
		{
			case CAMERA_OPEN_CMD:
				{
					if(camera_service == NULL)
					{
					      camera_ret = (void *)(-2);
					      signal_cmd_executed();
					      return NULL;
					}
#if defined(__aarch64__)
					cameraId = (uintptr_t)cam_arg1;
#else
					cameraId = (unsigned int)cam_arg1;
#endif

				    mNumberOfCameras = camera_service->getNumberOfCameras();
					ALOGE("cameraHandleLoop::no of camera %d",mNumberOfCameras);
					CameraInfo info;
					status_t rc = camera_service->getCameraInfo(cameraId,&info);


					ALOGE("cameraHandleLoop::Camera id %d",cameraId);

					ALOGE("cameraHandleLoop::return value of camera info %d",rc);

					if(rc!=0)
						{
						camera_ret = (void *)(-2);
						signal_cmd_executed();
						return NULL;
						}

					facing = info.facing;

					ALOGE("cameraHandleLoop::ImsCameraOpen Camera info facing : %d orientation : %d camera id %d",info.facing,info.orientation,cameraId);
					ptr->state = ENGINE_INITIALIZING;

						client = new BufferClient();

                                                /*Using connectLegacy with version 1.0 to use HAL1. It will fail in DPM and fallback to default HAL which is again HAL1 in DPM*/
						rc  = camera_service->connectLegacy(client,cameraId,CAMERA_DEVICE_API_VERSION_1_0,(const String16&)*packagename,-1,ptr->iCamera);
                                                if (NO_ERROR != rc) {
                                                    ALOGE("cameraHandleLoop::connectLegacy failed. falling back to connect");
                                                    camera_service->connect(client,cameraId,(const String16&)*packagename,-1,ptr->iCamera);
                                                }
						ALOGE("camera service connect done");

				    if(ptr->iCamera.get()==NULL)
					{
						 ALOGE("camera connect api trial after %p",ptr->iCamera.get());
						 camera_ret = (void *)-1;
						 signal_cmd_executed();
						 return NULL;
					}
					ptr->state = ENGINE_INITIALIZED;

					//surfaceNear = NULL;

					gotPreviewSurface = 0;
					startPreviewCalled = 0;
					startRecordingCalled = 0;
					previewTransform = 0;

					camera_ret = (void *)0;
				}
				break;

			case CAMERA_CLOSE_CMD:
				{
				if(ptr->iCamera.get() == NULL)
				{
				ALOGE("cameraHandleLoop::camera error");
				camera_ret = (void *)-1;
				signal_cmd_executed();
				return 0;
				}

				if(startRecordingCalled == 1)
					{
						if(ptr->iCamera.get()) {
							ptr->iCamera->stopRecording();
						}
						cleanUpEncoderNotification();
					}
				startRecordingCalled = 0;

				if(startPreviewCalled == 1)
					{
						if(ptr->iCamera.get()) {
							ptr->iCamera->stopPreview();
						}
					}

				startPreviewCalled = 0;

				ptr->state = ENGINE_UNINITIALIZED;
				ptr->iCamera->setPreviewCallbackFlag(CAMERA_FRAME_CALLBACK_FLAG_NOOP);
				//ptr->iCamera->setPreviewTarget(NULL);
				ptr->iCamera->disconnect();
				ptr->iCamera = 0;
				client = 0;
				previewRotationOffset = 0;
				previewRotationAngle = 0;
				camera_ret = (void *)0;
				signal_cmd_executed();
				return 0;
				}

			case SET_NEAR_END_SURFACE_CMD:
				{

				ALOGE("cameraHandleLoop::ImsSetNearEndSurface mPreviewNativeWindow : %x",(int)(uintptr_t) mPreviewNativeWindow.get());

				if(ptr->iCamera.get()) {
					{
					ALOGE("cameraHandleLoop::ImsSetNearEndSurface Camera Set preview window : %d",ptr->iCamera->setPreviewTarget(gproducer));
					}
				gotPreviewSurface = 1;

				if(startPreviewCalled)
					{
						startPreviewInternal(ptr);

					}

				if(startRecordingCalled)
					{
						startRecordingInternal(ptr);
					}
				camera_ret = (void *)0;
				}
				else
				{
					camera_ret = (void *)-1;
				}


				}

				break;

			case SET_PREVIEW_DISPLAY_ORIENTATION_CMD :

				{

/* IMS_ARCH_64 */
#if defined(__aarch64__)
				previewRotationAngle = (unsigned int) (uintptr_t)cam_arg1;

				ALOGE("cameraHandleLoop::previewRotationOffset %d previewRotationAngle %d cam_arg1 %d",previewRotationOffset,previewRotationAngle,(uintptr_t)cam_arg1);
#else
				previewRotationAngle = (unsigned int) cam_arg1;
				ALOGE("cameraHandleLoop::previewRotationOffset %d previewRotationAngle %d cam_arg1 %p",previewRotationOffset,previewRotationAngle,(int)cam_arg1);
#endif
				setPreviewOrientationInternal((previewRotationOffset + previewRotationAngle)%360);

				}

					break;

			case START_PREVIEW_CMD:

				{
					startPreviewInternal(ptr);

				}

				break;
			case STOP_PREVIEW_CMD:
				{

				if(startRecordingCalled == 1)
					{
						if(ptr->iCamera.get()) {
							ptr->iCamera->stopRecording();
						}
						cleanUpEncoderNotification();
					}
				startRecordingCalled = 0;

				if(startPreviewCalled == 1)
					{
						if(ptr->iCamera.get()) {
							ptr->iCamera->stopPreview();
						}
					}

				startPreviewCalled = 0;
				camera_ret = (void *)0;

				}

				break;
			case START_RECORDING_CMD:
				{

				startRecordingInternal(ptr);

				}

				break;
			case STOP_RECORDING_CMD:
				{

					int count = 0;
					if(startRecordingCalled == 1)
					{
						if(ptr->iCamera.get()) {
							ptr->iCamera->stopRecording();
						}
						cleanUpEncoderNotification();
					}
					ALOGE("Stop recording returned %d %d",processingCameraFrameCount,processFrameCount);
					startRecordingCalled = 0;

				camera_ret = (void *)0;
				}

				break;
			case SET_CAMERA_PARAM_CMD:
				{
				CameraParamContainer* setParams = (CameraParamContainer *) cam_arg1;
				switch(setParams->type)
					{
						case SET_FPS:
							fps = setParams->params.fps;
							ALOGE("cameraHandleLoop::setCameraParameter fps %d",fps);
							break;
						case SET_RESOLUTION:
							height = setParams->params.cameraResolution.height;
							width = setParams->params.cameraResolution.width;
							ALOGE("cameraHandleLoop::setCameraParameter width %d height %d",width,height);
							break;
						default:
							break;
					}
					camera_ret = (void *)0;
				}
				break;
			case GET_CAMERA_PARAM_CMD:
				break;
			case SET_ZOOM_CMD:
				{

				if(ptr->iCamera.get() == NULL)
				{
				ALOGE("cameraHandleLoop::camera error");
				camera_ret = (void *)-1;
				signal_cmd_executed();
				return 0;
				}
#if !defined(__aarch64__)
				unsigned int value = (unsigned int) cam_arg1;
#else
				unsigned int value = (unsigned int) (uintptr_t)cam_arg1;

#endif
				ALOGE("cameraHandleLoop::setZoom value %d",value);
				String8 params = ptr->iCamera->getParameters();
				CameraParameters* cParams = new CameraParameters();

				cParams->unflatten((const String8 &)params);
				cParams->set("zoom",value);
				params = cParams->flatten();

				ptr->iCamera->setParameters(params);

				delete cParams;

				camera_ret = (void *)0;
				ALOGE("cameraHandleLoop::setZoom value %d returned",value);
				}
				break;
			case GET_MAX_ZOOM_CMD:
				{
					if(ptr->iCamera.get() == NULL)
					{
					ALOGE("cameraHandleLoop::camera error");
					camera_ret = (void *)-1;
					signal_cmd_executed();
					return 0;
					}

				String8 params = ptr->iCamera->getParameters();
				CameraParameters* cParams = new CameraParameters();

				cParams->unflatten((const String8 &)params);
				long return_value = cParams->getInt("max-zoom");
                                camera_ret=(void *)return_value;

				delete cParams;

				}
				break;
			case RELEASE_CAMERA_FRAME:
				{
				const sp<IMemory>* dataPtr;

				dataPtr = (const sp<IMemory> *) cam_arg1;

				DLOG("cameraHandleLoop::Released recording frame %p",dataPtr->get());

				if(ptr->iCamera.get() != NULL)
				{
					ptr->iCamera->releaseRecordingFrame(*dataPtr);
				}

				}
				break;
			case IS_ZOOM_SUPPORTED:
				{
					if(ptr->iCamera.get() == NULL)
					{
					ALOGE("cameraHandleLoop::camera error");
					camera_ret = (void *)-1;
					signal_cmd_executed();
					return 0;
					}

					String8 params = ptr->iCamera->getParameters();
					CameraParameters* cParams = new CameraParameters();

					cParams->unflatten((const String8 &)params);
					const char * isSupported = cParams->get("zoom-supported");

					ALOGE("isZoom supported %s",isSupported);

					if(isSupported != NULL && ((isSupported[0] == 't')||(isSupported[0] == 'T')))
					{
						camera_ret = (void *)1;
					}
					else
					{
						camera_ret = (void *)0;
					}

					delete cParams;


				}
				break;



		}
		signal_cmd_executed();


	}
}




short cameraOpen(unsigned int cameraid)
{
	ImsCameraClient *ptr = ImsCameraClient::getInstance();

	//Camera acquire part
	ALOGE("imscamera::ImsCameraOpen >");
	int err;
	int ret;

	sem_init(&cameraEventSem, 0, 0);
	sem_init(&cameraEventLoopSem, 0, 0);
	sem_init(&cameraEventIntSem, 0, 0);
	sem_init(&cameraEventReleaseSem, 0, 1);

	processingCameraFrameCount = 0;
	processFrameIndex = 0;
	processFrameCount = 0;
	stopping = 0;

	packagename = new String16("com.android.dialer");

	if ((err = pthread_create(&cameraLoopThread, NULL, cameraHandleLoop, (void *) NULL))
			< 0) {
		ALOGE("Error during creation of the thread\n");
		return err;
	}

	release_lock_for_cmd();

#if !defined(__aarch64__)
	ret =  (int) executeCommand(CAMERA_OPEN_CMD,(void*)cameraid,0,0);
#else
	ret =  (int) (uintptr_t)executeCommand(CAMERA_OPEN_CMD,(void*)(uintptr_t)cameraid,0,0);
#endif

	//Cleanup the resources we have allocated
	if(ret != 0)
	{
		ptr->state = ENGINE_UNINITIALIZED;
	        (void)pthread_join(cameraLoopThread, NULL);
	        sem_destroy(&cameraEventSem);
	        sem_destroy(&cameraEventIntSem);
	        sem_destroy(&cameraEventLoopSem);
	        sem_destroy(&cameraEventReleaseSem);

	        delete packagename;
	        packagename = NULL;

                /* Camera client should be deleted incase of release */
	        if(cameraclient)
	        {
	                delete cameraclient;
	                cameraclient = NULL;
	        }

	}
	ALOGE("imscamera::ImsCameraOpen ret : %d<",ret);
	return ret;
}



short cameraOpen2(unsigned int cameraid,String16* apppackagename)
{
	ImsCameraClient *ptr = ImsCameraClient::getInstance();

	//Camera acquire part
	ALOGE("imscamera::ImsCameraOpen2 >");
	int err;
	int ret;

	sem_init(&cameraEventSem, 0, 0);
	sem_init(&cameraEventLoopSem, 0, 0);
	sem_init(&cameraEventIntSem, 0, 0);
	sem_init(&cameraEventReleaseSem, 0, 1);

	processingCameraFrameCount = 0;
	processFrameIndex = 0;
	processFrameCount = 0;
	stopping = 0;

	if(apppackagename == NULL)
		{
			ALOGE("Invalid package name");
			return -2;
		}

	packagename = new String16(*apppackagename);


	if ((err = pthread_create(&cameraLoopThread, NULL, cameraHandleLoop, (void *) NULL))
			< 0) {
		ALOGE("Error during creation of the thread\n");
		return err;
	}

	release_lock_for_cmd();

#if !defined(__aarch64__)
        ret =  (int) executeCommand(CAMERA_OPEN_CMD,(void*)cameraid,0,0);
#else
	ret =  (int) (uintptr_t)executeCommand(CAMERA_OPEN_CMD,(void*)(uintptr_t)cameraid,0,0);
#endif

        //Cleanup the resources we have allocated
        if(ret != 0)
        {
                ptr->state = ENGINE_UNINITIALIZED;
                (void)pthread_join(cameraLoopThread, NULL);
                sem_destroy(&cameraEventSem);
                sem_destroy(&cameraEventIntSem);
                sem_destroy(&cameraEventLoopSem);
                sem_destroy(&cameraEventReleaseSem);

                delete packagename;
                packagename = NULL;

                /* Camera client should be deleted incase of release */
                if(cameraclient)
                {
                        delete cameraclient;
                        cameraclient = NULL;
                }

        }


	ALOGE("imscamera::ImsCameraOpen2 ret : %d <",ret);

	return ret;
}



short cameraRelease()
{
	ImsCameraClient *ptr = ImsCameraClient::getInstance();

	if(ptr->state != ENGINE_INITIALIZED)
	{
		return -1;
	}
	ALOGE("imscamera::cameraRelease >");

	ptr->state = ENGINE_DEINITIALIZING;

#if !defined(__aarch64__)
	int ret =  (int) executeCommand(CAMERA_CLOSE_CMD,0,0,0);
#else
int ret =  (int) (uintptr_t)executeCommand(CAMERA_CLOSE_CMD,0,0,0);

#endif
        (void)pthread_join(cameraLoopThread, NULL);
	sem_destroy(&cameraEventSem);
	sem_destroy(&cameraEventIntSem);
	sem_destroy(&cameraEventLoopSem);
	sem_destroy(&cameraEventReleaseSem);

	delete packagename;
	packagename = NULL;

		/* Camera client should be deleted incase of release */
	if(cameraclient)
	{
		delete cameraclient;
		cameraclient = NULL;
	}
	ALOGE("imscamera::cameraRelease <");


	return ret;
}
short setNearEndSurface(JNIEnv * env, jobject jSurfacenear)
{
        int ret=0;

	ImsCameraClient *ptr = ImsCameraClient::getInstance();

	if(ptr->state != ENGINE_INITIALIZED)
		{
			return -1;
		}

	ALOGE("imscamera::ImsSetNearEndSurface> %p",jSurfacenear);



		if (jSurfacenear == NULL) {
					ALOGE("imscamera::ImsSetNearEndSurface Near surface is null so not setting it");
					return 0;
				}



		sp<IGraphicBufferProducer> producer;
                 jclass surfaceClass = env->FindClass("android/view/Surface");

jfieldID surfaceID = env->GetFieldID(surfaceClass, "mNativeObject", "J");
   surfaceNear = reinterpret_cast<Surface *>(env->GetLongField(jSurfacenear,surfaceID));
 if (surfaceNear != NULL) {
       producer = surfaceNear->getIGraphicBufferProducer();
   }
gproducer = producer;
   mPreviewNativeWindow = new Surface(producer, true);


   if (mPreviewNativeWindow == NULL) {
        return -1;
   }


	if(mPreviewNativeWindow.get() == NULL)
	{
	return -1;
	}
                #if 0
#ifdef ANDROID_MEDIA_JB
				jclass surfaceClass = env->FindClass("android/graphics/SurfaceTexture");
				if (surfaceClass == NULL) {
					ALOGE("imscamera::ImsSetNearEndSurface Can't find android/graphics/SurfaceTexture");
					return -1;
				}

				jfieldID surfaceID = env->GetFieldID(surfaceClass,
						ANDROID_GRAPHICS_SURFACETEXTURE_JNI_ID, "I");
				if (surfaceID == NULL) {
					ALOGE("imscamera::ImsSetNearEndSurface Can't find fields.surfaceTexture");
					return -1;
				}


		#ifdef FAREND_WITH_SURFACETEXTURE
				if (surfaceNear
						== reinterpret_cast<SurfaceTexture*>(env->GetIntField(jSurfacenear,
								surfaceID))) {
					ALOGE("imscamera::ImsSetNearEndSurface surface is same, so returning");
					return -1;
				}

				surfaceNear = reinterpret_cast<SurfaceTexture*>(env->GetIntField(jSurfacenear,
						surfaceID));


#else
					if (jSurfacenear != NULL) {
						sp<GLConsumer> surfaceTexture =  reinterpret_cast<GLConsumer*>(env->GetIntField(jSurfacenear, surfaceID));
						if (surfaceTexture != NULL) {

							if(surfaceNear == surfaceTexture->getBufferQueue())
								{
								ALOGE("surface is same, so returning");
								return -1;
								}

							surfaceNear = surfaceTexture->getBufferQueue();
						}
						else
						{
						return -1;
						}

					}
#endif


				if (surfaceNear != NULL) {

#ifdef FAREND_WITH_SURFACETEXTURE

					mPreviewNativeWindow = new SurfaceTextureClient(surfaceNear);

#else
					mPreviewNativeWindow = new Surface(surfaceNear);
#endif

					if(mPreviewNativeWindow.get()!=NULL)
					{
					//native_window_set_buffers_transform(mPreviewNativeWindow.get(),previewTransform);
					ptr->iCamera->sendCommand(CAMERA_CMD_SET_DISPLAY_ORIENTATION, rotation, 0);
					ALOGE("imscamera::Near End Surface Transform set %d",previewTransform);
					}
					else
					{
					ALOGE("imscamera::Near End Surface Transform set %d error",previewTransform);
					}

				}
				else
				{
				  ALOGE("imscamera: surface Null");
				  return -1;
				}



#else
	 sp<IGraphicBufferProducer> producer(SurfaceTexture_getProducer(env, jSurfacenear));
   if (producer == NULL) {
       return -1;
 }
   gproducer = producer;
   mPreviewNativeWindow = new Surface(producer, true);


   if (mPreviewNativeWindow == NULL) {
        return -1;
   }


	if(mPreviewNativeWindow.get() == NULL)
	{
	return -1;
	}
#endif


#endif


	ALOGE("imscamera::ImsSetNearEndSurface<");
#if !defined(__aarch64__)
	 ret = (int) executeCommand(SET_NEAR_END_SURFACE_CMD,0,0,0);
#else
	ret = (int) (uintptr_t)executeCommand(SET_NEAR_END_SURFACE_CMD,0,0,0);
#endif
	return ret;
}
short setPreviewDisplayOrientation(unsigned int rotation)
{
	ALOGE("imscamera::setPreviewDisplayOrientation %d",rotation);

	ImsCameraClient *ptr = ImsCameraClient::getInstance();

	if(ptr->state != ENGINE_INITIALIZED)
		{
			return -1;
		}

#if !defined(__aarch64__)
	int ret = (int) executeCommand(SET_PREVIEW_DISPLAY_ORIENTATION_CMD,(void*)rotation,0,0);
#else
	int ret = (int) (uintptr_t)executeCommand(SET_PREVIEW_DISPLAY_ORIENTATION_CMD,(void*)(uintptr_t)rotation,0,0);
#endif


	ALOGE("imscamera::setPreviewDisplayOrientation <");

	return ret;
}
short startCameraPreview()
{
	ImsCameraClient *ptr = ImsCameraClient::getInstance();


	if(ptr->state != ENGINE_INITIALIZED)
	{
		return -1;
	}

	ALOGE("imscamera::startCameraPreview >");

#if !defined(__aarch64__)
	int ret = (int) executeCommand(START_PREVIEW_CMD,0,0,0);
#else
int ret = (int) (uintptr_t)executeCommand(START_PREVIEW_CMD,0,0,0);
#endif

	ALOGE("imscamera::startCameraPreview<");

	return 0;
}
short stopCameraPreview()
{
	ImsCameraClient *ptr = ImsCameraClient::getInstance();

	if(ptr->state != ENGINE_INITIALIZED)
	{
		return -1;
	}

	ALOGE("imscamera::stopCameraPreview>");

#if !defined(__aarch64__)
	int ret = (int) executeCommand(STOP_PREVIEW_CMD,0,0,0);
#else
	int ret = (int) (uintptr_t)executeCommand(STOP_PREVIEW_CMD,0,0,0);
#endif

	ALOGE("imscamera::stopCameraPreview<");


	return 0;
}
short startCameraRecording()
{
	ImsCameraClient *ptr = ImsCameraClient::getInstance();


	if(ptr->state != ENGINE_INITIALIZED)
	{
		return -1;
	}

	ALOGE("imscamera::startCameraRecording>");

	stopping = 0;
#if !defined(__aarch64__)
	int ret = (int) executeCommand(START_RECORDING_CMD,0,0,0);
#else
	int ret = (int) (uintptr_t)executeCommand(START_RECORDING_CMD,0,0,0);
#endif

	ALOGE("imscamera::startCameraRecording<");

	return 0;
}

short stopCameraRecording()
{

	ImsCameraClient *ptr = ImsCameraClient::getInstance();

	if(ptr->state != ENGINE_INITIALIZED)
	{
		return -1;
	}

	sem_timedwait(&cameraEventReleaseSem);

	stopping = 1;

	ALOGE("imscamera::stopCameraRecording>");

#if !defined(__aarch64__)
	int ret = (int) executeCommand(STOP_RECORDING_CMD,0,0,0);
#else
	int ret = (int) (uintptr_t)executeCommand(STOP_RECORDING_CMD,0,0,0);
#endif
	sem_post(&cameraEventReleaseSem);

	ALOGE("imscamera::stopCameraRecording <");

	return 0;
}



short setCameraParameter(CameraParamContainer setParams)
{

	ImsCameraClient *ptr = ImsCameraClient::getInstance();

	if(ptr->state != ENGINE_INITIALIZED)
	{
		return -1;
	}
	ALOGE("imscamera::setCameraParameter>");

#if !defined(__aarch64__)
	int ret = (int) executeCommand(SET_CAMERA_PARAM_CMD,&setParams,0,0);
#else
	int ret = (int) (uintptr_t)executeCommand(SET_CAMERA_PARAM_CMD,&setParams,0,0);
#endif
	ALOGE("imscamera::setCameraParameter<");

	return ret;
}
CameraParams getCameraParameter(eParamType paramKey)
{
  /* IMS_ARCH_64 */
  VT_UNUSED(paramKey);
	CameraParams p;

	return p;
}
void startSmoothZoom(unsigned int value)
{
	/* IMS_ARCH_64 */
	VT_UNUSED(value);

}

void stopSmoothZoom()
{

}
short getMaxZoom()
{

	ImsCameraClient *ptr = ImsCameraClient::getInstance();

	if(ptr->state != ENGINE_INITIALIZED)
	{
		return -1;
	}

#if !defined(__aarch64__)
	int ret = (int) executeCommand(GET_MAX_ZOOM_CMD,0,0,0);
#else
	int ret = (int) (uintptr_t)executeCommand(GET_MAX_ZOOM_CMD,0,0,0);

#endif


	return ret;
}


short isZoomSupported()
{

	ImsCameraClient *ptr = ImsCameraClient::getInstance();

	if(ptr->state != ENGINE_INITIALIZED)
	{
		return 0;
	}

#if !defined(__aarch64__)
	int ret = (int) executeCommand(IS_ZOOM_SUPPORTED,0,0,0);
#else
int ret = (int) (uintptr_t)executeCommand(IS_ZOOM_SUPPORTED,0,0,0);

#endif



	ALOGE("imscamera::isZoomSupported %d",ret);

	return ret;
}

void setZoom(unsigned int value)
{

	ImsCameraClient *ptr = ImsCameraClient::getInstance();

	if(ptr->state != ENGINE_INITIALIZED)
	{
		return;
	}

	ALOGE("setZoom %d",value);

#if defined(__aarch64__)

	int ret = (int) (uintptr_t)executeCommand(SET_ZOOM_CMD,(void *)(uintptr_t)value,0,0);

#else

	int ret = (int) executeCommand(SET_ZOOM_CMD,(void*)value,0,0);

#endif


}

short registerCallbacks(ims_notify_callback notify_cb,
		ims_data_callback data_cb,
		ims_data_callback_timestamp data_cb_timestamp,
		ims_custom_params_callback custom_cb,
		void* user)
{

    ALOGE("imscamera::registercallbacks ");

    /* Having the callbacks in global context */
    global_imscamera_cb.notify_cb= notify_cb;
    global_imscamera_cb.data_cb = data_cb;
    global_imscamera_cb.data_cb_timestamp = data_cb_timestamp;
    global_imscamera_cb.custom_cb = custom_cb;
	return 0;
}

void registerCameraCallbacks(ims_notify_callback notifyCb)
{
    /* IMS_ARCH_64 */
    VT_UNUSED(notifyCb);
	return ;
}
/*===========================================================================

 FUNCTION ImsCameraClient

 DESCRIPTION
 Constructor for ImsCameraClient

 DEPENDENCIES
 None

 ARGUEMENTS IN
 None

 RETURN VALUE
 success or failiure code

 SIDE EFFECTS
 None

 ===========================================================================*/

ImsCameraClient::ImsCameraClient() {
	state = ENGINE_UNINITIALIZED;
	ALOGE("imscamera::ImsCameraClient %p",this);
	sp < IServiceManager > service = defaultServiceManager();
	sp < IBinder > service_binder = service->getService(
			String16("media.camera"));
	camera_service = interface_cast
			< ICameraService > (service_binder);
	iCamera = NULL;

}

/*===========================================================================

 FUNCTION ~ImsCameraClient

 DESCRIPTION
 Destructor for ImsCameraClient

 DEPENDENCIES
 None

 ARGUEMENTS IN
 None

 RETURN VALUE
 success or failiure code

 SIDE EFFECTS
 None

 ===========================================================================*/

ImsCameraClient::~ImsCameraClient() {


}
/*===========================================================================

 FUNCTION getInstance

 DESCRIPTION
 Static interface to get singleton ImsCameraClient object

 DEPENDENCIES
 None

 ARGUEMENTS IN
 None

 RETURN VALUE
 ImsCameraClient object pointer

 SIDE EFFECTS
 None

 ===========================================================================*/
ImsCameraClient* ImsCameraClient::getInstance() {

	if (cameraclient == NULL) {

		cameraclient = new ImsCameraClient();
	}

	return cameraclient;
}
void BufferClient::dataCallbackTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& data)
{
	DLOG("our dataCallbackTimestamp %p",data.get());
	ImsCameraClient *ptr = ImsCameraClient::getInstance();

	if(global_imscamera_cb.data_cb_timestamp && (stopping != 1)) {

		sem_timedwait(&cameraEventReleaseSem);
		if(stopping != 1)
			{
				processingCameraFrameCount++;
				global_imscamera_cb.data_cb_timestamp(timestamp,
					msgType,
					data,
					NULL);

				executeCommand(RELEASE_CAMERA_FRAME,(void*)(&data),0,0);
				DLOG("our dataCallbackTimestamp releasing data frame %p",data.get());
				processingCameraFrameCount--;
			}
		sem_post(&cameraEventReleaseSem);

	}
	DLOG("our dataCallbackTimestamp end");
}

/*===========================================================================

 FUNCTION init

 DESCRIPTION
 This is to initiate the Camera client state

 DEPENDENCIES
 None

 ARGUEMENTS IN
 params = not used not.

 RETURN VALUE
 Sucess or failure status

 SIDE EFFECTS
 None

 ===========================================================================*/
int ImsCameraClient::init(void* params) {
    /* IMS_ARCH_64 */
	VT_UNUSED(params);

	return 0;
}

/*===========================================================================

 FUNCTION release

 DESCRIPTION
 delete camera client object.

 DEPENDENCIES
 None

 ARGUEMENTS IN
 None

 RETURN VALUE
 sucess or failure status

 SIDE EFFECTS
 None

 ===========================================================================*/

int ImsCameraClient::release() {
return 0;
}
