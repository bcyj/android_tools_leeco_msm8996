/******************************************************************************
  @file    EngineSimulator.java
  @brief   engine simulator for VZW GPS Location Provider

  DESCRIPTION

  just for test. not used in production code

  ---------------------------------------------------------------------------
  Copyright (C) 2010 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
 ******************************************************************************/

package com.qualcomm.location.vzw_library.imp;

import java.util.LinkedList;

import com.qualcomm.location.vzw_library.IVzwHalGpsCallback;
import com.qualcomm.location.vzw_library.IVzwHalGpsLocationProvider;
import com.qualcomm.location.vzw_library.VzwHalCriteria;
import com.qualcomm.location.vzw_library.VzwHalLocation;

import android.util.Log;

class EngineSimulator implements ILocationEngine  {

    // note Log ahs 23-character limit on TAG
    private static final String TAG = "VzwHalEngineSim";

    public enum EngineState {UNKNOWN, IDLE, IDLE_START_REQ, RECV, RECV_STOP_REQ, FINAL_SENT, TERM_REQ}

    private LinkedList<Runnable> mCallbackQueue = new LinkedList<Runnable> ();

    private EngineSimulatorThread mThread = new EngineSimulatorThread();

    public EngineSimulator () {
    }

    // this thread is to detach the callback and actual engine running.
    // so it's save to call engine from the callback handler
    // this is closer to actual driver implementation
    private class CallbackThread extends Thread {

        public boolean fgContinue =true;

