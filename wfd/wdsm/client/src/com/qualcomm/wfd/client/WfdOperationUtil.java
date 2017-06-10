/*
 * Copyright (c) 2012 - 2014 QUALCOMM Technologies, Inc.  All Rights Reserved.
 * QUALCOMM Technologies Proprietary and Confidential.
*/

package com.qualcomm.wfd.client;

import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.util.Log;
import android.preference.PreferenceManager;
import android.content.SharedPreferences;

import com.qualcomm.wfd.WfdEnums;
import com.qualcomm.wfd.service.IWfdActionListener;

public class WfdOperationUtil {

    public static final String TAG = "Client.WfdOperationUtil";
    public static ProgressDialog wfdOperationProgressDialog = null;
    public static WfdMode wfdMode = WfdMode.INVALID;
    public static final int PLAY_CALLBACK = 0;
    public static final int PAUSE_CALLBACK = 1;
    public static final int STANDBY_CALLBACK = 2;
    public static final int UIBC_ACTION_COMPLETED = 3;
    public static final int TEARDOWN_CALLBACK = 4;
    public static final int INVALID_WFD_DEVICE = 5;
    public static final int SET_WFD_FINISHED = 6;
    public static final int SERVICE_BOUND = 7;
    public static final int START_SESSION_ON_UI = 8;
    public static final int CLEAR_UI = 10;
    public static final int UIBC_ENABLED = 1;
    public static final int UIBC_DISABLED = 0;
    public static final int AUDIO_ONLY_NOTIFICATION = 11;

    public static enum WfdOperation {
        PLAY, PAUSE, STANDBY, RESUME, START_UIBC, STOP_UIBC, SELECT_TCP, SELECT_UDP, FLUSH, SET_DECODER_LATENCY, TEARDOWN
    }

    public enum WfdMode {
        PLAYING, PAUSED, STANDBY, INVALID
    }

