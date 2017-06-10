/******************************************************************************
  @file    Tracking.java
  @brief   tracking screen for VZW GPS Location Provider test app

  DESCRIPTION

  test app for VZW GPS Location Provider test app

  ---------------------------------------------------------------------------
  Copyright (C) 2010 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
 ******************************************************************************/

package com.qualcomm.location.qvtester;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.net.InetSocketAddress;
import java.text.DateFormat;
import java.util.Date;
import java.util.List;

import com.qualcomm.location.vzw_library.IVzwHalGpsCallback;
import com.qualcomm.location.vzw_library.IVzwHalGpsLocationProvider;
import com.qualcomm.location.vzw_library.VzwHalCriteria;
import com.qualcomm.location.vzw_library.VzwHalLocation;
import com.qualcomm.location.vzw_library.VzwHalSvInfo;
import com.qualcomm.location.vzw_library.imp.VzwHalGpsLocationProviderImp;

import android.app.Activity;
import android.content.Intent;
import android.location.GpsStatus;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.LocationProvider;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;

public class Tracking extends Activity {


    private static final String TAG = "QVTester.Tracking";

    /** Called when the activity is first created. */
    // criteria with default values
    int iMode;
    int iNumOfFix;
    int iTimeBetweenFix;
    String strPdeAddr;
    int iPdePort;
    int iAccuracyH;
    int iAccuracyV;
    int iMaxResponseTime;
    String strAppCredential;

    int iRunStateMachineTestSuite = 0;

    int TBF_NormalStack = 5000;

    int mCounter = 0;

    private boolean fgRunWithNormalStack;

    private class ClientCard {
        public int mClientId = -1;
        public VzwHalCriteria mCriteria = new VzwHalCriteria();
        public int mSessionId = -1;

        public Button btnStop = null;
        public Button btnRestart = null;

        public TextView textViewPosition;
        public TextView textViewSessionId;
        public TextView textViewFixMode;
        public TextView textViewFixType;
        public TextView textViewTTF;

        public long mTimestampLastFix = 0;
        public int mNumRemainingSession = 0;
    }

    private final static int client_vzw = 0;
    private final static int client_normal = 1;
    ClientCard [] clients = new ClientCard[2];

    private LocationProvider NormalLocationProvider;
    private LocationManager NormalLocationManager;
    private Location NormalGpsLastReportedLocation;
    private int NormalGpsEngineStatus = 0;
    private GpsStatus NormalGpsStatus;
    private long timestampLastGpsStatusUpdate = 0;

    private VzwCallbackHandler mVzwCallbackHandler;
    private IVzwHalGpsLocationProvider mVzwProvider;

    private boolean mRestartOnFinal = false;

    OutputStreamWriter mLogFile;
    public static String newline = System.getProperty("line.separator");

    private Handler mHandler = new Handler();

