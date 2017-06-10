/* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.ims.vt;

import android.content.Context;
import android.util.Log;

import com.android.internal.telephony.DriverCall;

import org.codeaurora.ims.DriverCallIms;
import org.codeaurora.ims.ImsCallSessionImpl;
import org.codeaurora.ims.ImsServiceSub;

import java.util.List;

// TODO Maybe rename to ImsGlobals..
public class ImsVideoGlobals {
    private static String TAG = "VideoCall_ImsVideoGlobals";

    private static ImsVideoGlobals sInstance = null;
    private ImsServiceSub mServiceSub;
    private CameraController mCameraController;
    private MediaController mMediaController;

    private Context mContext;


    public static synchronized void init(ImsServiceSub serviceSub, Context context) {
        if (serviceSub==null) throw new IllegalArgumentException("Default subscription is null.");

        if (sInstance == null) {
            sInstance = new ImsVideoGlobals(serviceSub, context);
        } else {
            throw new RuntimeException("ImsVideoGlobals: Multiple initializaiton.");
        }
    }

    public synchronized static ImsVideoGlobals getInstance() {
        if (sInstance == null) {
            throw new RuntimeException("ImsVideoGlobals: Multiple initializaiton.");
        }
        return sInstance;
    }

    // TODO Introduce onSubscriptonChanged event and subscribe for that event.
    public void setActiveSub(ImsServiceSub serviceSub) {
        if (serviceSub==null) throw new IllegalArgumentException("Active subscription is null.");

        log("SetActiveSub, Sub # " + serviceSub.getSubscription());
        if (mServiceSub != null) {
            mServiceSub.removeListener(mMediaController);
        }

        mServiceSub = serviceSub;
        mServiceSub.addListener(mMediaController);
    }

    private ImsVideoGlobals(ImsServiceSub serviceSub, Context context) {
        mServiceSub = serviceSub;

        mContext = context;
        CameraController.init(context, ImsMedia.getInstance());
        MediaController.init(context, ImsMedia.getInstance());

        mCameraController = CameraController.getInstance();
        mMediaController = MediaController.getInstance();
        mServiceSub.addListener(mMediaController);
    }

    /* package */
    ImsCallSessionImpl getActiveCallSession() {
        List<ImsCallSessionImpl> sessionList =
                mServiceSub.getCallSessionByState(DriverCallIms.State.ACTIVE);
        if (sessionList.size() > 1) log("Multiple Active Calls: " + sessionList);
        return sessionList.isEmpty() ? null : sessionList.get(0);
    }

    /* package */
    ImsCallSessionImpl getOutgoingCallSession() {
        List<ImsCallSessionImpl> sessionList =
                mServiceSub.getCallSessionByState(DriverCallIms.State.ALERTING);
        if (sessionList.isEmpty()) {
            sessionList =mServiceSub.getCallSessionByState(DriverCallIms.State.DIALING);
        }

        if (sessionList.size() > 1) log("Multiple Outgoing Calls: " + sessionList);
        return sessionList.isEmpty() ? null : sessionList.get(0);
    }

    /* package */
    ImsCallSessionImpl getActiveOrOutgoingCallSession() {
        ImsCallSessionImpl session = getActiveCallSession();
        if (session==null) {
            session = getOutgoingCallSession();
        }
        return session;
    }

    private ImsVideoCallProviderImpl getImsVideoCallProviderImpl(ImsCallSessionImpl session) {
        return session == null ? null : session.getImsVideoCallProviderImpl();
    }

    /* package */
    ImsVideoCallProviderImpl getActiveCallVideoProvider() {
        return getImsVideoCallProviderImpl(getActiveCallSession());
    }

    /* package */
    ImsVideoCallProviderImpl getOutgoingCallVideoProvider() {
        return getImsVideoCallProviderImpl(getOutgoingCallSession());
    }

    /* package */
    ImsVideoCallProviderImpl getActiveOrOutgoingCallVideoProvider() {
        return getImsVideoCallProviderImpl(getActiveOrOutgoingCallSession());
    }

    /* package */
    ImsCallSessionImpl findSessionbyMediaId(int mediaId) {
        return mServiceSub.findSessionByMediaId(mediaId);
    }

    /* package */
    ImsVideoCallProviderImpl findVideoCallProviderbyMediaId(int mediaId) {
        return getImsVideoCallProviderImpl(mServiceSub.findSessionByMediaId(mediaId));
    }

    /* package */
    CameraController getCameraController() { return mCameraController; }

    /* package */
    MediaController getMediaController() { return mMediaController; }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }
    private static void loge(String msg) {
        Log.e(TAG, msg);
    }
}
