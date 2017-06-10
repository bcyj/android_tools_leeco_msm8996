/*
 * Copyright (c) 2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "WifiFtmTest.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <signal.h>

#define LOG_TAG "WifiFtm"
#include <cutils/log.h>
#include <cutils/misc.h>

#define WIFI_DRIVER_MODULE_PATH         "/system/lib/modules/wlan.ko"
#define WIFI_DRIVER_MODULE_NAME         "wlan"

#define WLAN_FTM_PRIV_SET_INT_GET_NONE    (SIOCIWFIRSTPRIV + 0)
#define WE_FTM_ON_OFF         1
#define WE_TX_PKT_GEN         2
#define WE_SET_CHANNEL        6
#define WE_SET_TX_POWER       7
#define WE_CLEAR_RX_PKT_CNT   8
#define WE_RX                 9
#define WE_SET_PWR_CNTL_MODE 11
#define WE_ENABLE_CHAIN      10
#define WE_ENABLE_DPD        12
#define WE_TX_CW_RF_GEN		 14

#define WLAN_FTM_PRIV_SET_VAR_INT_GET_NONE   (SIOCIWFIRSTPRIV + 7)
#define WE_SET_TX_WF_GAIN  1

/* Private ioctls and their sub-ioctls */
#define WLAN_FTM_PRIV_SET_NONE_GET_INT    (SIOCIWFIRSTPRIV + 1)
#define WE_GET_RX_PKT_CNT   3

#define WLAN_FTM_PRIV_SET_CHAR_GET_NONE   (SIOCIWFIRSTPRIV + 3)
#define WE_SET_TX_RATE       2

WifiFtmTest* WifiFtmTest::ftm_test = 0;

extern "C" int init_module(void *, unsigned long, const char *);
extern "C" int delete_module(const char *, unsigned int);

WifiFtmTest::WifiFtmTest() :
        channel(1), tx_rate(0), pwr_mode(1), module_loaded(0), socketfd(-1) {

    if (insmod(WIFI_DRIVER_MODULE_PATH, "con_mode=5") < 0)
        ALOGE("Failed to insert wifi module");
    else {
        module_loaded = 1;
    }

    tx_rate = new char[64];
    socketfd = socket(PF_INET, SOCK_DGRAM, 0);

}

WifiFtmTest* WifiFtmTest::createWifiFtmTest() {

    if (ftm_test == 0) {
        ftm_test = new WifiFtmTest();
    }

    return ftm_test;
}

WifiFtmTest::~WifiFtmTest() {
    if (socketfd >= 0)
        close(socketfd);

    delete[] tx_rate;

    if (module_loaded && rmmod(WIFI_DRIVER_MODULE_NAME) < 0)
        ALOGE("Failed to remove wifi module");

}

void WifiFtmTest::setChannel(unsigned int _channel) {
    channel = _channel;
}

void WifiFtmTest::setTxPower(unsigned int _tx_power) {

    tx_power = _tx_power;
}

void WifiFtmTest::setPowerMode(unsigned int _pwr_mode) {

    pwr_mode = _pwr_mode;
}

void WifiFtmTest::setTxRate(char* _tx_rate) {
    int len = strlen(_tx_rate);
    len = (len > 63 ? 63 : len);
    strncpy(tx_rate, _tx_rate, len);
    tx_rate[len] = '\0';
}

void WifiFtmTest::startFtm() {
    buildIntSetIwReq(WE_FTM_ON_OFF, 1);
    if (doIoctl(WLAN_FTM_PRIV_SET_INT_GET_NONE))
        ALOGE("start ftm failed");
}

void WifiFtmTest::stopFtm() {
    buildIntSetIwReq(WE_FTM_ON_OFF, 0);
    if (doIoctl(WLAN_FTM_PRIV_SET_INT_GET_NONE))
        ALOGE("stop ftm failed");
}

