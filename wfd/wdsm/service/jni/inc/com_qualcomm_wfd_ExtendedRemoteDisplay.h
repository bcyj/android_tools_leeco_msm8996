/*==============================================================================
*       com_qualcomm_wfd_ExtendedRemoteDisplay.h
*
*  DESCRIPTION:
*       Native inteface for ExtendedRemoteDisplay.
*
*  Copyright (c) 2013-2014 by Qualcomm Technologies, Incorporated.
*  All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
*===============================================================================
*/
/*==============================================================================
                             Edit History
================================================================================
*/#include <jni.h>
/* Header for class com_qualcomm_wfd_ExtendedRemoteDisplay */

#ifndef _Included_com_qualcomm_wfd_ExtendedRemoteDisplay
#define _Included_com_qualcomm_wfd_ExtendedRemoteDisplay
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_qualcomm_wfd_ExtendedRemoteDisplay
 * Method:    getNativeObject
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_qualcomm_wfd_ExtendedRemoteDisplay_getNativeObject
  (JNIEnv *, jclass);

/*
 * Class:     com_qualcomm_wfd_ExtendedRemoteDisplay
 * Method:    getSurface
 * Signature: (JII)Landroid/view/Surface;
 */
JNIEXPORT jobject JNICALL Java_com_qualcomm_wfd_ExtendedRemoteDisplay_getSurface
  (JNIEnv *, jclass, jlong, jint, jint);

/*
 * Class:     com_qualcomm_wfd_ExtendedRemoteDisplay
 * Method:    destroySurface
 * Signature: (JLandroid/view/Surface;)I
 */
JNIEXPORT int JNICALL Java_com_qualcomm_wfd_ExtendedRemoteDisplay_destroySurface
  (JNIEnv *, jclass, jlong, jobject);

/*
 * Class:     com_qualcomm_wfd_ExtendedRemoteDisplay
 * Method:    destroyNativeObject
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_qualcomm_wfd_ExtendedRemoteDisplay_destroyNativeObject
  (JNIEnv *, jclass, jlong);


#ifdef __cplusplus
}
#endif
#endif
