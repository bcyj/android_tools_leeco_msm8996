/*
 * Copyright (c) 2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __WIFI_FTM_CONTROLLER_H__
#define __WIFI_FTM_CONTROLLER_H__

#include <sys/socket.h>
#include <sys/un.h>

class WifiFtmTest;

class WifiFtmController {
public:

    WifiFtmController(const char* socke_name);
    ~WifiFtmController();
    void start();

private:
    int processMessage(const char* msg);
    int processStartCmd(const char* msg);
    int processStopCmd(const char* msg);
    int processSetCmd(const char* msg);
    int processReportCmd(const char* msg);

    WifiFtmTest* test;
    int sock_fd;
    struct sockaddr_un client;
    socklen_t from_len;
};

#endif
