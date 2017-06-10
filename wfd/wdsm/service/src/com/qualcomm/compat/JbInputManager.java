/* ==============================================================================
 * JbInputManager.java
 *
 * Copyright (c) 2012 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 ============================================================================== */

package com.qualcomm.compat;

import android.hardware.input.InputManager;
import android.view.KeyEvent;
import android.view.MotionEvent;

public class JbInputManager extends VersionedInputManager {

	@Override
	public boolean injectPointerEvent(MotionEvent me, boolean sync) {
		return InputManager.getInstance().injectInputEvent(me,
				InputManager.INJECT_INPUT_EVENT_MODE_WAIT_FOR_RESULT);
	}

	@Override
	public boolean injectTrackballEvent(MotionEvent me, boolean sync) {
		return InputManager.getInstance().injectInputEvent(me,
				InputManager.INJECT_INPUT_EVENT_MODE_WAIT_FOR_RESULT);
	}

	@Override
	public boolean injectKeyEvent(KeyEvent ke, boolean sync) {
		return InputManager.getInstance().injectInputEvent(ke,
				InputManager.INJECT_INPUT_EVENT_MODE_WAIT_FOR_RESULT);
	}

}
