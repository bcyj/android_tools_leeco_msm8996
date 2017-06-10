/*******************************************************************************
    Copyright (c) 2010,2011 Qualcomm Technologies, Inc.  All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

package com.qualcomm.atfwd;

public interface AtCmdHandler {
    public class AtCmdParseException extends Exception {
        private static final long serialVersionUID = 1L;

        public AtCmdParseException() {
            super();
        }

        public AtCmdParseException(String detailMessage, Throwable throwable) {
            super(detailMessage, throwable);
        }

        public AtCmdParseException(String detailMessage) {
            super(detailMessage);
        }

        public AtCmdParseException(Throwable throwable) {
            super(throwable);
        }
    }

    public class AtCmdHandlerInstantiationException extends Exception {

        private static final long serialVersionUID = 1L;

        public AtCmdHandlerInstantiationException() {
            super();
        }

        public AtCmdHandlerInstantiationException(String detailMessage, Throwable throwable) {
            super(detailMessage, throwable);
        }

        public AtCmdHandlerInstantiationException(String detailMessage) {
            super(detailMessage);
        }

        public AtCmdHandlerInstantiationException(Throwable throwable) {
            super(throwable);
        }
    }

    public static class PauseEvent {
        private long mTime;
        public PauseEvent(long time) {
            mTime = time;
        }
        public long getTime() {
            return mTime;
        }
    }

    public String getCommandName();
    public AtCmdResponse handleCommand(AtCmd cmd);
}
