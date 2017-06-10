/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount.http.service;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Handler.Callback;

public abstract class PAHttpService implements Runnable {

    private static final int TAG_CALLBACK = 1;

    protected Request request;

    protected Response response;

    protected Callback callback;

    protected Message msg;

    protected boolean cancel = false;

    private Handler mCallbackHandler = new Handler(Looper.getMainLooper()) {
        public void handleMessage(Message msg) {
            if (TAG_CALLBACK == msg.what) {
                if (cancel)
                    return;
                if (callback != null)
                    callback.handleMessage(getResponseMsg());
            }
        };
    };

    public abstract void service();

    public PAHttpService(Request req) {
        this.request = req;
        this.callback = request.callback;
        this.response = new Response(req);
    }

    @Override
    public void run() {
        service();
    }

    public void cancel() {
        cancel = true;
    }

    protected void callback() {
        mCallbackHandler.sendEmptyMessage(TAG_CALLBACK);
    }

    protected Message getResponseMsg() {
        Message msg = Message.obtain();
        msg.obj = response;
        return msg;
    }

    public static class Request {

        public Callback callback;

        public int downloadFileType;

        public String requestUrl;
    }

    public static class Response {

        public static final int SECCESS = 0;

        public static final int FAIL = 1;

        public static final int PROGRESS = 3;

        public int state;

        public Object responseObject;

        public Request req;

        public String feedback;

        public Response(Request req) {
            this.req = req;
        }
    }

}
