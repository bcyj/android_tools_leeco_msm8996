/******************************************************************************
 * @file    EmbmsTestAppActivity.java
 *
 * ---------------------------------------------------------------------------
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * ---------------------------------------------------------------------------
 *
 *******************************************************************************/
package com.qualcomm.embmstest;

import java.util.Arrays;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.widget.Button;
import android.widget.TextView;
import android.widget.EditText;
import android.view.View;
import android.view.View.OnClickListener;

import com.qualcomm.embms.IEmbmsService;
import com.qualcomm.embms.IEmbmsServiceCallback;


public class EmbmsTestAppActivity extends Activity {

    private static final String LOG_TAG = "EmbmsApp";
    private IEmbmsService mEmbmsService;
    private Handler mHandler;
    private boolean mIsBound;
    private int mTraceId = 0;

    private TextView mCallbackText;
    private TextView mLogText;
    private EditText mTmgiText;
    public static final ComponentName DEFAULT_CONTAINER_COMPONENT =
            new ComponentName("com.qualcomm.embms", "com.qualcomm.embms.EmbmsService");


    private void setupButton(int id, View.OnClickListener l) {
        Button button = (Button)findViewById(id);
        button.setOnClickListener(l);
    }

    private int getTraceId() {
        return mTraceId++;
    }

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.i(LOG_TAG, "onCreate received");

        setContentView(R.layout.main);

        // Watch for button clicks
        setupButton(R.id.bind, mBindListener);
        setupButton(R.id.unbind, mUnbindListener);
        setupButton(R.id.register, mRegisterListener);
        setupButton(R.id.deregister, mDeregisterListener);
        setupButton(R.id.enable, mEnableListener);
        setupButton(R.id.disable, mDisableListener);
        setupButton(R.id.activate, mActivateListener);
        setupButton(R.id.deactivate, mDeActivateListener);
        setupButton(R.id.get_available, mGetAvailableListener);
        setupButton(R.id.get_active, mGetActiveListener);
        setupButton(R.id.get_coverage, mGetCoverageListener);
        setupButton(R.id.get_log_packet_id, mGetActiveLogPacketIDs);
        setupButton(R.id.deliver_log_packet, mDeliverLogPacket);
        setupButton(R.id.get_e911_state, mGetE911State);

        // Update text on button clicks
        mCallbackText = (TextView)findViewById(R.id.Callback);
        mCallbackText.setText("Not attached");
        mTmgiText = (EditText)findViewById(R.id.tmgi);
        mLogText = (TextView)findViewById(R.id.Logs);

