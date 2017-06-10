/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.gesture;

interface IGestureManager{

    boolean onTouch(int x, int y);

    boolean onTouchDown();

    boolean onTouchUp();

}