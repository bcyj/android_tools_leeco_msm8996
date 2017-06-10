/*
 * ©2012-2014 Borqs Software Solutions Pvt Ltd. All rights reserved.
 *
 * This file is originated from Android Open Source Project,
 * platform/packages/services/Telephony.git/src/com/android/phone/
 * CallWaitingCheckBoxPreference.java
 */


package com.borqs.videocall;

import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.Phone;

import static com.borqs.videocall.TimeConsumingPreferenceActivity.RESPONSE_ERROR;

import android.content.Context;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.preference.CheckBoxPreference;
import android.util.AttributeSet;
import android.util.Log;

import com.android.internal.telephony.Phone;

public class VideoCallWaitingCheckBoxPreference extends CheckBoxPreference {
	private static final String LOG_TAG = "VideoCallWaitingCheckBoxPreference";
	private final boolean DBG = true;  //(PhoneApp.DBG_LEVEL >= 2);
	private boolean reading = false;

	private final Handler mHandler = new Handler()
	{
		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case IVTConnection.MESSAGE_GET_CALL_WAITING:
				handleGetVideoCallWaitingResponse(msg);
				break;

			}
		}
		private void handleGetVideoCallWaitingResponse(Message msg) {
//			AsyncResult ar = (AsyncResult) msg.obj;

			Boolean enabled = (Boolean) msg.obj;
			Log.d(LOG_TAG, "handleGetVideoCallWaitingResponse:" + msg);

			if (mTcpListener != null) {
					mTcpListener.onFinished(VideoCallWaitingCheckBoxPreference.this, reading);
			}

			setChecked(enabled);

//			if (ar.exception != null) {
//				if (DBG) {
//					Log.d(LOG_TAG, "handleGetVideoCallWaitingResponse: ar.exception=" + ar.exception);
//				}
//				if (mTcpListener != null) {
//					mTcpListener.onException(VideoCallWaitingCheckBoxPreference.this,
//							(CommandException)ar.exception);
//				}
//			} else if (ar.userObj instanceof Throwable) {
//				if (mTcpListener != null) {
//					mTcpListener.onError(VideoCallWaitingCheckBoxPreference.this, RESPONSE_ERROR);
//				}
//			} else {
//				if (DBG) {
//					Log.d(LOG_TAG, "handleGetVideoCallWaitingResponse: CW state successfully queried.");
//				}
//				int[] cwArray = (int[])ar.result;
//				// If cwArray[0] is = 1, then cwArray[1] must follow,
//				// with the TS 27.007 service class bit vector of services
//				// for which call waiting is enabled.
//				try {
//					Log.d(LOG_TAG, "handleGetVideoCallWaitingResponse: cwArray" + cwArray[1]);
//					setChecked(((cwArray[0] == 1) && ((cwArray[1] & 0x10) == 0x10)));
//				} catch (ArrayIndexOutOfBoundsException e) {
//					Log.e(LOG_TAG, "handleGetVideoCallWaitingResponse: improper result: err ="
//							+ e.getMessage());
//				}
//			}
		}
	};
	private Phone mPhone;
	private TimeConsumingPreferenceListener mTcpListener;
	private VTPhoneConnection  mvtPhoneConnection;

	public VideoCallWaitingCheckBoxPreference(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
		mvtPhoneConnection = new VTPhoneConnection(context, VTCallUtils.getCsvtService());//VideoCallApp.getInstance().getVTConnection();
		mvtPhoneConnection.setVideoCallWaitingHandler(mHandler);
	}

	public VideoCallWaitingCheckBoxPreference(Context context, AttributeSet attrs) {
		this(context, attrs, com.android.internal.R.attr.checkBoxPreferenceStyle);
	}

	public VideoCallWaitingCheckBoxPreference(Context context) {
		this(context, null);
	}

	/*package*/ void init(TimeConsumingPreferenceListener listener,
			boolean skipReading) {
		// Get the selected subscription


		//  mPhone = PhoneApp.getPhone(subscription);

		mTcpListener = listener;

		if (!skipReading) {
			mvtPhoneConnection.getCallWaiting(mHandler.obtainMessage(IVTConnection.MESSAGE_GET_CALL_WAITING,
					IVTConnection.MESSAGE_GET_CALL_WAITING, IVTConnection.MESSAGE_GET_CALL_WAITING));
			reading = true;
			if (mTcpListener != null) {
				mTcpListener.onStarted(this, reading);
			}
		}
	}

	@Override
	protected void onClick() {
		super.onClick();
		Log.d(LOG_TAG, "onClick" + isChecked());
		mvtPhoneConnection.setCallWaiting(isChecked(),
				mHandler.obtainMessage(IVTConnection.MESSAGE_GET_CALL_WAITING));
		mvtPhoneConnection.getCallWaiting(mHandler.obtainMessage(IVTConnection.MESSAGE_GET_CALL_WAITING));
		reading = true;
		if (mTcpListener != null) {
			mTcpListener.onStarted(this, reading);
		}
	}

	/*private class MyHandler extends Handler {
		static final int MESSAGE_GET_CALL_WAITING = 0;
		static final int MESSAGE_SET_CALL_WAITING = 1;

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case MESSAGE_GET_CALL_WAITING:
				handleGetVideoCallWaitingResponse(msg);
				break;
			case MESSAGE_SET_CALL_WAITING:
				handleSetVideoCallWaitingResponse(msg);
				break;
			}
		}

		private void handleGetVideoCallWaitingResponse(Message msg) {
			AsyncResult ar = (AsyncResult) msg.obj;
			Log.d(LOG_TAG, "handleGetVideoCallWaitingResponse:" + msg);

			if (mTcpListener != null) {
				if (msg.arg2 == MESSAGE_SET_CALL_WAITING) {
					mTcpListener.onFinished(VideoCallWaitingCheckBoxPreference.this, false);
				} else {
					mTcpListener.onFinished(VideoCallWaitingCheckBoxPreference.this, true);
				}
			}

			if (ar.exception != null) {
				if (DBG) {
					Log.d(LOG_TAG, "handleGetVideoCallWaitingResponse: ar.exception=" + ar.exception);
				}
				if (mTcpListener != null) {
					mTcpListener.onException(VideoCallWaitingCheckBoxPreference.this,
							(CommandException)ar.exception);
				}
			} else if (ar.userObj instanceof Throwable) {
				if (mTcpListener != null) {
					mTcpListener.onError(VideoCallWaitingCheckBoxPreference.this, RESPONSE_ERROR);
				}
			} else {
				if (DBG) {
					Log.d(LOG_TAG, "handleGetVideoCallWaitingResponse: CW state successfully queried.");
				}
				int[] cwArray = (int[])ar.result;
				// If cwArray[0] is = 1, then cwArray[1] must follow,
				// with the TS 27.007 service class bit vector of services
				// for which call waiting is enabled.
				try {
					Log.d(LOG_TAG, "handleGetVideoCallWaitingResponse: cwArray" + cwArray[1]);
					setChecked(((cwArray[0] == 1) && ((cwArray[1] & 0x10) == 0x10)));
				} catch (ArrayIndexOutOfBoundsException e) {
					Log.e(LOG_TAG, "handleGetVideoCallWaitingResponse: improper result: err ="
							+ e.getMessage());
				}
			}
		}

		private void handleSetVideoCallWaitingResponse(Message msg) {
			AsyncResult ar = (AsyncResult) msg.obj;

			if (ar.exception != null) {
				if (DBG) {
					Log.d(LOG_TAG, "handleSetVideoCallWaitingResponse: ar.exception=" + ar.exception);
				}
				//setEnabled(false);
			}
			if (DBG) Log.d(LOG_TAG, "handleSetVideoCallWaitingResponse: re get");

			mvtPhoneConnection.getCallWaiting(obtainMessage(MESSAGE_GET_CALL_WAITING,
					MESSAGE_SET_CALL_WAITING, MESSAGE_SET_CALL_WAITING, ar.exception));
		}
	}*/


}
