/*******************************************************************************
    Copyright (c) 2010,2011 Qualcomm Technologies, Inc.  All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

package com.qualcomm.atfwd;

import com.qualcomm.atfwd.AtCmd;
import com.qualcomm.atfwd.AtCmdResponse;

/** {@hide} */
interface IAtCmdFwd {

    AtCmdResponse processAtCmd(in AtCmd cmd);

}

