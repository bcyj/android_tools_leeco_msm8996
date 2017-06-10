/*
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution
 * Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (C) 2006 The Android Open Source Project
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
 */

package org.codeaurora.ims;

import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.telecom.VideoProfile;
import android.util.Log;

import com.android.internal.telephony.CallManager;
import com.android.internal.telephony.CallStateException;

import java.util.Map;

public class ImsCallModification {
    public ImsCallSessionImpl mImsCallSessionImpl;
    private ImsSenderRxr mCi;
    private CallModify callModifyRequest = null;
    public int mIndex = -1; // index in ImsCallTracker.connections[], -1 if unassigned
    private int mPrevVideoCallType = CallDetails.CALL_TYPE_UNKNOWN;
    private boolean mAvpRetryAllowed = true;
    private int mAvpCallType = CallDetails.CALL_TYPE_UNKNOWN;
    static final int EVENT_AVP_UPGRADE_DONE = 5;
    static final int EVENT_MODIFY_CALL_INITIATE_DONE = 6;
    static final int EVENT_VIDEO_PAUSE_DONE = 7;
    static final int EVENT_MODIFY_CALL_CONFIRM_DONE = 8;
    static final int EVENT_VIDEO_PAUSE_RETRY = 9;

    private int mMultiTaskRetryCount = 0;
    private static final int MAX_MULTITASK_RETRIES = 1;
    static final int PAUSE_DELAY_MILLIS = 3 * 1000;
    private static final String LOG_TAG = "VideoCall_ImsCallModification";
    Handler mHandler;
    private static final boolean DBG = true;

    private enum PauseState {
        NONE, PAUSE, RESUME
    };

    private PauseState mPendingVTMultitask = PauseState.NONE;

    // Indicates if pause request sent from local UE succeeded
    private boolean mIsLocallyPaused = false;

    public boolean isLocallyPaused() {
        return mIsLocallyPaused;
    }

    public ImsCallModification(ImsCallSessionImpl imsCallSessionImpl, ImsSenderRxr senderRxr) {
        if (DBG) {
            log("ImsCallModification instance created imsCallSessionImpl=" + imsCallSessionImpl);
        }
        mCi = senderRxr;
        mImsCallSessionImpl = imsCallSessionImpl;
        mHandler = new ImsCallModificationHandler();
    }

    private boolean isVTMultitaskRequest(int callType) {
        return callType == CallDetails.CALL_TYPE_VT_PAUSE
                || callType == CallDetails.CALL_TYPE_VT_RESUME;
    }

    private void triggerOrQueueVTMultitask(int callType) {
        log("triggerOrQueueVTMultitask callType= " + callType + " conn= " + this);
        boolean isPauseRequested = callType == CallDetails.CALL_TYPE_VT_PAUSE;
        // Update the pending pause flag
        mPendingVTMultitask = isPauseRequested ? PauseState.PAUSE : PauseState.RESUME;
        if (callModifyRequest == null) {
            processPendingVTMultitask();
        }
    }

    private int toRilCallType(int callType) {
        int rilCallType = CallDetails.CALL_TYPE_UNKNOWN;
        if (callType == CallDetails.CALL_TYPE_VT_PAUSE) {
            rilCallType = CallDetails.CALL_TYPE_VT_NODIR;
        } else if (callType == CallDetails.CALL_TYPE_VT_RESUME) {
            rilCallType = mPrevVideoCallType;
        } else {
            loge("toRilCallType unexpected calltype for VT multitask="
                    + callType);
        }
        return rilCallType;
    }

    private void createAndSendMultiTaskRequest(int rilCallType) {
        mHandler.removeMessages(EVENT_VIDEO_PAUSE_RETRY);
        Message message = mHandler.obtainMessage(EVENT_VIDEO_PAUSE_DONE);
        modifyCallInitiate(message, rilCallType, null);
    }

