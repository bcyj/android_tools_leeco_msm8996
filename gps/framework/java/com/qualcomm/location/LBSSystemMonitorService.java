/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2012-2014 Qualcomm Atheros, Inc.
  All Rights Reserved.
  Qualcomm Atheros Confidential and Proprietary.
=============================================================================*/

package com.qualcomm.location;

import java.lang.ref.WeakReference;
import java.util.ArrayList;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.os.SystemProperties;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.util.Log;

import java.io.File;
import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.util.Properties;
import java.io.IOException;
import java.io.BufferedReader;
import java.io.FileReader;

public class LBSSystemMonitorService extends Service implements MonitorInterface {
    private static final String TAG = "LBSSystemMonitorService";
    private static final boolean VERBOSE_DBG = Log.isLoggable(TAG, Log.VERBOSE);
    private HandlerThread mHandlerThread;
    private ArrayList<Monitor> mMonitors;
    private MyHandler mHandler;
    private Properties mProperties;
    private Properties mProperties_sglte;
    private Properties mProperties_ES_SUPL;
    private static final String PROPERTIES_FILE = "/etc/izat.conf";
    private static final String PROPERTIES_FILE_GPS = "/etc/gps.conf";
    private static final int LISTENER_FLAG_BIT_MAX = 15;

    static final int SGLTE_NO_ES_SUPL = 0;
    static final int SGLTE_WITH_ES_SUPL = 1;
    static final int NON_SGLTE_WITH_ES_SUPL = 2;

    @Override
    public void onCreate() {
        mMonitors = new ArrayList<Monitor>();
        if (VERBOSE_DBG)
            Log.v(TAG, "onCreate()");
        mHandlerThread = new HandlerThread(TAG);
        mHandlerThread.start();
        mHandler = new MyHandler(mHandlerThread.getLooper(), this);

        mProperties = new Properties();
        try {
            File file = new File(PROPERTIES_FILE);
            FileInputStream stream = new FileInputStream(file);
            mProperties.load(stream);
            stream.close();
        } catch (IOException e) {
            Log.e(TAG, "Could not open IZAT configuration file or " + PROPERTIES_FILE);
        }

        mProperties_sglte = new Properties();
        try {
            File file_2 = new File(PROPERTIES_FILE_GPS);
            FileInputStream stream_2 = new FileInputStream(file_2);
            mProperties_sglte.load(stream_2);
            stream_2.close();
        } catch (IOException e) {
            Log.e(TAG, "Could not open gps configuration file " + PROPERTIES_FILE_GPS);
        }

        String mListenerFlag;
        mListenerFlag = mProperties.getProperty("NLP_WIFI_LISTENER_MODE");

        String str_sglte;
        str_sglte = mProperties_sglte.getProperty("SGLTE_TARGET");
        int isSGLTE = 0;
        if (str_sglte != null) {
            try {
                isSGLTE = Integer.parseInt(str_sglte.trim());
                if(VERBOSE_DBG)
                    Log.d(TAG, "isSGLTE is" + isSGLTE);
            } catch(NumberFormatException e) {
                if(VERBOSE_DBG)
                    Log.e(TAG, "Unable to parse SGLTE_TARGET");
            }
        } else {
            if(VERBOSE_DBG)
                Log.e(TAG, "SGLTE_TARGET is not defined in gps.conf");
        }

        int es_supl_bit = 0;
        mProperties_ES_SUPL = new Properties();
        try {
            File file = new File(PROPERTIES_FILE_GPS);
            FileInputStream stream = new FileInputStream(file);
            mProperties_ES_SUPL.load(stream);
            stream.close();

            // read the SUPL_ES form gps.conf
            String isSuplESEnabled = mProperties_ES_SUPL.getProperty("SUPL_ES");
            if (isSuplESEnabled != null) {
                try {
                    es_supl_bit = Integer.parseInt(isSuplESEnabled.trim());
                    //GpsNetInitiatedHandler.obj.recordES_SUPLBit(es_supl_bit);
                    Log.d(TAG, "SUPL_ES is: " + es_supl_bit);
                } catch (NumberFormatException e) {
                    Log.e(TAG, "unable to parse SUPL_ES: " + isSuplESEnabled);
                }
            } else {
                Log.e(TAG, "unable to read SUPL_ES from" + PROPERTIES_FILE);
            }
        } catch (IOException e) {
            Log.e(TAG, "Could not open GPS configuration file " + PROPERTIES_FILE);
        }

        int msgBase = 1;
        Monitor m;
        int wiperFlag = 0;
        // we start the Monitors here
        // keep wiper be the first to load
        if(mListenerFlag != null){
            try {
                wiperFlag = Integer.parseInt(mListenerFlag);
            } catch(NumberFormatException e) {
                if(VERBOSE_DBG)
                    Log.e(TAG, "Unable to parse nlp listener flag");
            }

            if(wiperFlag > 0 && wiperFlag <= LISTENER_FLAG_BIT_MAX) {
                synchronized(mMonitors) {
                    m = new Wiper(this, msgBase, wiperFlag);
                    subscribe(m);
                    msgBase += m.getNumOfMessages();
                }
            }
        }

        String str=SystemProperties.get("ro.baseband", "");

        if ( str.equals("sglte")||str.equals("sglte2")||(isSGLTE == 1)){
            Log.w(TAG, "Starting RilInfoMonitor for SGLTE device.");
            synchronized(mMonitors) {
                if (es_supl_bit == 0) {
                    Log.d(TAG, "SGLTE without ES SUPL");
                    // SGLTE without ES SUPL
                    m = new RilInfoMonitor(this, msgBase, SGLTE_NO_ES_SUPL);
                } else {
                    Log.d(TAG, "SGLTE with ES SUPL");
                    // SGLTE with ES SUPL
                    m = new RilInfoMonitor(this, msgBase, SGLTE_WITH_ES_SUPL);
                }

                subscribe(m);
                msgBase += m.getNumOfMessages();
            }
        } else {
            if (es_supl_bit == 1) {
                Log.d(TAG, "nonSGLTE device with ES SUPL");
                // nonSGLTE device with ES SUPL
                m = new RilInfoMonitor(this, msgBase, NON_SGLTE_WITH_ES_SUPL);
                subscribe(m);
                msgBase += m.getNumOfMessages();
            } else {
                Log.d(TAG, "nonSGLTE device without ES SUPL");
            }
        }

        synchronized(mMonitors) {
            m = new DeviceContext(this, msgBase);
            subscribe(m);
            msgBase += m.getNumOfMessages();
        }
        // end of adding monitors
    }

