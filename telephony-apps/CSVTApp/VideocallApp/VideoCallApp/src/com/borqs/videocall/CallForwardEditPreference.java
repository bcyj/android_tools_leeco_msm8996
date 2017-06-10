/*
* Copyright (C) 2010 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

package com.borqs.videocall;

import java.util.List;

import com.android.internal.telephony.CallForwardInfo;
import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.Phone;
import org.codeaurora.ims.csvt.CallForwardInfoP;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.res.TypedArray;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.os.Messenger;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;

import static com.borqs.videocall.TimeConsumingPreferenceActivity.EXCEPTION_ERROR;
import static com.borqs.videocall.TimeConsumingPreferenceActivity.RESPONSE_ERROR;

public class CallForwardEditPreference extends EditPhoneNumberPreference {
    private static final String LOG_TAG = "CallForwardEditPreference";
    private static final boolean DBG =true;
    private boolean reading = true;

    private static final String SRC_TAGS[]       = {"{0}"};
    private CharSequence mSummaryOnTemplate;
    /**
     * Remembers which button was clicked by a user. If no button is clicked yet, this should have
     * {@link DialogInterface#BUTTON_NEGATIVE}, meaning "cancel".
     *
     * TODO: consider removing this variable and having getButtonClicked() in
     * EditPhoneNumberPreference instead.
     */
    private int mButtonClicked;
    private int mServiceClass;
    private boolean mAlreadyQuery = false;
    private int mSubscription = 0;
    private boolean mFirsTimetException = true;
    int reason;
    Phone phone;
    CallForwardInfo callForwardInfo;
    TimeConsumingPreferenceListener tcpListener;
    private CharSequence mSummaryText = null;
    private VTPhoneConnection  mvtPhoneConnection;
    private int action = CommandsInterface.CF_ACTION_DISABLE;
    private Messenger mMessenger = null;
    private Handler mHandler = new Handler(){

	@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case IVTConnection.MESSAGE_GET_CF:
				handleGetCFResponse(msg);
				break;
			case IVTConnection.MESSAGE_SET_CF:
				handleSetCFResponse(msg);
			}
		}

        private void handleSetCFResponse(Message msg) {
//            AsyncResult ar = (AsyncResult) msg.obj;
//            if (ar.exception != null) {
//            if (DBG) Log.d(LOG_TAG, "handleSetCFResponse: ar.exception=" + ar.exception);
                // setEnabled(false);
//            }

            if (DBG) Log.d(LOG_TAG, "handleSetCFResponse: re get");
            msg = mHandler.obtainMessage(IVTConnection.MESSAGE_GET_CF);
            msg.replyTo = mMessenger;
            mvtPhoneConnection.getCallForwardingOption(reason, msg);
        }

	private void handleGetCFResponse(Message msg) {
            if (DBG) Log.d(LOG_TAG, "handleGetCFResponse: done");



            //when set CF happen exception,do not retry
//            if (msg.arg2 == MESSAGE_SET_CF) {
//                tcpListener.onFinished(CallForwardEditPreference.this, false);
//            } else {
//                if (ar.exception != null && mFirsTimetException) {
//                    CommandException e = (CommandException) ar.exception;
//                    mFirsTimetException = false;
//
//                    Log.d(LOG_TAG, "e:" + e.getCommandError());
//
//                    if (e.getCommandError() == CommandException.Error.GENERIC_FAILURE) {
//                        sendEmptyMessageDelayed(MESSAGE_RETRY_GET_CF, RETRY_GET_CF_DELAY);
//                        return;
//                    }
//                }
//                tcpListener.onFinished(CallForwardEditPreference.this, true);
//            }

//            tcpListener.onFinished(CallForwardEditPreference.this, reading);
            callForwardInfo = null;

				List<CallForwardInfoP> cfInfoP = (List<CallForwardInfoP>) msg.obj;

                // Handle the response of sendRequestStatus from CsvtService.
                if (msg.arg1 == 1 /* CsvtConstants.ERROR_FAILED */) {
                    tcpListener.onFinished(CallForwardEditPreference.this, reading);
                    tcpListener.onError(CallForwardEditPreference.this, EXCEPTION_ERROR);
                    return;
                }  else { /* CsvtConstants.ERROR_SUCCESS */
                    if (cfInfoP == null) {
                        // we will receive the response of notifyCallForwardingOptions later,
                        // so ignore it here.
                        return;
                    }
                }

                if (cfInfoP.isEmpty()) {
                    if (DBG) Log.d(LOG_TAG, "handleGetCFResponse: cfInfoArray.length==0");
                    setEnabled(false);
                    tcpListener.onFinished(CallForwardEditPreference.this, reading);
                    tcpListener.onError(CallForwardEditPreference.this, RESPONSE_ERROR);
                } else {
                    for (int i = 0, length = cfInfoP.size(); i < length; i++) {
					if (DBG)
						Log.d(LOG_TAG, "handleGetCFResponse, cfInfoArray[" + i
								+ "]=" + cfInfoP.get(i));

					if (cfInfoP.get(i).reason == reason) {

						tcpListener.onFinished(CallForwardEditPreference.this, reading);
						// corresponding class
						CallForwardInfo info = new CallForwardInfo();
						info.number = cfInfoP.get(i).number;
						info.reason = cfInfoP.get(i).reason;
						info.status = cfInfoP.get(i).status;
						info.timeSeconds = cfInfoP.get(i).timeSeconds;
						info.toa = cfInfoP.get(i).toa;
						info.serviceClass = mServiceClass;

						handleCallForwardResult(info);

						// Show an alert if we got a success response but
						// with unexpected values.
						// Currently only handle the fail-to-disable case
						// since we haven't observed fail-to-enable.
						if (!reading
								&& (action == CommandsInterface.CF_ACTION_DISABLE)
								&& (info.status == 1)) {
							CharSequence s;
							if (DBG)
								Log.d(LOG_TAG,
										"handleGetCFResponse, fail to disable");
							switch (info.reason) {
							case CommandsInterface.CF_REASON_BUSY:
								s = getContext().getText(
										R.string.disable_cfb_forbidden);
								break;
							case CommandsInterface.CF_REASON_NO_REPLY:
								s = getContext().getText(
										R.string.disable_cfnry_forbidden);
								break;
							default: // not reachable
								s = getContext().getText(
										R.string.disable_cfnrc_forbidden);
							}
							AlertDialog.Builder builder = new AlertDialog.Builder(
									getContext());
							builder.setNeutralButton(R.string.close_dialog,
									null);
							builder.setTitle(getContext().getText(
									R.string.error_updating_title));
							builder.setMessage(s);
							builder.setCancelable(true);
							builder.create().show();
						}

					}
					// if all is normal, update summary
					updateSummaryText();
				}
			}

            // Now whether or not we got a new number, reset our enabled
            // summary text since it may have been replaced by an empty
            // placeholder.
            //if exception happen,do not update summary here
            //updateSummaryText();
        }
    };

    public CallForwardEditPreference(Context context, AttributeSet attrs) {
        super(context, attrs);

        mSummaryOnTemplate = this.getSummaryOn();

        TypedArray a = context.obtainStyledAttributes(attrs,
                R.styleable.CallForwardEditPreference, 0, R.style.EditPhoneNumberPreference);
        mServiceClass = CommandsInterface.SERVICE_CLASS_DATA_SYNC;
        reason = a.getInt(R.styleable.CallForwardEditPreference_reason,
                CommandsInterface.CF_REASON_UNCONDITIONAL);
        a.recycle();
        mvtPhoneConnection = new VTPhoneConnection(context, VTCallUtils.getCsvtService());
        mMessenger = new Messenger(mHandler);
		mvtPhoneConnection.setVideoCallForwardingHandler(mHandler);
        if (DBG) Log.d(LOG_TAG, "mServiceClass=" + mServiceClass + ", reason=" + reason);
    }

    public CallForwardEditPreference(Context context) {
        this(context, null);
    }

    public void setParameter(TimeConsumingPreferenceListener listener, int subscription) {
        tcpListener = listener;
        mSubscription = subscription;
    }

    void init(boolean skipReading) {
        init(skipReading, false);
    }

    void init(boolean skipReading, boolean onlyGetCF) {

        // getting selected subscription
        if (DBG) Log.d(LOG_TAG, "Getting CallForwardEditPreference subscription =" + mSubscription);
      //  phone = PhoneApp.getPhone(mSubscription);

        if (!skipReading) {
            Message msg = mHandler.obtainMessage(IVTConnection.MESSAGE_GET_CF,
                    // unused in this case
                    CommandsInterface.CF_ACTION_DISABLE,
                    IVTConnection.MESSAGE_GET_CF, null);
            msg.replyTo = mMessenger;
            mvtPhoneConnection.getCallForwardingOption(reason, msg);
            if (tcpListener != null && !onlyGetCF) {
		reading = true;
                tcpListener.onStarted(this, reading);
            }
        }
    }