    private int pendingPauseStatetoRilCallType() {
        int rilCallType = CallDetails.CALL_TYPE_UNKNOWN;
        if (mPendingVTMultitask == PauseState.PAUSE) {
            rilCallType = CallDetails.CALL_TYPE_VT_NODIR;
        }
        else if (mPendingVTMultitask == PauseState.RESUME) {
            rilCallType = mPrevVideoCallType;
        }
        return rilCallType;
    }

    private boolean validateCanModifyConnectionType(Message msg, int newCallType) {
        log("validateCanModifyConnectionType newCallType=" + newCallType );

        final int currCallType = mImsCallSessionImpl.getInternalCallType();
        final boolean modifyToCurrCallType = newCallType == currCallType;

        final int index = Integer.parseInt(mImsCallSessionImpl.getCallId());
        final boolean isIndexValid = index >= 0;
        final boolean isValid = isIndexValid && !modifyToCurrCallType;

        log("validateCanModifyConnectionType currCallType= " + currCallType +
                " index=" + index + " modifyToCurrCallType=" + modifyToCurrCallType);
        if (msg == null) {return isValid;}

        if (!isValid) {
            Exception exception = modifyToCurrCallType ? null :
                    new Exception("Can't modify call.");
            msg.obj = AsyncResult.forMessage(msg, null, exception);
            msg.sendToTarget();
        }
        return isValid;
    }

    public static boolean hasError(CallModify callModify) {
        return (callModify != null && callModify.error());
    }

    // UNSOL_MODIFY_CALL
    public void onReceivedModifyCall(CallModify callModify) {
        Log.d(LOG_TAG, "onReceivedCallModify(" + callModify + ")");

        boolean ret = false;
        // Is this a new modify call request or an error notification for
        // the previous modify call request?
        if (callModify.error()) {
            if (this.callModifyRequest != null
                    && this.callModifyRequest.call_index == callModify.call_index
                    && this.callModifyRequest.call_details.call_type
                        == callModify.call_details.call_type
                    && this.callModifyRequest.call_details.call_domain
                        == callModify.call_details.call_domain
                    && !this.callModifyRequest.error()) {
                // Update the previous request.
                this.callModifyRequest.error = callModify.error;
                ret = true;
            } else {
                Log.e(LOG_TAG, "onReceivedModifyCall: Call Modify request not found."
                        + "Dropping the Modify Call Request Failed. Cached Request: "
                        + this.callModifyRequest + ", Received Request:" + callModify);
                ret = false;
            }
        } else { // This a new modify call request.
            ret = validateCanModifyConnectionType(null, callModify.call_details.call_type);
            if (this.callModifyRequest == null) {
                this.callModifyRequest = callModify;
            } else {
                Log.e(LOG_TAG, "videocall onReceivedModifyCall: not notifying user about"
                        + " incoming modify call request as there is pending callModifyRequest="
                        + callModifyRequest);
                ret = false;
            }

            if (!ret) {
                // reject/clear modify request if call cannot be modifed to request callType
                rejectConnectionTypeChange();
            }
        }

        /*
         * notify only in below cases:
         * 1. when call can be modified to the requested callType
         * 2. to diplay a toast message as a result of modify request time-out
         */
        if (ret) {
            mImsCallSessionImpl.notifyUnsolCallModify(callModify);
        }

        if (hasError(callModify)) {
            clearPendingModify();
        }
    }

    private boolean isOldAndNewPauseRequestSame() {
        boolean ret = false;
        if (callModifyRequest != null) {
            loge("isOldAndNewPauseRequestSame Unexpectedly callModifyRequest:" + callModifyRequest);
            return ret;
        }
        ret = (mIsLocallyPaused && mPendingVTMultitask == PauseState.PAUSE)
                || (!mIsLocallyPaused && mPendingVTMultitask == PauseState.RESUME);
        log("isOldAndNewPauseRequestSame " + ret);
        return ret;
    }

    private void clearPendingVTMultiTask() {
        log("clearPendingVTMultiTask imsconn=" + this);
        mPendingVTMultitask = PauseState.NONE;
    }

