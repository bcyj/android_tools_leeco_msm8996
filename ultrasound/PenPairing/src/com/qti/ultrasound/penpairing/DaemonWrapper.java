/******************************************************************************
 * ---------------------------------------------------------------------------
 *  Copyright (C) 2014 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *******************************************************************************/
package com.qti.ultrasound.penpairing;

import android.util.Log;
import java.io.FileInputStream;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;

public class DaemonWrapper {

    private boolean mIsDaemonRunning = false;

    private Process mDaemonProcess = null;
    private static final String mPidDirectory = "/data/usf/";
    private static final String mPidFileExtention = ".pid";
    private String mDaemonName;
    private static final String mExePath = "/system/bin/";

    public DaemonWrapper(String daemonName) {
        mDaemonName = daemonName;
    }

    public boolean start() {
        try {
            mDaemonProcess = Runtime.getRuntime().exec(mExePath + mDaemonName);
        } catch (IOException e) {
            Log.e(toString(), "Error trying to execute " + mExePath + mDaemonName, e);
            return false;
        }
        mIsDaemonRunning = true;
        return true;
    }

    /**
     * getPid() function reads the daemon's pid file and returns its
     * pid.
     * @return int the current daemon's pid
     *             -1 in case of an error
     *             -2 in case non-integer is read from the pid file
     */
    int getPid() {
        String str = "";
        StringBuffer buf = new StringBuffer();
        int retPid;
        BufferedReader reader = null;
        try {
            // Try to read pid file
            // This file should include one integer, which is the daemon's pid
            FileInputStream fStream = new FileInputStream(mPidDirectory +
                                                          mDaemonName +
                                                          mPidFileExtention);
            reader = new BufferedReader(new InputStreamReader(fStream));
            while (null != (str = reader.readLine())) {
                buf.append(str);
            }
        } catch (IOException e) {
            return -1;
        } finally {
            if (null != reader) {
                try {
                    reader.close();
                } catch (IOException e) {
                    return -1;
                }
            }
        }

        try {
            retPid = Integer.parseInt(buf.toString());
        } catch (NumberFormatException e) {
            Log.e(this.toString(), "Daemon pid file does not contain an integer");
            return -2;
        }
        return retPid;
    }

    /**
     * This function tries to stop the daemon appropriately by
     * sending it a SIGTERM signal, instead of just calling destroy
     * process which in turn sends a SIGKILL. Thus, in
     * exterme cases, where an unexpected error happens, this
     * function calls destroy process.
     */
    private void stopDaemon() {
        int pid = getPid();
        if (-1 != pid) { // No problems
            try {
                // Stop daemon with SIGTERM
                Runtime.getRuntime().exec("kill -15 " + pid);
                mDaemonProcess.waitFor();
                return;
            } catch (IOException e) {
                Log.e(this.toString(),
                      e.getMessage());
            } catch (InterruptedException e) {
                Log.e(this.toString(), "Interrupted while stopping daemon: " + mDaemonName);
            }
        }
        else { // Problem getting pid
            // Stop daemon process
            if (null != mDaemonProcess) {
                mDaemonProcess.destroy();
            }
        }
    }

    public void stop() {
        stopDaemon();
        mDaemonProcess = null;
        mIsDaemonRunning = false;
    }

    public boolean isStarted() {
        return mIsDaemonRunning;
    }

}
