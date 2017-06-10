/*
 * Copyright (c) 2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __WIFI_FTM_TEST_H__
#define __WIFI_FTM_TEST_H__

#include <sys/socket.h>
#include <linux/wireless.h>

class WifiFtmTest {
public:

    static WifiFtmTest* createWifiFtmTest();
    ~WifiFtmTest();

    void startFtm();
    void stopFtm();

    void setChannel(unsigned int _channel);
    void setTxPower(unsigned int _tx_power);
    void setPowerMode(unsigned int _pwr_mode);
    void setTxRate(char* _tx_rate);
    void startTx();
    void stopTx();

    void startRx();
    void stopRx();

    void startSCW();
    void stopSCW();

    int reportRx();

private:
    unsigned int channel;
    unsigned int tx_power;
    unsigned int pwr_mode;

    char* tx_rate;
    int module_loaded;
    int socketfd;
    struct iwreq wrq;
    static WifiFtmTest* ftm_test;
    int gain[2];

    WifiFtmTest();
    void buildCharSetIwReq(int subcmd, const char* buf);
    void buildIntSetIwReq(int subcmd, int value);
    void buildIntGetIwReq(int subcmd);
    void buildIntSetIwReq1(int subcmd, int value1, int value2);
    int doIoctl(int cmd);

    static int insmod(const char *filename, const char *args);
    static int rmmod(const char *modname);

};

#endif
