/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount.controller;

import com.suntek.mway.rcs.publicaccount.controller.ServiceExecutor.Feedback;
import com.suntek.mway.rcs.publicaccount.controller.ServiceExecutor.MessageBean;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;

public class ThreadPool {

    private static final int POOL_MAX_SIZE = 5;

    private static final ExecutorService mThreadPoolForText = Executors
            .newFixedThreadPool(POOL_MAX_SIZE);


    public static ExecutorService getInstance() {
        return mThreadPoolForText;
    }

    public static abstract class ExRunnable implements Runnable {

        public Feedback mFeedback;

        public ExRunnable() {
        }

        public ExRunnable(Feedback feedback) {
            this.mFeedback = feedback;
        }

        public abstract Object execute();

        @Override
        public final void run() {

            Object dataObj = execute();

            if (mFeedback == null)
                return;

            mFeedbackInMainHandler.sendMessage(mFeedbackInMainHandler.obtainMessage(FEEDBACK,
                    new MessageBean(mFeedback, dataObj)));
        }
    }

    private static Handler mFeedbackInMainHandler = new Handler(Looper.getMainLooper()) {
        public void handleMessage(Message msg) {
            if (msg.what == FEEDBACK) {
                MessageBean bean = (MessageBean)msg.obj;
                bean.feedback.feedback(bean.resultData);
            }
        }
    };

    private static final int FEEDBACK = 1;
}