        private void waitCallback () {
            try {
                synchronized (mCallbackQueue)
                {
                    while(!mCallbackQueue.isEmpty())
                    {
                        Runnable piece = mCallbackQueue.removeFirst();
                        piece.run();
                    }
                    mCallbackQueue.wait();
                }
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void run() {
            while(true)
            {
                if(fgContinue)
                {
                    waitCallback();
                }
            }
        }
    }

    private class EngineSimulatorThread extends Thread {

        //private static final int TIME_SEC_WAIT_FOR_MSS = 10;
        private static final int TIME_SEC_WAIT_FOR_MSA = 5;
        //private static final int TIME_SEC_WAIT_FOR_MSB = 7;
    //  private static final int TIME_SEC_WAIT_FOR_AFLT = 2;
        //private static final int TIME_SEC_BETWEEN_FINAL_AND_SESSION_END = 2;

        private EngineState mState = EngineState.UNKNOWN;
        private int mMode;
        private IVzwHalGpsCallback mCallback;
        private CallbackThread mCallbackThread = new CallbackThread();

        private double mLat = 1.2345;
        private double mLon = 4.5678;


        private synchronized EngineState getEngineState () {
            return mState;
        }

        private synchronized void state_idle () {
            try {
                wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        private synchronized void state_recv_req () {
            mState = EngineState.RECV;
        }

        private void state_recv_stop_req () {
            // final report
            VzwHalLocation location = new VzwHalLocation();

            // callback must happen outside of this thread, to 1) better emulate the native behavior,
            // and 2) avoid reentrant from the upper layer callback handler
            synchronized(mCallbackQueue)
            {
                final VzwHalLocation location_copy = new VzwHalLocation(location);
                mCallbackQueue.addLast(new Runnable () {
                    public void run() {
                        mCallback.ReportLocation(location_copy);
                    }
                });
            }
            mCallbackQueue.notify();

            synchronized(this)
            {
                mState = EngineState.FINAL_SENT;
                try {
                    wait(200);
                } catch (InterruptedException e) {
                }
                // session end
                mState = EngineState.IDLE;
            }

            // callback must happen outside of this thread, to 1) better emulate the native behavior,
            // and 2) avoid reentrant from the upper layer callback handler
            synchronized(mCallbackQueue)
            {
                mCallbackQueue.addLast(new Runnable () {
                    public void run() {
                        mCallback.ReportEngineStatus(IVzwHalGpsLocationProvider.ENGINE_STATUS_SESSION_END);
                    }
                });
            }
            mCallbackQueue.notify();

        }

        private void state_recv () {
            try {

                // callback must happen outside of this thread, to 1) better emulate the native behavior,
                // and 2) avoid reentrant from the upper layer callback handler
                synchronized(mCallbackQueue)
                {
                    mCallbackQueue.addLast(new Runnable () {
                        public void run() {
                            mCallback.ReportEngineStatus(IVzwHalGpsLocationProvider.ENGINE_STATUS_SESSION_BEGIN);
                        }
                    });
                    mCallbackQueue.notify();
                }

                for(int i = 0; i < TIME_SEC_WAIT_FOR_MSA; ++i)
                {
                    //VzwHalLocation location = new VzwHalLocation();
                    synchronized(this)
                    {
                        wait (200);

                        // intermediate report
                        /*
                        location.setFinalFix(false);
                        location.setLatitude(mLat);
                        location.setLongitude(mLon);
                        location.setSessionId(mSessionId);
                        mLat += 0.001;
                        mLon += 0.001;
                        */
                    }

                    // callback must happen outside of this thread, to 1) better emulate the native behavior,
                    // and 2) avoid reentrant from the upper layer callback handler
                    /*
                    synchronized(mCallbackQueue)
                    {
                        final VzwHalLocation location_copy = new VzwHalLocation(location);
                        mCallbackQueue.addLast(new Runnable () {
                            @Override
                            public void run() {
                                mCallback.ReportLocationSvStatus(location_copy);
                            }
                        });
                        mCallbackQueue.notify();
                    }
                    */
                }

                VzwHalLocation location = new VzwHalLocation();
                synchronized(this)
                {
                    // final report
                    location.setLatitude(mLat);
                    location.setLongitude(mLon);
                    location.setSessionId(mSessionId);
                                        location.setFixMode(mMode);

                    mLat += 0.001;
                    mLon += 0.001;
                }


                synchronized(this)
                {
                    //mState = EngineState.FINAL_SENT;
                    //wait(200);
                    mState = EngineState.IDLE;
                }

                // callback must happen outside of this thread, to 1) better emulate the native behavior,
                // and 2) avoid reentrant from the upper layer callback handler
                synchronized(mCallbackQueue)
                {
                    final VzwHalLocation location_copy = new VzwHalLocation(location);
                    mCallbackQueue.addLast(new Runnable () {
                        public void run() {
                            mCallback.ReportLocation(location_copy);
                        }
                    });
                    mCallbackQueue.notify();
                }


                // callback must happen outside of this thread, to 1) better emulate the native behavior,
                // and 2) avoid reentrant from the upper layer callback handler
                synchronized(mCallbackQueue)
                {
                    mCallbackQueue.addLast(new Runnable () {
                        public void run() {
                            mCallback.ReportEngineStatus(IVzwHalGpsLocationProvider.ENGINE_STATUS_SESSION_END);
                        }
                    });
                    mCallbackQueue.notify();
                }

            } catch (InterruptedException e) {

            }
        }

        public void run() {
            boolean fgContinue = true;
            mCallbackThread.start();

            // callback must happen outside of the lock, to avoid reentrant from the
            // upper layer callback handler
            //mCallback.ReportEngineStatus(VzwGpsLocationProvider.ENGINE_STATUS_ENGINE_ON);
            while(fgContinue) {
                switch(getEngineState()) {
                case IDLE:
                    state_idle();
                    break;
                case IDLE_START_REQ:
                    state_recv_req ();
                    break;
                case RECV:
                    state_recv();
                    break;
                case RECV_STOP_REQ:
                    state_recv_stop_req ();
                    break;
                case TERM_REQ:
                default:
                    // callback must happen outside of this thread, to 1) better emulate the native behavior,
                    // and 2) avoid reentrant from the upper layer callback handler
                    synchronized(mCallbackQueue)
                    {
                        mCallbackQueue.addLast(new Runnable () {
                            public void run() {
                                mCallback.ReportEngineStatus(IVzwHalGpsLocationProvider.ENGINE_STATUS_ENGINE_OFF);
                                mCallbackThread.fgContinue = false;
                            }
                        });
                        mCallbackQueue.notify();
                    }
                    fgContinue = false;
                    break;
                }
            }
        }

        private int mSessionId;

        public synchronized void cleanup() {
            mState = EngineState.TERM_REQ;
            notifyAll();
        }

        public synchronized boolean init() {
            mState = EngineState.IDLE;
            start();
            return true;
        }

        public synchronized void setCallbackInterface(IVzwHalGpsCallback callback) {
            mCallback = callback;
        }

        public synchronized void set_agps_server(int type, String hostname, int port) {
        }

        public synchronized void resetGps(int bits) {
            // do nothing
        }

        public synchronized boolean simStart(VzwHalCriteria criteria, int sessionId) {
            Log.v (TAG,"start request");

            mSessionId = sessionId;

            if(EngineState.IDLE == mState)
            {
                mMode = criteria.getFixMode();
                mState = EngineState.IDLE_START_REQ;
                notifyAll();
            }
            else
            {
                // this could be an error, should be noted !
                //throw new RuntimeException ("EngineSimulator: Cannot start engine in any state other than IDLE");
                return false;
            }
            return true;
        }

        public synchronized boolean simStop() {
            Log.v (TAG,"stop request");

            if(EngineState.RECV == mState)
            {
                mState = EngineState.RECV_STOP_REQ;
                notifyAll();
            }
            else
            {
                // do nothing. this should be fine
                Log.v (TAG,"stop request not taken, for engine is not in RECV state: " + mState);
            }
            return true;
        }
    }

    public void cleanup() {
        mThread.cleanup();
    }

    public boolean init() {
        return mThread.init();
    }

    public void setCallbackInterface(IVzwHalGpsCallback callback) {
        mThread.setCallbackInterface(callback);
    }

    public void set_agps_server(int type, String hostname, int port) {
        mThread.set_agps_server(type, hostname, port);
    }

    public boolean start(VzwHalCriteria criteria, int sessionId, String app) {
        return mThread.simStart(criteria, sessionId);
    }

    public boolean stop() {
        mThread.simStop();
        for(int i = 0; i < 10; ++i)
        {
            if((mThread.getEngineState() == EngineState.IDLE) || (mThread.getEngineState() == EngineState.TERM_REQ))
            {
                break;
            }
            else
            {
                try {
                    Log.v (TAG,"stop requested, waiting. engine state : " + mThread.getEngineState());
                    Thread.sleep(300);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }
        return ((mThread.getEngineState() == EngineState.IDLE) || (mThread.getEngineState() == EngineState.TERM_REQ));
    }

    public void resetGps(int bits) {
        mThread.resetGps(bits);
    }
}
