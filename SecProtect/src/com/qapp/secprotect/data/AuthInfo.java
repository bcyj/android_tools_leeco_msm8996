/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.secprotect.data;

public class AuthInfo {

    public int uid = -1;
    public int gid = -1;
    public String packageName = "unknown";
    public int mode = 0;
    public int remember = 0;
    public long time = -1;
    public String lastPath = "unknown";
    public int pid;

    public static final int DENY = -1;
    public static final int PROMPT = 0;
    public static final int ALLOW = 1;

    public AuthInfo(int uid, int gid, String packageName, String lastPath,
            int mode, int remember, int time) {
        this.uid = uid;
        this.gid = gid;
        this.packageName = packageName;
        this.lastPath = lastPath;
        this.mode = mode;
        this.remember = remember;
        this.time = time;
    }

    public AuthInfo() {
    }

    @Override
    public String toString() {
        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append("uid=" + uid + ";");
        stringBuilder.append("gid=" + gid + ";");
        stringBuilder.append("package_name=" + packageName + ";");
        stringBuilder.append("last_path=" + lastPath + ";");
        stringBuilder.append("mode=" + mode + ";");
        stringBuilder.append("remember=" + remember + ";");
        stringBuilder.append("time=" + time + ";");
        return stringBuilder.toString();
    }
}
