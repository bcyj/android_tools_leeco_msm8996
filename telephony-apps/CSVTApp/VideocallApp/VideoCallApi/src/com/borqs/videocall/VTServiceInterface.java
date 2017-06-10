/* BORQS Software Solutions Pvt Ltd. CONFIDENTIAL
 * Copyright (c) 2012 All rights reserved.
 *
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by BORQS Software
 * Solutions Pvt Ltd. No part of the Material may be used,copied,
 * reproduced, modified, published, uploaded,posted, transmitted,
 * distributed, or disclosed in any way without BORQS Software
 * Solutions Pvt Ltd. prior written permission.
 *
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you
 * by disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel or otherwise. Any license
 * under such intellectual property rights must be express and
 * approved by BORQS Software Solutions Pvt Ltd. in writing.
 *
 */

package com.borqs.videocall;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.media.AudioManager;

public class VTServiceInterface {

    static final String TAG = " VTServiceInterface.";
    // true for new camera HAL
    private boolean mIsUseVTCamera = true;
    private static Context mCtx;

    public IVTConnection loopback(Context ctx) {
        return (IVTConnection) (new VTLoopBackConnection(ctx));
    }

    public void end(VTService mVTService) {
        mVTService.disconnect();
    }

    public void reject(VTService mVTService) {
        mVTService.disconnect();
    }

    public void fallBack(IVTConnection mConnection) {
        mConnection.fallBack();
    }

    public void answer(IVTConnection mConnection) {
        mConnection.acceptCall();
    }

    public boolean setMute(boolean muted, VTService mVTService)

    {
        return mVTService.setMut(muted);
    }

    public void captureScreen(int where, String strDstPath, VTService mVTService) {
        mVTService.captureScreen(where, strDstPath);
    }

    public void setRemoteDisplay(SurfaceTexture surface, VTService mVTService) {
        mVTService.setRemoteDisplay(surface);
    }

    public void setLocalDisplay(SurfaceTexture surface, VTService mVTService) {
        mVTService.setLocalDisplay(surface);
    }

    public void setCameraParameter(String key, String value, VTService mVTService) {

        mVTService.setCameraParameter(key, value);
    }

    public final boolean isUseVTCamera() {
        return mIsUseVTCamera;
    }

    public final void SetisUseVTCamera(boolean isVtcamera) {
        mIsUseVTCamera = isVtcamera;
    }

    public static void setAppContext(Context ctx) {
        mCtx = ctx;
    }

    public static Context getAppContext() {
        return mCtx;
    }

    public static void turnOnSpeaker(Context ctx, boolean flag) {
        if (MyLog.DEBUG)
            MyLog.v(TAG, "turn on speaker is called=====================" + flag);

        AudioManager audioManager = (AudioManager) ctx.getSystemService(Context.AUDIO_SERVICE);

        audioManager.setSpeakerphoneOn(flag);
    }

    public static boolean isBluetoothScoOn(Context ctx) {
        AudioManager audioManager = (AudioManager) ctx.getSystemService(Context.AUDIO_SERVICE);

        return audioManager.isBluetoothScoOn();
    }

    public static boolean isSpeakerOn(Context ctx) {
        if (MyLog.DEBUG)
            MyLog.v(TAG, "isSpeakeron...");

        AudioManager audioManager = (AudioManager) ctx.getSystemService(Context.AUDIO_SERVICE);

        return audioManager.isSpeakerphoneOn();
    }

    public static void setAudioMode(Context ctx, int mode) {

        if (MyLog.DEBUG)
            MyLog.d(TAG, "PhoneUtils.setAudioMode(" + (mode) + ")...");

        AudioManager audioManager = (AudioManager) ctx.getSystemService(Context.AUDIO_SERVICE);
        audioManager.setMode(mode);

        return;
    }

}