    public static void startWfdOperationTask (final Context inContext, final WfdOperation inWfdOperation) {
        if (inWfdOperation == WfdOperation.PLAY) {
            if (wfdMode == WfdMode.PAUSED) {
                Log.d(TAG, "startWfdOperationTask: Play button clicked in wfdMode PAUSED");
                WfdOperationTask task = new WfdOperationTask(inContext, "Playing WFD Session", WfdOperation.PLAY);
                task.execute();
            } else if (wfdMode == WfdMode.STANDBY) {
                Log.d(TAG, "startWfdOperationTask: Play button clicked in wfdMode STANDBY");
                WfdOperationTask task = new WfdOperationTask(inContext, "Playing WFD Session", WfdOperation.RESUME);
                task.execute();
            } else {
                Log.d(TAG, "startWfdOperationTask: Play button clicked in an incorrect wfdMode- " + wfdMode.ordinal());
            }
        } else if (inWfdOperation == WfdOperation.PAUSE) {
            if (wfdMode == WfdMode.PLAYING) {
                Log.d(TAG, "startWfdOperationTask: Pause button clicked in wfdMode PLAYING");
                WfdOperationTask task = new WfdOperationTask(inContext, "Pausing WFD Session", WfdOperation.PAUSE);
                task.execute();
            } else if (wfdMode == WfdMode.STANDBY) {
                Log.d(TAG, "startWfdOperationTask: Pause button clicked in wfdMode STANDBY");
                AlertDialog.Builder builder = new AlertDialog.Builder(inContext);
                builder.setTitle("Cannot perform pause when in the standby state")
                       .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int id) {
                                Log.d(TAG, "startWfdOperationTask: Can't pause in standby- User clicked OK");
                                return;
                    }
                });
                AlertDialog alert = builder.create();
                alert.show();
            } else {
                Log.d(TAG, "startWfdOperationTask: Pause button clicked in an incorrect wfdMode- " + wfdMode.ordinal());
            }
        } else if (inWfdOperation == WfdOperation.STANDBY) {
            if (wfdMode == WfdMode.PLAYING || wfdMode == WfdMode.PAUSED ) {
               if (wfdMode == WfdMode.PLAYING) {
                  Log.d(TAG, "startWfdOperationTask: Standby button clicked in wfdMode PLAYING");
		} else {
		  Log.d(TAG, "startWfdOperationTask: Standby button clicked in wfdMode PAUSED");
	        }
                WfdOperationTask task = new WfdOperationTask(inContext, "Moving WFD Session to Standby", WfdOperation.STANDBY);
                task.execute();
            } else {
                Log.d(TAG, "startWfdOperationTask: Standby button clicked in an incorrect wfdMode- " + wfdMode.ordinal());
            }
        } else if (inWfdOperation == WfdOperation.RESUME) {
            if (wfdMode == WfdMode.STANDBY) {
                Log.d(TAG, "Resume button clicked in wfdMode STANDBY");
                WfdOperationTask task = new WfdOperationTask(inContext, "Playing WFD Session", WfdOperation.RESUME);
                task.execute();
            } else {
                Log.d(TAG, "startWfdOperationTask: Resume button clicked in an incorrect wfdMode- " + wfdMode.ordinal());
            }
        } else if (inWfdOperation == WfdOperation.START_UIBC || inWfdOperation == WfdOperation.STOP_UIBC) {
            boolean proceedUIBC = true;
            if (wfdMode != WfdMode.PLAYING) {
                AlertDialog.Builder builder = new AlertDialog.Builder(inContext, AlertDialog.THEME_HOLO_DARK);
                builder.setTitle("UIBC requested in invalid WFD state")
                       .setMessage("Do you really want to proceed with UIBC?")
                       .setPositiveButton("Yes", new DialogInterface.OnClickListener() {
                           public void onClick(DialogInterface dialog, int id) {
                               Log.d(TAG, "startWfdOperationTask: UIBC dialog user clicked Yes");
                               WfdOperationTask task = new WfdOperationTask(inContext, (inWfdOperation == WfdOperation.START_UIBC)
                                        ?"Starting UIBC":"Stopping UIBC", inWfdOperation);
                               task.execute();
                           }
                       })
                       .setNegativeButton("No", new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            Log.d(TAG, "startWfdOperationTask: UIBC dialog user clicked No" );
                            return;
                        }
                    });
                AlertDialog alert = builder.create();
                alert.show();
            } else {
                WfdOperationTask task = new WfdOperationTask(inContext, (inWfdOperation == WfdOperation.START_UIBC)
                        ?"Starting UIBC":"Stopping UIBC", inWfdOperation);
                task.execute();
            }
        } else if (inWfdOperation == WfdOperation.SELECT_TCP || inWfdOperation == WfdOperation.SELECT_UDP) {
            if (wfdMode != WfdMode.PLAYING) {
                AlertDialog.Builder builder = new AlertDialog.Builder(inContext, AlertDialog.THEME_HOLO_DARK);
                builder.setTitle("Invalid WFD state for transport change")
                       .setMessage("Transport Change is allowed only while Playing. OK?")
                       .setPositiveButton("Yes", new DialogInterface.OnClickListener() {
                           public void onClick(DialogInterface dialog, int id) {
                               Log.d(TAG, "startWfdOperationTask: Transport change dialog user clicked Yes");
                           }
                       });

                AlertDialog alert = builder.create();
                alert.show();
            } else {
                WfdOperationTask task = new WfdOperationTask(inContext, (inWfdOperation == WfdOperation.SELECT_TCP)
                        ?"Starting TCP":"Starting UDP", inWfdOperation);
                task.execute();
            }
        } else if (inWfdOperation == WfdOperation.SET_DECODER_LATENCY) {
            if (wfdMode == WfdMode.PLAYING){
                WfdOperationTask task = new WfdOperationTask(inContext, "Starting SET_DECODER_LATENCY", inWfdOperation);
                task.execute();
            }
        }else if (inWfdOperation == WfdOperation.FLUSH) {
            if (wfdMode != WfdMode.PLAYING) {
                AlertDialog.Builder builder = new AlertDialog.Builder(inContext, AlertDialog.THEME_HOLO_DARK);
                builder.setTitle("Invalid WFD state for Flush")
                       .setMessage("Flush is allowed only while Playing. OK?")
                       .setPositiveButton("Yes", new DialogInterface.OnClickListener() {
                           public void onClick(DialogInterface dialog, int id) {
                               Log.d(TAG, "startWfdOperationTask: Flush dialog user clicked Yes");
                           }
                       });
                AlertDialog alert = builder.create();
                alert.show();
            } else {
                WfdOperationTask task = new WfdOperationTask(inContext, "Flushing WFD session ", WfdOperation.FLUSH);
                task.execute();
            }
        }
    }

    public static class WfdOperationTask extends AsyncTask<Void, Void, Void> {

        private Context context;
        private String dialogText;
        private WfdOperation controlOperation;
        private SharedPreferences mSharedPrefs;

        public WfdOperationTask(Context inContext, String inDialogText, WfdOperation inControlOperation) {
            this.context = inContext;
            this.dialogText = inDialogText;
            this.controlOperation = inControlOperation;
            this.mSharedPrefs = PreferenceManager.getDefaultSharedPreferences(inContext);
        }

        @Override
        protected Void doInBackground(Void... params) {
            Log.d(TAG, "Control Operation- do in background started");
            int ret = -1;
            switch (this.controlOperation) {
            case PLAY:
                try {
                    Log.d(TAG, "Control Operation- play()");
                        ret = ServiceUtil.getInstance().play();
                    if (ret < 0) {
                        Log.e(TAG, "Error when calling play(): " + ret);
                    }
                } catch (RemoteException e) {
                        Log.e(TAG, "Remote exception when calling play()", e);
                }
                break;
            case PAUSE:
                try {
                    Log.d(TAG, "Control Operation- pause()");
                        ret = ServiceUtil.getInstance().pause();
                    if (ret < 0) {
                        Log.e(TAG, "Error when calling pause(): " + ret);
                    } else {
                        Log.d(TAG, "Called pause() successfully");
                    }
                } catch (RemoteException e) {
                    Log.e(TAG, "Remote exception when calling pause()", e);
                }
                Log.d(TAG, "Called pause() successfully- outside try block");
                break;
            case STANDBY:
                try {
                    Log.d(TAG, "Control Operation- standby()");
                        ret = ServiceUtil.getInstance().standby();
                    if (ret < 0) {
                        Log.e(TAG, "Error when calling standby():" + ret);
                    }
                } catch (RemoteException e) {
                    Log.e(TAG, "Remote exception when calling standby()", e);
                }
                break;
            case RESUME:
                try {
                    Log.d(TAG, "Control Operation- resume()");
                        ret = ServiceUtil.getInstance().play();
                    if (ret < 0) {
                        Log.e(TAG, "Error when calling resume()/play(): " + ret);
                    }
                } catch (RemoteException e) {
                        Log.e(TAG, "Remote exception when calling resume()/play()", e);
                }
                break;
            case START_UIBC:
                try {
                    Log.d(TAG, "Control Operation- startUibcSession()");
                        ret = ServiceUtil.getInstance().startUibcSession();
                    if (ret < 0) {
                        Log.e(TAG, "Error when calling startUibcSession(): " + ret);
                    }
                } catch (RemoteException e) {
                        Log.e(TAG, "Remote exception when calling startUibcSession()", e);
                }
                break;
            case STOP_UIBC:
                try {
                    Log.d(TAG, "Control Operation- stopUibcSession()");
                        ret = ServiceUtil.getInstance().stopUibcSession();
                    if (ret < 0) {
                        Log.e(TAG, "Error when calling stopUibcSession(): " + ret);
                    }
                } catch (RemoteException e) {
                        Log.e(TAG, "Remote exception when calling stopUibcSession()", e);
                }
                break;
            case SELECT_TCP:
                try {
                    Log.d(TAG, "Control Operation- setRtpTransport()");
                        ret = ServiceUtil.getInstance().setRtpTransport(WfdEnums.RtpTransportType.TCP.ordinal(), Integer.parseInt(mSharedPrefs.getString("tcp_buffering_delay_value", "5")), 0);
                    if (ret < 0) {
                        Log.e(TAG, "Error when calling setRtpTransport(): " + ret);
                    }
                } catch (RemoteException e) {
                        Log.e(TAG, "Remote exception when calling setRtpTransport()", e);
                }
                break;
            case SELECT_UDP:
                try {
                    Log.d(TAG, "Control Operation- setRtpTransport()");
                        ret = ServiceUtil.getInstance().setRtpTransport(WfdEnums.RtpTransportType.UDP.ordinal(), 0, 0);
                    if (ret < 0) {
                        Log.e(TAG, "Error when calling setRtpTransport(): " + ret);
                    }
                } catch (RemoteException e) {
                        Log.e(TAG, "Remote exception when calling setRtpTransport()", e);
                }
                break;
            case SET_DECODER_LATENCY:
                try {
                    Log.d(TAG, "Control Operation- setDecoderLatency()");
                    ret = ServiceUtil.getInstance().setDecoderLatency(Integer.parseInt(mSharedPrefs.getString("udp_decoder_latency_value", "5")));
                    if (ret < 0) {
                        Log.e(TAG, "Error when calling setDecoderLatency(): " + ret);
                    }
                } catch (RemoteException e) {
                        Log.e(TAG, "Remote exception when calling setDecoderLatency()", e);
                }
                break;
            case FLUSH:
                try {
                    Log.d(TAG, "Control Operation- flush()");
                        ret = ServiceUtil.getInstance().tcpPlaybackControl(WfdEnums.ControlCmdType.FLUSH.ordinal(), 0);
                    if (ret < 0) {
                        Log.e(TAG, "Error when calling flush(): " + ret);
                    }
                } catch (RemoteException e) {
                        Log.e(TAG, "Remote exception when calling flush()", e);
                }
                break;
            case TEARDOWN:
                try {
                    Log.e(TAG, "Control Operation- teardown()");
                    ServiceUtil.getInstance().teardown();
                    Log.d(TAG, "After teardown");
                } catch (RemoteException e) {
                    Log.e(TAG, "Remote exception", e);
                }
                break;
            }
            return null;
        }

        @Override
        protected void onPreExecute() {
            Log.d(TAG, "Control Operation- onPreExecute");
            wfdOperationProgressDialog = ProgressDialog.show(context,
                    dialogText, "Press back to cancel", true, true,
                    new  DialogInterface.OnCancelListener() {
                        @Override
                        public void onCancel(DialogInterface dialog) {
                            Log.d(TAG, "onCancel called- before cancel()");
                            //cancel(true);
                            Log.d(TAG, "onCancel called- after cancel()");
                        }
            });
            Log.d(TAG, "class name: " +
                    context.getClass().getName().substring(context.getClass().getName().lastIndexOf(".")+1));
        };
    }

    public static IWfdActionListener createmActionListener(Handler inHandler) {
        Log.d(TAG, "bindWfdService() called");

        return new WfdActionListenerImpl(inHandler);
    }

    public static class WfdActionListenerImpl extends IWfdActionListener.Stub {
        Handler mHandler;

        WfdActionListenerImpl(Handler handler) {
            super();
            mHandler = handler;
        }

        @Override
        public void onStateUpdate(int newState, int sessionId) throws RemoteException {
            WfdEnums.SessionState state = WfdEnums.SessionState.values()[newState];
            switch (state) {
                case INITIALIZED:
                    Log.d(TAG, "WfdEnums.SessionState==INITIALIZED");
                    if (sessionId > 0) {
                        Log.d(TAG, "WfdEnums.SessionState==INITIALIZED, sessionId > 0");
                        Message messageTeardown = mHandler.obtainMessage(TEARDOWN_CALLBACK);
                        mHandler.sendMessage(messageTeardown);
                    }
                    break;
                case INVALID:
                    Log.d(TAG, "WfdEnums.SessionState==INVALID");
                    wfdMode = WfdMode.INVALID;
                    break;
                case IDLE:
                    Log.d(TAG, "WfdEnums.SessionState==IDLE");
                    break;
                case PLAY:
                    Log.d(TAG, "WfdEnums.SessionState==PLAY");
                    wfdMode = WfdMode.PLAYING;
                    Message messagePlay = mHandler.obtainMessage(PLAY_CALLBACK);
                    mHandler.sendMessage(messagePlay);
                    break;
                case PAUSE:
                    Log.d(TAG, "WfdEnums.SessionState==PAUSE");
                    wfdMode = WfdMode.PAUSED;
                    Message messagePause = mHandler.obtainMessage(PAUSE_CALLBACK);
                    mHandler.sendMessage(messagePause);
                    break;
                case STANDBY:
                    Log.d(TAG, "WfdEnums.SessionState = STANDBY");
                    wfdMode = WfdMode.STANDBY;
                    Message messageStandby = mHandler.obtainMessage(STANDBY_CALLBACK);
                    mHandler.sendMessage(messageStandby);
                    break;
                case ESTABLISHED:
                    Log.d(TAG, "WfdEnums.SessionState==ESTABLISHED");
                    Message messageEstablishedCallback = mHandler.obtainMessage(START_SESSION_ON_UI);
                    mHandler.sendMessage(messageEstablishedCallback);
                    break;
                case TEARDOWN:
                    Log.d(TAG, "WfdEnums.SessionState==TEARDOWN");
                    Message messageTeardown = mHandler.obtainMessage(TEARDOWN_CALLBACK);
                    mHandler.sendMessage(messageTeardown);
                    break;
            }
        }

        @Override
        public void notifyEvent(int event, int sessionId) throws RemoteException {
            if (event == WfdEnums.WfdEvent.UIBC_ENABLED.ordinal()) {
                Log.d(TAG, "notifyEvent- UIBC enabled callback");
                Message messageUibcAction = mHandler.obtainMessage(UIBC_ACTION_COMPLETED);
                messageUibcAction.arg1 = UIBC_ENABLED;
                mHandler.sendMessage(messageUibcAction);
            } else if (event == WfdEnums.WfdEvent.UIBC_DISABLED.ordinal()) {
                Log.d(TAG, "notifyEvent- UIBC disabled callback");
                Message messageUibcAction = mHandler.obtainMessage(UIBC_ACTION_COMPLETED);
                messageUibcAction.arg1 = UIBC_DISABLED;
                mHandler.sendMessage(messageUibcAction);
            }else if (event == WfdEnums.WfdEvent.START_SESSION_FAIL.ordinal()) {
                Log.d(TAG, "notifyEvent- START_SESSION_FAIL");
                Message messageFailSession = mHandler.obtainMessage(CLEAR_UI);
                mHandler.sendMessage(messageFailSession);
            }else if (event == WfdEnums.WfdEvent.AUDIO_ONLY_SESSION.ordinal()) {
                Log.d(TAG, "notifyEvent- AUDIO_ONLY_SESSION");
                Message audioOnlySession = mHandler.obtainMessage(AUDIO_ONLY_NOTIFICATION);
                mHandler.sendMessage(audioOnlySession);
            } else {
                Log.d(TAG, "notifyEvent- unrecognized event: " + event);
            }
        }

        @Override
        public void notify(Bundle b, int sessionId) throws RemoteException {
            if(b != null) {
                Log.d(TAG, "Notify from WFDService");
                String event = b.getString("event");
                if("setDecoderLatency".equalsIgnoreCase(event)) {
                    WfdOperationUtil.wfdOperationProgressDialog.dismiss();
                    Log.d(TAG, "clientProgressDialog dismissed");
                }
            }
        }
    }
}
