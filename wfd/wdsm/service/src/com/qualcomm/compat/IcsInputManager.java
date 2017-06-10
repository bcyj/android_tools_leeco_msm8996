/* ==============================================================================
 * IcsInputManager.java
 *
 * Copyright (c) 2012 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 ============================================================================== */

package com.qualcomm.compat;

import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;
import android.view.IWindowManager;
import android.view.KeyEvent;
import android.view.MotionEvent;

public class IcsInputManager extends VersionedInputManager {
	
	private static final String TAG = "IcsInputManager";

	private IWindowManager wm;

	public IcsInputManager() {
		wm = IWindowManager.Stub.asInterface(ServiceManager
				.getService("window"));
		if (wm == null) {
			Log.e(TAG, "Unable to connect to window manager.");
		}
	}

	@Override
	public boolean injectPointerEvent(MotionEvent me, boolean sync) throws RemoteException {
		if (wm != null) {
			return wm.injectPointerEvent(me, sync);
		}
		return false;
	}

	@Override
	public boolean injectTrackballEvent(MotionEvent me, boolean sync) throws RemoteException {
		if (wm != null) {
			return wm.injectTrackballEvent(me, sync);
		}
		return false;
	}

	@Override
	public boolean injectKeyEvent(KeyEvent ke, boolean sync) throws RemoteException {
		if (wm != null) {
			return wm.injectKeyEvent(ke, sync);
		}
		return false;
	}

}
