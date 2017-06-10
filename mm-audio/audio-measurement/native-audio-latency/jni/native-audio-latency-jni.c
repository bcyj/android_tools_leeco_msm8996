/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Confidential and Proprietary.
 * Modifications to this file Developed by {group/entity that performed the modifications}.
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */
/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

/* This is a native application where we use native methods to measure
 * latency using OpenSL ES. See the corresponding Java source file
 * located at:
 *
 *   src/com/android/nativeaudiolatency/NativeAudioLatency.java
 */

#include <assert.h>
#include <jni.h>
#include <string.h>

#include <android/log.h>

// for native audio
#include <SLES/OpenSLES.h>
#include "SLES/OpenSLES_Android.h"

// for native asset manager
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <pthread.h>
#include <sys/system_properties.h>
#include <cutils/properties.h>
#define LOGW(...) __android_log_print(ANDROID_LOG_DEBUG , "NATIVEAUDIOLATENCY", __VA_ARGS__)

// engine interfaces
static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine;

// output mix interfaces
static SLObjectItf outputMixObject = NULL;
static SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;

// buffer queue player interfaces
static SLObjectItf bqPlayerObject = NULL;
static SLPlayItf bqPlayerPlay;
static SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
static SLEffectSendItf bqPlayerEffectSend;
// aux effect on the output mix, used by the buffer queue player
static const SLEnvironmentalReverbSettings reverbSettings =
    SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;
// recorder interfaces
static SLObjectItf recorderObject = NULL;
static SLRecordItf recorderRecord;
static SLAndroidSimpleBufferQueueItf recorderBufferQueue;

#define PLAYBACK_BUFFER_SIZE 960
static unsigned char coldBuffer[PLAYBACK_BUFFER_SIZE];
static unsigned char warmBuffer[PLAYBACK_BUFFER_SIZE];
static unsigned char contBuffer[PLAYBACK_BUFFER_SIZE];
static unsigned recorderFrames = 0;
static unsigned char *recorderBuffer;

static unsigned recorderSize = 0;
static SLmilliHertz recorderSR;

// pointer and size of the next player buffer to enqueue, and number of remaining buffers
static unsigned char *nextBuffer;
static unsigned nextSize;
static int nextCount;
static int audioLatency;
static struct timeval coldPlayApp;
static struct timeval warmPlayApp;
static struct timeval contPlayApp;
static struct timeval coldRecApp;
static struct timeval coldRecAppStart;
static int recordCount  = 0;
static int coldRecordTime;
static struct timeval contRecApp;
jboolean created;

pthread_mutex_t lock;
pthread_cond_t asyncCompletion;
#define COLD_OUTPUT_LATENCY 1
#define WARM_OUTPUT_LATENCY 2
#define CONTINUOUS_OUTPUT_LATENCY 3
#define COLD_INPUT_LATENCY 4
#define CONTINUOUS_INPUT_LATENCY 5

__attribute__((constructor)) static void onDlOpen(void) {

}


// this callback handler is called every time a buffer finishes playing
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    pthread_mutex_lock(&lock);
    LOGW(" ### bqPlayerCallback nextCount = %d , latency = %d,nextSize = %d", nextCount, audioLatency,nextSize);
    assert(bq == bqPlayerBufferQueue);
    assert(NULL == context);
    SLresult result;
    if (--nextCount > 0 && NULL != nextBuffer && 0 != nextSize) {
        // enqueue another buffer
        if(nextCount == 12 && audioLatency == WARM_OUTPUT_LATENCY) {
            pthread_cond_signal(&asyncCompletion);
            pthread_mutex_unlock(&lock);
             return;
        }
        else if (nextCount == 25 && audioLatency == CONTINUOUS_OUTPUT_LATENCY) {
            gettimeofday(&contPlayApp, NULL);
            LOGW( "CB - Continuous output latency -  time: %lld \n", (int64_t)contPlayApp.tv_sec * 1000 + contPlayApp.tv_usec/1000);
            result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, contBuffer, nextSize);
        }
        else {
            result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, nextBuffer, nextSize);
        }
        // the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
        // which for this code example would indicate a programming error
        assert(SL_RESULT_SUCCESS == result);
    }
    else {
        LOGW("Stopped - playback");
        // set the player's state to STOPPED
        result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_STOPPED);
        assert(SL_RESULT_SUCCESS == result);
        result = (*bqPlayerBufferQueue)->Clear(bqPlayerBufferQueue);
        assert(SL_RESULT_SUCCESS == result);
        pthread_cond_signal(&asyncCompletion);
    }
    pthread_mutex_unlock(&lock);
}

