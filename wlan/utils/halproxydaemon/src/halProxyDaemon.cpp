/*
* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*
*/

#include "halLog.hpp"
#include "wifi_hal.h"
#ifdef NAN_2_0
#include "nan_test.hpp"
#endif
#include "gscan_test.hpp"
#include "llstats_test.hpp"
#include "rtt_test.hpp"
#include "tdls_test.hpp"
#include "wifihal_test.hpp"

#include <netinet/in.h>
#include <sys/select.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <poll.h>


#undef LOG_TAG
#define LOG_TAG "HalProxyDaemon"

#define UDP_PORT 51000

wifi_handle gbl_handle;

#ifndef HAL_PROXY_DAEMON_VER
#define HAL_PROXY_DAEMON_VER "1.0.0"
#endif /* HAL_PROXY_DAEMON_VER */

wifi_interface_handle wifi_get_iface_handle(wifi_handle handle, char *name);
#define SERVER "127.0.0.1"
#define BUFLEN 512  /* Max length of buffer */
#define PORT 8888   /* The port on which to send data */
struct sockaddr_in si;
int app_sock, s2, i, slen=sizeof(si);
char buf[BUFLEN];
char message[BUFLEN];

namespace HAL
{
    class HalProxyDaemon
    {
    public:
        HalProxyDaemon(
            int argc,
            char** argv);

        ~HalProxyDaemon();

        inline bool
        isGood() const
            {
                return isGood_;
            }

        inline wifi_handle
        getWifiHandle()
            {
                return wifiHandle_;
            }
        int
        operator()();

    private:
        enum MsgTypes
        {
            /* Control messages */
            MSG_TYPE_ERROR_RSP         = 0,
            MSG_TYPE_DUMP_STATS_REQ    = 1,
            MSG_TYPE_DUMP_STATS_RSP    = 2,
            MSG_TYPE_CLEAR_STATS_REQ   = 3,
            MSG_TYPE_CLEAR_STATS_RSP   = 4,
            MSG_TYPE_REGISTER_REQ      = 5,
            MSG_TYPE_REGISTER_RSP      = 6,
            MSG_TYPE_DEREGISTER_REQ    = 7,
            MSG_TYPE_DEREGISTER_RSP    = 8,

            /* Data messages */
            MSG_TYPE_RAW_REQ           = 9,
            MSG_TYPE_EVENT             = 10
        };

        void
        hexdump(
            uint8_t* data,
            size_t len);

        void
        clearStats();

        void
        usage() const;

        int
        parseOptions(
            int& argc,
            char** argv);

        const char* program_;
        bool isGood_;

        wifi_handle wifiHandle_;

        struct Stats_t {
            uint32_t numUdpRawReqRcvd_;
            uint32_t numUdpMsgSent_;

            uint32_t numSuppRawReqSent_;
            uint32_t numSuppMsgRcvd_;
        } stats_;

        /* Not thread safe. */
        static const size_t MAX_BUF_SIZE = 2048;
        static char ipcMsgData_[ MAX_BUF_SIZE ];
    };

    /* We do a one-time allocation of this so we don't chew up stack
     * space for this.
     */
    char HalProxyDaemon::ipcMsgData_[
        MAX_BUF_SIZE ];

    HalProxyDaemon::HalProxyDaemon(
        int argc,
        char** argv) :
        program_(argv[0]),
        isGood_(false),
        wifiHandle_(NULL),
        stats_()
    {
        if (parseOptions(argc, argv) < 0)
        {
            HAL_LOG_MSG(
                HAL_LOG_LEVEL_ERROR,
                "parseOptions() failed");
            fprintf(stderr, "parseOptions() failed. \n");
            return;
        }
        clearStats();

        wifi_error err = wifi_initialize(&wifiHandle_);
        if (err)
        {
            HAL_LOG_MSG(
                HAL_LOG_LEVEL_ERROR,
                "wifi hal initialize failed");
            fprintf(stderr, "wifi hal initialize failed. \n");
            return;
        }

        HAL_LOG_MSG(
            HAL_LOG_LEVEL_WARN,
            "halProxyDaemon running");
        fprintf(stderr, "halProxyDaemon running. \n");

        isGood_ = true;
    }