        // This handler only handles the async requests from
        // non-Main threads to log messages in the text view.
        mHandler = new Handler() {
            public void handleMessage (Message msg) {
                String str = (String) msg.obj;
                logUtil(msg.what, str);
            }
        };
    }

    @Override
    public void onResume() {
        super.onResume();
        Log.i(LOG_TAG, "onResume received");
    }

    @Override
    public void onPause() {
        super.onPause();
        Log.i(LOG_TAG, "On Pause received");
        disableService();
    }

    private void disableService() {
        mCallbackText.setText("Unbinding");
        if (mIsBound) {
            logUtil(Log.INFO, "Unbinding the service");
            unbindService(mServiceConnection);
        }
        mIsBound = false;
    }

    /**
     * Utility to log messages into the log text box.
     * This should only be called from the Main thread.
     */
    private void logUtil(int type, String str) {
        switch (type) {
            case Log.ERROR:
                Log.e(LOG_TAG, str);
                break;
            case Log.DEBUG:
                Log.d(LOG_TAG, str);
                break;
            case Log.INFO:
                Log.i(LOG_TAG, str);
                break;
        }

        if (mLogText != null) {
            //TODO: test if append works
            //mLogText.append("\n" + str);
            mLogText.setText(str);
        }
    }

    /**
     * Utility to log messages into the log
     * text box. This can be called from any thread.
     */
    private void logUtilAsync(int type, String str) {
        Message msg = mHandler.obtainMessage(type, str);
        msg.sendToTarget();
    }

    private IEmbmsServiceCallback mCb = new IEmbmsServiceCallback.Stub() {
        public void stateNotification(int state, String ipaddress, int ifIndex) {
            logUtilAsync(Log.INFO, "State recd in callback " + state +
                                   " ipaddress = " + ipaddress + " index = " + ifIndex);
        }

        public void enableResponse(int debugTraceId, int responseCode, String interfaceName,
                                   int interfaceIndex) {
            logUtilAsync(Log.INFO, "Enable response recd in callback "
                                    + " trace id = " + debugTraceId
                                    + " status = " + responseCode
                                    + " ifname = " + interfaceName
                                    + " index = " + interfaceIndex);
        }

        public void disableResponse(int debugTraceId, int responseCode) {
            logUtilAsync(Log.INFO, "Disable response recd in callback "
                                    + " trace id = " + debugTraceId
                                    + " status = " + responseCode);
        }

        public void activateTMGIResponse(int debugTraceId, int responseCode, byte[] tmgi) {
            logUtilAsync(Log.INFO, "activateTMGIResponse recd in callback "
                                    + " trace id = " + debugTraceId
                                    + " status = " + responseCode
                                    + " tmgi = " + Arrays.toString(tmgi));
        }

        public void deactivateTMGIResponse(int debugTraceId, int responseCode, byte[] tmgi) {
            logUtilAsync(Log.INFO, "deactivateTMGIResponse recd in callback "
                                    + " trace id = " + debugTraceId
                                    + " status = " + responseCode
                                    + " tmgi = " + Arrays.toString(tmgi));
        }

        public void actDeactTMGIResponse(int debugTraceId, int actResponseCode, byte[] actTMGI,
                                            int deactResponseCode, byte[] deactTMGI) {
            logUtilAsync(Log.INFO, "actDeactTMGIResponse recd in callback "
                    + " trace id = " + debugTraceId
                    + " Actstatus = " + actResponseCode
                    + " acttmgi = " + Arrays.toString(actTMGI)
                    + " deactStatus = " + deactResponseCode
                    + " deactTmgi = " + Arrays.toString(deactTMGI));
        }

        public void signalStrengthResponse(int debugTraceId, int responseCode, int[] mbsfnAreaId,
                                            float[] SNR, float[] excessSNR, int[] numTmgiPerMbsfn,
                                            byte [] activeTmgiList) {
            logUtilAsync(Log.INFO, "signalStrengthResponse recd in callback "
                    + " trace id = " + debugTraceId
                    + " status = " + responseCode
                    + " mbfsnAreaId = " + Arrays.toString(mbsfnAreaId)
                    + " snr = " + Arrays.toString(SNR)
                    + " excessSNR = " + Arrays.toString(excessSNR)
                    + " numberOfTmgiperMBSFN = " + Arrays.toString(numTmgiPerMbsfn)
                    + " activeTmgiList = " + Arrays.toString(activeTmgiList));
        }

        public void availableTMGIListNotification(int debugTraceId, byte[] tmgiList) {
            logUtilAsync(Log.INFO, "availableTMGIListNotification recd in callback "
                                    + " trace id = " + debugTraceId
                                    + " tmgiList = " + Arrays.toString(tmgiList));
        }

        public void activeTMGIListNotification(int debugTraceId, byte[] tmgiList) {
            logUtilAsync(Log.INFO, "activeTMGIListNotification recd in callback list = "
                                    + " trace id = " + debugTraceId
                                    + Arrays.toString(tmgiList));
        }

        public void broadcastCoverageNotification(int debugTraceId, int state) {
            logUtilAsync(Log.INFO, "broadcastCoverageNotification recd in callback "
                                    + " trace id = " + debugTraceId
                                    + " state = " + state);
        }

        public void oosNotification(int debugTraceId, int reason, byte[] tmgiList) {
            logUtilAsync(Log.INFO, "oosNotification recd in callback "
                                    + " trace id = " + debugTraceId
                                    + "reason = " + reason
                                    + "tmgis = " + Arrays.toString(tmgiList));
        }
        public void cellGlobalIdNotification(int debugTraceId, String mcc, String mnc,
                                                                           String cellId) {
            logUtilAsync(Log.INFO, "cellGlobalIdNotification recd in callback "
                                    + " trace id = " + debugTraceId
                                    + "cellId = " + cellId
                                    + "mcc = " + mcc
                                    + "mnc = " + mnc);
        }
        public void radioStateNotification(int debugTraceId, int state) {
            logUtilAsync(Log.INFO, "radioStateNotification recd in callback "
                    + " trace id = " + debugTraceId
                    + "state = " + state);
        }
        public void activeLogPacketIDsResponse(int debugTraceId, int[] activeLogPacketIdList){
            logUtilAsync(Log.INFO, "activeLogPacketIDsResponse recd in callback "
                    + " trace id = " + debugTraceId
                    + "activeLogPacketIdList = " + Arrays.toString(activeLogPacketIdList));
        }
        public void saiNotification (int debugTraceId, int[] campedSAIList,
                                        int[] numberofSAIperGroup, int[] availableSAIList){
            logUtilAsync(Log.INFO, "saiNotification recd in callback "
                    + " trace id = " + debugTraceId
                    + "camped list = " + Arrays.toString(campedSAIList)
                    + "sai per group list = " + Arrays.toString(numberofSAIperGroup)
                    + "available list = " + Arrays.toString(availableSAIList));
        }
        public void timeResponse (int debugTraceId,int responseCode, long timeMseconds,
                 boolean additionalInfo, boolean dayLightSaving, int leapSeconds,
                 long localTimeOffset){
            logUtilAsync(Log.INFO, "timeResponse recd in callback "
                    + " trace id = " + debugTraceId
                    + "status = " + responseCode
                    + "time milli seconds = " + timeMseconds
                    + "additional info = " + additionalInfo
                    + "day light saving = " + dayLightSaving
                    + "leap seconds = " + leapSeconds
                    + "local time offset = " + localTimeOffset);
        }
        public void e911Notification(int debugTraceId,int state){
           logUtilAsync(Log.INFO, "e911Notification recd in callback "
                   + " trace id = " + debugTraceId
                   + " state = " + state);
       }
       public void contentDescriptionPerObjectControl(int debugTraceId,byte[] tmgi,
               int perObjectContentControl, int perObjectStatusControl){
           logUtilAsync(Log.INFO, "contentDescriptionPerObjectControl recd in callback "
                   + " trace id = " + debugTraceId
                   + " tmgi = " + Arrays.toString(tmgi)
                   + "perObjectContentControl" + perObjectContentControl
                   + "perObjectStatusControl" + perObjectStatusControl);
       }
       public void getPLMNListResponse(int debugTraceId, byte[] plmnList){
           logUtilAsync(Log.INFO, "getPLMNListResponse recd in callback "
                   + " trace id = " + debugTraceId
                   + " plmnList = " + Arrays.toString(plmnList));
       }
    };

    /**
     * Listeners to handle all the button clicks
     */
    private OnClickListener mBindListener = new OnClickListener()
    {
        public void onClick(View v) {
            mCallbackText.setText("Binding");
            if (mEmbmsService == null) {

                try {
                    boolean bRet = false;
                    logUtil(Log.INFO, "Binding to the service " + IEmbmsService.class.getName());
                    Intent service = new Intent().setComponent(DEFAULT_CONTAINER_COMPONENT);
                    bRet = getApplicationContext().bindService (service, mServiceConnection,
                            BIND_AUTO_CREATE);
                    if (false == bRet) {
                        logUtil(Log.ERROR, "bindService returned false");
                    } else {
                        mIsBound = true;
                    }

                } catch (SecurityException e) {
                    logUtil(Log.ERROR, "Security exception on binding");
                }
            }
        }
    };

    private OnClickListener mUnbindListener = new OnClickListener() {
        public void onClick(View v) {
            mCallbackText.setText("Unbinding");
            if (mIsBound) {
                logUtil(Log.INFO, "Unbinding the service");
                unbindService(mServiceConnection);
                mIsBound = false;
            } else {
                logUtil(Log.ERROR, "Service already unbound");
            }
        }
    };

    private OnClickListener mRegisterListener = new OnClickListener() {
        public void onClick(View v) {
            mCallbackText.setText("Registering Callbacks");
            if (null != mEmbmsService) {
                logUtil(Log.INFO, "Registering Callbacks");
                try {
                    int ret;
                    ret = mEmbmsService.registerCallback(getTraceId(), mCb);
                    logUtil(Log.INFO, "registerCallnback returned " + ret);
                } catch (RemoteException e) {
                    logUtil(Log.ERROR, "RemoteException caught on Registering");
                }
            }
        }
    };

    private OnClickListener mDeregisterListener = new OnClickListener() {
        public void onClick(View v) {
            mCallbackText.setText("Deregistering Callbacks");
            if (null != mEmbmsService) {
                logUtil(Log.INFO, "Deregistering Callbacks");
                try {
                    int ret = mEmbmsService.deregisterCallback(getTraceId(), mCb);
                    logUtil(Log.INFO, "deregisterCallback returned " + ret);
                } catch (RemoteException e) {
                    logUtil(Log.ERROR, "Remote exception caught on de-registering");
                }
            }
        }
    };

    private OnClickListener mEnableListener = new OnClickListener() {
        public void onClick(View v) {
            mCallbackText.setText("Enabling EMBMS");
            if (null != mEmbmsService) {
                logUtil(Log.INFO, "Enabling EMBMS");
                try {
                    int ret;

                    ret = mEmbmsService.enable(getTraceId());
                    logUtil(Log.INFO, "enable returned " + ret);
                } catch (RemoteException e) {
                    logUtil(Log.ERROR, "Remote exception caught on enabling");
                }
            }
        }
    };

    private OnClickListener mDisableListener = new OnClickListener() {
        public void onClick(View v) {
            mCallbackText.setText("Disabling EMBMS");
            if (null != mEmbmsService) {
                logUtil(Log.INFO, "Disabling EMBMS");
                try {
                    int ret;

                    ret = mEmbmsService.disable(getTraceId());
                    logUtil(Log.INFO, "disable returned " + ret);
                } catch (RemoteException e) {
                    logUtil(Log.ERROR, "Remote exception caught on disabling");
                }
            }
        }
    };

    private OnClickListener mActivateListener = new OnClickListener() {
        public void onClick(View v) {
            mCallbackText.setText("Activating TMGI");
            if (null != mEmbmsService) {
                logUtil(Log.INFO, "Activating TMGI");

                long tmgiLong = Long.parseLong(mTmgiText.getText().toString());
                logUtil(Log.INFO, "TMGI in dec = " + tmgiLong);
                try {
                    int ret;
                    byte[] tmgi = {(byte)(tmgiLong >>> 40),
                            (byte)(tmgiLong >>> 32),
                            (byte)(tmgiLong >>> 24),
                            (byte)(tmgiLong >>> 16),
                            (byte)(tmgiLong >>> 8),
                            (byte)(tmgiLong)};
                    int[] saiList = {4, 5};
                    int[] earfcnList = {6, 7};
                    int priority = 3;

                    ret = mEmbmsService.activateTMGI(getTraceId(), tmgi, priority, saiList,
                            earfcnList);
                    logUtil(Log.INFO, "Activate returned " + ret);
                } catch (RemoteException e) {
                    logUtil(Log.ERROR, "Remote exception caught on activating");
                }
            }
        }
    };

    private OnClickListener mDeActivateListener = new OnClickListener() {
        public void onClick(View v) {
            mCallbackText.setText("DeActivating TMGI");
            if (null != mEmbmsService) {
                logUtil(Log.INFO, "DeActivating TMGI");
                long tmgiLong = Long.parseLong(mTmgiText.getText().toString());
                logUtil(Log.INFO, "TMGI in dec = " + tmgiLong);

                try {
                    int ret;
                    byte[] tmgi = {(byte)(tmgiLong >>> 40),
                            (byte)(tmgiLong >>> 32),
                            (byte)(tmgiLong >>> 24),
                            (byte)(tmgiLong >>> 16),
                            (byte)(tmgiLong >>> 8),
                            (byte)(tmgiLong)};

                    ret = mEmbmsService.deactivateTMGI(getTraceId(), tmgi);
                    logUtil(Log.INFO, "DeActivate returned " + ret);
                } catch (RemoteException e) {
                    logUtil(Log.ERROR, "Remote exception caught on deactivating");
                }
            }
        }
    };

    private OnClickListener mActDeactivateListener = new OnClickListener() {
        public void onClick(View v) {
            mCallbackText.setText("ActDeactivating TMGI");
            if (null != mEmbmsService) {
                logUtil(Log.INFO, "ActDeactivating TMGI");

                long tmgiLong = Long.parseLong(mTmgiText.getText().toString());
                logUtil(Log.INFO, "Act TMGI in dec = " + tmgiLong);
                try {
                    int ret;
                    byte[] tmgiAct = {(byte)(tmgiLong >>> 40),
                            (byte)(tmgiLong >>> 32),
                            (byte)(tmgiLong >>> 24),
                            (byte)(tmgiLong >>> 16),
                            (byte)(tmgiLong >>> 8),
                            (byte)(tmgiLong)};
                    // For TMGi to be deactivated simply used the activate TMGI plus
                    // 10
                    tmgiLong += 10;
                    logUtil(Log.INFO, "Deact TMGI in dec = " + tmgiLong);
                    byte[] tmgiDeact = {(byte)(tmgiLong >>> 40),
                            (byte)(tmgiLong >>> 32),
                            (byte)(tmgiLong >>> 24),
                            (byte)(tmgiLong >>> 16),
                            (byte)(tmgiLong >>> 8),
                            (byte)(tmgiLong)};

                    int priority = 3;
                    int[] saiList = {4, 5};
                    int[] earfcnList = {6, 7};
                    ret = mEmbmsService.actDeactTMGI(getTraceId(), tmgiAct, priority, saiList,
                                                        earfcnList, tmgiDeact);
                    logUtil(Log.INFO, "ActDeactivating returned " + ret);
                } catch (RemoteException e) {
                    logUtil(Log.ERROR, "Remote exception caught on act-deact");
                }
            }
        }
    };


    private OnClickListener mGetActiveListener = new OnClickListener() {
        public void onClick(View v) {
            mCallbackText.setText("Getting Active TMGIs");
            if (null != mEmbmsService) {
                logUtil(Log.INFO, "Getting Active TMGIs");

                try {
                    int ret;

                    ret = mEmbmsService.getActiveTMGIList(getTraceId());
                    logUtil(Log.INFO, "getActiveTMGIList returned " + ret);
                } catch (RemoteException e) {
                    logUtil(Log.ERROR, "Remote exception caught on getActiveTMGIList");
                }
            }
        }
    };

    private OnClickListener mGetAvailableListener = new OnClickListener() {
        public void onClick(View v) {
            mCallbackText.setText("Getting Available TMGIs");
            if (null != mEmbmsService) {
                logUtil(Log.INFO, "Getting Available TMGIs");

                try {
                    int ret;

                    ret = mEmbmsService.getAvailableTMGIList(getTraceId());
                    logUtil(Log.INFO, "getAvailableTMGIList returned " + ret);
                } catch (RemoteException e) {
                    logUtil(Log.ERROR, "Remote exception caught on getAvailableTMGIList");
                }
            }
        }
    };

    private OnClickListener mGetCoverageListener = new OnClickListener() {
        public void onClick(View v) {
            mCallbackText.setText("Getting Coverage State");
            if (null != mEmbmsService) {
                logUtil(Log.INFO, "Getting Coverage State");

                try {
                    int ret;

                    ret = mEmbmsService.getCoverageState(getTraceId());
                    logUtil(Log.INFO, "getCoverageState returned " + ret);
                } catch (RemoteException e) {
                    logUtil(Log.ERROR, "Remote exception caught on getCoverageState");
                }
            }
        }
    };

    private OnClickListener mGetSigStrengthListener = new OnClickListener() {
        public void onClick(View v) {
            mCallbackText.setText("Getting Signal Strength");
            if (null != mEmbmsService) {
                logUtil(Log.INFO, "Getting Signal Strength");

                try {
                    int ret;

                    ret = mEmbmsService.getSignalStrength(getTraceId());
                    logUtil(Log.INFO, "getSignalStrength returned " + ret);
                } catch (RemoteException e) {
                    logUtil(Log.ERROR, "Remote exception caught on getSignalStrength");
                }
            }
        }
    };

    /**
     * Connection to the service.
     */
    private ServiceConnection mServiceConnection = new ServiceConnection () {
        // Called when the connection with the service is established
        public void onServiceConnected(ComponentName className, IBinder service) {
            logUtil(Log.INFO, "Service connected");
            mEmbmsService = IEmbmsService.Stub.asInterface(service);

            if (null != mEmbmsService) {
                logUtil(Log.INFO, "Got valid instance of service");
                try {
                    String version = mEmbmsService.getVersion(getTraceId());
                    logUtil(Log.INFO, "Version is: " + version);
                } catch (RemoteException e) {
                    logUtil(Log.ERROR, "Remote exception caught on getVersion");
                }
            } else {
                logUtil(Log.ERROR, "Service instance is NULL");
            }
        }

        // Called when the connection with the service is disconnected
        public void onServiceDisconnected(ComponentName name) {

            logUtil(Log.ERROR, "Service disconn");
            mEmbmsService = null;
        }
    };

    private OnClickListener mGetActiveLogPacketIDs = new OnClickListener() {
        public void onClick(View v) {
            mCallbackText.setText("Getting Active Log Packet ID's");
            if (null != mEmbmsService) {
                logUtil(Log.INFO, "Getting Active Log Packet ID's");

                try {
                    int ret;
                    int[] supportedLogPacketIdList = {0,1,2};
                    ret = mEmbmsService.getActiveLogPacketIDs(getTraceId(),
                            supportedLogPacketIdList);
                    logUtil(Log.INFO, "getActiveLogPacketIDs returned " + ret);
                } catch (RemoteException e) {
                    logUtil(Log.ERROR, "Remote exception caught on getActiveLogPacketIDs");
                }
            }
        }
    };

    private OnClickListener mDeliverLogPacket = new OnClickListener() {
        public void onClick(View v) {
            mCallbackText.setText("Deliver Log Packet");
            if (null != mEmbmsService) {
                logUtil(Log.INFO, "Deliver Log Packet");

                try {
                    int ret;
                    int logPacketId = 1;
                    byte[] logPacket = {1, 2, 3};
                    ret = mEmbmsService.deliverLogPacket(getTraceId(), logPacketId,
                            logPacket);
                    logUtil(Log.INFO, "DeliverLogPacket returned " + ret);
                } catch (RemoteException e) {
                    logUtil(Log.ERROR, "Remote exception caught on DeliverLogPacket");
                }
            }
        }
    };

    private OnClickListener mGetE911State = new OnClickListener() {
        public void onClick(View v) {
            mCallbackText.setText("Get E911 State");
            if (null != mEmbmsService) {
                logUtil(Log.INFO, "Get E911 State");

                try {
                    int ret;
                    ret = mEmbmsService.getE911State(getTraceId());
                    logUtil(Log.INFO, "Get E911 State returned " + ret);
                } catch (RemoteException e) {
                    logUtil(Log.ERROR, "Remote exception caught in getE911State");
                }
            }
        }
    };

   // TO DO: Add button for contentDescription
   // TO DO: Add button for getPLMNList
}
