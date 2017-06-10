/*
 * Copyright (c) 2012-2013, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.datamonitor;

import static com.qualcomm.datamonitor.DataUtils.ACTION_CONNECTIVITY_CHANGE;
import static com.qualcomm.datamonitor.DataUtils.ACTION_MOBILE_CONNECTIVITY_CHANGE;
import static com.qualcomm.datamonitor.DataUtils.ACTION_NETWORK_TOOGLE;
import static com.qualcomm.datamonitor.DataUtils.DEBUG_MODE;
import static com.qualcomm.datamonitor.DataUtils.DISABLE_CONNECTIVITY;
import static com.qualcomm.datamonitor.DataUtils.ENABLE_CONNECTIVITY;
import static com.qualcomm.datamonitor.DataUtils.LOG_ON;
import static com.qualcomm.datamonitor.DataUtils.widgetTextColor;

import java.lang.reflect.Method;

import android.app.PendingIntent;
import android.appwidget.AppWidgetManager;
import android.appwidget.AppWidgetProvider;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Message;
import android.provider.Settings;
import android.telephony.PhoneStateListener;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.format.Time;
import android.util.Log;
import android.widget.RemoteViews;
import android.widget.Toast;

public class DataWidget extends AppWidgetProvider {

    private static final String TAG = "DataWidget";
    private static Context mContext = null;

    private static final int MSG_ENABLE_NETWORK_TOGGLE = 0;
    private static final int SCHEDULE_NETWORK_TOGGLE_INTERVAL = 500;
    private static final String METHOD_SET_ENABLED = "setEnabled";

    private static final int STATE_OTHERS = 0;
    private static final int STATE_ENABLED = 1;
    private static final int STATE_DISABLED = 2;
    private static final int STATE_CHANGING = 3;
    private static final int STATE_UNKNOWN = 4;
    private static final NetworkToggleHelper mNetworkToggleHelper = new NetworkToggleHelper();

    private static RemoteViews mRemoteViews;
    private static ComponentName mWidget;

    // Control repetivity updates widget.
    private static String mCurrentWidgetText = null;
    private static int mCurrentWidgetState = -1;
    private static int mCurrentNetworkType = -1;
    private static TelephonyManager mTelephonyManager;

    // Add the static method for DataWidgetApp
    // get the RemoteViews.
    static RemoteViews getRemoteViews() {
        return mRemoteViews;
    }

    // Add the static method for DataWidgetApp
    // get the ComponentName of widget.
    static ComponentName getWidget() {
        return mWidget;
    }

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_ENABLE_NETWORK_TOGGLE:
                    // Enable network toggle.
                    mRemoteViews.setBoolean(R.id.network_toggle, METHOD_SET_ENABLED,
                            true);
                    try {
                        AppWidgetManager.getInstance(mContext).updateAppWidget(mWidget,
                                mRemoteViews);
                    } catch (Exception e) {
                        loge("Can't update AppWidget: " + e);
                    }
                    break;
            }
        }
    };

    private void init(Context context) {

        mContext = context;
        DataManager.init(context);
        DataUtils.getTelephonyManager(context).listen(mPhoneStateListener,
                PhoneStateListener.LISTEN_DATA_CONNECTION_STATE);

        /** Set PendingIntent */
        mRemoteViews = new RemoteViews(context.getPackageName(),
                R.layout.widget_layout);
        mWidget = new ComponentName(context, DataWidget.class);

        Intent mIntent = new Intent();
        mIntent.setClassName("com.android.settings",
                "com.android.settings.Settings$DataUsageSummaryActivity");
        PendingIntent trafficInfoPendingIntent = PendingIntent.getActivity(
                mContext, 0, mIntent, 0);
        mRemoteViews.setOnClickPendingIntent(R.id.data_info,
                trafficInfoPendingIntent);

        Intent networkToggleIntent = new Intent(context, DataWidget.class);
        networkToggleIntent.setAction(ACTION_NETWORK_TOOGLE);
        PendingIntent networkTogglePendingIntent = PendingIntent.getBroadcast(
                context, 0, networkToggleIntent, 0);
        mRemoteViews.setOnClickPendingIntent(R.id.network_toggle,
                networkTogglePendingIntent);
    }

    @Override
    public void onUpdate(Context context, AppWidgetManager appWidgetManager,
            int[] appWidgetIds) {

        logd("");

        if (mContext == null || mRemoteViews == null || mWidget == null)
            init(context);

        refreshWidgetView(context, mRemoteViews);
        try {
            AppWidgetManager.getInstance(context).updateAppWidget(mWidget,
                    mRemoteViews);
        } catch (Exception e) {
            loge("Can't update AppWidget: " + e);
        }
   }

    // DataWidgetApp also can use this method to refresh widget view.
    static void refreshWidgetView(Context context,
            RemoteViews mRemoteViews) {

        refreshWigetText(context, mRemoteViews);
        refreshNetworkImg(context, mRemoteViews);
    }

    private static void refreshWigetText(Context context,
            RemoteViews mRemoteViews) {

        if (mRemoteViews == null)
            return;
        if (DataManager.getContext() == null)
            DataManager.setContext(context);
        mRemoteViews.setTextColor(R.id.data_month, widgetTextColor);
        String text = DataManager.getMonthDataString();
        // Flow did not change not to update widget.
        if (mCurrentWidgetText != null && mCurrentWidgetText.equals(text)) {
            return;
        } else {
            mCurrentWidgetText = text;
        }
        if (DEBUG_MODE) {
            Time time = new Time();
            time.setToNow();
            text += "\n" + time.second;
        }
        mRemoteViews.setTextViewText(R.id.data_month, text);
    }

    private static void refreshNetworkImg(Context context,
            RemoteViews mRemoteViews) {

        int mState = mNetworkToggleHelper.getDeviceState();
        // Data network switch does not change do not need to update widget.
        if (mCurrentWidgetState != -1 && mCurrentWidgetState == mState) {
            return;
        } else {
            mCurrentWidgetState = mState;
        }
        mNetworkToggleHelper.setCurState(mState);
        mNetworkToggleHelper.setImage(mState, mRemoteViews);
    }

    /**
     * Handle the received intent broadcast.
     *
     * @param context The Context in which the receiver is running.
     * @param intent The Intent being received.
     * @param mRemoteViews RemoteViews that describes the child.
     */
    private void toDoAction(final Context context, Intent intent,
            final RemoteViews mRemoteViews) {

        String action = intent.getAction();
        logd("Action=" + action + " Context=" + context);

        /** Action */
        if (ACTION_NETWORK_TOOGLE.equals(action)) {
            final int mState = mNetworkToggleHelper.getCurState();
            if (mState != STATE_CHANGING) {
                new AsyncTask<Void, Void, Void> () {
                    @Override
                    protected void onPreExecute() {
                        // Disable network toggle.
                        mRemoteViews.setBoolean(R.id.network_toggle, METHOD_SET_ENABLED,
                                false);

                        if (mState == STATE_ENABLED) {
                            toast(context.getText(R.string.network_being_disabled).toString());
                        } else if (mState == STATE_DISABLED) {
                            toast(context.getText(R.string.network_being_enabled).toString());
                        }
                    }

                    @Override
                    protected Void doInBackground(Void... args) {
                        mNetworkToggleHelper.toggleState();
                        return null;
                    }

                    @Override
                    protected void onPostExecute(Void result) {
                        // Enable the click action with a little delay, for avoiding the network
                        // toggle being clicked too often, which will cause launcher unresponsive.
                        final int mStateChang = mNetworkToggleHelper.getCurState();
                        mHandler.sendEmptyMessageDelayed(MSG_ENABLE_NETWORK_TOGGLE,
                                SCHEDULE_NETWORK_TOGGLE_INTERVAL);
                        mNetworkToggleHelper.setImage(mStateChang, mRemoteViews);
                        refreshWigetText(context, mRemoteViews);
                    }
                }.execute();
            } else {
                if (DataUtils.getMobileDataEnabled(context) == true)
                    toast(context.getText(R.string.network_being_enabled).toString());
                else
                    toast(context.getText(R.string.network_being_disabled).toString());
            }
        } else if (ACTION_CONNECTIVITY_CHANGE.equals(action)) {
            // Date network off do not need to update widget.
            if (DataUtils.getMobileDataEnabled(context) == false) {
                return;
            }
            ConnectivityManager cm = (ConnectivityManager) mContext
                    .getSystemService(Context.CONNECTIVITY_SERVICE);
            if (cm != null) {
                NetworkInfo info = cm.getActiveNetworkInfo();
                if (info != null) {
                    int type = info.getType();
                    if (mCurrentNetworkType == type) {
                        return;
                    }
                    mCurrentNetworkType = type;
                }
            }
            // Even if there is no the widget, still can go here
            refreshNetworkImg(context, mRemoteViews);
            refreshWigetText(context, mRemoteViews);
        } else if (ACTION_MOBILE_CONNECTIVITY_CHANGE.equals(action)) {
            // When we setMobileDataEnabled() in Settings, we should update this widget.
            boolean enabled = intent.getBooleanExtra("enable", true);
            int state = STATE_OTHERS;
            state = enabled == true ? STATE_ENABLED : STATE_DISABLED;
            mNetworkToggleHelper.setCurState(state);
            mNetworkToggleHelper.setImage(state, mRemoteViews);
            refreshWigetText(context, mRemoteViews);
        }
    }

    @Override
    public void onDeleted(Context context, int[] appWidgetIds) {

        logd("");
        mCurrentWidgetText = null;
        mCurrentWidgetState = -1;
        mCurrentNetworkType = -1;
    }

    @Override
    public void onDisabled(Context context) {

        logd("");
        mCurrentWidgetText = null;
        mCurrentWidgetState = -1;
        mCurrentNetworkType = -1;
        ((TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE))
                .listen(mPhoneStateListener, PhoneStateListener.LISTEN_NONE);
        super.onDisabled(context);
    }

    @Override
    public void onEnabled(Context context) {

        logd("");
        if (mContext == null || mRemoteViews == null || mWidget == null)
            init(context);

        refreshWidgetView(context, mRemoteViews);
        try {
            AppWidgetManager.getInstance(context).updateAppWidget(mWidget,
                    mRemoteViews);
        } catch (Exception e) {
            loge("Can't update AppWidget: " + e);
        }
        super.onEnabled(context);
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        super.onReceive(context, intent);

        mContext = context;
        String action = intent.getAction();
        logd("Action=" + action + " Context=" + mContext);

        if (AppWidgetManager.ACTION_APPWIDGET_DISABLED.equals(action)) {
            return;
        }

        boolean isProcessKilled = false;
        if (mWidget == null) {
            mWidget = new ComponentName(context, DataWidget.class);
            isProcessKilled = true;
        }
        if (AppWidgetManager.getInstance(context).getAppWidgetIds(mWidget).length < 1) {
            // There is not widget in screen, return
            return;
        }
        if (isProcessKilled) {
            onUpdate(context, AppWidgetManager.getInstance(context),
                    AppWidgetManager.getInstance(context).getAppWidgetIds(mWidget));
        }
        toDoAction(context, intent, mRemoteViews);
        try {
            AppWidgetManager.getInstance(context).updateAppWidget(mWidget,
                    mRemoteViews);
        } catch (Exception e) {
            loge("Can't update AppWidget: " + e);
        }
    }

    /** ToggleHelper */

    private abstract static class ToggleHelper {

        public abstract void toggleState();

        public abstract void setDeviceState(boolean state);

        public abstract int getDeviceState();

        public abstract void setImage(int state, RemoteViews mRemoteViews);

        public abstract int formateState(int state);

        public abstract void setCurState(int state);

        public abstract int getCurState();

    }

    public static final class NetworkToggleHelper extends ToggleHelper {

        private static int mCurState = STATE_UNKNOWN;

        @Override
        public void toggleState() {
            ConnectivityManager cm = (ConnectivityManager) mContext
                    .getSystemService(Context.CONNECTIVITY_SERVICE);
            NetworkInfo info = null;
            boolean changingStateFlag = false;

            if (null != cm) {
                info = cm.getActiveNetworkInfo();
                if (null != info) {
                    int networkType = info.getType();
                    changingStateFlag = info.isConnected()
                            && cm.isNetworkTypeMobile(networkType);
                }
            }
            if (mCurState == STATE_UNKNOWN)
                mCurState = getDeviceState();

            switch (mCurState) {
            case STATE_ENABLED:
                setDeviceState(DISABLE_CONNECTIVITY);
                if (changingStateFlag) {
                    setCurState(STATE_CHANGING);
                } else {
                    setCurState(STATE_DISABLED);
                }
                break;
            case STATE_DISABLED:
                setDeviceState(ENABLE_CONNECTIVITY);
                if (changingStateFlag) {
                    setCurState(STATE_CHANGING);
                } else {
                    setCurState(STATE_ENABLED);
                }
                break;
            default:
                logd("nothing");
                break;
            }
        }

        @Override
        public void setDeviceState(boolean state) {

            DataUtils.setMobileDataEnabled(mContext, state);
        }

        public int getDeviceState() {

            int state = STATE_OTHERS;
            state = DataUtils.getMobileDataEnabled(mContext) == true ? STATE_ENABLED
                    : STATE_DISABLED;
            return state;

        }

        @Override
        public void setImage(int state, RemoteViews mRemoteViews) {
            switch (state) {
            case STATE_ENABLED:
                mRemoteViews.setImageViewResource(R.id.network_toggle_image,
                        R.drawable.on);
                break;
            case STATE_DISABLED:
                mRemoteViews.setImageViewResource(R.id.network_toggle_image,
                        R.drawable.off);
                break;
            case STATE_CHANGING:
                mRemoteViews.setImageViewResource(R.id.network_toggle_image,
                        R.drawable.changing);
                break;
            default:
                mRemoteViews.setImageViewResource(R.id.network_toggle_image,
                        R.drawable.off);
                break;
            }

        }

        @Override
        public int formateState(int state) {

            int myState;
            switch (state) {
            case TelephonyManager.DATA_CONNECTED:
                myState = STATE_ENABLED;
                break;
            case TelephonyManager.DATA_DISCONNECTED:
                myState = STATE_DISABLED;
                break;
            case TelephonyManager.DATA_CONNECTING:
                myState = STATE_CHANGING;
                break;
            case TelephonyManager.DATA_SUSPENDED:
                myState = STATE_OTHERS;
            default:
                myState = STATE_OTHERS;
                break;
            }

            return myState;
        }

        @Override
        public void setCurState(int state) {

            mCurState = state;

        }

        @Override
        public int getCurState() {

            return mCurState;
        }

    }// NetworkToggleHelper

    private final PhoneStateListener mPhoneStateListener = new PhoneStateListener() {

        @Override
        public void onDataConnectionStateChanged(int state) {

            if (DEBUG_MODE)
                toast("Connection:" + String.valueOf(state));
            logd("ConnectionStateChanged");
            refreshWidgetView(mContext,mRemoteViews);
            try {
                AppWidgetManager.getInstance(mContext).updateAppWidget(mWidget,
                        mRemoteViews);
            } catch (Exception e) {
                loge("Can't update AppWidget: " + e);
            }
        }
    };

    public void toast(Object s) {

        if (s == null)
            return;
        Toast.makeText(mContext, s + "", Toast.LENGTH_SHORT).show();
    }

    private static void loge(Object e) {

        if (e == null)
            return;
        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();
        e = "[" + mMethodName + "] " + e;
        Log.e(TAG, e + "");
    }

    private static void logd(Object s) {

        if (!LOG_ON)
            return;
        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();

        s = "[" + mMethodName + "] " + s;
        Log.d(TAG, s + "");
    }

}