    HalProxyDaemon::~HalProxyDaemon()
    {
    }


    void
    HalProxyDaemon::usage() const
    {
        fprintf(stderr, "Usage: %s [-option]\n", program_);
        fprintf(stderr, " -h                Display help\n");
        fprintf(stderr, "\n");
    }

    int
    HalProxyDaemon::parseOptions(
        int& argc,
        char** argv)
    {
        return 0;
    }


    int
    HalProxyDaemon::operator()()
    {
        return 0;
    }

    void
    HalProxyDaemon::hexdump(
        uint8_t* data,
        size_t len)
    {
        char buf[512];
        uint16_t index;
        uint8_t* ptr;
        int pos;

        ptr = data;
        pos = 0;
        for (index=0; index<len; index++)
        {
            pos += snprintf(&(buf[pos]), sizeof(buf) - pos, "%02x ", *ptr++);
            if (pos > 508)
            {
                break;
            }

        }

        HAL_LOG_MSG(
            HAL_LOG_LEVEL_ERROR,
            "HEX DUMP len="
            << len
            << "["
            << buf
            << "]");
    }

    void
    HalProxyDaemon::clearStats()
    {
        memset(&stats_, 0, sizeof(stats_));
    }

}

void app_process_event_handler(void * bufi, int len)
{
    char *data = (char *)buf;
    int i=0;

    printf("######################################\n");
    for(i=0; i<len; i++)
    {
        printf("%c", data[i]);
    }
    printf("\n######################################\n");
}

static int app_internal_pollin_handler(wifi_handle handle)
{
    memset(buf,'\0', BUFLEN);
    /* Try to receive some data, this is a blocking call */
    /* read datagram from client socket */
    if (recvfrom(app_sock, buf, BUFLEN, 0, (struct sockaddr *) &si,
        (socklen_t*)&slen) == -1)
    {
        ALOGE("Error recvfrom");
    }
    app_process_event_handler(buf, slen);
    return 0;
}

static void app_internal_event_handler(wifi_handle handle, int events)
{
    if (events & POLLERR) {
        ALOGE("Error reading from socket");
    } else if (events & POLLHUP) {
        ALOGE("Remote side hung up");
    } else if (events & POLLIN) {
        ALOGI("Found some events!!!");
        app_internal_pollin_handler(handle);
    } else {
        ALOGE("Unknown event - %0x", events);
    }
}
void app_event_loop(wifi_handle handle)
{
    pollfd pfd;
    memset(&pfd, 0, sizeof(pfd));

    pfd.fd = app_sock;
    pfd.events = POLLIN;

    do {
        int timeout = -1;                   /* Infinite timeout */
        pfd.revents = 0;
        ALOGI("Polling socket");
        int result = poll(&pfd, 1, -1);
        ALOGI("Poll result = %0x", result);
        if (result < 0) {
            ALOGE("Error polling socket");
        } else if (pfd.revents & (POLLIN | POLLHUP | POLLERR)) {
            app_internal_event_handler(handle, pfd.revents);
        }
    } while (1);
}

void * my_app_thread_function (void *ptr) {
    app_event_loop(gbl_handle);
    pthread_exit(0);
    return (void *)NULL;
}

void * my_thread_function (void *ptr) {
    wifi_event_loop(gbl_handle);
    pthread_exit(0);
    return (void *)NULL;
}