// this callback handler is called every time a buffer finishes recording
void bqRecorderCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    pthread_mutex_lock(&lock);
    assert(bq == recorderBufferQueue);
    assert(NULL == context);
    SLresult result;
    if(recordCount <= 20) {
        if(recordCount == 0 && audioLatency == COLD_INPUT_LATENCY) {
            gettimeofday(&coldRecApp, NULL);
            LOGW("Cold input latency - time: %lld \n", (int64_t)coldRecApp.tv_sec * 1000 + coldRecApp.tv_usec/1000);
            coldRecordTime = (((int64_t)coldRecApp.tv_sec * 1000 + coldRecApp.tv_usec/1000) - ((int64_t)coldRecAppStart.tv_sec * 1000 + coldRecAppStart.tv_usec/1000));
            LOGW("Cold input latency - coldRecordTime: %d ",coldRecordTime );
        }
        if(recordCount == 8 && audioLatency == CONTINUOUS_INPUT_LATENCY) {
            gettimeofday(&contRecApp, NULL);
            LOGW("Continuous input latency - time: %lld \n", (int64_t)contRecApp.tv_sec * 1000 + contRecApp.tv_usec/1000);
        }

        result = (*recorderBufferQueue)->Enqueue(recorderBufferQueue, recorderBuffer,
            recorderFrames);
        assert(SL_RESULT_SUCCESS == result);
        recordCount++;
    }
    else {
        LOGW("Stop recording");
        result = (*recorderRecord)->SetRecordState(recorderRecord, SL_RECORDSTATE_STOPPED);
        if (SL_RESULT_SUCCESS == result) {
            recorderSize = recorderFrames * recordCount;
            recorderSR = SL_SAMPLINGRATE_8;
            recordCount = 0;
            result = (*recorderBufferQueue)->Clear(recorderBufferQueue);
            assert(SL_RESULT_SUCCESS == result);
        } else {
            LOGW("Stop recording failed: %d", result);
        }
        pthread_cond_signal(&asyncCompletion);
        if(recorderBuffer)
            free(recorderBuffer);
    }
    pthread_mutex_unlock(&lock);
}

// create the engine and output mix objects
void Java_com_android_nativeaudiolatency_NativeAudioLatency_createEngine(JNIEnv* env, jclass clazz)
{
    SLresult result;
    // create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    assert(SL_RESULT_SUCCESS == result);
    if((engineObject) == NULL) {
        LOGW("Engine Object NULL");
        return;
    }
    assert((*engineObject) == NULL);

    // realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);

    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    assert(SL_RESULT_SUCCESS == result);
    assert ((*engineEngine) == NULL);

    // create output mix, with environmental reverb specified as a non-required interface
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, NULL, NULL);

    assert(SL_RESULT_SUCCESS == result);
    assert((*outputMixObject) == NULL);

    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);

}


// create buffer queue audio player
void Java_com_android_nativeaudiolatency_NativeAudioLatency_createBufferQueueAudioPlayer(JNIEnv* env,
        jclass clazz)
{
    SLresult result;

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_48,
        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT/*SL_SPEAKER_FRONT_CENTER*/, SL_BYTEORDER_LITTLEENDIAN};
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    // create audio player
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &audioSrc, &audioSnk,
            1, ids, req);
    assert(SL_RESULT_SUCCESS == result);
    assert((*bqPlayerObject) == NULL);
    // realize the player
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);

    // get the play interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    assert(SL_RESULT_SUCCESS == result);
    assert((*bqPlayerPlay) == NULL);

    // get the buffer queue interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
            &bqPlayerBufferQueue);
    assert(SL_RESULT_SUCCESS == result);
    assert((*bqPlayerBufferQueue) == NULL);

    // register callback on the buffer queue
    result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, NULL);
    assert(SL_RESULT_SUCCESS == result);

}