void WifiFtmTest::startTx() {
    buildIntSetIwReq(WE_TX_PKT_GEN, 0);
    if (doIoctl(WLAN_FTM_PRIV_SET_INT_GET_NONE) < 0)
        ALOGE("stop tx failed");

    buildIntSetIwReq(WE_SET_CHANNEL, channel);
    if (doIoctl(WLAN_FTM_PRIV_SET_INT_GET_NONE) < 0)
        ALOGE("set channel failed");

    if (channel >= 36) { //for 5G, set channel twice
        if (doIoctl(WLAN_FTM_PRIV_SET_INT_GET_NONE) < 0)
            ALOGE("set channel failed");
    }

    buildIntSetIwReq(WE_ENABLE_CHAIN, 2);
    if (doIoctl(WLAN_FTM_PRIV_SET_INT_GET_NONE) < 0)
        ALOGE("ena_chain failed");

    buildIntSetIwReq(WE_SET_PWR_CNTL_MODE, pwr_mode); //scpc or clpc
    if (doIoctl(WLAN_FTM_PRIV_SET_INT_GET_NONE) < 0)
        ALOGE("Set pwr control mode failed");

    if (channel >= 36) {
        //do dpd here
        buildIntSetIwReq(WE_ENABLE_DPD, 1);
        if (doIoctl(WLAN_FTM_PRIV_SET_INT_GET_NONE) < 0)
            ALOGE("enable dpd failed");
    }

    buildIntSetIwReq(WE_SET_TX_POWER, tx_power);
    if (doIoctl(WLAN_FTM_PRIV_SET_INT_GET_NONE) < 0)
        ALOGE("set tx power failed, %d", tx_power);

    buildCharSetIwReq(WE_SET_TX_RATE, tx_rate);
    if (doIoctl(WLAN_FTM_PRIV_SET_CHAR_GET_NONE) < 0)
        ALOGE("set tx rate failed, %s", tx_rate);

    buildIntSetIwReq(WE_TX_PKT_GEN, 1);
    if (doIoctl(WLAN_FTM_PRIV_SET_INT_GET_NONE) < 0)
        ALOGE("start tx failed");
}

void WifiFtmTest::stopTx() {
    buildIntSetIwReq(WE_TX_PKT_GEN, 0);
    if (doIoctl(WLAN_FTM_PRIV_SET_INT_GET_NONE))
        ALOGE("stop tx failed");
}

void WifiFtmTest::startRx() {
    buildIntSetIwReq(WE_SET_CHANNEL, channel);
    if (doIoctl(WLAN_FTM_PRIV_SET_INT_GET_NONE) < 0)
        ALOGE("set channel failed");

    buildIntSetIwReq(WE_RX, 0);
    if (doIoctl(WLAN_FTM_PRIV_SET_INT_GET_NONE))
        ALOGE("stop rx failed");

    buildIntSetIwReq(WE_CLEAR_RX_PKT_CNT, 1);
    if (doIoctl(WLAN_FTM_PRIV_SET_INT_GET_NONE))
        ALOGE("clr rx failed");

    buildIntSetIwReq(WE_RX, 1);
    if (doIoctl(WLAN_FTM_PRIV_SET_INT_GET_NONE))
        ALOGE("start rx failed");
}

void WifiFtmTest::stopRx() {
    buildIntSetIwReq(WE_RX, 0);
    if (doIoctl(WLAN_FTM_PRIV_SET_INT_GET_NONE))
        ALOGE("stop rx failed");

    buildIntSetIwReq(WE_CLEAR_RX_PKT_CNT, 1);
    if (doIoctl(WLAN_FTM_PRIV_SET_INT_GET_NONE))
        ALOGE("clr rx failed");
}

