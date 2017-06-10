/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

package com.qualcomm.location.lci;

import java.util.Map;
import java.util.HashMap;

/**
 * @hide
 */
public enum LciStatusCode{
    SUCCESS("Success", 0),
    FAIL("Fail", 1),
    FAIL_APP_NOT_FOREGROUND("Fail. App not in foreground.", 2),
    FAIL_CALL_NOT_SCREEN_ON("Fail. Call while screen not on.", 3),
    UNKNOWN("Unknown", -1);

    private final String name;
    private final int value;

    private LciStatusCode(String name, int value) {
        this.name = name;
        this.value = value;
    }

    public int getInt() {
        return value;
    }

    @Override
    public String toString() {
        return name;
    }

    public static LciStatusCode fromInt(int i) {
        LciStatusCode type = intToTypeMap.get(Integer.valueOf(i));
        if (type == null)
            return LciStatusCode.UNKNOWN;
        return type;
    }

    private static final Map<Integer, LciStatusCode> intToTypeMap = new HashMap<Integer, LciStatusCode>();
    static {
        for (LciStatusCode type : LciStatusCode.values()) {
            intToTypeMap.put(type.value, type);
        }
    }
}
