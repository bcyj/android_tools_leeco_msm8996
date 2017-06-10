/*
 * Copyright (c) 2011-2013, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.factory.Bluetooth;
class DeviceInfo{
    private String name="";
    private String address="";
    
    public DeviceInfo(String name, String address) {

        super();
        this.name = name;
        this.address = address;
    }

    public String getName() {
    
        return name;
    }
    
    public void setName(String name) {
    
        this.name = name;
    }
    
    public String getAddress() {
    
        return address;
    }
    
    public void setAddress(String address) {
    
        this.address = address;
    }
    
}