    private boolean shouldRetryVideoPause() {
        return (mMultiTaskRetryCount <= MAX_MULTITASK_RETRIES);
    }

    private void processPendingVTMultitask() {
        log("processPendingVTMultitask mPendingVTMultitask=" + mPendingVTMultitask);
        if (isOldAndNewPauseRequestSame()) {
            log("Old and new Pause Request is Same so clearing Pending VT multitask");
            clearPendingVTMultiTask();
        } else if (mPendingVTMultitask != PauseState.NONE) {
            if (callModifyRequest == null) {
                createAndSendMultiTaskRequest(pendingPauseStatetoRilCallType());
                clearPendingVTMultiTask();
            } else {
                loge("processPendingVTMultitask callModifyRequest not null");
            }

        }
    }

    // Clears the callModifyRequest object
    private void clearPendingModify() {
        log("clearPendingModify imsconn=" + this);
        callModifyRequest = null;
    }

    public boolean isAvpRetryAllowed() {
        return mAvpRetryAllowed;
    }

    public void update(DriverCallIms dc) {
        if (DBG) {
            log("update dc=" + dc);
        }
        int videoPauseState = CallDetails.VIDEO_PAUSE_STATE_RESUMED;
        // int callType = currentDc.callDetails.call_type;
        // CallDetails callDetails = new CallDetails(callType,
        // CallDetails.CALL_DOMAIN_AUTOMATIC,null);
        if (ImsCallUtils.isVideoCallTypeWithDir(dc.callDetails.call_type)) {
            // Entered video call with peer so do not allow AVP retry for future
            mAvpRetryAllowed = false;
        }
        if (!ImsCallUtils.isVideoCall(dc.callDetails.call_type)) {
            log("videocall: update: Not a videocall CASE");
            clearPendingVTMultiTask();
            mMultiTaskRetryCount = 0;
            mHandler.removeMessages(EVENT_VIDEO_PAUSE_RETRY);
        }
        boolean isAvpRetryDialingCase = ImsCallUtils.isAvpRetryDialing(mImsCallSessionImpl, dc);
        if (isAvpRetryDialingCase) {
            // Update AVP call type with current call type for dialed video call
            // mAvpCallType = callDetails.call_type;
            mAvpCallType = mImsCallSessionImpl.getInternalCallType();
        }
        boolean isAvpRetryUpgradeCase = ImsCallUtils.isAvpRetryUpgrade(mImsCallSessionImpl, dc);
        if (isAvpRetryDialingCase || isAvpRetryUpgradeCase) {
            log("videocall AVP RETRY CASE dc= " + dc + " conn= " + this);
            retryAvpUpgrade(mImsCallSessionImpl.getInternalState() == DriverCallIms.State.ACTIVE);
        } else if (ImsCallUtils.isVideoPaused(mImsCallSessionImpl, dc)) {
            // Display toast Video Paused
            log("videocall Video Paused CASE");
        } else if (ImsCallUtils.isVideoResumed(mImsCallSessionImpl, dc)) {
            // Display toast Video Resumed
            log("videocall Video Resumed CASE");
        }

        updatePreviousVTCallType();

        CallDetails newDetails = new CallDetails(dc.callDetails);

        /*
         * //TODO confirm and remove - not necessary for L if (!callDetails.equals(dc.callDetails))
         * { Log.d(LOG_TAG, "[Connection " + mImsCallSessionImpl.getCallId() +
         * "] Updating call Details to " + newDetails); callDetails = newDetails; // Update call
         * type to notify multitasking state to UI callDetails.setVideoPauseState(videoPauseState);
         * // If video is paused then update UI call type with previous calltype if (videoPauseState
         * == CallDetails.VIDEO_PAUSE_STATE_PAUSED) { log("Updating UI CallType from " +
         * callDetails.call_type + " to mPrevVideoCallType=" + mPrevVideoCallType);
         * callDetails.call_type = mPrevVideoCallType; } }
         */
        // Reset the mIsLocallyPaused flag if call is not active videocall
        if (!ImsCallUtils.canVideoPause(mImsCallSessionImpl)) {
            mIsLocallyPaused = false;
        }
    }

