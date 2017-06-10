/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 */

package com.qualcomm.presencelist;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Timer;
import java.util.TimerTask;



import android.content.Context;
import android.content.SharedPreferences;
import android.os.Handler;
import android.util.Log;

public class Utility {
    private final static String TAG = "Utility";

    private static FileWriter mFwLiveLogging;
    private static FileWriter mFwNotifyXML;

    public static void rescheduleSubcribeTimer(int i, Contact temp) {
        Log.e(TAG, "Periodic timer are disabled for the time being.");
        if(false) {
            cancelSubscribeTimerTask(temp);
            TimerTask t = resetSubscribeTimer(i);
            temp.setSubscribeTimerTask(t);
        }
    }

    public static void removeSubcribeTimer(Contact temp) {
        cancelSubscribeTimerTask(temp);
        temp.setSubscribeTimerTask(null);
    }



    public static TimerTask setTimer(Timer tm, final Runnable r, final long when,
            final long repeat) {
        final Handler handler = AppGlobalState.getListenerHandler();
        TimerTask timer = new TimerTask() {
            @Override
                public void run() {
                    handler.post(r);
                }
        };

        tm.schedule(timer, when, repeat);
        return timer;
    }

    public static TimerTask resetSubscribeTimer(final int index ) {


        return setTimer(AppGlobalState.getTimerManager(), new Runnable() {
            public void run() {

                Context c = getContextOfDisplayedActivity();
                AppGlobalState.setPendingSubscriptionUri(index);
                Log.d(TAG, "Running SubscribeSimpleTask, index="+index);
                SubscribeSimpleTask subscribeSimpleTask =
                      (SubscribeSimpleTask) new SubscribeSimpleTask(
                    c, index, true).execute();
            }
        }, 60000, 60000);

    }



    public static Context getContextOfDisplayedActivity() {
        Context c = AppGlobalState.getContactInfo();
        if(c == null) {
            c = AppGlobalState.getMainActivityContext();
        }

        return c;
    }

    public static void cancelSubscribeTimerTask(Contact temp) {
        TimerTask t = temp.getSubscribeTimerTask();
        if(t != null) {
            Log.d(TAG, "scheduledExecutionTime="+t.scheduledExecutionTime());
        }
        cancelPreviousTimer(t);

    }

    public static void cancelPreviousTimer(TimerTask t) {
        if(t != null) {
            t.cancel();
            AppGlobalState.getTimerManager().purge();

        }
    }

    public static SharedPreferences getSharedPrefHandle(Context context,
            String imsPresencePref) {
        SharedPreferences settings = context.getSharedPreferences(imsPresencePref, 0);

        return settings;
    }


    public static boolean isContactOptedOut(int index) {
        return !(AppGlobalState.getContacts().get(index).getContactParticipation());

    }

    public static void prepareExcludedContactList() {
        ArrayList<String> excludedNumber = AppGlobalState.excludedNumberList;

        excludedNumber.add("800");
        excludedNumber.add("822");
        excludedNumber.add("833");
        excludedNumber.add("844");
        excludedNumber.add("855");
        excludedNumber.add("866");
        excludedNumber.add("877");
        excludedNumber.add("880882");
        excludedNumber.add("888");
        excludedNumber.add("900");
        excludedNumber.add("911");
        excludedNumber.add("*");
    }

    public static boolean isContactExcluded(int index) {

        String currentNum =AppGlobalState.getContacts().get(index).getPhone();

        for(String num : AppGlobalState.excludedNumberList) {
            if(currentNum.startsWith(num)) {
                return true;
            }
        }
        return false;
    }

    public static boolean isContactExcluded(Contact c) {

        for(String num : AppGlobalState.excludedNumberList) {
            if(c.getPhone().startsWith(num)) {
                return true;
            }
        }
        return false;
    }

    public static void sendLogMesg(Context c, String string) {
        Log.d("LiveLogging", string);
        Utility.writeToFile(c, string+"\n");
    }


    public static void initLiveLogging(Context c) {

        File myDir = new File(c.getFilesDir().getAbsolutePath());
        String s = "";

        try {
            mFwLiveLogging = new FileWriter(myDir+"/"+
                    AppGlobalState.IMS_PRESENCE_LIVE_LOGGING);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static void initNotifyXMLFile(Context c) {

        File myDir = new File(c.getFilesDir().getAbsolutePath());
        String s = "";

        try {
            mFwNotifyXML = new FileWriter(myDir+"/"+
                    AppGlobalState.IMS_PRESENCE_NOTIFY_XML_FILE);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static void writeToFile(Context c, String s) {
        try {
            mFwLiveLogging.write(s);
            mFwLiveLogging.flush();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }


    public static void appendToXMLFile(Context c, String s) {
        try {
            mFwNotifyXML.write(s);
            mFwNotifyXML.flush();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static void closeLiveLoggingFile() {
        try {
            mFwLiveLogging.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static void closeNotifyXMLFile() {
        try {
            mFwNotifyXML.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }




    public static int getNotifyFmt(Context mContext) {
        SharedPreferences setting = getSharedPrefHandle(mContext,
                AppGlobalState.IMS_PRESENCE_SETTINGS);
        String fmt = setting.getString(
                mContext.getString(R.string.set_notify_fmt_text),
                mContext.getString(R.string.fmt_struct_text));
        Log.d(TAG, "Current fmt=" + fmt);
        return (fmt
                .equals(mContext
                    .getString(R.string.fmt_struct_text)))?
                       AppGlobalState.NOTIFY_FMT_STRUCT :
                       AppGlobalState.NOTIFY_FMT_XML;

    }

    public static String readXMLFromFile(Context context, String file) {
        InputStream instream;
        String xml = new String("");
        try {
            instream = context.openFileInput(file);
            if (instream != null) {
                InputStreamReader inputreader = new InputStreamReader(instream);
                BufferedReader buffreader = new BufferedReader(inputreader);

                String line;

                while ((line = buffreader.readLine()) != null) {
                    xml += line+"\n";
                }
                instream.close();
            }

        } catch (FileNotFoundException e1) {
            e1.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return xml;
    }
}
