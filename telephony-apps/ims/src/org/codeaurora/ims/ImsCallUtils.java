/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package org.codeaurora.ims;

import android.telecom.VideoProfile;
import android.telecom.Connection.VideoProvider;
import android.util.Log;

import com.android.ims.ImsCallProfile;
import com.android.internal.telephony.Call;
import com.android.internal.telephony.DriverCall;

public class ImsCallUtils {
    private static String TAG = "ImsCallUtils";
    /** Checks if a call type is any valid video call type with or without direction
     */
    public static boolean isVideoCall(int callType) {
        return callType == CallDetails.CALL_TYPE_VT
                || callType == CallDetails.CALL_TYPE_VT_TX
                || callType == CallDetails.CALL_TYPE_VT_RX
                || callType == CallDetails.CALL_TYPE_VT_PAUSE
                || callType == CallDetails.CALL_TYPE_VT_RESUME
                || callType == CallDetails.CALL_TYPE_VT_NODIR;
    }

    /** Check if call type is valid for lower layers
     */
    public static boolean isValidRilModifyCallType(int callType){
        return callType == CallDetails.CALL_TYPE_VT
                || callType == CallDetails.CALL_TYPE_VT_TX
                || callType == CallDetails.CALL_TYPE_VT_RX
                || callType == CallDetails.CALL_TYPE_VOICE
                || callType == CallDetails.CALL_TYPE_VT_NODIR;
    }

    /** Checks if videocall state transitioned to Video Paused state
     * @param conn - Current connection object
     * @param dc - Latest DriverCallIms instance
     */
    public static boolean isVideoPaused(ImsCallSessionImpl conn, DriverCallIms dc) {
        int currCallType = conn.getInternalCallType();
        DriverCallIms.State currCallState = conn.getInternalState();
        int nextCallType = dc.callDetails.call_type;
        DriverCallIms.State nextCallState = dc.state;

        boolean isHoldingPaused = isVideoCall(currCallType)
                && (currCallState == DriverCallIms.State.HOLDING)
                && isVideoCallTypeWithoutDir(nextCallType)
                && (nextCallState == DriverCallIms.State.ACTIVE);
        boolean isActivePaused =  (isVideoCall(currCallType)
                && (currCallState == DriverCallIms.State.ACTIVE)
                && isVideoCallTypeWithoutDir(nextCallType)
                && (nextCallState == DriverCallIms.State.ACTIVE));
        return isHoldingPaused || isActivePaused;
    }

    /** Detects active video call
     */
    public static boolean canVideoPause(ImsCallSessionImpl conn) {
        return isVideoCall(conn.getInternalCallType()) && conn.isCallActive();
    }

    /** Checks if videocall state transitioned to Video Resumed state
     * @param conn - Current connection object
     * @param dc - Latest DriverCallIms instance
     */
    public static boolean isVideoResumed(ImsCallSessionImpl conn, DriverCallIms dc) {
        int currCallType = conn.getInternalCallType();
        DriverCallIms.State currCallState = conn.getInternalState();
        int nextCallType = dc.callDetails.call_type;
        DriverCallIms.State nextCallState = dc.state;

        return (isVideoCallTypeWithoutDir(currCallType)
                && (currCallState == DriverCallIms.State.ACTIVE)
                && isVideoCall(nextCallType)
                && (nextCallState == DriverCallIms.State.ACTIVE));
    }

    /** Checks if AVP Retry needs to be triggered during dialing
     * @param conn - Current connection object
     * @param dc - Latest DriverCallIms instance
     */
    public static boolean isAvpRetryDialing(ImsCallSessionImpl conn, DriverCallIms dc) {
        int currCallType = conn.getInternalCallType();
        DriverCallIms.State currCallState = conn.getInternalState();
        int nextCallType = dc.callDetails.call_type;
        DriverCallIms.State nextCallState = dc.state;

        boolean dialingAvpRetry = (isVideoCall(currCallType)
                    && (currCallState == DriverCallIms.State.DIALING || currCallState == DriverCallIms.State.ALERTING)
                    && isVideoCallTypeWithoutDir(nextCallType)
                    && nextCallState == DriverCallIms.State.ACTIVE);
        return (conn.getImsCallModification().isAvpRetryAllowed() && dialingAvpRetry);
    }

    /** Checks if AVP Retry needs to be triggered during upgrade
     * @param conn - Current connection object
     * @param dc - Latest DriverCallIms instance
     */
    public static boolean isAvpRetryUpgrade(ImsCallSessionImpl conn, DriverCallIms dc) {
        int currCallType = conn.getInternalCallType();
        DriverCallIms.State currCallState = conn.getInternalState();
        int nextCallType = dc.callDetails.call_type;
        DriverCallIms.State nextCallState = dc.state;

        boolean upgradeAvpRetry = (currCallType == CallDetails.CALL_TYPE_VOICE
                    && currCallState == DriverCallIms.State.ACTIVE
                    && isVideoCallTypeWithoutDir(nextCallType)
                    && nextCallState == DriverCallIms.State.ACTIVE);
        return (conn.getImsCallModification().isAvpRetryAllowed() && upgradeAvpRetry);
    }