//    void init(TimeConsumingPreferenceListener listener, boolean skipReading) {
//        tcpListener = listener;
//       // phone = PhoneApp.getInstance().getPhone(mSubscription);
//        if (!skipReading) {
//        	mvtPhoneConnection.getCallForwardingOption(reason,
//                    mHandler.obtainMessage(MyHandler.MESSAGE_GET_CF,
//                           // unused in this case
//                           CommandsInterface.CF_ACTION_DISABLE,
//                           MyHandler.MESSAGE_GET_CF, null));
//            if (tcpListener != null) {
//                tcpListener.onStarted(this, true);
//            }
//        }
//    }

    @Override
    protected void onClick() {
        if (mAlreadyQuery == false) {
            // query state
            init(false);
            mAlreadyQuery = true;
        } else {
            super.onClick();
        }
    }

    @Override
    protected void onBindDialogView(View view) {
        // default the button clicked to be the cancel button.
        mButtonClicked = DialogInterface.BUTTON_NEGATIVE;
        super.onBindDialogView(view);
    }

    @Override
    public void onClick(DialogInterface dialog, int which) {
        super.onClick(dialog, which);
        mButtonClicked = which;
    }

    @Override
    protected void onDialogClosed(boolean positiveResult) {
        super.onDialogClosed(positiveResult);

        if (DBG) Log.d(LOG_TAG, "mButtonClicked=" + mButtonClicked
                + ", positiveResult=" + positiveResult);
        // Ignore this event if the user clicked the cancel button, or if the dialog is dismissed
        // without any button being pressed (back button press or click event outside the dialog).
        if (this.mButtonClicked != DialogInterface.BUTTON_NEGATIVE) {
            action = (isToggled() || (mButtonClicked == DialogInterface.BUTTON_POSITIVE)) ?
                    CommandsInterface.CF_ACTION_REGISTRATION :
                    CommandsInterface.CF_ACTION_DISABLE;
            int time = (reason != CommandsInterface.CF_REASON_NO_REPLY) ? 0 : 20;
            final String number = getPhoneNumber();

            if (DBG) Log.d(LOG_TAG, "callForwardInfo=" + callForwardInfo);

            if (action == CommandsInterface.CF_ACTION_REGISTRATION
                    && callForwardInfo != null
                    && callForwardInfo.status == 1
                    && number.equals(callForwardInfo.number)) {
                // no change, do nothing
                if (DBG) Log.d(LOG_TAG, "no change, do nothing");
            } else {
                // set to network
				Log.d(LOG_TAG, "reason=" + reason + ", action=" + action
						+ ", number=" + number + "mServiceClass="
						+ mServiceClass);

                // Display no forwarding number while we're waiting for
                // confirmation
                setSummaryOn("");

                // the interface of Phone.setCallForwardingOption has error:
                // should be action, reason...
                Log.d(LOG_TAG, "setVideoCallForwardingOption..");
                Message msg= mHandler.obtainMessage(IVTConnection.MESSAGE_SET_CF,
                        action,
                        IVTConnection.MESSAGE_SET_CF);
                msg.replyTo = mMessenger;
                mvtPhoneConnection.setCallForwardingOption(reason,
                        action,
                        number,
                        time,
                        msg);
                 //else {
//                	Log.d(LOG_TAG, "setCallForwardingOption..");
//                phone.setCallForwardingOption(action,
//                        reason,
//                        number,
//                        time,
//                        mHandler.obtainMessage(MyHandler.MESSAGE_SET_CF,
//                                action,
//                                MyHandler.MESSAGE_SET_CF));
//                }

//                msg = mHandler.obtainMessage(IVTConnection.MESSAGE_GET_CF);
//                msg.replyTo = mMessenger;
//                mvtPhoneConnection.getCallForwardingOption(reason, msg);

                if (tcpListener != null) {
			reading = true;
                    tcpListener.onStarted(this, reading);
                }
            }
        }
    }

    void handleCallForwardResult(CallForwardInfo cf) {
        callForwardInfo = cf;
        if (DBG) Log.d(LOG_TAG, "handleGetCFResponse done, callForwardInfo=" + callForwardInfo);

        setToggled(callForwardInfo.status == 1);
        setPhoneNumber(callForwardInfo.number);

        if (callForwardInfo.status == 0) {
            setSummaryOff(R.string.sum_cfb_disabled);
            mSummaryText = getContext().getString(R.string.sum_cfb_disabled);
        }
    }

    private void updateSummaryText() {
        if (isToggled()) {
            CharSequence summaryOn;
            final String number = getRawPhoneNumber();
            if (number != null && number.length() > 0) {
                String values[] = { number };
                summaryOn = TextUtils.replace(mSummaryOnTemplate, SRC_TAGS, values);
            } else {
                summaryOn = getContext().getString(R.string.sum_cfu_enabled_no_number);
            }
            mSummaryText = summaryOn;
            setSummaryOn(summaryOn);
        }

    }

    // Message protocol:
    // what: get vs. set
    // arg1: action -- register vs. disable
    // arg2: get vs. set for the preceding request
    /*private class MyHandler extends Handler {
        static final int MESSAGE_GET_CF = 0;
        static final int MESSAGE_SET_CF = 1;
        private static final int MESSAGE_RETRY_GET_CF = 2;

        private static final int RETRY_GET_CF_DELAY = 2500; // delay one second

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MESSAGE_GET_CF:
                    handleGetCFResponse(msg);
                    break;
                case MESSAGE_SET_CF:
                    handleSetCFResponse(msg);
                    break;
                case MESSAGE_RETRY_GET_CF:
                    handleRetryGetCF();
                    break;
            }
        }

        private void handleRetryGetCF() {
            Log.d(LOG_TAG, "handleRetryGetCF");
            init(false, true);
        }

        private void handleGetCFResponse(Message msg) {
            if (DBG) Log.d(LOG_TAG, "handleGetCFResponse: done");

            AsyncResult ar = (AsyncResult) msg.obj;

            //when set CF happen exception,do not retry
            if (msg.arg2 == MESSAGE_SET_CF) {
                tcpListener.onFinished(CallForwardEditPreference.this, false);
            } else {
                if (ar.exception != null && mFirsTimetException) {
                    CommandException e = (CommandException) ar.exception;
                    mFirsTimetException = false;

                    Log.d(LOG_TAG, "e:" + e.getCommandError());

                    if (e.getCommandError() == CommandException.Error.GENERIC_FAILURE) {
                        sendEmptyMessageDelayed(MESSAGE_RETRY_GET_CF, RETRY_GET_CF_DELAY);
                        return;
                    }
                }
                tcpListener.onFinished(CallForwardEditPreference.this, true);
            }

            callForwardInfo = null;
            if (ar.exception != null) {
                if (DBG) Log.d(LOG_TAG, "handleGetCFResponse: ar.exception=" + ar.exception);
                tcpListener.onException(CallForwardEditPreference.this,
                        (CommandException) ar.exception);
                //if exception happend, we set current summary to last summary
                setSummaryOn(mSummaryText);
            } else {
                if (ar.userObj instanceof Throwable) {
                    tcpListener.onError(CallForwardEditPreference.this, RESPONSE_ERROR);
                }
                CallForwardInfo cfInfoArray[] = (CallForwardInfo[]) ar.result;
                if (cfInfoArray.length == 0) {
                    if (DBG) Log.d(LOG_TAG, "handleGetCFResponse: cfInfoArray.length==0");
                    setEnabled(false);
                    tcpListener.onError(CallForwardEditPreference.this, RESPONSE_ERROR);
                } else {
                    for (int i = 0, length = cfInfoArray.length; i < length; i++) {
                        if (DBG) Log.d(LOG_TAG, "handleGetCFResponse, cfInfoArray[" + i + "]="
                                + cfInfoArray[i]);
                        if ((mServiceClass & cfInfoArray[i].serviceClass) != 0) {
                            // corresponding class
                            CallForwardInfo info = cfInfoArray[i];
                            handleCallForwardResult(info);

                            // Show an alert if we got a success response but
                            // with unexpected values.
                            // Currently only handle the fail-to-disable case
                            // since we haven't observed fail-to-enable.
                            if (msg.arg2 == MESSAGE_SET_CF &&
                                    msg.arg1 == CommandsInterface.CF_ACTION_DISABLE &&
                                    info.status == 1) {
                                CharSequence s;
                                switch (reason) {
                                    case CommandsInterface.CF_REASON_BUSY:
                                        s = getContext().getText(R.string.disable_cfb_forbidden);
                                        break;
                                    case CommandsInterface.CF_REASON_NO_REPLY:
                                        s = getContext().getText(R.string.disable_cfnry_forbidden);
                                        break;
                                    default: // not reachable
                                        s = getContext().getText(R.string.disable_cfnrc_forbidden);
                                }
                                AlertDialog.Builder builder = new AlertDialog.Builder(getContext());
                                builder.setNeutralButton(R.string.close_dialog, null);
                                builder.setTitle(getContext().getText(R.string.error_updating_title));
                                builder.setMessage(s);
                                builder.setCancelable(true);
                                builder.create().show();
                            }
                        }
                    }
                    //if all is normal, update summary
                    updateSummaryText();
                }
            }

            // Now whether or not we got a new number, reset our enabled
            // summary text since it may have been replaced by an empty
            // placeholder.
            //if exception happen,do not update summary here
            //updateSummaryText();
        }

        private void handleSetCFResponse(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;

            if (ar.exception != null) {
                if (DBG) Log.d(LOG_TAG, "handleSetCFResponse: ar.exception=" + ar.exception);
                // setEnabled(false);
            }
            if (DBG) Log.d(LOG_TAG, "handleSetCFResponse: re get");
            mvtPhoneConnection.getCallForwardingOption(reason,
                    obtainMessage(MESSAGE_GET_CF, msg.arg1, MESSAGE_SET_CF, ar.exception));
        }
    }*/
}