    private void notifyVideoPaused(int videoState) {
        VideoProfile vcp = new VideoProfile(videoState, VideoProfile.QUALITY_DEFAULT);
        mImsCallSessionImpl.getImsVideoCallProviderImpl().receiveSessionModifyRequest(vcp);
    }

    private void retryAvpUpgrade(boolean shouldNotifyUser) {
        log("retryAvpUpgrade: AVPF failed so retrying using AVP. mAvpCallType=" + mAvpCallType
                + " shouldNotifyUser=" + shouldNotifyUser);
        // try {
        Message msg = mHandler.obtainMessage(EVENT_AVP_UPGRADE_DONE, shouldNotifyUser);
        modifyCallInitiate(msg, mAvpCallType, null);
        // } catch (CallStateException e) {
        // loge("retryAvpUpgrade CallStateException " + e);
        // }
    }

    private void updatePreviousVTCallType() {
        int callType = mImsCallSessionImpl.getInternalCallType();
        if (callType == CallDetails.CALL_TYPE_VT
                || callType == CallDetails.CALL_TYPE_VT_TX
                || callType == CallDetails.CALL_TYPE_VT_RX) {
            mPrevVideoCallType = callType;
            log("Updating mPrevVideoCallType to " + mPrevVideoCallType);
        }
    }

    // MODIFY_CALL_INITIATE
    public void changeConnectionType(Message msg, int newCallType, Map<String, String> newExtras)
    {
        log("changeConnectionType " + " newCallType=" + newCallType + " newExtras= "
                + newExtras);
        // mImsCallProfile = mImsCallSessionImpl.getCallId().getCallProfile();
        mIndex = Integer.parseInt(mImsCallSessionImpl.getCallId());
        if (isVTMultitaskRequest(newCallType)) {
            // Video pause/resume request
            triggerOrQueueVTMultitask(newCallType);
        } else {
            // Regular upgrade/downgrade request
            if (isAvpRetryAllowed() && ImsCallUtils.isVideoCallTypeWithDir(newCallType)) {
                mAvpCallType = newCallType;
            }

            Message newMsg = mHandler.obtainMessage(EVENT_MODIFY_CALL_INITIATE_DONE, msg);
            if (callModifyRequest == null) {
                if (validateCanModifyConnectionType(newMsg, newCallType)) {
                    modifyCallInitiate(newMsg, newCallType, newExtras);
                }
            } else {
                Log.e(LOG_TAG,
                        "videocall changeConnectionType: not invoking modifyCallInitiate "
                                + "as there is pending callModifyRequest="
                                + callModifyRequest);

                String errorStr = "Pending callModifyRequest so not sending modify request down";
                RuntimeException ex = new RuntimeException(errorStr);
                if (msg != null) {
                    AsyncResult.forMessage(msg, null, ex);
                    msg.sendToTarget();
                }

            }
        }
    }

    private void modifyCallInitiate(Message newMsg, int newCallType, Map<String, String> newExtras)
    {
        if (!ImsCallUtils.isValidRilModifyCallType(newCallType)) {
            loge("modifyCallInitiate not a Valid RilCallType" + newCallType);
            return;
        }

        /*
         * CallDetails callDetails = new CallDetails(newCallType, getCallDetails().call_domain,
         * CallDetails.getExtrasFromMap(newExtras));
         */
        CallDetails callDetails = new CallDetails(newCallType, mImsCallSessionImpl.getCallDomain(),
                CallDetails.getExtrasFromMap(newExtras));
        CallModify callModify = new CallModify(callDetails, mIndex);
        // Store the outgoing modify call request in the connection
        if (callModifyRequest != null) {
            log("Overwriting callModifyRequest: " + callModifyRequest + " with callModify:"
                    + callModify);
        }
        callModifyRequest = callModify;
        mCi.modifyCallInitiate(newMsg, callModify);
    }