void WifiFtmTest::startSCW() {
    buildIntSetIwReq(WE_SET_CHANNEL, channel);
    if (doIoctl(WLAN_FTM_PRIV_SET_INT_GET_NONE) < 0)
        ALOGE("set channel failed");
    buildIntSetIwReq(WE_ENABLE_CHAIN, 2);
    if (doIoctl(WLAN_FTM_PRIV_SET_INT_GET_NONE) < 0)
        ALOGE("ena_chain failed");
    buildIntSetIwReq1(WE_SET_TX_WF_GAIN, 0, 0);
    if (doIoctl(WLAN_FTM_PRIV_SET_VAR_INT_GET_NONE) < 0)
        ALOGE("set tx gain failed");
    buildIntSetIwReq(WE_TX_CW_RF_GEN, 1);
    if (doIoctl(WLAN_FTM_PRIV_SET_INT_GET_NONE) < 0)
        ALOGE("gen waveform failed");
}

void WifiFtmTest::stopSCW() {
    buildIntSetIwReq(WE_TX_CW_RF_GEN, 0);
    if (doIoctl(WLAN_FTM_PRIV_SET_INT_GET_NONE) < 0)
        ALOGE("Stop gen waveform failed");
}

int WifiFtmTest::reportRx() {
    buildIntGetIwReq(WE_GET_RX_PKT_CNT);
    if (doIoctl(WLAN_FTM_PRIV_SET_NONE_GET_INT))
        ALOGE("report rx failed");
    return ((int *) (wrq.u.name))[0];
}

void WifiFtmTest::buildCharSetIwReq(int subcmd, const char* buf) {
    memset(&wrq, 0, sizeof(wrq));
    strncpy(wrq.ifr_name, "wlan0", 5);
    wrq.u.data.length = strlen(buf) + 1;
    if (wrq.u.data.length > 64)
        wrq.u.data.length = 64;

    wrq.u.data.pointer = (caddr_t) buf;
    wrq.u.data.flags = subcmd;
}

void WifiFtmTest::buildIntSetIwReq(int subcmd, int value) {
    memset(&wrq, 0, sizeof(wrq));
    strncpy(wrq.ifr_name, "wlan0", 5);
    wrq.u.data.length = 1;
    wrq.u.mode = subcmd;
    ((int *) (wrq.u.name))[0] = subcmd;
    ((int *) (wrq.u.name))[1] = value;
}

void WifiFtmTest::buildIntSetIwReq1(int subcmd, int dgain, int rfgain) {
    memset(&wrq, 0 , sizeof(wrq));
    strncpy(wrq.ifr_name, "wlan0", 5);
    wrq.u.data.length = 2;
    wrq.u.data.flags = subcmd;
    gain[0] = dgain;
    gain[1] = rfgain;
    wrq.u.data.pointer = gain;
}

void WifiFtmTest::buildIntGetIwReq(int subcmd) {
    memset(&wrq, 0, sizeof(wrq));
    strncpy(wrq.ifr_name, "wlan0", 5);
    wrq.u.data.length = 1;
    wrq.u.mode = subcmd;
}

int WifiFtmTest::doIoctl(int cmd) {
    if (ioctl(socketfd, cmd, &wrq) < 0) {
        ALOGE("do ioctl failed");
        return (-1);
    }

    return 0;

}

int WifiFtmTest::insmod(const char *filename, const char *args) {
    void *module;
    unsigned int size;
    int ret;

    module = load_file(filename, &size);
    if (!module)
        return -1;

    ret = ::init_module(module, size, args);

    free(module);

    return ret;
}

int WifiFtmTest::rmmod(const char *modname) {
    int ret = -1;
    int maxtry = 10;

    while (maxtry-- > 0) {
        ret = ::delete_module(modname, O_NONBLOCK | O_EXCL);
        if (ret < 0 && errno == EAGAIN)
            usleep(500000);
        else
            break;
    }

    if (ret != 0)
        ALOGD("Unable to unload driver module \"%s\": %s\n", modname,
                strerror(errno));
    return ret;
}
