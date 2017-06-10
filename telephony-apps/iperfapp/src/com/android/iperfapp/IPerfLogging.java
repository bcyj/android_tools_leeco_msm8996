/*************************************************************************
        Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
                   Qualcomm Technologies Proprietary and Confidential.
 **************************************************************************/

package com.android.iperfapp;

import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.util.Calendar;
import java.util.GregorianCalendar;
import android.util.Log;

public class IPerfLogging {
    private static final String TAG = "iPerfAppLogs";

    private static final String IPERF_FILE_STRING = "/data/data/com.android.iperfapp/files";

    private static final String IPERF_OUTPUT_STRING = "/data/data/com.android.iperfapp"
                                                     + "/files/output_";

    InputStream is2 = null;

    OutputStream fout = null;

    OutputStreamWriter osw = null;

    Process process2 = null;

    int interval2 = 0;

    BufferedReader reader = null;

    String line;

    String separator;

    StringBuilder log;

    boolean binding = false;

    String ts = "0";

    public void setupLogging(boolean fileState) {
        try {

            File file = new File(IPERF_FILE_STRING);
            if (!file.isDirectory()) {
            } else {
                if (fileState == true) {
                    Calendar c = new GregorianCalendar();
                    long mHour = c.get(Calendar.HOUR);
                    long mMinute = c.get(Calendar.MINUTE);
                    long mMSec = c.get(Calendar.MILLISECOND);
                    long mSec = c.get(Calendar.SECOND);
                    ts = String.valueOf(mHour) + "_" + String.valueOf(mMinute) + "_"
                            + String.valueOf(mSec) + "_" + String.valueOf(mMSec);
                    BufferedOutputStream fout = new BufferedOutputStream(new FileOutputStream(
                            IPERF_OUTPUT_STRING + ts + ".txt", true));
                    osw = new OutputStreamWriter(fout);
                }
                log = new StringBuilder();
                separator = System.getProperty("line.separator");
                reader = new BufferedReader(new InputStreamReader(process2.getInputStream()));
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
    }

    public String returnLogLine() {
        return line;
    }

    public IPerfLogging(Process process, int interval) {
        process2 = process;
        interval2 = interval;
    }

    public void runLogging(boolean fileState) {
        try {
            if (reader != null) {
                line = reader.readLine();
                if (line != null) {
                    if (line.indexOf("invalid option") < 0) {
                        if (line.matches("bind failed: Address already in use"))
                            binding = true;
                        if (fileState == true) {
                            osw.write(line + "\n");
                            osw.flush();
                        }
                        Log.v(TAG, line);
                        log.append(line);
                        log.append(separator);
                    } else {
                        line = null;
                    }
                }
            }
        }

        catch (Exception e) {
            Log.v(TAG, "Exception: " + e);
        }
    }

    public void stopLogging() {
        try {
            osw.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public boolean checkBinding() {
        return binding;
    }

    public void refreshBinding() {
        binding = false;
    }

}
