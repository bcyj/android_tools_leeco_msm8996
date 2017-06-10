/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __PM_PERIPHERAL_H__
#define __PM_PERIPHERAL_H__

#include <list>
#include <time.h>

#include <utils/Mutex.h>
#include <utils/RefBase.h>

#include "qmi_idl_lib.h"
#include "qmi_csi_common.h"

#include "Client.h"
#include "pm-service.h"

using namespace std;

namespace android {

class Peripheral {
  public:
    // name         - peripheral device name as returned by libmdmdetect
    // node         - device file, as returned by libmdmdetect
    // subSystemId  - ssreq_qmi_ss_identifier_enum_type_v01
    // offTout      - How long to wait before power-off device in case
    //                last voter goes away
    // ackTout      - How long to wait for ACK's from APP clients before change
    //                peripehral device state
    // debugMode    - When set to true will not touch device file.
    //                Used for simulation
    Peripheral(char *name, char *node, int subSystemId, int offTout,
              int ackTout, bool debugMode);

    status_t init();
    void deInit();

    void addClient(Client *client);
    void removeClient(Client *client);
    bool findClient(Client *searched);

    status_t increaseVoters(Client *client);
    status_t decreaseVoters(Client *client, bool crash);

    void shutdownStart();
    void shutdownWait();

    status_t notifyClientAck(Client *client, enum pm_event evAck);

    const String8 &getName();
    int getState();
    void dumpStatistics();
    int getSubSytemId();

    bool setRestartRequested(void *handle);
    bool isRestartRequested();
    void sendRestartIndication();

    void addPermissions(int cnt, int perm[]);
    bool checkPermission(int perm);

  private:
    // The name of the peripheral
    String8 mName;
    // The QMI client identifier used in QMI communication
    int mSubSytemId;
    // The node in the file system used to control on/off state
    String8 mDeviceFile;

    mutable Mutex mLock;
    // Accessed from notification-timeout-handler
    // All APP clients that are currently registered
    list<Client *> mClients;
    // Set to true when last voter crashed. This instruct peripheral
    // control class to keep device powered up, even after last listener
    // acknowledge going-off-line event, until mAckTimer expires with
    // mOffTimeout. This will give a chance to APP Peripheral manager client
    // (probably RIL) to restart. This we will avoid peripheral device power
    // cycle, which in case of Radio Modem is important.
    bool mLastClientCrashed;

    // How many clients are voting for this peripheral to be powered on.
    // If 0 => no voters and peripheral should be shut down.
    int mNumVoters;
    // Set to to true when peripheral have to be shutdownStart no matter how
    // many voters it has. Subsequent requests for power-on will be refused.
    bool mShutdown;
    // Posted when all clients acknowledge off-line device
    // state or timeout expire
    mutable Mutex mShutdownComplete;
    // Current state of the peripheral, defined in pm_event enum
    enum pm_event mState;
    // File descriptor obtained by opening the mDeviceFile
    int mFileDesc;
    // The last time when a change of the state occurred in CLOCK_MONOTONIC time
    // scale. E.g. transition between on and off
    nsecs_t mStateCanaged;
    // How many times the peripheral has been on
    int mPowerOnCount;
    // How many times the peripheral has been off
    int mPowerOffCount;
    int mMaxVotersCount;
    int mMaxListenersCount;
    // Timer used to wait for clients acknowledge
    timer_t mAckTimer;
    // Wait this time for Ack from APP clients before device power-off
    int mOffTimeout;
    // Wait this time for Ack from APP clients before transition to new state
    int mAckTimeout;
    // For debug purposes, when set to false will not touch
    // device file. Used for simulation.
    bool mSimulationMode;
    // QMI client handle. Assume that only one QMI client could send SSREQ
    // for particular peripheral. Used to send back an indication to QMI
    // clients. Non NULL if SS request for restart has come and we are in
    // a process of closing and then opening the device
    void *mRestartRequestor;
    // User Ids that can access this peripheral. Defined in
    // namespace android_filesystem_config.h
    list<int> mPermissions;

    const char *getStateName();
    void notifyClients(enum pm_event ev);
    int timerStart(int ms);
    void timerStop();
};

}          ; // namespace android
#endif
