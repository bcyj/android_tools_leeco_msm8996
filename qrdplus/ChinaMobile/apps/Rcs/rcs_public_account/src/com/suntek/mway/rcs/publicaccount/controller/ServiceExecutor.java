/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount.controller;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;

public class ServiceExecutor extends HandlerThread {

    private static final int FEEDBACK = 1;

    private static final ServiceExecutor instance = new ServiceExecutor("THREAD#[to do time consumed operation]");

    private static final Handler mMainHanlder = new Handler(Looper.getMainLooper());

    private static Handler mServiceHandler = new Handler(ServiceExecutor.getInstance().getLooper()) {
        public void handleMessage(Message msg) {
            TimeConsumedService service = (TimeConsumedService) msg.obj;
            service.run();
        }
    };

    private static Handler mFeedbackInMainHandler = new Handler(Looper.getMainLooper()) {
        public void handleMessage(Message msg) {
            if (msg.what == FEEDBACK) {
                MessageBean bean = (MessageBean) msg.obj;
                bean.feedback.feedback(bean.resultData);
            }
        }
    };

    private ServiceExecutor(String name) {
        super(name);
        start();
    }

    public static ServiceExecutor getInstance() {
        return instance;
    }

    public void execute(TimeConsumedService service) {
        mServiceHandler.sendMessage(mServiceHandler.obtainMessage(0, service));
    }

    public static Handler getMainHandler() {
        return mMainHanlder;
    }

    public static Handler getServiceHandler() {
        return mServiceHandler;
    }

    public interface Feedback {
        public void feedback(Object resultData);
    }

    public static abstract class TimeConsumedService {

        public Feedback mFeedback;

        public TimeConsumedService() {
        }

        public TimeConsumedService(Feedback feedback) {
            this.mFeedback = feedback;
        }

        public abstract Object execute();

        public final void run() {

            Object dataObj = execute();

            if (mFeedback == null)
                return;

            mFeedbackInMainHandler.sendMessage(
                    mFeedbackInMainHandler.obtainMessage(
                            FEEDBACK, new MessageBean(mFeedback, dataObj)));
        }
    }

    public static class MessageBean {
        Feedback feedback;
        Object resultData;

        public MessageBean(Feedback feedback, Object resultData) {
            this.feedback = feedback;
            this.resultData = resultData;
        }
    }

}
