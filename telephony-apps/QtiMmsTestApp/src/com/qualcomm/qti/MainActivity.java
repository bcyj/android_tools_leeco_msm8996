/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.app;

import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.app.Activity;
import android.text.Editable;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.net.NetworkInfo;
import android.widget.EditText;
import android.widget.TextView;
import android.net.ConnectivityManager;
import android.net.LinkProperties;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import android.content.Context;
import com.android.internal.telephony.Phone;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import java.util.Random;
import android.telephony.SubscriptionManager;
import android.os.SystemProperties;

import java.util.ArrayList;

/*
 * Run the app in automation mode with below command
 * adb shell am start -n com.qualcomm.qti.app/.MainActivity --ei CLIENTS 2 --eia CLIENT0 2,0,30,10 --eia
 * CLIENT1 1,1,30,10
 *
 *      CLIENTS => How many simultaneous thread needs to be created.
 *      CLIENT<n> => replace n with client index, CLIENT0, CLINET1 etc.
 *
 * Repeat below structure for n number of clients.
 *      CLINET<N> <SubId>, <ApnType>, <Duration to keep PDP alive>, <Total numer of iterations>
*/

public class MainActivity extends Activity {
    static final String TAG = "QcMmsTestMainActivity";

    static final int EVENT_PRINT = 0;
    static final int EVENT_PROGRESS_UPDATE = 1;

    final int DELAY = 30*1000;
    int mWorkerId = 0;

    ArrayList<Worker> mWorkerList = new ArrayList<Worker>();

    ConnectivityManager mConnMgr;
    Handler mainThreadHandler;

    int mMaxCountVal = 1;
    boolean isStopped = false;
    BroadcastReceiver mAnyDataStateReciever;
    BroadcastReceiver mConnectivityChangeReceiver;
    BroadcastReceiver mDefaultDdsReceiver;
    int mVerbose = 0;

    final int CLIENT_PARAM_MAX_COUNT = 4;

    MainActivity mApp;


    @Override
        protected void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            mApp = this;

