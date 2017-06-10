/*
 * Copyright (c) 2011-2012, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * Developed by QRD Engineering team.
 */
package com.android.profile;

public class Profile {

    private int id = -1;
    private String profile_name = "new";
    private boolean silent = false;
    private boolean vibrate = true;
    private int ringVolume = 0;
    private boolean data = true;
    private boolean wifi = false;
    private boolean bluetooth = false;
    private boolean gpsLocation = false;
    private boolean networkLocation = false;

    private int brightness = 0;
    private String ringtone1 = null;
    private String ringtone2 = null;

    private boolean matchCurProfile = false;

    public Profile() {

    }

    public Profile(String name) {

        profile_name = name;
    }

    public Profile(String name, int id) {

        profile_name = name;
        this.id = id;
    }

    public Profile(String profile_name, boolean silent, boolean vibrate, String ringtone1,
            String ringtone2, int ring_volume, boolean data, boolean wifi, boolean bluetooth,
            boolean gpsLocation, boolean networkLocation, int brightness) {

        this.profile_name = profile_name;
        this.silent = silent;
        this.vibrate = vibrate;
        this.ringtone1 = ringtone1;
        this.ringtone2 = ringtone2;
        this.ringVolume = ring_volume;
        this.data = data;
        this.wifi = wifi;
        this.bluetooth = bluetooth;
        this.gpsLocation = gpsLocation;
        this.networkLocation = networkLocation;
        this.brightness = brightness;
    }

    public int getId() {

        return id;
    }

    public void setId(int id) {

        this.id = id;
    }

    public String getProfileName() {

        return profile_name;
    }

    public void setProfileName(String profileName) {

        profile_name = profileName;
    }

    public boolean isSilent() {

        return silent;
    }

    public void setSilent(boolean silent) {

        this.silent = silent;
    }

    public boolean isVibrate() {

        return vibrate;
    }

    public void setVibrate(boolean vibrate) {

        this.vibrate = vibrate;
    }

    public int getRingVolume() {

        return ringVolume;
    }

    public void setRingVolume(int ringVolume) {

        this.ringVolume = ringVolume;
    }

    public boolean isData() {

        return data;
    }

    public void setData(boolean data) {

        this.data = data;
    }

    public boolean isWifi() {

        return wifi;
    }

    public void setWifi(boolean wifi) {

        this.wifi = wifi;
    }

    public boolean isBluetooth() {

        return bluetooth;
    }

    public void setBluetooth(boolean bluetooth) {

        this.bluetooth = bluetooth;
    }

    public int getBrightness() {

        return brightness;
    }

    public void setBrightness(int brightness) {

        this.brightness = brightness;
    }

    public String getRingtone1() {

        return ringtone1;
    }

    public void setRingtone1(String ringtone1) {

        this.ringtone1 = ringtone1;
    }

    public String getRingtone2() {

        return ringtone2;
    }

    public void setRingtone2(String ringtone2) {

        this.ringtone2 = ringtone2;
    }

    public boolean isMatchCurProfile() {

        return matchCurProfile;
    }

    public void setMatchCurProfile(boolean matchCurProfile) {

        this.matchCurProfile = matchCurProfile;
    }

    public boolean isGpsLocation() {

        return gpsLocation;
    }

    public void setGpsLocation(boolean gpsLocation) {

        this.gpsLocation = gpsLocation;
    }

    public boolean isNetworkLocation() {

        return networkLocation;
    }

    public void setNetworkLocation(boolean networkLocation) {

        this.networkLocation = networkLocation;
    }

}