    @Override
    public void onDestroy() {
        Log.v(TAG, "onDestroy");
        synchronized (mHandlerThread) {
            if (mHandlerThread != null) {
                mHandlerThread.quit();
                mHandlerThread = null;
            }
        }
        // the process is going away. We shouldn't need to do anything else.
        // it is expected that QMI notifies modem. And the rest will be handled
        // by locMW.
    }

    private class MyHandler extends Handler {
        private final WeakReference<LBSSystemMonitorService> mService;

        private MyHandler(Looper looper, LBSSystemMonitorService ms) {
            super(looper);
            mService = new WeakReference<LBSSystemMonitorService>(ms);
        }

        @Override
        public final void handleMessage(Message msg) {
            int msgID = msg.what;
            Log.d(TAG, "handleMessage what - " + msgID);

            if (mService.get() != null) {
                Monitor monitorHandler = null;
                synchronized (mMonitors) {
                    for (Monitor monitor : mMonitors) {
                        int handlerStart = monitor.getMsgIdBase();
                        int handlerEnd = handlerStart + monitor.getNumOfMessages();
                        if (msgID < handlerEnd && msgID >= handlerStart) {
                            // rebase the message id, so that each monitor
                            // could handle msgs with 0 based ids. This
                            // enables the use of switch
                            msg.what -= handlerStart;
                            monitorHandler = monitor;
                            break;
                        }
                    }
                }
                // handle message outside mMonitors lock
                if (monitorHandler != null) {
                    monitorHandler.handleMessage(msg);
                }
            }
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public Context getContext() {
        return this;
    }

    @Override
    public Handler getHandler() {
        return mHandler;
    }

    @Override
    public void subscribe(Monitor m) {
        synchronized(mMonitors) {
            mMonitors.add(m);
        }
    }

    @Override
    public void unsubscribe(Monitor m) {
        synchronized(mMonitors) {
            mMonitors.remove(m);
        }
    }
}
