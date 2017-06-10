/*
 *Copyright (c) 2014 Qualcomm Technologies, Inc.
 *All Rights Reserved.
 *Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.ims.intermediate;

import android.content.Context;

public interface IComponentFactory {

    IMediaHandlerInterface getMediaHandlerInterface();

    ICallInterface getCallInterface(Context context);

    ICameraHandlerInterface getCameraHandler(Context context);

}