    private boolean isValidCallModifyConfirmRequest(int callType) {
        boolean allowed = false;
        if(callModifyRequest == null) {
            loge("callModifyRequest is null");
            return false;
        }
        //Accept with different call types is supported only for VoLTE to VT upgrade
        if (mImsCallSessionImpl.getInternalCallType() == CallDetails.CALL_TYPE_VOICE &&
                callModifyRequest.call_details.call_type == CallDetails.CALL_TYPE_VT &&
                (callType == CallDetails.CALL_TYPE_VT_RX
                        || callType == CallDetails.CALL_TYPE_VT_TX
                        || callType == CallDetails.CALL_TYPE_VT
                        || callType == CallDetails.CALL_TYPE_VOICE)) {
            allowed = true;
        } else if (callType == callModifyRequest.call_details.call_type) {
            allowed = true;
        }
        return allowed;
    }

    /**
     * Approve a request to change the call type. Optionally, provide new extra values.
     * @param newExtras
     * @throws CallStateException
     */
    public void acceptConnectionTypeChange(int callType, Map<String, String> newExtras)
    {
        Log.d(LOG_TAG, "Confirming call type change request: " + callModifyRequest);

        if (!isValidCallModifyConfirmRequest(callType)) {
            loge("acceptConnectionTypeChange: MODIFY_CALL_CONFIRM called with invalid calltype "
                    + callType);
            rejectConnectionTypeChange();
            return;
        }

        if (callModifyRequest != null) {
            callModifyRequest.call_details.call_type = callType;
            if (newExtras != null) {
                callModifyRequest.call_details.setExtrasFromMap(newExtras);
            }
            Message newMsg = mHandler.obtainMessage(EVENT_MODIFY_CALL_CONFIRM_DONE);
            mCi.modifyCallConfirm(newMsg, callModifyRequest);
        }
    }

    /**
     * Reject a previously received request to change the call type.
     * @throws CallStateException
     */
    public void rejectConnectionTypeChange() {
        if (callModifyRequest == null) {
            Log.e(LOG_TAG, "rejectConnectionTypeChange callModifyRequest is null");
            return;
        } else if (callModifyRequest.error()) {
            Log.d(LOG_TAG, "rejectConnectionTypeChange callModifyRequest timed out.");
            clearPendingModify();
            return; // Don't notify lower layers. They are aware of this.
        }
        CallDetails callDetails = new CallDetails(mImsCallSessionImpl.getInternalCallType(),
                mImsCallSessionImpl.getCallDomain(),
                null);
        CallModify callModify = new CallModify();
        callModify.call_index  = Integer.parseInt(mImsCallSessionImpl.getCallId());
        callModify.call_details = new CallDetails(callDetails);

        Log.d(LOG_TAG, "Rejecting Change request: " + callModifyRequest + " keep as " + callModify);

        Message newMsg = mHandler.obtainMessage(EVENT_MODIFY_CALL_CONFIRM_DONE);
        mCi.modifyCallConfirm(newMsg, callModify);
    }

    class ImsCallModificationHandler extends Handler {
        public void handleMessage(Message msg) {
            AsyncResult ar;
            switch (msg.what) {
                case EVENT_MODIFY_CALL_INITIATE_DONE:
                    onModifyCallInitiateDone(msg);
                    break;
                case EVENT_VIDEO_PAUSE_DONE:
                    onVideoPauseDone(msg);
                    break;
                case EVENT_MODIFY_CALL_CONFIRM_DONE:
                    log("EVENT_MODIFY_CALL_CONFIRM_DONE received");
                    clearPendingModify();
                    processPendingVTMultitask();
                    break;
                case EVENT_VIDEO_PAUSE_RETRY:
                    onVideoPauseRetry();
                    break;
                case EVENT_AVP_UPGRADE_DONE:
                    onAvpRetry(msg);
                    break;
            }
        }

