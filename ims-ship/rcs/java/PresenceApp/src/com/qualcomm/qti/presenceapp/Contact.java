/*************************************************************************
 Copyright (c)2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.
*************************************************************************/

package com.qualcomm.qti.presenceapp;

import java.util.TimerTask;



class Contact {
    String name;
    String phone;
    int availability;

    boolean isSubscriptionOn = false;
    boolean isCapabilityPollingSuccess = false;
    boolean isMultiSelected = false;
    boolean isExcluded = false;
    boolean isContactOptedIn = true;
    TimerTask mSubscribeTimerTask = null;

    private String individualContactURI = "";

    // XML Based param
    String statusIcon = "<Not Subscribed>";
    String basicStatus = "<Not Subscribed>";
    String timeStamp = "<Not Subscribed>";
    String audio = "<Not Subscribed>";
    String video = "<Not Subscribed>";
    String note = "<Not Subscribed>";

    String listContactUri = "<Not Subscribed>",
            listName = "<Not Subscribed>",
            listVersion = "<Not Subscribed>",
            listFullState = "<Not Subscribed>",
            resourceUri = "<Not Subscribed>",
            isVolteContact = "<Not Subscribed>",
            resourceId = "<Not Subscribed>",
            resourceState = "<Not Subscribed>",
            resourceReason = "<Not Subscribed>",
            resourceCid = "<Not Subscribed>",
            contactUri = "<Not Subscribed>",
            description = "<Not Subscribed>",
            version = "<Not Subscribed>",
            serviceId = "<Not Subscribed>",
            audioCapabilities = "<Not Subscribed>",
            videoCapabilities = "<Not Subscribed>";

    public Contact(String name, String phone, int availability, String status,
            String individualContactURI) {
        this.name = name;
        this.phone = phone;
        this.availability = availability;
        this.basicStatus = status;
        this.individualContactURI = individualContactURI;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getPhone() {
        return phone;
    }

    public void setPhone(String phone) {
        this.phone = phone;
    }

    public int getAvailability() {
        return availability;
    }

    public void setAvailability(int availability) {
        this.availability = availability;
    }

    public String getStatusIcon() {
        return statusIcon;
    }

    public void setStatusIcon(String statusIcon) {
        this.statusIcon = statusIcon;
    }

    public String getBasicStatus() {
        return basicStatus;
    }

    public void setBasicStatus(String basicStatus) {
        this.basicStatus = basicStatus;
    }

    public String getTimeStamp() {
        return timeStamp;
    }

    public void setTimeStamp(String timeStamp) {
        this.timeStamp = timeStamp;
    }

    public String getAudio() {
        return audio;
    }

    public void setAudio(String audio) {
        this.audio = audio;
    }

    public String getVideo() {
        return video;
    }

    public void setVideo(String video) {
        this.video = video;
    }

    public String getListContactUri() {
        return listContactUri;
    }

    public void setListContactUri(String listContactUri) {
        this.listContactUri = listContactUri;
    }

    public String getListName() {
        return listName;
    }

    public void setListName(String listName) {
        this.listName = listName;
    }

    public String getListVersion() {
        return listVersion;
    }

    public void setListVersion(String listVersion) {
        this.listVersion = listVersion;
    }

    public String getListFullState() {
        return listFullState;
    }

    public void setListFullState(String listFullState) {
        this.listFullState = listFullState;
    }

    public String getResourceUri() {
        return resourceUri;
    }

    public void setResourceUri(String resourceUri) {
        this.resourceUri = resourceUri;
    }

    public String getIsVolteContact() {
        return isVolteContact;
    }

    public void setIsVolteContact(String isVolteContact) {
        this.isVolteContact = isVolteContact;
    }

    public String getResourceId() {
        return resourceId;
    }

    public void setResourceId(String resourceId) {
        this.resourceId = resourceId;
    }

    public String getResourceState() {
        return resourceState;
    }

    public void setResourceState(String resourceState) {
        this.resourceState = resourceState;
    }

    public String getResourceReason() {
        return resourceReason;
    }

    public void setResourceReason(String resourceReason) {
        this.resourceReason = resourceReason;
    }

    public String getResourceCid() {
        return resourceCid;
    }

    public void setResourceCid(String resourceCid) {
        this.resourceCid = resourceCid;
    }

    public String getContactUri() {
        return contactUri;
    }

    public void setContactUri(String contactUri) {
        this.contactUri = contactUri;
    }

    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
    }

    public String getVersion() {
        return version;
    }

    public void setVersion(String version) {
        this.version = version;
    }

    public String getServiceId() {
        return serviceId;
    }

    public void setServiceId(String serviceId) {
        this.serviceId = serviceId;
    }

    public String getAudioCapabilities() {
        return audioCapabilities;
    }

    public void setAudioCapabilities(String audioCapabilities) {
        this.audioCapabilities = audioCapabilities;
    }

    public String getVideoCapabilities() {
        return videoCapabilities;
    }

    public void setVideoCapabilities(String videoCapabilities) {
        this.videoCapabilities = videoCapabilities;
    }

    public void setSubscriptionOnFlag(boolean flag) {
        this.isSubscriptionOn = flag;
    }

    public boolean getSubscriptionOnFlag() {
        return isSubscriptionOn;
    }

    @Override
    public String toString() {
        return "Contact [name=" + name + ", phone=" + phone
                + ", availability=" + availability + ", status=" + basicStatus
                + "]";
    }

    public void setContactExcluded() {
        this.isExcluded = true;
    }

    public boolean isContactExcluded() {
        return this.isExcluded;
    }

    public void setMultiSelected(boolean checkBoxStatus) {
        this.isMultiSelected = checkBoxStatus;
    }

    public boolean isMultiSelected() {
        return this.isMultiSelected;
    }

    public boolean getContactParticipation() {
        return isContactOptedIn;
    }

    public void setContactParticipation(boolean flag) {
        this.isContactOptedIn = flag;
    }

    public TimerTask getSubscribeTimerTask() {
        return this.mSubscribeTimerTask;
    }

    public void setSubscribeTimerTask(TimerTask t) {
        this.mSubscribeTimerTask = t;
    }

    public void setNote(String note) {
        this.note = note;
    }

    public String getNote() {
        return this.note;
    }

    public String getIndividualContactURI() {
        return individualContactURI;
    }

    public void setIndividualContactURI(String indContactURI) {
        individualContactURI = indContactURI;
    }
}
