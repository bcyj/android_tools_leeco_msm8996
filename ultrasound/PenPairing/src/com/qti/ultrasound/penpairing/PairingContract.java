/******************************************************************************
 * ---------------------------------------------------------------------------
 *  Copyright (C) 2013-2014 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *******************************************************************************/

package com.qti.ultrasound.penpairing;

import android.provider.BaseColumns;

public final class PairingContract {
    // To prevent someone from accidentally instantiating the contract class,
    // give it an empty constructor.
    public PairingContract() {
    }

    // Inner class that defines the table contents
    public static abstract class PairingEntry implements BaseColumns {
        public static final String TABLE_NAME = "pens";

        public static final String COLUMN_NAME_PEN_ID = "pen_id";

        public static final String COLUMN_NAME_PEN_TYPE = "pen_type";

        public static final String COLUMN_NAME_PEN_NAME = "pen_name";
    }
}
