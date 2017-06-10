/*************************************************************************
        Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
                   Qualcomm Technologies Proprietary and Confidential.
 **************************************************************************/

package com.android.iperfapp;

public class IPerfResultSummary {
    private String hostname;

    private String clientname;

    private String throughput;

    private float totaltime;

    private float totaldatatransfer;

    public void setHostname(String hostname) {
        this.hostname = hostname;
    }

    public String getHostname() {
        return hostname;
    }

    public void setClientname(String clientname) {
        this.clientname = clientname;
    }

    public String getClientname() {
        return clientname;
    }

    public void setThroughput(String throughput) {
        this.throughput = throughput;
    }

    public String getThroughput() {
        return throughput;
    }

    public void setTotaldatatransfer(float totaldatatransfer) {
        this.totaldatatransfer = totaldatatransfer;
    }

    public float getTotaldatatransfer() {
        return totaldatatransfer;
    }

    public void setTotaltime(float totaltime) {
        this.totaltime = totaltime;
    }

    public float getTotaltime() {
        return totaltime;
    }

}