    @Override
    protected void onDestroy() {
        Log.v(TAG,"OnDestroy");
        super.onDestroy();
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.tracking);
    }

    @Override
    protected void onStart() {

        super.onStart();

        Log.v(TAG,"new VzwCallbackHandler()");

        mVzwCallbackHandler = new VzwCallbackHandler();

        Log.v(TAG,"IVzwHalGpsLocationProvider.getInstance()");


        mVzwProvider = IVzwHalGpsLocationProvider.getInstance();

        Log.v(TAG,"mVzwProvider.init(mVzwCallbackHandler)");

        mVzwProvider.init(mVzwCallbackHandler);

        mRestartOnFinal = true;

        ClientCard client = new ClientCard();
        client.btnStop = (Button) findViewById(R.id.ButtonStop01);
        client.btnStop.setOnClickListener(btn_OnClick);
        client.btnRestart = (Button) findViewById(R.id.ButtonRestart01);
        client.btnRestart.setOnClickListener(btn_OnClick);
        client.textViewPosition = (TextView)findViewById(R.id.TextViewPosition01);
        client.textViewSessionId = (TextView)findViewById(R.id.TextViewSessionId01);
        client.textViewFixMode = (TextView)findViewById(R.id.TextViewFixMode01);
        client.textViewFixType = (TextView)findViewById(R.id.TextViewFixType01);
        client.textViewTTF = (TextView)findViewById(R.id.TextViewTTF01);
        clients[client_vzw] = client;

        client = new ClientCard();
        client.btnStop = (Button) findViewById(R.id.ButtonStop02);
        client.btnStop.setOnClickListener(btn_OnClick);
        client.btnRestart = (Button) findViewById(R.id.ButtonRestart02);
        client.btnRestart.setOnClickListener(btn_OnClick);
        client.textViewPosition = (TextView)findViewById(R.id.TextViewPosition02);
        client.textViewSessionId = (TextView)findViewById(R.id.TextViewSessionId02);
        client.textViewFixMode = (TextView)findViewById(R.id.TextViewFixMode02);
        client.textViewFixType = (TextView)findViewById(R.id.TextViewFixType02);
        client.textViewTTF = (TextView)findViewById(R.id.TextViewTTF02);
        clients[client_normal] = client;

        LoadDefaultCriteria ();
        Intent i = getIntent();

        iMode = i.getIntExtra("Mode", iMode);
        iNumOfFix = i.getIntExtra("NumOfFix", iNumOfFix);
        iTimeBetweenFix = i.getIntExtra("TimeBetweenFix", iTimeBetweenFix);
        strPdeAddr = i.getStringExtra("PdeAddr");
        iPdePort = i.getIntExtra("PdePort", iPdePort);
        iAccuracyH = i.getIntExtra("AccuracyH", iAccuracyH);
        iAccuracyV = i.getIntExtra("AccuracyV", iAccuracyV);
        iMaxResponseTime = i.getIntExtra("MaxResponseTime", iMaxResponseTime);
        strAppCredential = i.getStringExtra("AppCredential");
        iRunStateMachineTestSuite = i.getIntExtra("RunStateTest", 0);

        if(strPdeAddr.length() != 0) {
            InetSocketAddress pdeAddr = new InetSocketAddress(strPdeAddr, iPdePort);
            mVzwProvider.setPdeAddress(pdeAddr);
        }
        else
        {
            mVzwProvider.setPdeAddress(null);
        }

        LogCriteria ();

        try {
            Log.v(TAG,"Logging started");

            mLogFile = new OutputStreamWriter(openFileOutput("location.log", MODE_APPEND));

            mLogFile.write("====================================================================" + newline);
            mLogFile.write("Experiment started: " +
                           DateFormat.getDateTimeInstance (DateFormat.FULL, DateFormat.FULL).format(new Date()) + newline);
            mLogFile.write("====================================================================" + newline);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            mLogFile = null;
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            mLogFile = null;
        }

        hookupNormalProvider ();
        hookupVzwProvider ();

        prepareStateMachineTestSuite (iRunStateMachineTestSuite);

        mVzwProvider.setCredentials(strAppCredential);

        // kick off
        if(iRunStateMachineTestSuite == 0)
        {
            mVzwProvider.start(clients[client_vzw].mCriteria, clients[client_vzw].mSessionId);
        }
        else
        {
            // disable buttons
            clients[client_vzw].btnStop.setEnabled(false);
            clients[client_vzw].btnRestart.setEnabled(false);

            // adjust number of fix
            clients[client_vzw].mNumRemainingSession = fsmTestSuite.last + 1;

            mVzwProvider.start(fsmTestSuite.criteria[0], clients[client_vzw].mSessionId);
        }

        fgRunWithNormalStack = i.getBooleanExtra("RunWithNormalStack", false);

        if(fgRunWithNormalStack)
        {
            clients[client_normal].mCriteria =new VzwHalCriteria (clients[client_vzw].mCriteria);
            clients[client_normal].mClientId= client_normal;
            clients[client_normal].mNumRemainingSession = iNumOfFix;
            clients[client_normal].mSessionId = 1;

            if (0 != iMode) {
                Log.v(TAG, "Not MSA, not singleshot");
                NormalLocationManager.requestLocationUpdates(
                    NormalLocationProvider.getName(),
                    //1000*clients[client_normal].mCriteria.getHintNextFixArriveInSec(),
                    TBF_NormalStack,
                    0,NormalLocationListener);
            } else {
                Log.v(TAG, "MSA, singleshot");
                NormalLocationManager.requestSingleUpdate(
                    NormalLocationProvider.getName(),
                    NormalLocationListener,
                    getMainLooper());
            }
        }

    }

    @Override
    protected void onStop() {

        Log.v(TAG,"onStop");

        mRestartOnFinal = false;

        // shutdown vzw gps provider
        mVzwProvider.shutdown();

        // remove listeners from normal gps stack
        NormalLocationManager.removeGpsStatusListener(NormalGpsStatusListener);
        NormalLocationManager.removeUpdates(NormalLocationListener);


        if(mLogFile != null)
        {
            try {
                Log.v(TAG,"Closing log file");

                mLogFile.write("====================================================================" + newline);
                mLogFile.write("Experiment stopped: " +
                               DateFormat.getDateTimeInstance (DateFormat.FULL, DateFormat.FULL).format(new Date()) + newline);
                mLogFile.write("====================================================================" + newline);

                mLogFile.close();
                mLogFile = null;
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        super.onStop();
    }

    private OnClickListener btn_OnClick = new OnClickListener ()
    {
        public void onClick(View v)
        {
            if(v.equals(clients[client_vzw].btnStop))
            {
                // stop
                mVzwProvider.stop();

                // prevent the reportLocation handler to start next request again
                mRestartOnFinal = false;
                synchronized(clients[client_vzw])
                {
                    clients[client_vzw].mNumRemainingSession = 0;
                }
            }
            else if(v.equals(clients[client_vzw].btnRestart))
            {
                // stop and go
                mVzwProvider.stop();
                if(clients[client_vzw].mNumRemainingSession > 0)
                {
                    synchronized(clients[client_vzw])
                    {
                        ++clients[client_vzw].mSessionId;
                        --clients[client_vzw].mNumRemainingSession;
                        clients[client_vzw].mTimestampLastFix = 0;
                    }
                    mVzwProvider.setCredentials(strAppCredential);
                    mVzwProvider.start(clients[client_vzw].mCriteria, clients[client_vzw].mSessionId);
                }
                else
                {
                    // upper limit reached
                }
            }
            else if(v.equals(clients[client_normal].btnStop))
            {
                // stop
                NormalLocationManager.removeUpdates(NormalLocationListener);
            }
            else if(v.equals(clients[client_normal].btnRestart))
            {
                // stop and go
                NormalLocationManager.removeUpdates(NormalLocationListener);
                if (fgRunWithNormalStack && (clients[client_normal].mNumRemainingSession > 0))
                {
                    synchronized(clients[client_normal])
                    {
                        ++clients[client_normal].mSessionId;
                        --clients[client_normal].mNumRemainingSession;
                    }
                    // start
                    NormalLocationManager.requestLocationUpdates(
                        NormalLocationProvider.getName(),
                        //1000*clients[client_normal].mCriteria.getHintNextFixArriveInSec(),
                        TBF_NormalStack,
                        0,
                        NormalLocationListener);
                }
                else
                {
                    // upper limit reached
                }
            }
        }
    };

    private class FSMTestSuite {
        VzwHalCriteria [] criteria = null;
        int current = 0;
        int last = 0;
    }

    private FSMTestSuite fsmTestSuite = new FSMTestSuite();

    private void prepareStateMachineTestSuite (int iSuite)
    {
        if(0 == iSuite)
        {
            // do nothing
        }
        else if (1 == iSuite)
        {
            VzwHalCriteria original = clients[client_vzw].mCriteria;

            fsmTestSuite.current = 0;
            fsmTestSuite.criteria = new VzwHalCriteria[10];
            int i = 0;

            // start with dormant

            fsmTestSuite.criteria[i] = new VzwHalCriteria (original);
            fsmTestSuite.criteria[i].setFixMode(IVzwHalGpsLocationProvider.GPS_POSITION_MODE_STANDALONE);
            fsmTestSuite.criteria[i].setHintNextFixArriveInSec(-1);

            // should be back to dormant, for we do not have hint

            ++i;
            fsmTestSuite.criteria[i] = new VzwHalCriteria (original);
            fsmTestSuite.criteria[i].setHintNextFixArriveInSec(-1);
            fsmTestSuite.criteria[i].setFixMode(IVzwHalGpsLocationProvider.GPS_POSITION_MODE_MS_BASED);

            // should be back to dormant, for we do not have hint

            ++i;
            fsmTestSuite.criteria[i] = new VzwHalCriteria (original);
            fsmTestSuite.criteria[i].setHintNextFixArriveInSec(-1);
            fsmTestSuite.criteria[i].setFixMode(IVzwHalGpsLocationProvider.GPS_POSITION_MODE_MS_ASSISTED);

            // should be back to dormant, for we do not have hint

            ++i;
            fsmTestSuite.criteria[i] = new VzwHalCriteria (original);
            fsmTestSuite.criteria[i].setHintNextFixArriveInSec(-1);
            fsmTestSuite.criteria[i].setFixMode(IVzwHalGpsLocationProvider.GPS_POSITION_MODE_OPTIMAL_SPEED);

            // should be back to dormant, for we do not have hint

            ++i;
            fsmTestSuite.criteria[i] = new VzwHalCriteria (original);
            fsmTestSuite.criteria[i].setHintNextFixArriveInSec(0);
            fsmTestSuite.criteria[i].setFixMode(IVzwHalGpsLocationProvider.GPS_POSITION_MODE_STANDALONE);

            // should be in standalone state, for we have hint
            ++i;
            fsmTestSuite.criteria[i] = new VzwHalCriteria (original);
            fsmTestSuite.criteria[i].setHintNextFixArriveInSec(0);
            fsmTestSuite.criteria[i].setFixMode(IVzwHalGpsLocationProvider.GPS_POSITION_MODE_STANDALONE);

            // should be in standalone state, for we have hint
            ++i;
            fsmTestSuite.criteria[i] = new VzwHalCriteria (original);
            fsmTestSuite.criteria[i].setHintNextFixArriveInSec(0);
            fsmTestSuite.criteria[i].setFixMode(IVzwHalGpsLocationProvider.GPS_POSITION_MODE_MS_ASSISTED);

            // request should be transformed to standalone
            // should be in standalone state, for this is MSA in standalone
            ++i;
            fsmTestSuite.criteria[i] = new VzwHalCriteria (original);
            fsmTestSuite.criteria[i].setHintNextFixArriveInSec(0);
            fsmTestSuite.criteria[i].setFixMode(IVzwHalGpsLocationProvider.GPS_POSITION_MODE_MS_BASED);

            // should be in MSB state
            ++i;
            fsmTestSuite.criteria[i] = new VzwHalCriteria (original);
            fsmTestSuite.criteria[i].setHintNextFixArriveInSec(0);
            fsmTestSuite.criteria[i].setFixMode(IVzwHalGpsLocationProvider.GPS_POSITION_MODE_MS_ASSISTED);

            // request should be transformed to MSB
            // should be in MSB state, for this is MSA in MSB

            // since we should stop after this test, the state should be moved back to dormant

            fsmTestSuite.last = i;
        }
        else if (2 == iSuite)
        {
            VzwHalCriteria original = clients[client_vzw].mCriteria;

            fsmTestSuite.current = 0;
            fsmTestSuite.criteria = new VzwHalCriteria[10];
            int i = 0;

            // start with dormant

            fsmTestSuite.criteria[i] = new VzwHalCriteria (original);
            fsmTestSuite.criteria[i].setHintNextFixArriveInSec(0);
            fsmTestSuite.criteria[i].setFixMode(IVzwHalGpsLocationProvider.GPS_POSITION_MODE_MS_ASSISTED);

            // should be in MSA state, for we have hint

            ++i;
            fsmTestSuite.criteria[i] = new VzwHalCriteria (original);
            fsmTestSuite.criteria[i].setHintNextFixArriveInSec(0);
            fsmTestSuite.criteria[i].setFixMode(IVzwHalGpsLocationProvider.GPS_POSITION_MODE_MS_ASSISTED);

            // should be in MSA state, for we have hint
            ++i;
            fsmTestSuite.criteria[i] = new VzwHalCriteria (original);
            fsmTestSuite.criteria[i].setHintNextFixArriveInSec(0);
            fsmTestSuite.criteria[i].setFixMode(IVzwHalGpsLocationProvider.GPS_POSITION_MODE_OPTIMAL_ACCURACY);

            // should be in MSB state, for this is optimal in MSA
            ++i;
            fsmTestSuite.criteria[i] = new VzwHalCriteria (original);
            fsmTestSuite.criteria[i].setHintNextFixArriveInSec(0);
            fsmTestSuite.criteria[i].setFixMode(IVzwHalGpsLocationProvider.GPS_POSITION_MODE_MS_BASED);

            // should be in MSB state
            // since we should stop after this test, the state should be moved back to dormant

            fsmTestSuite.last = i;
        }
        else
        {
            // do nothing
        }
    }

    private void LoadDefaultCriteria ()
    {
        // criteria with default values
        iMode = 0;
        iNumOfFix = 1;
        iTimeBetweenFix = 1;
        strPdeAddr = "";
        iPdePort = 0;
        iAccuracyH = 50;
        iAccuracyV = 50;
        iMaxResponseTime = 120;
        strAppCredential = "";
    }

    private void LogCriteria ()
    {
        Log.i (TAG, "Mode: " + iMode);
        Log.i (TAG, "num of fix: " + iNumOfFix);
        Log.i (TAG, "time between fix: " + iTimeBetweenFix);
        Log.i (TAG, "PDE addr: " + strPdeAddr);
        Log.i (TAG, "PDE port: " + iPdePort);
        Log.i (TAG, "accuracy H: " + iAccuracyH);
        Log.i (TAG, "accuracy V: " + iAccuracyV);
        Log.i (TAG, "max reponse time: " + iMaxResponseTime);
        Log.i (TAG, "app credential" + strAppCredential);

        if(mLogFile != null)
        {
            try {
                mLogFile.write("Mode: " + iMode + newline);
                mLogFile.write("num of fix: " + iNumOfFix + newline);
                mLogFile.write("time between fix: " + iTimeBetweenFix + newline);
                mLogFile.write("PDE addr: " + strPdeAddr + newline);
                mLogFile.write("PDE port: " + iPdePort + newline);
                mLogFile.write("accuracy H: " + iAccuracyH + newline);
                mLogFile.write("accuracy V: " + iAccuracyV + newline);
                mLogFile.write("max reponse time: " + iMaxResponseTime + newline);
                mLogFile.write("app credential" + strAppCredential + newline);

                mLogFile.flush();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }


    private class VzwCallbackHandler implements IVzwHalGpsCallback {

        public void ReportEngineStatus(int statusCode) {
            Log.i (TAG, "Engine status: " + statusCode);

            synchronized(mLogFile)
            {
                try {
                    mLogFile.write("------------------- phone time " + System.currentTimeMillis()+ " --------------------" + newline);
                    mLogFile.write("[V]Engine status: " + statusCode + newline);

                    mLogFile.flush();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }

            final int statusCode_copy = statusCode;
            mHandler.post(
                new Runnable () {
                    public void run() {
                        TextView textView = (TextView)findViewById(R.id.TextViewEngineStatus);
                        String msg = new String();
                        msg += statusCode_copy;
                        textView.setText(msg);
                    }
                }
                );
        }

        public void ReportGpsStatus(int statusCode) {
            Log.i (TAG, "Gps status: " + statusCode);

            if(statusCode == IVzwHalGpsLocationProvider.ENGINE_STATUS_SESSION_END)
            {
                // session end...
            }

            synchronized(mLogFile)
            {
                try {
                    mLogFile.write("------------------- phone time " + System.currentTimeMillis()+ " --------------------" + newline);
                    mLogFile.write("[V]GPS status: " + statusCode + newline);

                    mLogFile.flush();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }

            final int statusCode_copy = statusCode;
            mHandler.post(
                new Runnable () {
                    public void run() {
                        TextView textView = (TextView)findViewById(R.id.TextViewGpsStatus);
                        String msg = new String();
                        msg += statusCode_copy;
                        textView.setText(msg);
                    }
                }
                );
        }

        public void ReportLocation(VzwHalLocation location) {

            final VzwHalLocation location_copy = new VzwHalLocation(location);

            Log.i (TAG, "session id: " + location.getSessionId());
            Log.i (TAG, "pos: " + location.getLatitude() + ", " + location.getLongitude());

            LogLocationReport(location);

            final ClientCard current_client = clients[client_vzw];
            final int current_session_id = current_client.mSessionId;

            if(location.getSessionId() != current_session_id)
            {
                Log.w (TAG, "session id mismatch");
                return;
            }

            long now = System.currentTimeMillis();
            long lastTimestamp = 0;
            boolean first_fix = true;

            synchronized(current_client)
            {
                lastTimestamp = current_client.mTimestampLastFix;
                first_fix = (lastTimestamp == 0);
                --current_client.mNumRemainingSession;
                current_client.mTimestampLastFix = now;

                final long diff = (now - lastTimestamp);
                if(!first_fix)
                {
                    // only update TTF if we are not dealing with the first fix
                    mHandler.post(
                        new Runnable () {
                            public void run() {
                                current_client.textViewTTF.setText(String.format("%d", diff));
                            }
                        });
                }

                if(mRestartOnFinal && (current_client.mNumRemainingSession > 0))
                {
                    ++current_client.mSessionId;

                    // compensate for the fix time
                    long delay_time_msec = current_client.mCriteria.getHintNextFixArriveInSec()*1000;

                    Log.i (TAG, "delay for " + current_client.mCriteria.getHintNextFixArriveInSec() + " sec");

                    if(1 == current_client.mNumRemainingSession)
                    {
                        // remove the hint if this is the last fix request
                        current_client.mCriteria.setHintNextFixArriveInSec(-1);
                    }

                    mHandler.postDelayed(
                        new Runnable () {
                            public void run() {
                                try{
                                    Log.i (TAG, "wake up from delayed execution");

                                    if(iRunStateMachineTestSuite > 0)
                                    {
                                        if(fsmTestSuite.current < fsmTestSuite.last)
                                        {
                                            ++fsmTestSuite.current;
                                            mVzwProvider.setCredentials(strAppCredential);
                                            mVzwProvider.start(fsmTestSuite.criteria[fsmTestSuite.current], current_client.mSessionId);
                                        }
                                    }
                                    else
                                    {
                                        mVzwProvider.setCredentials(strAppCredential);
                                        mVzwProvider.start(current_client.mCriteria, current_client.mSessionId);
                                    }
                                }
                                catch(RuntimeException e)
                                {
                                    e.printStackTrace();
                                }
                            }
                        },
                        delay_time_msec);
                }
                else
                {
                    Log.i (TAG, "sending STOP to engine");
                    mVzwProvider.stop();
                }


                // note: this runs in the main Activity/UI thread (not in the call back thread)
                // modifying client state is not a good idea...
                mHandler.post(
                    new Runnable () {
                        public void run() {
                            if(current_client != null)
                            {
                                String msg;
                                msg = String.format("%1.3f, %1.3f", location_copy.getLatitude(), location_copy.getLongitude());
                                current_client.textViewPosition.setText(msg);

                                msg = String.format("%d", location_copy.getSessionId());
                                if(location_copy.getSessionId() != current_session_id)
                                {
                                    msg += "!";
                                }
                                current_client.textViewSessionId.setText(msg);

                                msg = String.format("%d", current_client.mCriteria.getFixMode());
                                current_client.textViewFixMode.setText(msg);

                                current_client.textViewFixType.setText("Final");
                            }
                        }
                    }
                    );
            }
        }

        public void ReportSvStatus(VzwHalSvInfo svSvInfo) {

            LogSvInfo (svSvInfo);
        }

    }

    private void hookupVzwProvider()
    {
        VzwHalCriteria c = new VzwHalCriteria ();

        c.setAltitudeRequired(true);
        c.setCostAllowed(true);
        c.setSpeedRequired(false);
        c.setFixMode((int)iMode);

        c.setHintNextFixArriveInSec(iTimeBetweenFix);
        c.setHintNextFixMode((int)iMode);
        c.setHintNextFixHorizontalAccuracy(iAccuracyH);

        c.setMaximumResponseTime(iMaxResponseTime);
        clients[client_vzw].mCriteria = c;

        clients[client_vzw].mSessionId = 1;
        clients[client_vzw].mNumRemainingSession = iNumOfFix;
    }

    private void hookupNormalProvider()
    {
        NormalLocationManager = (LocationManager) getSystemService(LOCATION_SERVICE);

        if(NormalLocationManager != null)
        {
            List<String> strProviders = NormalLocationManager.getProviders(true);
            Log.i (TAG, "Normal providers begin ");
            if(strProviders != null)
            {
                for(String name : strProviders)
                {
                    Log.i (TAG, "Normal providers: " + name);
                }
            }
            Log.i (TAG, "Normal providers end");

            NormalLocationProvider = NormalLocationManager.getProvider(LocationManager.GPS_PROVIDER);
            if(NormalLocationProvider != null)
            {
                Log.i (TAG, "Normal provider name: " + NormalLocationProvider.getName());

                NormalLocationManager.addGpsStatusListener (NormalGpsStatusListener);
                //NormalLocationManager.addNmeaListener(NormalNmeaListener);
                //NormalLocationManager.requestLocationUpdates(NormalLocationProvider.getName(), 0, 0,NormalLocationListener);
            }
            else
            {
                Log.w (TAG, "Normal gps provider doesn't exist");
            }
        }
        else
        {
            Log.w (TAG, "Normal manager doesn't exist");
        }
    }

    private LocationListener NormalLocationListener = new LocationListener() {
        public void onLocationChanged(Location location) {
            NormalGpsLastReportedLocation = location;
            Log.i (TAG, "Normal location update: " + location.getLatitude() + "," + location.getLongitude());

            LogLocationReport(location);

            final ClientCard current_client = clients[client_normal];
            final Location location_copy = new Location(location);
            final int current_session_id = current_client.mSessionId;
            ++current_client.mSessionId;
            --current_client.mNumRemainingSession;

            long now = System.currentTimeMillis();
            final long diff = (now - current_client.mTimestampLastFix);
            final boolean first_fix = (current_client.mTimestampLastFix == 0);

            current_client.mTimestampLastFix = now;

            if(current_client.mNumRemainingSession <= 0)
            {
                // stop
                NormalLocationManager.removeUpdates(NormalLocationListener);
            }

            mHandler.post(
                new Runnable () {
                    public void run() {
                        if(current_client != null)
                        {
                            String msg;
                            msg = String.format("%1.3f, %1.3f", location_copy.getLatitude(), location_copy.getLongitude());
                            current_client.textViewPosition.setText(msg);

                            if(!first_fix)
                            {
                                current_client.textViewTTF.setText(String.format("%d", diff));
                            }

                            msg = String.format("%d", current_session_id);
                            current_client.textViewSessionId.setText(msg);
                        }
                    }
                }
                );
        }

        public void onStatusChanged(String provider, int status,
                                    Bundle extras) {
            Log.i (TAG, "Normal status update: " + status);

            synchronized(mLogFile)
            {
                try {
                    mLogFile.write("------------------- phone time " + System.currentTimeMillis()+ " --------------------" + newline);
                    mLogFile.write("[N]Location status event: " + status + newline);

                    mLogFile.flush();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }

        public void onProviderDisabled(String provider) {
            Log.i (TAG, "Normal provider disabled: " + provider);
        }

        public void onProviderEnabled(String provider) {
            Log.i (TAG, "Normal provider re-enabled: " + provider);
        }
    };

    private GpsStatus.Listener NormalGpsStatusListener = new GpsStatus.Listener () {

        public void onGpsStatusChanged(int event) {
            NormalGpsStatus = NormalLocationManager.getGpsStatus(null);
            NormalGpsEngineStatus = event;
            timestampLastGpsStatusUpdate = System.currentTimeMillis();
            Log.i (TAG, "Normal GPS status: " + event);

            synchronized(mLogFile)
            {
                try {
                    mLogFile.write("------------------- phone time " + System.currentTimeMillis()+ " --------------------" + newline);
                    mLogFile.write("[N]GPS status event: " + event + newline);

                    mLogFile.flush();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    };
    /*
      private GpsStatus.NmeaListener NormalNmeaListener = new GpsStatus.NmeaListener () {

      public void onNmeaReceived(long timestamp, String nmea) {
      timestampLastNmeaUpdate = timestamp;
      NormalGpsLastNmea = nmea;
      Log.i ("QVTester_Tracking", "Normal nmea: " + nmea);
      }

      };
    */

    private synchronized void LogLocationReport (Location location) {
        VzwHalLocation vzwLocation = null;
        if(location instanceof VzwHalLocation)
        {
            vzwLocation = (VzwHalLocation)location;
        }

        int mask = 0;
        String strSource;
        if(vzwLocation == null)
        {
            strSource = "[N] ";
            // in standard location, lat/lon is always valid
            mask = (VzwHalLocation.GPS_VALID_LATITUDE | VzwHalLocation.GPS_VALID_LONGITUDE);
        }
        else
        {
            strSource = "[V] ";
            mask = vzwLocation.getValidFieldMask();
        }

        synchronized(mLogFile)
        {
            try {

                mLogFile.write("------------------- phone time " + System.currentTimeMillis()+ " --------------------" + newline);

                if(vzwLocation!= null)
                {
                    mLogFile.write(strSource + "session id: " + vzwLocation.getSessionId() + newline);
                }

                if((mask & VzwHalLocation.GPS_VALID_LATITUDE) != 0)
                {
                    mLogFile.write(strSource + "Latitude: " + location.getLatitude() + newline);
                }
                else
                {
                    mLogFile.write(strSource + "Latitude: invalid" + newline);
                }

                if((mask & VzwHalLocation.GPS_VALID_LONGITUDE) != 0)
                {
                    mLogFile.write(strSource + "Longitude: " + location.getLongitude() + newline);
                }
                else
                {
                    mLogFile.write(strSource + "Longitude: invalid" + newline);
                }

                if(location.hasAltitude())
                {
                    mLogFile.write(strSource + "Altitude w.r.t WGS84: " + location.getAltitude() + newline);
                }
                else
                {
                    mLogFile.write(strSource + "Altitude w.r.t WGS84: invalid" + newline);
                }

                if((mask & VzwHalLocation.GPS_VALID_ALTITUDE_WRT_SEA_LEVEL) != 0)
                {
                    mLogFile.write(strSource + "Altitude w.r.t MSL: " + vzwLocation.getAltitudeWrtSeaLevel() + newline);
                }
                else
                {
                    mLogFile.write(strSource + "Altitude w.r.t MSL: invalid" + newline);
                }


                if((mask & VzwHalLocation.GPS_VALID_VERTICAL_ACCURACY) != 0)
                {
                    mLogFile.write(strSource + "Altitude accuracy: " + vzwLocation.getVerticalAccuracy() + newline);
                }
                else
                {
                    mLogFile.write(strSource + "Altitude accuracy: invalid" + newline);
                }

                if(location.hasAccuracy())
                {
                    mLogFile.write(strSource + "Circular Accuracy: " + location.getAccuracy() + newline);
                }
                else
                {
                    mLogFile.write(strSource + "Circular Accuracy: invalid" + newline);
                }

                if((mask & VzwHalLocation.GPS_VALID_ELLIPTICAL_ACCURACY) != 0)
                {
                    mLogFile.write(strSource + "Elliptical Accuracy-Major: " + vzwLocation.getMajorAxis() + newline);
                    mLogFile.write(strSource + "Elliptical Accuracy-Minor: " + vzwLocation.getMinorAxis() + newline);
                    mLogFile.write(strSource + "Elliptical Accuracy-Angle: " + vzwLocation.getMajorAxisAngle() + newline);
                }
                else
                {
                    mLogFile.write(strSource + "Elliptical Accuracy: invalid" + newline);
                }

                if((mask & VzwHalLocation.GPS_VALID_HORIZONTAL_CONFIDENCE) != 0)
                {
                    mLogFile.write(strSource + "Elliptical Accuracy-Confidence: " + vzwLocation.getHorizontalConfidence() + newline);
                }
                else
                {
                    mLogFile.write(strSource + "Elliptical Accuracy-Confidence: invalid" + newline);
                }

                if((mask & VzwHalLocation.GPS_VALID_SATELLITES_USED_PRNS)!= 0)
                {
                    int [] prnArray = vzwLocation.getSatellitesUsedPRN();
                    mLogFile.write(strSource + "Number of SV used: " + prnArray.length + newline);

                    String strPrn = new String();
                    for( int prn: prnArray )
                    {
                        strPrn += prn;
                        strPrn += ',';
                    }
                    mLogFile.write(strSource + "SV used: \t\t" + strPrn + newline);
                }
                else
                {
                    mLogFile.write(strSource + "Number of SV used : invalid" + newline);
                    mLogFile.write(strSource + "SV used : invalid" + newline);
                }

                if((mask & VzwHalLocation.GPS_VALID_POSITION_DILUTION_OF_PRECISION) != 0)
                {
                    mLogFile.write(strSource + "PDOP: " + vzwLocation.getPositionDilutionOfPrecision() + newline);
                }
                else
                {
                    mLogFile.write(strSource + "PDOP: invalid" + newline);
                }

                if((mask & VzwHalLocation.GPS_VALID_HORIZONTAL_DILUTION_OF_PRECISION) != 0)
                {
                    mLogFile.write(strSource + "HDOP: " + vzwLocation.getHorizontalDilutionOfPrecision() + newline);
                }
                else
                {
                    mLogFile.write(strSource + "HDOP: invalid" + newline);
                }

                if((mask & VzwHalLocation.GPS_VALID_VERTICAL_DILUTION_OF_PRECISION) != 0)
                {
                    mLogFile.write(strSource + "VDOP: " + vzwLocation.getVerticalDilutionOfPrecision() + newline);
                }
                else
                {
                    mLogFile.write(strSource + "VDOP: invalid" + newline);
                }

                if((mask & VzwHalLocation.GPS_VALID_SATELLITES_IN_VIEW_PRNS)!= 0)
                {
                    int [] PRN = vzwLocation.getSatellitesInViewPRNs();
                    int countSvInView = PRN.length;
                    mLogFile.write(strSource + "Number of SV in View: " + countSvInView + newline);
                    mLogFile.write(strSource + "PRN,\tAzimuth,\tElevation,\tSN Ratio" + newline);

                    float[] Azimuth = vzwLocation.getSatellitesInViewAzimuth();
                    float[] Elevation = vzwLocation.getSatellitesInViewElevation();
                    float[] SNR = vzwLocation.getSatellitesInViewSignalToNoiseRatio();

                    for(int i = 0; i < countSvInView; ++i )
                    {
                        String strSv = new String();
                        if((mask & vzwLocation.GPS_VALID_SATELLITES_IN_VIEW_PRNS)!= 0)
                        {
                            strSv += PRN[i];
                        }
                        else
                        {
                            strSv += "x";
                        }
                        strSv += ",\t";

                        if((mask & vzwLocation.GPS_VALID_SATELLITES_IN_VIEW_AZIMUTH)!= 0)
                        {
                            strSv += Azimuth[i];
                        }
                        else
                        {
                            strSv += "x";
                        }
                        strSv += ",\t";

                        if((mask & vzwLocation.GPS_VALID_SATELLITES_IN_VIEW_ELEVATION)!= 0)
                        {
                            strSv += Elevation[i];
                        }
                        else
                        {
                            strSv += "x";
                        }
                        strSv += ",\t";

                        if((mask & vzwLocation.GPS_VALID_SATELLITES_IN_VIEW_SIGNAL_TO_NOISE_RATIO)!= 0)
                        {
                            strSv += SNR[i];
                        }
                        else
                        {
                            strSv += "x";
                        }
                        mLogFile.write(strSource + strSv + newline);
                    }
                }

                if((mask & VzwHalLocation.GPS_VALID_MAGNETIC_VARIATION) != 0)
                {
                    mLogFile.write(strSource + "Magnetic Variation: " + vzwLocation.getMagneticVariation() + newline);
                }
                else
                {
                    mLogFile.write(strSource + "Magnetic Variation: invalid" + newline);
                }

                if((mask & VzwHalLocation.GPS_VALID_TIME) != 0)
                {
                    mLogFile.write(strSource + "Timestamp: " + vzwLocation.getTime() + newline);
                }
                else if (null == vzwLocation) {
                    // this is from a normal stack
                    mLogFile.write(strSource + "Timestamp: " + location.getTime() + newline);
                }
                else {
                    mLogFile.write(strSource + "Timestamp: invalid value:(" + vzwLocation.getTime() + ")" + newline);
                }

                if((mask & VzwHalLocation.GPS_VALID_FIX_MODE) != 0)
                {
                    mLogFile.write(strSource + "Fix Mode: " + vzwLocation.getFixMode() + newline);
                }
                else
                {
                    mLogFile.write(strSource + "Fix Mode: invalid" + newline);
                }

                if(location.hasBearing())
                {
                    mLogFile.write(strSource + "Bearing: " + location.getBearing() + newline);
                }
                else
                {
                    mLogFile.write(strSource + "Bearing: invalid" + newline);
                }

                if(location.hasSpeed())
                {
                    mLogFile.write(strSource + "Speed: " + location.getSpeed() + newline);
                }
                else
                {
                    mLogFile.write(strSource + "Speed: invalid" + newline);
                }

                // flush the file, so we get more up-to-date log before we actually close it
                mLogFile.flush();

            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    private void LogSvInfo(VzwHalSvInfo svSvInfo)
    {
        synchronized(mLogFile)
        {
            try {
                mLogFile.write("------------------- phone time " + System.currentTimeMillis()+ " --------------------" + newline);

                String strSource = "[V] ";
                int mask = svSvInfo.getValidFieldMask();

                int countSvInView = 0;
                if((mask & VzwHalSvInfo.GPS_VALID_SATELLITES_IN_VIEW_COUNT)!= 0)
                {
                    countSvInView = svSvInfo.getNumSatellitesInView();
                    mLogFile.write(strSource + "Number of SV in View: " + countSvInView + newline);
                    mLogFile.write(strSource + "PRN,\tAzimuth,\tElevation,\tSN Ratio" + newline);

                    float[] Azimuth = svSvInfo.getSatellitesInViewAzimuth();
                    int [] PRN = svSvInfo.getSatellitesInViewPRNs();
                    float[] Elevation = svSvInfo.getSatellitesInViewElevation();
                    float[] SNR = svSvInfo.getSatellitesInViewSignalToNoiseRatio();

                    for(int i = 0; i < countSvInView; ++i )
                    {
                        String strSv = new String();
                        if((mask & VzwHalSvInfo.GPS_VALID_SATELLITES_IN_VIEW_PRNS)!= 0)
                        {
                            strSv += PRN[i];
                        }
                        else
                        {
                            strSv += "x";
                        }
                        strSv += ",\t";

                        if((mask & VzwHalSvInfo.GPS_VALID_SATELLITES_IN_VIEW_AZIMUTH)!= 0)
                        {
                            strSv += Azimuth[i];
                        }
                        else
                        {
                            strSv += "x";
                        }
                        strSv += ",\t";

                        if((mask & VzwHalSvInfo.GPS_VALID_SATELLITES_IN_VIEW_ELEVATION)!= 0)
                        {
                            strSv += Elevation[i];
                        }
                        else
                        {
                            strSv += "x";
                        }
                        strSv += ",\t";

                        if((mask & VzwHalSvInfo.GPS_VALID_SATELLITES_IN_VIEW_SIGNAL_TO_NOISE_RATIO)!= 0)
                        {
                            strSv += SNR[i];
                        }
                        else
                        {
                            strSv += "x";
                        }
                        mLogFile.write(strSource + strSv + newline);
                    }
                }
                else
                {
                    mLogFile.write(strSource + "Number of SV in View: invalid" + newline);
                }

                if((mask & VzwHalSvInfo.GPS_VALID_SATELLITES_WITH_EPHEMERIS)!= 0)
                {
                    int [] SvArray = svSvInfo.getSatellitesWithEphemeris();

                    String strPrn = new String();
                    for( int i: SvArray )
                    {
                        strPrn += i;
                        strPrn += ',';
                    }
                    mLogFile.write(strSource + "SV with ephemeris: \t" + strPrn + newline );
                }
                else
                {
                    mLogFile.write(strSource + "SV with ephemeris: invalid" + newline);
                }

                if((mask & VzwHalSvInfo.GPS_VALID_SATELLITES_WITH_ALMANAC)!= 0)
                {
                    int [] SvArray = svSvInfo.getSatellitesWithAlmanac();

                    String strPrn = new String();
                    for( int i: SvArray )
                    {
                        strPrn += i;
                        strPrn += ',';
                    }
                    mLogFile.write(strSource + "SV with almanac: \t" + strPrn  + newline);
                }
                else
                {
                    mLogFile.write(strSource + "SV with almanac: invalid" + newline);
                }

                // flush the file, so we get more up-to-date log before we actually close it
                mLogFile.flush();

            } catch (IOException e) {
                e.printStackTrace();
            }
        }

    }

}