// select the desired clip and play count, and enqueue the first buffer if idle
jboolean Java_com_android_nativeaudiolatency_NativeAudioLatency_selectClip(JNIEnv* env, jclass clazz, jint which,
        jint count) {

    LOGW("%s: ### count = %d",__func__,count);
    unsigned char *oldBuffer = nextBuffer;
    SLresult result;
    pthread_mutex_lock(&lock);
    int j =0;
    switch (which) {
        case 0:     // CLIP_NONE
            LOGW("%s: CLIP_NONE ###",__func__);
            nextSize = 0;
            break;
        case 1:     // COLD_OUTPUT_LATENCY
            LOGW("%s: CLIP_COLD ###",__func__);
            audioLatency = COLD_OUTPUT_LATENCY;
            nextBuffer = (unsigned char *) coldBuffer;
            nextSize = sizeof(coldBuffer);
            LOGW("nextSize = %d", nextSize);
            gettimeofday(&coldPlayApp, NULL);
            LOGW( "Cold output latency - Track.play time: %lld \n", (int64_t)coldPlayApp.tv_sec * 1000 + coldPlayApp.tv_usec/1000);
            break;
        case 2:     // WARM_OUTPUT_LATENCY
            LOGW("%s: CLIP_WARM ###",__func__);
            audioLatency = WARM_OUTPUT_LATENCY;
            nextBuffer = (unsigned char  *) coldBuffer;
            nextSize = sizeof(coldBuffer);
            LOGW("nextSize = %d", nextSize);
            break;
        case 3:     // CONTINUOUS_OUTPUT_LATENCY
            LOGW("%s: CLIP_CONTINUOUS ###",__func__);
            audioLatency = CONTINUOUS_OUTPUT_LATENCY;
            nextBuffer = (unsigned char *)coldBuffer;
            nextSize = sizeof(coldBuffer);
            LOGW("nextSize = %d", nextSize);
            break;
        default:
            LOGW("%s: default ###",__func__);
            nextSize = 0;
            break;
    }
    nextCount = count;
    if (nextSize > 0) {
        // here we only enqueue one buffer because it is a long clip,
        // but for streaming playback we would typically enqueue at least 2 buffers to start
        // set the player's state to playing
        result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
        assert(SL_RESULT_SUCCESS == result);

        result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, nextBuffer, nextSize);
        if (SL_RESULT_SUCCESS != result) {
            pthread_mutex_unlock(&lock);
            return JNI_FALSE;
        }
        pthread_cond_wait(&asyncCompletion,&lock);
        LOGW("Wait Play --");
        if(audioLatency == WARM_OUTPUT_LATENCY) {
            result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PAUSED);
            assert(SL_RESULT_SUCCESS == result);
            result = (*bqPlayerBufferQueue)->Clear(bqPlayerBufferQueue);
            assert(SL_RESULT_SUCCESS == result);
            LOGW("sleep++");
            usleep (1000 * 1000);
            LOGW("sleep--");
            result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
            assert(SL_RESULT_SUCCESS == result);
            gettimeofday(&warmPlayApp, NULL);
            LOGW( "CB-Warm output latency -  time: %lld \n", (int64_t)warmPlayApp.tv_sec * 1000 + warmPlayApp.tv_usec/1000);
            result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, warmBuffer, nextSize);
        }
    }
    pthread_mutex_unlock(&lock);
    return JNI_TRUE;
}

