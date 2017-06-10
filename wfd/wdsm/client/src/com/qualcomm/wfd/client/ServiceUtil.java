/*
 * Copyright (c) 2012 - 2013 Qualcomm Technologies,Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.wfd.client;

import com.qualcomm.wfd.service.ISessionManagerService;
import static com.qualcomm.wfd.client.WfdOperationUtil.*;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.util.Log;

public class ServiceUtil {

	private static final String TAG = "Client.ServiceUtil";
	private static ISessionManagerService uniqueInstance = null;
	private static boolean mServiceAlreadyBound = false;
	private static Handler eventHandler = null;

	protected static boolean getmServiceAlreadyBound() {
		return mServiceAlreadyBound;
	}

	public static void bindService(Context context, Handler inEventHandler)
			throws ServiceFailedToBindException {
		if (!mServiceAlreadyBound  || uniqueInstance == null) {
			Log.d(TAG, "bindService- !mServiceAlreadyBound  || uniqueInstance == null");
			Intent serviceIntent = new Intent("com.qualcomm.wfd.service.WfdService");
                        serviceIntent.setPackage("com.qualcomm.wfd.service");
			eventHandler = inEventHandler;
			if (!context.bindService(serviceIntent, mConnection, Context.BIND_AUTO_CREATE)) {
				Log.e(TAG,"Failed to connect to Provider service");
				throw new ServiceFailedToBindException("Failed to connect to Provider service");
			}
		}
	}

	public static void unbindService(Context context) {
                if(mServiceAlreadyBound) {
                   context.unbindService(mConnection);
                   mServiceAlreadyBound = false;
                   uniqueInstance = null;
                }
	}

	public synchronized static ISessionManagerService getInstance() {
		while (uniqueInstance == null) {
			Log.d(TAG, "Waiting for service to bind ...");
			try {
				ServiceUtil.class.wait();
			} catch (InterruptedException e) {
				Log.e(TAG, "InterruptedException: " + e);
			}
		}
		return uniqueInstance;
	}

	public static class ServiceFailedToBindException extends Exception {
		public static final long serialVersionUID = 1L;

		private ServiceFailedToBindException(String inString)
		{
			super(inString);
		}
	}

	protected static ServiceConnection mConnection = new ServiceConnection() {
		public void onServiceConnected(ComponentName className, IBinder service) {
			Log.d(TAG, "Connection object created");
			mServiceAlreadyBound = true;
			uniqueInstance = ISessionManagerService.Stub.asInterface(service);
			synchronized(ServiceUtil.class) {
				ServiceUtil.class.notifyAll();
			}
			Message messageBound = eventHandler.obtainMessage(SERVICE_BOUND);
			eventHandler.sendMessage(messageBound);
		}

		public void onServiceDisconnected(ComponentName className) {
			Log.d(TAG, "Remote service disconnected");
			mServiceAlreadyBound = false;
                        uniqueInstance = null;
			/*Toast.makeText(getApplicationContext(), "WFD service disconnected",
					Toast.LENGTH_SHORT).show();*/
		}
	};

}
