/*************************************************************************
        Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
                   Qualcomm Technologies Proprietary and Confidential.
 **************************************************************************/

package com.android.iperfapp;

import android.os.AsyncTask;
import android.widget.TextView;

public class IPerfProcess {
    private static final String IPERF_FILE_STRING = "/data/data/com.android.iperfapp/iperf ";

    Process process = null;

    String params2 = null;

    IPerfResultSummary ips = null;

    IPerfLogging ipl = null;

    String paramsMain = null;

    IPerfProcessImpl processImpl = null;

    boolean isCancelTask = false;

    TextView textV = null;

    boolean firstLaunch = false;

    int destroy = 0;

    String disp = "";

    boolean dispState = false;

    boolean fileState = false;

    public IPerfProcess(String params, TextView t, boolean dispState, boolean fileState) {
        paramsMain = params;
        textV = t;
        this.dispState = dispState;
        this.fileState = fileState;
        processImpl = new IPerfProcessImpl();
        processImpl.execute(paramsMain);
        destroy = 0;
    }

    public void CancelTask() {
        destroy = 1;
        isCancelTask = processImpl.cancel(true);
        process.destroy();
    }

    public Process returnProcess() {
        return process;
    }

    public boolean CheckDispState() {
        return this.dispState;
    }

    public void SetDispState(boolean dispState) {
        this.dispState = dispState;
    }

    public void SetFileState(boolean fileState) {
        this.fileState = fileState;
    }

    public boolean CheckFileState() {
        return this.fileState;
    }

    private class IPerfProcessImpl extends AsyncTask<String, String, IPerfResultSummary> {
        @Override
        protected IPerfResultSummary doInBackground(String... params) {
            params2 = params[0];
            try {
                process = Runtime.getRuntime().exec(
                        IPERF_FILE_STRING + params2);
                ipl = new IPerfLogging(process, 0);
                ipl.setupLogging(fileState);
                while (process != null && destroy != 1) {
                    if (ipl.checkBinding() == true) {
                        ipl.refreshBinding();
                        break;
                    }
                    ipl.runLogging(fileState);
                    disp = ipl.returnLogLine();
                    if (disp != null && dispState == true)
                        publishProgress(disp);
                }
            } catch (Exception e) {

            } finally {
                process.destroy();
            }
            return ips;
        }

        protected void onPreExecute() {
        }

        @Override
        protected void onProgressUpdate(String... progress) {
            setProgressPercent(progress[0]);
        }

        protected void onPostExecute(IPerfResultSummary result) {

        }
    }

    public void setProgressPercent(String progress) {
        textV.append(progress);
        textV.append("\n");
    }
}