        private void onModifyCallInitiateDone(Message msg) {
            log("EVENT_MODIFY_CALL_INITIATE_DONE received");
            AsyncResult ar = (AsyncResult) msg.obj;
            if (ar != null && ar.exception != null) {
                loge("videocall error during modifyCallInitiate");
                // Context needs to come from ImsServiceSub to ImsCallSessionImpl
                // Toast.makeText(mContext, MODIFY_CALL_FAILED_MESSAGE, Toast.LENGTH_SHORT).show();
            }
            Message onComplete = (Message) ar.userObj;
            if (onComplete != null) {
                AsyncResult.forMessage(onComplete, ar.result, ar.exception);
                onComplete.sendToTarget();
            }
            clearPendingModify();
            processPendingVTMultitask();
        }

        private void onVideoPauseDone(Message msg) {
            AsyncResult ar;
            log("EVENT_VIDEO_PAUSE_DONE received");
            ar = (AsyncResult) msg.obj;
            if (ar != null) {
                if (ar.exception == null) {
                    // Success means VT Pause or Resume request succeeded.
                    // Update locally paused flag
                    if (callModifyRequest != null) {
                        mIsLocallyPaused = callModifyRequest.call_details.call_type
                                == CallDetails.CALL_TYPE_VT_NODIR;
                    }
                    clearPendingModify();
                    clearMultiTaskRetryCount();
                    processPendingVTMultitask();

                } else {
                    // Pause request failed, retry MAX_MULTITASK_RETRIES times
                    if (shouldRetryVideoPause()) {
                        loge("Error during video pause so retry");
                        mHandler.sendMessageDelayed(mHandler.obtainMessage(EVENT_VIDEO_PAUSE_RETRY),
                                PAUSE_DELAY_MILLIS);
                        mMultiTaskRetryCount++;
                    } else {
                        log("Video Pause retry limit reached.");
                        clearMultiTaskRetryCount();
                        clearPendingModify();
                        processPendingVTMultitask();
                    }
                }
            } else {
                loge("Error EVENT_VIDEO_PAUSE_DONE ar is null");
            }
        }

        private void onVideoPauseRetry() {
            log("EVENT_VIDEO_PAUSE_RETRY received mMultiTaskRetryCount=" + mMultiTaskRetryCount);
            if (mPendingVTMultitask == PauseState.NONE) {
                createAndSendMultiTaskRequest(callModifyRequest.call_details.call_type);
            } else {
                log("User pressed home/resume during retry so sending out new multitask request");
                clearPendingModify();
                clearMultiTaskRetryCount();
                processPendingVTMultitask();
            }
        }

        private void onAvpRetry(Message msg) {
            log("EVENT_AVP_UPGRADE received");
            AsyncResult ar = (AsyncResult) msg.obj;
            if (ar != null && ar.exception != null) {
                if (ar.userObj instanceof Boolean) {
                    boolean shouldNotifyUser = (Boolean) ar.userObj;
                    if (shouldNotifyUser) {
                        loge("AVP Retry error when Voice call was upgraded to video call");
                        // owner.phone.notifyAvpUpgradeFailure(((AsyncResult)
                        // msg.obj).exception.toString());

                        // Context can be obtained by changing ImsCallSessionImpl constructor
                        // Toast.makeText(mContext, MODIFY_CALL_FAILED_MESSAGE,
                        // Toast.LENGTH_SHORT).show();
                    } else {
                        loge("AVP Retry error when Video call was dialed");
                    }
                }
            }
            clearPendingModify();
            processPendingVTMultitask();
        }

        private int clearMultiTaskRetryCount() {
            log("Clearing MultiTaskRetryCount from " + mMultiTaskRetryCount + " to 0");
            return mMultiTaskRetryCount = 0;
        }
    }

    private void log(String msg) {
        Log.d(LOG_TAG, "" + msg);
    }

    private void loge(String msg) {
        Log.e(LOG_TAG, "" + msg);
    }
}