// create audio recorder
jboolean Java_com_android_nativeaudiolatency_NativeAudioLatency_createAudioRecorder(JNIEnv* env, jclass clazz, jint latency) {

    SLresult result;
    audioLatency = latency;
    pthread_mutex_lock(&lock);
    // configure audio source
    SLDataLocator_IODevice loc_dev = {SL_DATALOCATOR_IODEVICE, SL_IODEVICE_AUDIOINPUT,
            SL_DEFAULTDEVICEID_AUDIOINPUT, NULL};
    SLDataSource audioSrc = {&loc_dev, NULL};

    // configure audio sink
    SLDataLocator_AndroidSimpleBufferQueue loc_bq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_8,
        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
    SLDataSink audioSnk = {&loc_bq, &format_pcm};

    // create audio recorder
    // (requires the RECORD_AUDIO permission)
    const SLInterfaceID id[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    result = (*engineEngine)->CreateAudioRecorder(engineEngine, &recorderObject, &audioSrc,
            &audioSnk, 1, id, req);
    if (SL_RESULT_SUCCESS != result) {
        pthread_mutex_unlock(&lock);
        return JNI_FALSE;
    }
    assert((*recorderObject) == NULL);

    // realize the audio recorder
    result = (*recorderObject)->Realize(recorderObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        pthread_mutex_unlock(&lock);
        return JNI_FALSE;
    }

    // get the record interface
    result = (*recorderObject)->GetInterface(recorderObject, SL_IID_RECORD, &recorderRecord);
    assert(SL_RESULT_SUCCESS == result);
    assert((*recorderRecord) == NULL);

    // get the buffer queue interface
    result = (*recorderObject)->GetInterface(recorderObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
            &recorderBufferQueue);
    assert(SL_RESULT_SUCCESS == result);
    assert((*recorderBufferQueue) == NULL);

    // register callback on the buffer queue
    result = (*recorderBufferQueue)->RegisterCallback(recorderBufferQueue, bqRecorderCallback,
            NULL);
    assert(SL_RESULT_SUCCESS == result);
    created = JNI_TRUE;
    pthread_mutex_unlock(&lock);
    return JNI_TRUE;
}


// set the recording state for the audio recorder
void Java_com_android_nativeaudiolatency_NativeAudioLatency_startRecording(JNIEnv* env, jclass clazz, jint latency) {

    SLresult result;
    audioLatency = latency;
    pthread_mutex_lock(&lock);
    result = (*recorderRecord)->SetRecordState(recorderRecord, SL_RECORDSTATE_STOPPED);
    assert(SL_RESULT_SUCCESS == result);
    result = (*recorderBufferQueue)->Clear(recorderBufferQueue);
    assert(SL_RESULT_SUCCESS == result);

    // the buffer is not valid for playback yet
    recorderSize = 0;
    recorderFrames = 320;

    recorderBuffer = malloc (recorderFrames * sizeof (char));
    result = (*recorderBufferQueue)->Enqueue(recorderBufferQueue, recorderBuffer,
              recorderFrames);
    // the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
    // which for this code example would indicate a programming error
    assert(SL_RESULT_SUCCESS == result);
    if(audioLatency == COLD_INPUT_LATENCY) {
        gettimeofday(&coldRecAppStart, NULL);
        LOGW("Cold input latency - record.startRecord time: %lld \n", (int64_t)coldRecAppStart.tv_sec * 1000 + coldRecAppStart.tv_usec/1000);
    }

    // start recording
    result = (*recorderRecord)->SetRecordState(recorderRecord, SL_RECORDSTATE_RECORDING);
    assert(SL_RESULT_SUCCESS == result);
    //Lock here
    pthread_cond_wait(&asyncCompletion,&lock);

    pthread_mutex_unlock(&lock);
}

// shut down the native audio system
void Java_com_android_nativeaudiolatency_NativeAudioLatency_shutdown(JNIEnv* env, jclass clazz) {
    // destroy buffer queue audio player object, and invalidate all associated interfaces
    if (bqPlayerObject != NULL) {
        (*bqPlayerObject)->Destroy(bqPlayerObject);
        bqPlayerObject = NULL;
        bqPlayerPlay = NULL;
        bqPlayerBufferQueue = NULL;
        bqPlayerEffectSend = NULL;
    }
    // destroy audio recorder object, and invalidate all associated interfaces
    if (recorderObject != NULL) {
        (*recorderObject)->Destroy(recorderObject);
        recorderObject = NULL;
        recorderRecord = NULL;
        recorderBufferQueue = NULL;
        created = JNI_FALSE;
    }

    pthread_cond_destroy(&asyncCompletion);
    pthread_mutex_destroy(&lock);
    // destroy output mix object, and invalidate all associated interfaces
    if (outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironmentalReverb = NULL;
    }

    // destroy engine object, and invalidate all associated interfaces
    if (engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }
    coldPlayApp.tv_sec = 0;
    coldPlayApp.tv_usec = 0;
    warmPlayApp.tv_sec = 0;
    warmPlayApp.tv_usec = 0;
    contPlayApp.tv_sec = 0;
    contPlayApp.tv_usec = 0;
    coldRecordTime = 0;
    contRecApp.tv_sec = 0;
    contRecApp.tv_usec = 0;
}

jlong Java_com_android_nativeaudiolatency_NativeAudioLatency_getColdOutputLatency(JNIEnv* env, jclass clazz) {
    return (int64_t)coldPlayApp.tv_sec * 1000 + coldPlayApp.tv_usec/1000;
}

jlong Java_com_android_nativeaudiolatency_NativeAudioLatency_getWarmOutputLatency(JNIEnv* env, jclass clazz) {
    return (int64_t)warmPlayApp.tv_sec * 1000 + warmPlayApp.tv_usec/1000;
}

jlong Java_com_android_nativeaudiolatency_NativeAudioLatency_getContinuousOutputLatency(JNIEnv* env, jclass clazz) {
    return (int64_t)contPlayApp.tv_sec * 1000 + contPlayApp.tv_usec/1000;
}

jlong Java_com_android_nativeaudiolatency_NativeAudioLatency_getColdInputLatency(JNIEnv* env, jclass clazz) {
    return coldRecordTime;
}

jlong Java_com_android_nativeaudiolatency_NativeAudioLatency_getContinuousInputLatency(JNIEnv* env, jclass clazz) {
    return (int64_t)contRecApp.tv_sec * 1000 + contRecApp.tv_usec/1000;
}

jboolean Java_com_android_nativeaudiolatency_NativeAudioLatency_isRecorderCreated(JNIEnv* env, jclass clazz) {
    /* Destroy recorderObject if existing. This is deferred to this function to avoid deadlock
       between destroy() and bqRecorderCallback() after recording is stopped
    */
    if (recorderObject != NULL) {
       (*recorderObject)->Destroy(recorderObject);
        recorderObject = NULL;
        recorderRecord = NULL;
        recorderBufferQueue = NULL;
        created = JNI_FALSE;
    }
    return created;
}
void Java_com_android_nativeaudiolatency_NativeAudioLatency_init(JNIEnv* env, jclass clazz) {

    unsigned i;
    for(i = 0; i < PLAYBACK_BUFFER_SIZE; i=i+4) {
        coldBuffer[i] =  0x00;
        coldBuffer[i+1]=  0x00;
        coldBuffer[i+2]=  0x00;
        coldBuffer[i+3]=  0x00;
    }
    warmBuffer[0]=0x57;
    warmBuffer[1]=0x41;
    warmBuffer[2]=0x00;
    warmBuffer[3]=0x00;
    for(i = 4; i < PLAYBACK_BUFFER_SIZE; i=i+4) {
        warmBuffer[i]=0x00;
        warmBuffer[i+1]=0x00;
        warmBuffer[i+2]=0x00;
        warmBuffer[i+3]=0x00;
    }

    contBuffer[0]=0x00;
    contBuffer[1]=0x00;
    contBuffer[2]=0x4E;
    contBuffer[3]=0x54;
    for(i = 4; i < PLAYBACK_BUFFER_SIZE; i=i+4) {
        contBuffer[i]=0x00;
        contBuffer[i+1]=0x00;
        contBuffer[i+2]=0x00;
        contBuffer[i+3]=0x00;
    }
    created = JNI_FALSE;
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&asyncCompletion, NULL);
}

