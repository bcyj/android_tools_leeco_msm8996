/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2012-2013 Qualcomm Atheros, Inc.
  All Rights Reserved.
  Qualcomm Atheros Confidential and Proprietary.
=============================================================================*/

package com.qualcomm.location;

import android.content.Context;
import android.os.Handler;
import android.os.Message;

public interface MonitorInterface {
    public Context getContext();
    public Handler getHandler();
    public void subscribe(Monitor m);
    public void unsubscribe(Monitor m);

    public abstract class Monitor {
        protected final MonitorInterface mMoniterService;
        private final int mMsgIdBase;

        public Monitor(MonitorInterface service, int msgIdBase) {
            mMoniterService = service;
            mMsgIdBase = msgIdBase;
        }

        public final int getMsgIdBase() {
            return mMsgIdBase;
        }

        public final Message composeMessage(int what, int arg1, int arg2, Object obj) {
            Handler handler = mMoniterService.getHandler();
            Message msg = Message.obtain(handler, what+mMsgIdBase, arg1, arg2, obj);
            return msg;
        }

        public final void sendMessage(int what, int arg1, int arg2, Object obj) {
            Handler handler = mMoniterService.getHandler();
            Message msg = composeMessage(what, arg1, arg2, obj);
            handler.removeMessages(msg.what);
            handler.sendMessage(msg);
        }

        public abstract int getNumOfMessages();

        public abstract void handleMessage(Message msg);
    }
}
