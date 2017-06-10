/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.database.table;

public abstract class Table {
    public static final String ID = "_id";
    public static final String AUTHORITY = "com.chrrs.cherrymusic";

    public abstract String getTableName();

    public abstract String getCreateTableSql();

}