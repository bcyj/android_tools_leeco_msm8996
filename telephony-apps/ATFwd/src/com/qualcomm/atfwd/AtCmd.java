/*******************************************************************************
    Copyright (c) 2010,2011 Qualcomm Technologies, Inc.  All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

package com.qualcomm.atfwd;

import android.os.Parcel;
import android.os.Parcelable;

public class AtCmd implements Parcelable {

    public static final int AT_OPCODE_NA = 1;
    public static final int AT_OPCODE_EQ = 2;
    public static final int AT_OPCODE_QU = 4;
    public static final int AT_OPCODE_AR = 8;

    // AT_OPCODE_NA | AT_OPCODE_QU
    public static final int ATCMD_OPCODE_NA_QU    = 5;
    // AT_OPCODE_NA | AT_OPCODE_EQ | AT_OPCODE_QU
    public static final int ATCMD_OPCODE_NA_EQ_QU = 7;
    // AT_OPCODE_NA | AT_OPCODE_EQ | AT_OPCODE_AR
    public static final int ATCMD_OPCODE_NA_EQ_AR = 11;

    // Error codes are defined as per 3GPP spec TS 27.007, Section 9.2.1.
    public static final int AT_ERR_PHONE_FAILURE      = 0;
    public static final int AT_ERR_NO_CONN_TO_PHONE   = 1;
    public static final int AT_ERR_OP_NOT_ALLOW       = 3;
    public static final int AT_ERR_OP_NOT_SUPP        = 4;
    public static final int AT_ERR_INCORRECT_PASSWORD = 16;
    public static final int AT_ERR_NOT_FOUND          = 22;
    public static final int AT_ERR_INVALID_CHARS      = 25;
    public static final int AT_ERR_INCORRECT_PARAMS   = 50;
    public static final int AT_ERR_UNKNOWN            = 100;

    private String mName;
    private String mTokens[];
    private int mOpcode;

    public int getOpcode() {
        return mOpcode;
    }

    public void setOpcode(int mOpcode) {
        this.mOpcode = mOpcode;
    }

    public String getName() {
        return mName;
    }

    public void setName(String mName) {
        this.mName = mName;
    }

    public String[] getTokens() {
        return mTokens;
    }

    public void setTokens(String[] mTokens) {
        this.mTokens = mTokens;
    }

    public AtCmd(int opcode, String name, String tokens [])
    {
        init(opcode,name,tokens);
    }

    private AtCmd(Parcel source) {
        int opcode = source.readInt();
        String name = source.readString();
        String []tokens = source.readStringArray();
        init(opcode,name,tokens);
    }

    private void init(int opcode, String name, String tokens[]) {
        mOpcode = opcode;
        mName = name;
        mTokens = tokens;
    }

    public int describeContents() {
        // TODO Auto-generated method stub
        return 0;
    }

    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mOpcode);
        dest.writeString(mName);
        dest.writeStringArray(mTokens);
    }

    public String toString() {
        String ret = "AtCmd { opcode = " + mOpcode + ", name = " + mName + " mTokens = {";
        for (String token : mTokens) {
            ret += " " + token + ",";
        }
        ret += "}";
        return ret;
    }

    public String getAtCmdErrStr(int errCode) {

        /* Per 3GPP spec TS 27.007, section 9.2 states as follows :
         * "The format of <err> can be either numeric or verbose"
         * By default this method sends the error result in numeric format.
         * TBD :Consider cmee val (0,1,2) before sending relevant error string
         *      Note that cmee_val is treated as 1 while sending error strings
         */
        String errStrResult = "+CME ERROR: 100"; //default err string

        switch(errCode) {
            case AT_ERR_PHONE_FAILURE:
                errStrResult = "+CME ERROR: 0";
                break;
            case AT_ERR_NO_CONN_TO_PHONE:
                errStrResult = "+CME ERROR: 1";
                break;
            case AT_ERR_OP_NOT_ALLOW:
                errStrResult = "+CME ERROR: 3";
                break;
            case AT_ERR_OP_NOT_SUPP:
                errStrResult = "+CME ERROR: 4";
                break;
            case AT_ERR_INCORRECT_PASSWORD:
                errStrResult = "+CME ERROR: 16";
                break;
            case AT_ERR_NOT_FOUND:
                errStrResult = "+CME ERROR: 22";
                break;
            case AT_ERR_INVALID_CHARS:
                errStrResult = "+CME ERROR: 25";
                break;
            case AT_ERR_INCORRECT_PARAMS:
                errStrResult = "+CME ERROR: 50";
                break;
            case AT_ERR_UNKNOWN:
                break;
            default :
                break;
        }

        return errStrResult;
    }

    public static final Parcelable.Creator<AtCmd> CREATOR = new Parcelable.Creator<AtCmd>() {

        public AtCmd createFromParcel(Parcel source) {
            AtCmd ret = new AtCmd(source);
            return ret;
        }

        public AtCmd[] newArray(int size) {
            return new AtCmd[size];
        }

    };
}