int
main(
    int argc,
    char* argv[])
{

    int request_id;
    int cmdId = -1;
    int max = 0;
    int flush = 0;
    int band = 0;
    HAL::HalProxyDaemon halProxyDaemon(argc, argv);

    if (!halProxyDaemon.isGood())
    {
        fprintf(stderr, "%s: halProxyDaemon failed at startup.\n", argv[0]);
        ::exit(-1);
    }

    wifi_handle wifiHandle = halProxyDaemon.getWifiHandle();
    gbl_handle = wifiHandle;

    pthread_t thread1;  /* thread variables */
    pthread_t thread2;  /* thread variables */

    /* Create an app socket */
    if ( (app_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        fprintf(stderr, "%s: halProxy failed cause the app socket is not created.\n", __func__);
        ::exit(-1);
    }

    /* create threads 1 */
    pthread_create (&thread1, NULL, &my_thread_function, NULL);
    /* create threads 2 */
    pthread_create (&thread2, NULL, &my_app_thread_function, NULL);

    wifi_interface_handle ifaceHandle = wifi_get_iface_handle(wifiHandle, "wlan0");

    if (!ifaceHandle)
    {
        fprintf(stderr, "%s: halProxy failed cause it couldn't retrieve iface ptrs.\n", __func__);
        ::exit(-1);
    }

    if(argc >= 2)
    {
        fprintf(stderr, "%s: Version: " HAL_PROXY_DAEMON_VER "\n", argv[0]);
#ifdef NAN_2_0
        if(strcasecmp(argv[1], NAN_TEST::NanTestSuite::NAN_CMD) == 0)
        {
            NAN_TEST::NanTestSuite nan(wifiHandle);
            nan.processCmd(argc, argv);
        }
#endif
        if(strcasecmp(argv[1], LLStats::LLStatsTestSuite::LL_CMD) == 0)
        {
            LLStats::LLStatsTestSuite llstats(wifiHandle);
            llstats.processCmd(argc, argv);
        }
        else if(strcasecmp(argv[1], GSCAN_TEST::GScanTestSuite::GSCAN_CMD) == 0)
        {
            srand(time(NULL));
            request_id = rand();
            GSCAN_TEST::GScanTestSuite gscan(
                            wifiHandle,
                            request_id);
#ifdef HAL_PROXY_DAEMON_SNS_SUPPORT
            fprintf(stderr, "**********************\n");
            fprintf(stderr, "Enter ID for GSCAN Cmd, as follows:\n");
            fprintf(stderr, "GSCAN Start                            : 1\n");
            fprintf(stderr, "GSCAN Stop                             : 2\n");
            fprintf(stderr, "GSCAN Get Valid Channels               :"
                            " 3 max_ch band iterations interval\n");
            fprintf(stderr, "GSCAN Get Capabilities                 :"
                            " 4 iterations interval\n");
            fprintf(stderr, "GSCAN Get Cached Results               :"
                            " 5 max_res flush iterations interval\n");
            fprintf(stderr, "GSCAN Set BSSID Hotlist                :"
                            " 6 iterations interval\n");
            fprintf(stderr, "GSCAN Reset BSSID Hotlist              :"
                            " 7 iterations interval\n");
            fprintf(stderr, "GSCAN Set Significant change           :"
                            " 8 iterations interval\n");
            fprintf(stderr, "GSCAN Reset Significant change         :"
                            " 9 iterations interval\n");
            fprintf(stderr, "**********************\n");
            fprintf(stderr, "######################\n");
            fprintf(stderr, "interval - time delay between current "
                            "command and next commnad(may be same cmd) "
                            "in seconds\n");
            fprintf(stderr, "iterations - No.of times the command to run\n");
            fprintf(stderr, "######################\n");
            int temp = 1;
            /* Set request Id for GSCAN object. */
            gscan.setRequestId(temp);

            int i = 2, iter = 0, intr = 0;
            while(i<argc){
                cmdId = atoi(argv[i++]);
                fprintf(stderr, "cmd : %d\n", cmdId);
                if(cmdId < 0 || cmdId > 9)
                {
                    fprintf(stderr, "Unknown command\n");
                    break;
                }
                switch(cmdId) {
                    case 3:
                        if(argc <= i+3)
                        {
                            fprintf(stderr, "Insufficient Args for cmdId :"
                                    " %d\n", cmdId);
                            break;
                        }
                        max = atoi(argv[i++]);
                        fprintf(stderr, "Max number of channels : %d\n", max);
                        band = atoi(argv[i++]);
                        fprintf(stderr, "Channels band : %d\n", band);
                        iter = 0;
                        while(atoi(argv[i]) > iter){
                            iter++;
                            gscan.executeCmd(argc, argv, cmdId, max, flush,
                                             band);
                            if(atoi(argv[i]) >= 1)
                            {
                                sleep(atoi(argv[i+1]));
                            }
                        }
                        i += 2;
                        break;
                    case 5:
                        if(argc <= i+3)
                        {
                            fprintf(stderr, "Insufficient Args for cmdId :"
                                    " %d\n", cmdId);
                            break;
                        }
                        max = atoi(argv[i++]);
                        fprintf(stderr, "Max number of cached"
                                " results: %d\n" , max);
                        flush = atoi(argv[i++]);
                        fprintf(stderr, "flush : %d\n", flush);
                        /* Read the parameters and the command will be issued
                         * later.
                         * ##### No break here #######
                         */
                    case 4:
                    case 6:
                    case 7:
                    case 8:
                    case 9:
                        fprintf(stderr, "Issuing cmd : %d \n", cmdId);
                        iter = 0;
                        while(atoi(argv[i]) > iter){
                            iter++;
                            gscan.executeCmd(argc, argv, cmdId, max, flush,
                                             band);
                            if(atoi(argv[i]) >= 1)
                            {
                                sleep(atoi(argv[i+1]));
                            }
                        }
                        i += 2;
                        break;
                    case 1:
                    case 2:
                        gscan.executeCmd(argc, argv, cmdId, max, flush, band);
                        break;
                    default:
                        fprintf(stderr, "%s: Unknown input.\n", __func__);
                }
            }
#else
            //gscan.processCmd(argc, argv);
            fprintf(stderr, "**********************\n");
            fprintf(stderr, "Step 1: Enter Request ID for GSCAN Cmd\n");
            fprintf(stderr, "Step 2: Enter ID for GSCAN Cmd, as follows:\n");
            fprintf(stderr, "    Type 1 for GSCAN Start\n");
            fprintf(stderr, "    Type 2 for GSCAN Stop\n");
            fprintf(stderr, "    Type 3 for GSCAN Get Valid Channels\n");
            fprintf(stderr, "    Type 4 for GSCAN Get Capabilities \n");
            fprintf(stderr, "    Type 5 for GSCAN Get Cached Results \n");
            fprintf(stderr, "    Type 6 for GSCAN Set BSSID Hotlist\n");
            fprintf(stderr, "    Type 7 for GSCAN Reset BSSID Hotlist\n");
            fprintf(stderr, "    Type 8 for GSCAN Set Significant change \n");
            fprintf(stderr, "    Type 9 for GSCAN Reset Significant change\n");
            fprintf(stderr, "    Type 10 for GSCAN Set MAC OUI\n");
            fprintf(stderr, "    Type 1000 to exit.\n");
            fprintf(stderr, "**********************\n");
            int temp;
            while (cmdId != 1000) {
                fprintf(stderr, "*********************\n");
                fprintf(stderr, "Now Enter Request ID:\n");
                /* Set request Id for GSCAN object. */
                scanf("%d",&temp);
                gscan.setRequestId(temp);
                fprintf(stderr, "Step 2: Enter GSCAN Cmd ID:\n");
                scanf("%d",&cmdId);
                char addr[10];
                u32 temp_mac[3];
                u8 mac[3] = {0};
                oui scan_oui = {0};
                switch(cmdId) {
                    case 3:
                        fprintf(stderr, "Step 3: Enter max number of "
                                "channels:\n");
                        scanf("%d",&max);
                        fprintf(stderr, "Step 4: Enter channels band:\n");
                        scanf("%d",&band);
                        gscan.executeCmd(argc, argv, cmdId, max, flush, band,
                                scan_oui);
                        break;
                    case 5:
                        fprintf(stderr, "Step 3: Enter max number of "
                                "cached results:\n");
                        scanf("%d",&max);
                        fprintf(stderr, "Step 4: Enter flush (0/1):\n");
                        scanf("%d",&flush);
                        gscan.executeCmd(argc, argv, cmdId, max, flush, band,
                                scan_oui);
                        break;
                    case 10:
                        fprintf(stderr, "Step 3: Enter 3 Bytes of MAC address"
                                "in XX:XX:XX form:\n");
                        scanf("%s", addr);
                        sscanf(addr, "%x:%x:%x", &temp_mac[0], &temp_mac[1],
                                &temp_mac[2]);
                        for (int i=0; i<3; i++)
                            scan_oui[i] = (u8)temp_mac[i];
                        fprintf(stderr, "MAC OUI - %x:%x:%x", scan_oui[0],
                                scan_oui[1], scan_oui[2]);
                    case 1:
                    case 2:
                    case 4:
                    case 6:
                    case 7:
                    case 8:
                    case 9:
                        gscan.executeCmd(argc, argv, cmdId, max, flush, band,
                                scan_oui);
                        break;
                    default:
                        fprintf(stderr, "%s: Unknown input.\n", __func__);
                }
            }
#endif
        }
        else if(strcasecmp(argv[1], RTT_TEST::RttTestSuite::RTT_CMD) == 0)
        {
            srand(time(NULL));
            request_id = rand();
            RTT_TEST::RttTestSuite rtt(
                            ifaceHandle,
                            request_id);
            fprintf(stderr, "**********************\n");
            fprintf(stderr, "Step 1: Enter Request ID for RTT Cmd\n");
            fprintf(stderr, "Step 2: Enter ID for RTT Cmd, as follows:\n");
            fprintf(stderr, "    Type 1 for RTT Range Request\n");
            fprintf(stderr, "    Type 2 for RTT Range Cancel\n");
            fprintf(stderr, "    Type 3 for RTT Get Capabilities \n");
            fprintf(stderr, "    Type 1000 to exit.\n");
            fprintf(stderr, "**********************\n");
            int temp;
            while (cmdId != 1000) {
                fprintf(stderr, "*********************\n");
                fprintf(stderr, "Now Enter Request ID:\n");
                /* Set request Id for RTT object. */
                scanf("%d",&temp);
                rtt.setRequestId(temp);
                fprintf(stderr, "Step 2: Enter RTT Cmd ID:\n");
                scanf("%d",&cmdId);
                switch(cmdId) {
                    case 1:
                    case 2:
                    case 3:
                        rtt.executeCmd(argc, argv, cmdId);
                        break;
                    default:
                        fprintf(stderr, "%s: Unknown input.\n", __func__);
                }
            }
        }
        else if(strcasecmp(argv[1], TDLS_TEST::TDLSTestSuite::TDLS_CMD) == 0)
        {
            srand(time(NULL));
            request_id = rand();
            TDLS_TEST::TDLSTestSuite TDLS(ifaceHandle);
            char addr[20];
            u32 temp_mac[7];
            u8 mac[6] = {0};

            while (1) {
                fprintf(stderr, "**********************\n");
                fprintf(stderr, "Step 2: Enter ID for TDLS Cmd, as follows:\n");
                fprintf(stderr, "    Type 1 for TDLS Enable\n");
                fprintf(stderr, "    Type 2 for TDLS Disable\n");
                fprintf(stderr, "    Type 3 for TDLS GetStatus\n");
                fprintf(stderr, "    Type 1000 to exit.\n");
                fprintf(stderr, "*********************\n");
                fprintf(stderr, "TDLS Cmd : ");
                cmdId = 0;
                scanf("%d", &cmdId);
                if (cmdId == 1000) {
                    break;
                } else if (cmdId < 1 || cmdId > 3) {
                    fprintf(stderr, "Invalid Cmd : %d \n", cmdId);
                    while ( getchar() != '\n' );
                    continue;
                }

                memset(&addr[0], 0, 20 * sizeof(char));
                memset(&temp_mac[0], 0, 7 * sizeof(u32));
                memset(&mac[0], 0, 6 * sizeof(u8));
                fprintf(stderr, "Step 3: MAC ADDR in XX:XX:XX:XX:XX:XX"
                        " format :\n");
                scanf("%s", addr);
                if (sscanf(addr, "%2x:%2x:%2x:%2x:%2x:%2x%c",
                        &temp_mac[0], &temp_mac[1],
                        &temp_mac[2], &temp_mac[3],
                        &temp_mac[4], &temp_mac[5],
                        (char *)&temp_mac[6]) != 6) {
                    fprintf(stderr, "Invalid Mac Address \n");
                    while ( getchar() != '\n' );
                    continue;
                }
                int i = 0;
                while(i<6){
                    mac[i] = (u8)temp_mac[i];
                    i++;
                }

                switch(cmdId) {
                    case 1:
                    case 2:
                    case 3:
                        fprintf(stderr, "MAC ADDR in %x:%x:%x:%x:%x:%x"
                                ":\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                        TDLS.executeCmd(argc, argv, cmdId, mac);
                        break;
                    default:
                        fprintf(stderr, "%s: Unknown input.\n", __func__);
                }
            }
        }
        else if(strcasecmp(argv[1],
            WIFIHAL_TEST::WifiHalTestSuite::WIFIHAL_CMD) == 0)
        {
            WIFIHAL_TEST::WifiHalTestSuite wifiHal(
                            ifaceHandle);
            WIFIHAL_TEST::cmdData data;

            while (cmdId != 1000) {
                cmdId = 0;
                memset(&data, 0, sizeof(WIFIHAL_TEST::cmdData));
                data.no_dfs_flag = 1;

                fprintf(stderr, "**********************\n");
                fprintf(stderr, "Step 1: Enter ID for Wi-Fi HAL Cmd, as follows:\n");
                fprintf(stderr, "    Type 1 for Get Supported Features\n");
                fprintf(stderr, "    Type 2 for Set No DFS Flag\n");
                fprintf(stderr, "    Type 3 for Get Concurrency Matrix\n");
                fprintf(stderr, "    Type 4 for Get the Interfaces available\n");
                fprintf(stderr, "    Type 5 for Set Iface event handler for"
                        " Regulatory domain change monitoring\n");
                fprintf(stderr, "    Type 6 for Reset Iface event handler for"
                        " Regulatory domain change monitoring\n");
                fprintf(stderr, "    Type 1000 to exit.\n");
                fprintf(stderr, "**********************\n");

                fprintf(stderr, "*********************\n");
                fprintf(stderr, "Enter Wi-Fi HAL Cmd ID:\n");
                scanf("%d",&cmdId);
                switch(cmdId) {
                    case 1:
                        break;
                    case 2:
                        fprintf(stderr, "Enter value for No DFS Flag:\n");
                        fprintf(stderr, "0: to ENABLE DFS or 1: to DISABLE DFS\n");
                        scanf("%d",&data.no_dfs_flag);
                        break;
                    case 3:
                        fprintf(stderr, "Enter value for max concurrency set "
                            "size:\n");
                        scanf("%d",&data.set_size_max);
                        break;
                    case 4:
                        data.wifiHandle = wifiHandle;
                        break;
                    case 5:
                    case 6:
                        fprintf(stderr, "Enter Request ID : \n");
                        scanf("%d",&data.reqId);
                        break;
                    default:
                        fprintf(stderr, "%s: Unknown input.\n", __func__);
                        continue;
                }
                wifiHal.executeCmd(cmdId, data);
            }
        }
        else
        {
            fprintf(stderr, "%s: Unknown command %s\n", argv[0], argv[1]);
        }
    }
    else
    {
        fprintf(stderr, "%s: Version: " HAL_PROXY_DAEMON_VER "\n", argv[0]);
        fprintf(stderr, "%s: Insufficent args\n", argv[0]);
    }

    //wifi_event_loop(wifiHandle);
    while (1) {
        usleep(2000000);
        fprintf(stderr, "daemon: Sleep: \n");
    }

    return halProxyDaemon();
}