    /** Checks if a call type is video call type with direction
     * @param callType
     */
    public static boolean isVideoCallTypeWithDir(int callType) {
        return callType == CallDetails.CALL_TYPE_VT
                || callType == CallDetails.CALL_TYPE_VT_RX
                || callType == CallDetails.CALL_TYPE_VT_TX;
    }

    /** Checks if a call type is video call type without direction
     * @param callType
     */
    public static boolean isVideoCallTypeWithoutDir(int callType) {
        return callType == CallDetails.CALL_TYPE_VT_NODIR;
    }

    public static int convertVideoStateToCallType(int videoState) {
        int callType = CallDetails.CALL_TYPE_VOICE;
        switch (videoState) {
            case VideoProfile.VideoState.AUDIO_ONLY:
                callType = CallDetails.CALL_TYPE_VOICE;
                break;
            case VideoProfile.VideoState.RX_ENABLED:
                callType = CallDetails.CALL_TYPE_VT_RX;
                break;
            case VideoProfile.VideoState.TX_ENABLED:
                callType = CallDetails.CALL_TYPE_VT_TX;
                break;
            case VideoProfile.VideoState.BIDIRECTIONAL:
                callType = CallDetails.CALL_TYPE_VT;
                break;
            case VideoProfile.VideoState.PAUSED:
                callType = CallDetails.CALL_TYPE_VT_NODIR;
                break;
        }
        return callType;
    }

    public static int convertCallTypeToVideoState(int callType) {
        int videoState = VideoProfile.VideoState.AUDIO_ONLY;
        switch (callType) {
            case CallDetails.CALL_TYPE_VOICE:
                videoState = VideoProfile.VideoState.AUDIO_ONLY;
                break;
            case CallDetails.CALL_TYPE_VT_RX:
                videoState = VideoProfile.VideoState.RX_ENABLED;
                break;
            case CallDetails.CALL_TYPE_VT_TX:
                videoState = VideoProfile.VideoState.TX_ENABLED;
                break;
            case CallDetails.CALL_TYPE_VT:
                videoState = VideoProfile.VideoState.BIDIRECTIONAL;
                break;
            case CallDetails.CALL_TYPE_VT_PAUSE:
            case CallDetails.CALL_TYPE_VT_NODIR:
                videoState = VideoProfile.VideoState.PAUSED;
                break;
        // TODO add case for resumed when Google adds it
        }
        return videoState;
    }

    public static int convertToInternalCallType(int imsCallProfileCallType) {
        int internalCallType = CallDetails.CALL_TYPE_UNKNOWN;
        switch (imsCallProfileCallType) {
            case ImsCallProfile.CALL_TYPE_VOICE:
            case ImsCallProfile.CALL_TYPE_VOICE_N_VIDEO:
                internalCallType = CallDetails.CALL_TYPE_VOICE;
                break;
            case ImsCallProfile.CALL_TYPE_VT:
            case ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE:
                internalCallType = CallDetails.CALL_TYPE_VT;
                break;
            case ImsCallProfile.CALL_TYPE_VT_NODIR:
                internalCallType = CallDetails.CALL_TYPE_VT_NODIR;
                break;
            case ImsCallProfile.CALL_TYPE_VT_TX:
                internalCallType = CallDetails.CALL_TYPE_VT_TX;
                break;
            case ImsCallProfile.CALL_TYPE_VS_RX:
                internalCallType = CallDetails.CALL_TYPE_VT_RX;
                break;
            default:
                Log.e(TAG, "convertToInternalCallType invalid calltype " + imsCallProfileCallType);
                break;
        }
        return internalCallType;
    }

    public static VideoProfile convertToVideoProfile(int callType, int callQuality) {
        VideoProfile videoCallProfile = new VideoProfile(
                convertCallTypeToVideoState(callType), callQuality);
        // TODO in future - add quality to CallDetails
        return videoCallProfile;
    }

    public static int convertImsErrorToUiError(int error) {
        if (error == CallModify.E_CANCELLED) {
            return VideoProvider.SESSION_MODIFY_REQUEST_TIMED_OUT;
        } else if (error == CallModify.E_SUCCESS || error == CallModify.E_UNUSED) {
            return VideoProvider.SESSION_MODIFY_REQUEST_SUCCESS;
        } else {
            return VideoProvider.SESSION_MODIFY_REQUEST_FAIL;
        }
    }

    public static int getUiErrorCode(int imsErrorCode) {
        int status = VideoProvider.SESSION_MODIFY_REQUEST_SUCCESS;
        switch (imsErrorCode) {
            case ImsQmiIF.E_SUCCESS:
            case ImsQmiIF.E_UNUSED:
                status = VideoProvider.SESSION_MODIFY_REQUEST_SUCCESS;
                break;
            case ImsQmiIF.E_CANCELLED:
                status = VideoProvider.SESSION_MODIFY_REQUEST_TIMED_OUT;
                break;
            case ImsQmiIF.E_REJECTED_BY_REMOTE:
                status = VideoProvider.SESSION_MODIFY_REQUEST_REJECTED_BY_REMOTE;
                break;
            case ImsQmiIF.E_INVALID_PARAMETER:
                status = VideoProvider.SESSION_MODIFY_REQUEST_INVALID;
                break;
            default:
                status = VideoProvider.SESSION_MODIFY_REQUEST_FAIL;
        }
        return status;
    }
}
