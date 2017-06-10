/*******************************************************************************
    Copyright (c) 2010,2011 Qualcomm Technologies, Inc.  All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

package com.qualcomm.atfwd;

import java.util.HashMap;

import android.content.Context;
import android.content.pm.PackageManager;
import android.os.RemoteException;
import android.util.Log;

import com.qualcomm.atfwd.IAtCmdFwd;
import com.qualcomm.atfwd.AtCmdHandler.AtCmdHandlerInstantiationException;

public class AtCmdFwdService extends IAtCmdFwd.Stub {

    private static final String LOG_TAG = "AtCmdFwdService";

    private Context mContext;

    private HashMap<String, AtCmdHandler> mCmdHandlers;

    public AtCmdFwdService(Context c)
    {
        mContext = c;
        mCmdHandlers = new HashMap<String, AtCmdHandler>();

        AtCmdHandler cmd;

        try {
            cmd = new AtCkpdCmdHandler(c);
            mCmdHandlers.put(cmd.getCommandName().toUpperCase(), cmd);
        } catch (AtCmdHandlerInstantiationException e) {
            Log.e(LOG_TAG, "Unable to instantiate command", e);
        }

        try {
            cmd = new AtCtsaCmdHandler(c);
            mCmdHandlers.put(cmd.getCommandName().toUpperCase(), cmd);
        } catch (AtCmdHandlerInstantiationException e) {
            Log.e(LOG_TAG, "Unable to instantiate command", e);
        }

        try {
            cmd = new AtCfunCmdHandler(c);
            mCmdHandlers.put(cmd.getCommandName().toUpperCase(), cmd);
        } catch (AtCmdHandlerInstantiationException e) {
            Log.e(LOG_TAG, "Unable to instantiate command", e);
        }

        try {
            cmd = new AtCrslCmdHandler(c);
            mCmdHandlers.put(cmd.getCommandName().toUpperCase(), cmd);
        } catch (AtCmdHandlerInstantiationException e) {
            Log.e(LOG_TAG, "Unable to instantiate command", e);
        }

        try {
            cmd = new AtCssCmdHandler(c);
            mCmdHandlers.put(cmd.getCommandName().toUpperCase(), cmd);
        } catch (AtCmdHandlerInstantiationException e) {
            Log.e(LOG_TAG, "Unable to instantiate command", e);
        }

        try {
            cmd = new AtCmarCmdHandler(c);
            mCmdHandlers.put(cmd.getCommandName().toUpperCase(), cmd);
        } catch (AtCmdHandlerInstantiationException e) {
            Log.e(LOG_TAG, "Unable to instantiate command", e);
        }

        try {
            cmd = new AtQcpwrdnCmdHandler(c);
            mCmdHandlers.put(cmd.getCommandName().toUpperCase(), cmd);
        } catch (AtCmdHandlerInstantiationException e) {
            Log.e(LOG_TAG, "Unable to instantiate command", e);
        }

    }

    public AtCmdResponse processAtCmd(AtCmd cmd) throws RemoteException {
        Log.d(LOG_TAG, "processAtCmd(cmd: " + cmd.toString());

        int canProcess = mContext.checkCallingPermission("com.qualcomm.permission.ATCMD");
        if (canProcess != PackageManager.PERMISSION_GRANTED) {
               throw new SecurityException("Requires ATCMD permission");
         } else {
               Log.d(LOG_TAG, "processAtCmd : Permission is granted ");
        }

        AtCmdResponse ret;
        AtCmdHandler h = mCmdHandlers.get(cmd.getName().toUpperCase());
        if (h != null) {
            try {
            ret = h.handleCommand(cmd);
            } catch(Throwable e) {
                ret = new AtCmdResponse(AtCmdResponse.RESULT_ERROR, "+CME ERROR: 2");
            }
        } else {
            Log.e(LOG_TAG,"Unhandled command " + cmd);
            ret = new AtCmdResponse(AtCmdResponse.RESULT_ERROR, "+CME ERROR: 4");
        }
        return ret;
    }
}
