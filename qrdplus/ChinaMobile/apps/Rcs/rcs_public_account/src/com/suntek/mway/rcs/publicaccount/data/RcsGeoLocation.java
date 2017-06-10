/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount.data;

public class RcsGeoLocation {
    private double lat;
    private double lng;
    private String lable;

    public double getLat() {
        return lat;
    }

    public void setLat(double lat) {
        this.lat = lat;
    }

    public double getLng() {
        return lng;
    }

    public void setLng(double lng) {
        this.lng = lng;
    }

    public String getLabel() {
        return lable;
    }

    public void setLabel(String lable) {
        this.lable = lable;
    }

    @Override
    public String toString() {
        return "GeoLocation [lat=" + lat + ", lng=" + lng + ", lable=" + lable + "]";
    }


}
