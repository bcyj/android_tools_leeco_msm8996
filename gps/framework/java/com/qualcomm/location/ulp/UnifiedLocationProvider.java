/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2012-2013 Qualcomm Atheros, Inc.
  All Rights Reserved.
  Qualcomm Atheros Confidential and Proprietary.
=============================================================================*/

package com.qualcomm.location.ulp;

import com.android.location.provider.LocationProviderBase;
import com.android.location.provider.ProviderPropertiesUnbundled;
import com.android.location.provider.ProviderRequestUnbundled;

import android.content.Context;
import android.location.Criteria;
import android.location.LocationProvider;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.WorkSource;
import android.util.Log;

import java.io.FileDescriptor;
import java.io.PrintWriter;

public class UnifiedLocationProvider extends LocationProviderBase implements UlpEngine.Callback{
    private static final String TAG = "UnifiedLocationProvider";
    private static final boolean VERBOSE_DBG = Log.isLoggable(TAG, Log.VERBOSE);

    private static ProviderPropertiesUnbundled PROPERTIES = ProviderPropertiesUnbundled.create(
            false, false, false, false, true, true, true, Criteria.POWER_LOW,
            Criteria.ACCURACY_FINE);

    private static final int MSG_ENABLE = 1;
    private static final int MSG_DISABLE = 2;
    private static final int MSG_SET_REQUEST = 3;

    private final Context mContext;
    private final UlpEngine mEngine;

    private static class RequestWrapper {
        public ProviderRequestUnbundled request;
        public WorkSource source;
        public RequestWrapper(ProviderRequestUnbundled request, WorkSource source) {
            this.request = request;
            this.source = source;
        }
    }

    public UnifiedLocationProvider(Context context) {
        super(TAG, PROPERTIES);
        mContext = context;
        mEngine = UlpEngine.getInstance(context);
    }

    /**
     * For serializing requests to mEngine.
     */
    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            int msgID = msg.what;
            logv("handleMessage what - " + msgID);
            switch (msgID) {
                case MSG_ENABLE:
                    mEngine.enable(UnifiedLocationProvider.this);
                    break;
                case MSG_DISABLE:
                    mEngine.disable();
                    break;
                case MSG_SET_REQUEST:
                {
                    RequestWrapper wrapper = (RequestWrapper) msg.obj;
                    mEngine.setRequest(wrapper.request, wrapper.source);
                    break;
                }
            }
        }
    };

    @Override
    public void onEnable() {
        mHandler.sendEmptyMessage(MSG_ENABLE);
    }

    @Override
    public void onDisable() {
        mHandler.sendEmptyMessage(MSG_DISABLE);
    }

    @Override
    public void onSetRequest(ProviderRequestUnbundled request, WorkSource source) {
        mHandler.obtainMessage(MSG_SET_REQUEST, new RequestWrapper(request, source)).sendToTarget();
    }

    @Override
    public void onDump(FileDescriptor fd, PrintWriter pw, String[] args) {
        // perform synchronously
        mEngine.dump(fd, pw, args);
    }

    @Override
    public int onGetStatus(Bundle extras) {
        return LocationProvider.AVAILABLE;
    }

    @Override
    public long onGetStatusUpdateTime() {
        return 0;
    }

    private void logv(String s) {
        if (VERBOSE_DBG) Log.v(TAG, s);
    }
}
