/* ==============================================================================
 * IWfdActionListener.aidl
 *
 * Copyright (c) 2012 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 ============================================================================== */

package com.qualcomm.wfd.service;

import android.os.Bundle;

oneway interface IWfdActionListener {

	void onStateUpdate(int newState, int sessionId);

	void notifyEvent(int event, int sessionId);

	void notify(in Bundle b, int sessionId);

}