            mConnMgr = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);

            setContentView(R.layout.activity_main);
            final LogTextBox mOutputRes = (LogTextBox)findViewById(R.id.output);
            final TextView progressRes   = (TextView)findViewById(R.id.progress_text);
            final TextView statusRes   = (TextView)findViewById(R.id.status_text);

            mainThreadHandler = new Handler() {
                @Override
                    public void handleMessage(Message msg) {
                        String str = (String)msg.obj;
                        switch(msg.what) {
                            case EVENT_PRINT:
                                mOutputRes.append(str);
                                statusRes.setText(str);
                                break;
                            case EVENT_PROGRESS_UPDATE:
                                progressRes.setText(str);
                                break;
                        }
                    }
            };

            registerReceivers();

            progressStatus("Idle, no test running.");

            int clients = getIntent().getIntExtra("CLIENTS", 0);
            mVerbose = getIntent().getIntExtra("VERBOSE", 0);

            if ( clients > 0) {
                progressStatus("Running in automation mode for clients = ."+ clients);

                for (int i =0; i < clients; i++ ) {
                    int[] params = getIntent().getIntArrayExtra("CLIENT"+i);

                    if (params == null ||
                            ((params != null) && (params.length != CLIENT_PARAM_MAX_COUNT))) {
                        progressStatus("Invalid parameters.");
                        finish();
                        return;
                    }

                    int subId = params[0];
                    int apnType = params[1];
                    int duration = params[2];
                    int count = params[3];

                    print("----- SUBID = " + subId);
                    print("----- APNTYPE = " + apnType);
                    print("----- DURATION = " + duration);
                    print("----- COUNT = " + count);

                    runWorker(subId, apnType, duration, count);
                }


            }
        }





    class Worker {
        HandlerThread mWorkerThread;
        Handler workerThreadHandler;
        TestParam mParam;

        int mWorkerId;
        int mCurrentIteration = 0;

        ConnectivityManager.NetworkCallback mAllNetworkCallback;
        ConnectivityManager.NetworkCallback mMmsNetworkCallback;
        NetworkRequest mMmsNetworkRequest;
        boolean mSockOptStarted = false;

        final int EVENT_PROCESS_NEXT_REQ = 0;
        final int EVENT_STOP_NETWORK_REQ = 1;

        Worker(int id) {
            mWorkerId = id;
            HandlerThread mWorkerThread = new HandlerThread("Worker" + id);
            mWorkerThread.start();
            workerThreadHandler = new Handler(mWorkerThread.getLooper()) {
                public void handleMessage(Message msg) {
                    switch(msg.what){
                        case EVENT_PROCESS_NEXT_REQ:
                            if(!isStopped) {
                                //Done with the current active PDP, bring it down now.
                                releaseReq();
                            }
                            break;
                        case EVENT_STOP_NETWORK_REQ:
                            Random rand = new Random();
                            int sec = rand.nextInt(30);

                            print("Deactivating MMS PDP");
                            unregisterCallback();

                            try {
                            print_v("Sleeping... " + sec + " seconds");
                            Thread.sleep(sec*1000);
                            print_v("Sleeping...done");
                            } catch (Exception e) {
                                //nop
                            }

                           activatePdpUsingNewApi();
                            break;
                    }
                }
            };
        }

        public void setParam(TestParam param) {
            mParam = param;
        }

        void print(String s) {
            mApp.print("[T" + mWorkerId + ":" + mCurrentIteration +"] " + s);
        }

        void print_v(String s) {
            if (mVerbose == 1) {
                mApp.print("[T" + mWorkerId + ":" + mCurrentIteration + "] " + s);
            }
        }


        void progressStatus(String s) {
            mApp.progressStatus("[T" + mWorkerId + "] " + s);
        }


        ConnectivityManager.NetworkCallback  getNetworkCallback() {
            int mSubId = mParam.mSubId;
            final int mDurationVal = mParam.mDuration;

            return new ConnectivityManager.NetworkCallback() {
                @Override
                public void onPreCheck(Network network) {
                    String thread = Thread.currentThread().getName();
                    //super.onPreCheck(network);
                    print_v("NetworkCallback.onPrecheck: network=" + network);
                }
                @Override
                public void onAvailable(Network network) {
                    String thread = Thread.currentThread().getName();
                    print("NetworkCallback.onAvailable: network=" + network);
                    if(mMmsNetworkRequest != null && !mSockOptStarted) {
                        print_v("MMS connected, deactivating after ="+DELAY+"ms.");
                        workerThreadHandler.sendMessageDelayed(
                                workerThreadHandler.obtainMessage(EVENT_PROCESS_NEXT_REQ),
                                mDurationVal * 1000);
                        mSockOptStarted = true;
                    }

                }
                @Override
                public void onLosing(Network network, int timeToLive) {
                    String thread = Thread.currentThread().getName();
                    //super.onLosing(network, timeToLive);
                    print_v("NetworkCallback.onLosing: network="
                            + network + ", maxTimeToLive= " + timeToLive);
                }
                @Override
                public void onLost(Network network) {
                    String thread = Thread.currentThread().getName();
                    //super.onLost(network);
                    print("NetworkCallback.onLost: network=" + network);
                    Log.d(TAG, thread + "NetworkCallback.onLost: network=" + network);
                }

                @Override
                public void onUnavailable() {
                    String thread = Thread.currentThread().getName();
                    //super.onUnavailable();
                    print_v("NetworkCallback.onUnavailable");
                    Log.d(TAG, thread + "NetworkCallback.onUnavailable");

                }

                @Override
                public void onCapabilitiesChanged(Network network, NetworkCapabilities nc) {
                    String thread = Thread.currentThread().getName();
                    //super.onCapabilitiesChanged(network, nc);
                    print_v("NetworkCallback.onCapabilitiesChanged: network="
                            + network + ", Cap = " + nc);
                }

                @Override
                public void onLinkPropertiesChanged(Network network, LinkProperties lp) {
                    String thread = Thread.currentThread().getName();
                    //super.onLinkPropertiesChanged(network, lp);
                    print_v("NetworkCallback.onLinkPropertiesChanged: network="
                            + network + ", LP = " + lp);
                }

            };
        }
        public void releaseReq() {
            print_v("Releasing the current request.");
            workerThreadHandler.sendMessage(
                    workerThreadHandler.obtainMessage(EVENT_STOP_NETWORK_REQ));
        }

        void unregisterCallback() {
            print_v("unregisterCallback");
            if(mMmsNetworkCallback != null) {
                try {
                    mConnMgr.unregisterNetworkCallback(mMmsNetworkCallback);
                    mMmsNetworkCallback = null;
                    mMmsNetworkRequest = null;
                    mSockOptStarted = false;

                } catch(Exception e) {
                    print("Exception while trying to release the request!");
                }
            }
        }

        void stopWorker() {
            for(int i = 0; i < mWorkerList.size(); i++) {
                Worker w = mWorkerList.get(i);
                if (w.mWorkerId == mWorkerId) {
                    mWorkerList.remove(i);
                }
            }
            if (mWorkerList.size() == 0) {
                progressStatus("Finished test");
            }
        }

        public void activatePdpUsingNewApi() {

            if(mCurrentIteration == mParam.mCount) {
                print("No more attempts");
                mCurrentIteration =0;
                stopWorker();
                return;
            }

            if(mMmsNetworkRequest != null ) {
                print_v("Prev req is not yet released sub="+mParam.mSubId);
                return;
            }
            print("Requesting a PDP on sub="+mParam.mSubId + "Type = " + mParam.mApnType + ", for "
                    + mParam.mDuration + "Seconds");

            /*
               mMmsNetworkRequest = new NetworkRequest.Builder()
               .addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR)
               .addCapability(NetworkCapabilities.NET_CAPABILITY_MMS)
               .build();
               */
            NetworkCapabilities nc = new NetworkCapabilities();
            nc.addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR);
            nc.addCapability(mParam.mApnType);
            nc.setNetworkSpecifier("" + mParam.mSubId);

            mMmsNetworkRequest = new NetworkRequest(nc,
                    ConnectivityManager.TYPE_MOBILE_MMS,
                    ConnectivityManager.REQUEST_ID_UNSET);

            mMmsNetworkCallback = getNetworkCallback();

            mConnMgr.requestNetwork(
                    mMmsNetworkRequest, mMmsNetworkCallback, DELAY);
            print_v("Waiting for MMS PDP activation on sub"+mParam.mSubId);

            progressStatus("Requested = "+ ++mCurrentIteration);

        }
    }

    @Override
        public boolean onCreateOptionsMenu(Menu menu) {
            // Inflate the menu; this adds items to the action bar if it is present.
            getMenuInflater().inflate(R.menu.main, menu);
            return true;
        }

    private void print(String s) {
        mainThreadHandler.sendMessage(mainThreadHandler
                .obtainMessage(EVENT_PRINT, s));
    }

    private void print_v(String s) {
        if (mVerbose == 1) {
            mainThreadHandler.sendMessage(mainThreadHandler
                    .obtainMessage(EVENT_PRINT, s));
        }
    }


    private void progressStatus(String s) {
        mainThreadHandler.sendMessage(mainThreadHandler
                .obtainMessage(EVENT_PROGRESS_UPDATE, s));
    }

    class TestParam {
        public int mSubId = 1;
        public int mApnType = 0;
        public int mDuration = 30;
        public int mCount = 1;

        public TestParam(int subId, int apnType, int duration, int count) {
            mSubId = subId;
            mApnType = apnType;
            mDuration = duration;
            mCount = count;
        }
    };

    void runTest(Worker worker) {
        worker.activatePdpUsingNewApi();
    }

    void runWorker(int subId, int apnType, int duration, int count) {
        TestParam param = new TestParam(subId, apnType, duration, count);
        Worker worker = new Worker(mWorkerId++);
        worker.setParam(param);
        mWorkerList.add(worker);

        runTest(worker);
    }

    public void executeTest(View view) {
        isStopped = false;
        EditText subId  = (EditText)findViewById(R.id.sub_id_value);
        EditText count  = (EditText)findViewById(R.id.count_value);
        EditText apnType  = (EditText)findViewById(R.id.apn_type_value);
        EditText duration  = (EditText)findViewById(R.id.duration_value);
        String subStr = subId.getText().toString();
        String countStr = count.getText().toString();
        String apnTypeStr = apnType.getText().toString();
        String durationStr = duration.getText().toString();
        int subIdVal = 1;
        int apnTypeVal = 1;
        int durationVal = 30;

        try{
            subIdVal = Integer.parseInt(subStr);
        } catch(Exception e) {
            print("Input a subscription!");
            return;
        }

        try{
            mMaxCountVal = Integer.parseInt(countStr);
        } catch(Exception e) {
            print("Input a count value");
            return;
        }

        try{
            apnTypeVal = Integer.parseInt(apnTypeStr);
        } catch(Exception e) {
            print("No APN type specifed, assuming MMS type");
        }

        try{
            durationVal = Integer.parseInt(durationStr);
        } catch(Exception e) {
            print("No duration provided, assuming 30 seconds");
        }


        runWorker(subIdVal, apnTypeVal, durationVal, mMaxCountVal);

    }


    private void registerReceivers() {
        print_v("RegisterReceivers");
        registerReceiver(mAnyDataStateReciever = new BroadcastReceiver() {
            public void onReceive(Context context, Intent intent) {
                PhoneConstants.DataState state = Enum.valueOf(PhoneConstants.DataState.class,
                    intent.getStringExtra(PhoneConstants.STATE_KEY));

                String apntype = intent.getStringExtra(PhoneConstants.DATA_APN_TYPE_KEY);
                int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY, -1);
                String reason = intent.getStringExtra(PhoneConstants.STATE_CHANGE_REASON_KEY);

                if("default".equals(apntype)) {
                    print_v("ANY_DATA_STATE: sub" + subId
                        + apntype + ", " + state
                        + ", [" + reason +"]");
                }

                if("mms".equals(apntype)) {

                    print_v("ANY_DATA_STATE: sub" + subId
                        + apntype + ", " + state
                        + ", [" + reason +"]");

                }
            }
        }, new IntentFilter(TelephonyIntents.ACTION_ANY_DATA_CONNECTION_STATE_CHANGED));

        registerReceiver(mConnectivityChangeReceiver = new BroadcastReceiver() {
                public void onReceive(Context context, Intent intent) {
                Log.d(TAG, "CONNECTIVITY_CHANGE, networkInfo = "
                    + (NetworkInfo)intent.getParcelableExtra(
                        ConnectivityManager.EXTRA_NETWORK_INFO));
                }
                }, new IntentFilter(ConnectivityManager.CONNECTIVITY_ACTION));

        registerReceiver(mDefaultDdsReceiver = new BroadcastReceiver() {
                public void onReceive(Context context, Intent intent) {
                print("ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED dds= "
                    + intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY, -1));
                }
                }, new IntentFilter(TelephonyIntents.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED));

    }

    void unRegisterReceivers() {
        print_v("unregisterReceivers");

        unregisterReceiver(mAnyDataStateReciever);
        unregisterReceiver(mConnectivityChangeReceiver);
        unregisterReceiver(mDefaultDdsReceiver);
    }

    protected void onDestroy() {
        super.onDestroy();
        print("onStop");
        unRegisterReceivers();

        //stopTest(null);
    }

    public void stopTest(View view) {
        isStopped = true;
        print("STOP!!");
        for(int i =0; i < mWorkerList.size(); i++) {
            Worker w = mWorkerList.get(i);
            w.unregisterCallback();
        }
    }

}

