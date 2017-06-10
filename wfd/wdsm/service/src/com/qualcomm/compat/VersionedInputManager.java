/* ==============================================================================
 * VersionedInputManager.java
 *
 * Copyright (c) 2012 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 ============================================================================== */

package com.qualcomm.compat;

import android.os.Build;
import android.os.RemoteException;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;

public abstract class VersionedInputManager {
	private static final String TAG = "VersionedInputManager";

	public abstract boolean injectPointerEvent(MotionEvent me, boolean sync)
			throws RemoteException;

	public abstract boolean injectTrackballEvent(MotionEvent me, boolean sync)
			throws RemoteException;

	public abstract boolean injectKeyEvent(KeyEvent ke, boolean sync)
			throws RemoteException;

	@SuppressWarnings("unchecked")
	public static VersionedInputManager newInstance()
			throws InstantiationException, IllegalAccessException,
			ClassNotFoundException {
		Class<VersionedInputManager> vim;
		if (Build.VERSION.SDK_INT <= Build.VERSION_CODES.ICE_CREAM_SANDWICH_MR1) {
			Log.d(TAG, "Instantiating ICS input manager");
			vim = (Class<VersionedInputManager>) Class
					.forName("com.qualcomm.compat.IcsInputManager");
		} else {
			Log.d(TAG, "Instantiating JB input manager");
			vim = (Class<VersionedInputManager>) Class
					.forName("com.qualcomm.compat.JbInputManager");
		}
		return vim.newInstance();
	}
}
