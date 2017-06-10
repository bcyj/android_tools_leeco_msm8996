/*
* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*
*/

#include "rtt_test.hpp"
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <getopt.h>
#include<sys/socket.h>
#include <utils/Log.h>

extern struct sockaddr_in si;
extern int app_sock, slen;

#define MAC_ADDR_ARRAY(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MAC_ADDR_STR "%02x:%02x:%02x:%02x:%02x:%02x"

static void rtt_on_results(wifi_request_id id, unsigned num_results,
                           wifi_rtt_result rtt_result[])
{
    int i;
    fprintf(stderr, "      **********************************************\n.");
    ALOGE("%s: request_id:%d, num_results:%d\n.",
        __func__, id, num_results);
    fprintf(stderr, "%s: request_id:%d, num_results:%d\n.",
        __func__, id, num_results);
    wifi_rtt_result *result = NULL;
    if (rtt_result) {
        for(u32 i = 0; i < num_results; i++)
        {
            result = &rtt_result[i];
            if (result) {
                fprintf(stderr, "BSSID: %02x:%02x:%02x:%02x:%02x:%02x \n",
                        result->addr[0], result->addr[1], result->addr[2],
                        result->addr[3], result->addr[4], result->addr[5]);
                fprintf(stderr, "measurement_num  %d \n", result->measurement_num);
                fprintf(stderr, "status:  %u \n", result->status);
                fprintf(stderr, "type:  %u \n", result->type);
                fprintf(stderr, "peer:  %u \n", result->peer);
                fprintf(stderr, "channel center_freq:  %u \n", result->channel.center_freq);
                fprintf(stderr, "channel center_freq0: %u \n", result->channel.center_freq0);
                fprintf(stderr, "channel center_freq1: %u \n", result->channel.center_freq1);
                fprintf(stderr, "channel width: %u \n", result->channel.width);
                fprintf(stderr, "rssi:  %d \n", result->rssi);
                fprintf(stderr, "rssi_spread:  %u \n", result->rssi_spread);
                fprintf(stderr, "tx_rate:  %u \n", result->tx_rate);
                fprintf(stderr, "rtt  %llu \n", result->rtt);
                fprintf(stderr, "rtt_sd  %llu \n", result->rtt_sd);
                fprintf(stderr, "rtt_spread  %llu \n", result->rtt_spread);
                fprintf(stderr, "distance  %u \n", result->distance);
                fprintf(stderr, "distance_sd  %u \n", result->distance_sd);
                fprintf(stderr, "distance_spread  %d\n", result->distance_spread);
                fprintf(stderr, "ts  %llu \n", result->ts);
                fprintf(stderr, "\n", result->ts); // dummy empty line
            }
        }
    }
    fprintf(stderr, "      **********************************************\n.");
}

namespace RTT_TEST
{
    /* CLI cmd strings */
    const char *RttTestSuite::RTT_CMD = "rtt";
    const char *RttTestSuite::RTT_RANGE_REQUEST = "range_request";
    const char *RttTestSuite::RTT_RANGE_CANCEL = "range_cancel";
    const char *RttTestSuite::RTT_GET_CAPABILITIES = "get_capabilities";

    /* Constructor */
    RttTestSuite::RttTestSuite(wifi_interface_handle handle, wifi_request_id request_id)
        :wifiHandle_(handle)
    {
        fprintf(stderr, "RttTestSuite::RttTestSuite: Created a Rtt Test "
            "Suite with request_id:%d\n.", request_id);
        id = request_id;
    }

    void RttTestSuite::setRequestId(int reqId)
    {
        id = reqId;
    }

    int RttTestSuite::getRequestId()
    {
        return id;
    }

    /* process the command line args */
    void RttTestSuite::executeCmd(int argc,
                                      char **argv,
                                      int cmdIndex)
    {
        fprintf(stderr, "%s: Enter \n", __func__);
        switch(cmdIndex) {
            case 1:
                rttSendRangeRequest(argc, argv);
                break;
            case 2:
                rttSendCancelRangeRequest(argc, argv);
                break;
            case 3:
                rttSendGetCapabilitiesRequest(argc, argv);
                break;
            default:
                fprintf(stderr, "%s: Unknown Cmd ID.\n", __func__);
        }
    }

    /* process the command line args */
    void RttTestSuite::processCmd(int argc, char **argv)
    {
        if(argc <3)
        {
            fprintf(stderr, "%s: insufficient GSCAN args\n", argv[0]);
            return;
        }

        if(strcasecmp(argv[2], RTT_RANGE_REQUEST) == 0)
            return rttSendRangeRequest(argc, argv);

        if(strcasecmp(argv[2], RTT_RANGE_CANCEL) == 0)
            return rttSendCancelRangeRequest(argc, argv);

        if(strcasecmp(argv[2], RTT_GET_CAPABILITIES) == 0)
            return rttSendGetCapabilitiesRequest(argc, argv);

        fprintf(stderr, "%s: unknown  arg %s\n", argv[0], argv[2]);
    }

    /* Helper routine to print usage */
    void RttTestSuite::rttPrintCmdUsage(char **argv, const char *cmd,
       const char *sub_cmd, const struct option long_options[], int size)
    {
        fprintf(stderr, "Usage: %s %s %s\n", argv[0], cmd, sub_cmd);
        for(int i = 1; i <= size-2; i++)
        {
            if(long_options[i].has_arg)
                fprintf(stderr, "\t[--%s arg]\n", long_options[i].name);
            else
                fprintf(stderr, "\t[--%s]\n", long_options[i].name);
        }
        return;
    }

    void RttTestSuite::rttSendGetCapabilitiesRequest(int argc, char **argv)
    {
        int ret = 0;
        /* A string listing valid short options letters.  */
        const char* const short_options = "h:";
        /* An array describing valid long options.  */
        const struct option long_options[] = {
            { "help",         0,   NULL, 'h' },
            { NULL,           0,   NULL,  0  }   /* Required at end of array.  */
            };
        wifi_rtt_capabilities capa;
        /* Override with command line arguements */
        int long_index = 0, opt = 0;
        while ((opt = getopt_long(argc, argv, short_options,
                long_options, &long_index )) != -1)
        {
            switch (opt)
            {
                case 'h':
                default:
                    rttPrintCmdUsage(argv, RTT_CMD, RTT_GET_CAPABILITIES,
                        long_options,
                        sizeof(long_options)/sizeof(struct option));
                    return;
            }
        }

        fprintf(stderr, "%s: Sending Get Capabilities Request. \n",
            __FUNCTION__);
        ret = wifi_get_rtt_capabilities(wifiHandle_, &capa);
        fprintf(stderr, "%s: Received RTT Capabilities with value:%d. \n",
            __FUNCTION__, ret);
        if (ret)
            return;

        fprintf(stderr, "%s: Capabilities:\n    rtt_one_sided_supported:%01x,\n "
            "   rtt_11v_supported:%01x,\n    rtt_ftm_supported:%01x,\n ",
            __func__, capa.rtt_one_sided_supported,
            capa.rtt_11v_supported, capa.rtt_ftm_supported);
    }

    void RttTestSuite::rttSendRangeRequest(int argc, char **argv)
    {
        fprintf(stderr, "%s: Sending RTT Range Request. "
            "\n", __func__);
        wifi_rtt_event_handler nHandler;
        int i, j, ret = 0;
        /* A string listing valid short options letters.  */
        const char* const short_options = "h:";
        /* An array describing valid long options.  */
        const struct option long_options[] = {
            { "help",         0,   NULL, 'h' },
            { NULL,           0,   NULL,  0  }   /* Required at end of array.  */
            };
        /* Override with command line arguements */
        int long_index = 0, opt = 0;
        while ((opt = getopt_long(argc, argv, short_options,
                long_options, &long_index )) != -1)
        {
            switch (opt)
            {
                case 'h':
                default:
                    rttPrintCmdUsage(argv, RTT_CMD,
                        RTT_RANGE_REQUEST,
                        long_options,
                        sizeof(long_options)/sizeof(struct option));
                    return;
            }
        }

        unsigned num_rtt_config;
        wifi_rtt_config rtt_config[1000];
        u32 dummyIndex;

        FILE *f_read = fopen("/etc/wifi/rtt_range_request_params.txt", "r");
        fprintf(stderr, "\n");
        if (f_read) {
            if ( (fscanf(f_read, "%d", &num_rtt_config)) == 0 ) {
                fprintf(stderr, "%s: Failed parsing RTT Range Request "
                    "params file. Exit\n", __func__);
                return;
            }
            fprintf(stderr, "Number of RTT Config records:%d\n\n",
                    num_rtt_config);

            /* Outer loop for parsing scan buckets */
            for ( i = 0; i < num_rtt_config; i++ )
            {
                if (
                    (fscanf(f_read, "%d", &dummyIndex) &&
                    fscanf(f_read, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                            &rtt_config[i].addr[0], &rtt_config[i].addr[1],
                            &rtt_config[i].addr[2], &rtt_config[i].addr[3],
                            &rtt_config[i].addr[4], &rtt_config[i].addr[5]) &&
                    fscanf(f_read, "%d", &rtt_config[i].type) &&
                    fscanf(f_read, "%d", &rtt_config[i].peer) &&
                    fscanf(f_read, "%d", &rtt_config[i].channel.width) &&
                    fscanf(f_read, "%d", &rtt_config[i].channel.center_freq) &&
                    fscanf(f_read, "%d", &rtt_config[i].channel.center_freq0) &&
                    fscanf(f_read, "%d", &rtt_config[i].channel.center_freq1) &&
                    fscanf(f_read, "%d", &rtt_config[i].continuous) &&
                    fscanf(f_read, "%d", &rtt_config[i].interval) &&
                    fscanf(f_read, "%d", &rtt_config[i].num_measurements) &&
                    fscanf(f_read, "%d",
                        &rtt_config[i].num_samples_per_measurement) &&
                    fscanf(f_read,
                        "%d", &rtt_config[i].num_retries_per_measurement))
                        == 0 ) {
                    fprintf(stderr, "%s: Failed parsing RTT Range Request "
                        "params file. Exit\n", __func__);
                    return;
                }

                fprintf(stderr, "rtt_config[%d].index:%d\n",
                        i, dummyIndex);

                fprintf(stderr,
                        "rtt_config[%d].bssid:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx\n",
                        i,
                        rtt_config[i].addr[0], rtt_config[i].addr[1],
                        rtt_config[i].addr[2], rtt_config[i].addr[3],
                        rtt_config[i].addr[4], rtt_config[i].addr[5]);

                fprintf(stderr, "rtt_config[%d].type:%d\n "
                        "rtt_config[%d].peer:%d\n"
                        "rtt_config[%d].channel.width:%d\n"
                        "rtt_config[%d].channel.center_freq:%d\n"
                        "rtt_config[%d].channel.center_freq0:%d\n"
                        "rtt_config[%d].channel.center_freq1:%d\n"
                        "rtt_config[%d].continuous:%d\n"
                        "rtt_config[%d].interval:%d\n"
                        "rtt_config[%d].num_measurements:%d\n"
                        "rtt_config[%d].num_samples_per_measurement:%d\n"
                        "rtt_config[%d].num_retries_per_measurement:%d\n"
                        "\n",
                        i, rtt_config[i].type,
                        i, rtt_config[i].peer,
                        i, rtt_config[i].channel.width,
                        i, rtt_config[i].channel.center_freq,
                        i, rtt_config[i].channel.center_freq0,
                        i, rtt_config[i].channel.center_freq1,
                        i, rtt_config[i].continuous,
                        i, rtt_config[i].interval,
                        i, rtt_config[i].num_measurements,
                        i, rtt_config[i].num_samples_per_measurement,
                        i, rtt_config[i].num_retries_per_measurement
                        );
            }
            fclose(f_read);
        } else {
            fprintf(stderr, "gscanSendStartRequest: Failed to "
            "open file /etc/wifi/gscan_start_params.txt - "
            "use hard-coded defaults\n");
            num_rtt_config = 1;
            memcpy(&rtt_config[0].addr[0], "012345", sizeof(mac_addr));
            rtt_config[0].type = RTT_TYPE_1_SIDED;
            rtt_config[0].peer = WIFI_PEER_TDLS;
            rtt_config[0].channel.width = WIFI_CHAN_WIDTH_20;
            rtt_config[0].channel.center_freq = 2412;
            rtt_config[0].channel.center_freq0 = 2407;
            rtt_config[0].channel.center_freq1 = 2417;
            rtt_config[0].continuous = 0;
            rtt_config[0].interval = 10;
            rtt_config[0].num_measurements = 11;
            rtt_config[0].num_samples_per_measurement = 22;
            rtt_config[0].num_retries_per_measurement = 33;
        }

        /* Set the callback handler functions for related events. */
        nHandler.on_rtt_results = rtt_on_results;

        ret = wifi_rtt_range_request(id, wifiHandle_,
                num_rtt_config, rtt_config, nHandler);

        fprintf(stderr, "%s: Sending RTT Range request"
            " completed. Returned value: %d.\n", __func__, ret);

        if (ret)
            return;
    }

    void RttTestSuite::rttSendCancelRangeRequest(int argc, char **argv)
    {
        int i, ret = 0;
        unsigned num_devices = 0;
        mac_addr addr[1000];

        fprintf(stderr, "rttSendCancelRangeRequest: Sending RTT "
            "CancelRange Request. \n");

        /* A string listing valid short options letters.  */
        const char* const short_options = "h:";
        /* An array describing valid long options.  */
        const struct option long_options[] = {
            { "help",         0,   NULL, 'h' },
            { NULL,           0,   NULL,  0  }   /* Required at end of array.  */
            };

        /* Override with command line arguements */
        int long_index = 0, opt = 0;
        while ((opt = getopt_long(argc, argv, short_options,
                long_options, &long_index )) != -1)
        {
            switch (opt)
            {
                case 'h':
                default:
                    rttPrintCmdUsage(argv, RTT_CMD, RTT_RANGE_CANCEL,
                        long_options,
                        sizeof(long_options)/sizeof(struct option));
                    return;
            }
        }

        fprintf(stderr, "Parse params from "
            "/etc/wifi/rtt_range_cancel_params.txt \n");

        /* Initialize the newly allocated memory area with 0. */
        memset(addr, 0, sizeof(addr));

        FILE *f_read = fopen("/etc/wifi/rtt_range_cancel_params.txt", "r");
        fprintf(stderr, "\n");
        if (f_read) {
            if (fscanf(f_read, "%d", &num_devices)) {
                fprintf(stderr, "rttSendCancelRangeRequest: Parsed "
                    "number of MAC Addresses:%d\n", num_devices);
            }
            if (num_devices) {
                for ( i = 0; i < num_devices; i++ )
                {
                    if ( (fscanf(f_read, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                            &addr[i][0], &addr[i][1],
                            &addr[i][2], &addr[i][3],
                            &addr[i][4], &addr[i][5])) == 0 ) {
                        fprintf(stderr, "%s: Failed parsing RTT Range Cancel "
                            "params file. Exit\n", __func__);
                        return;
                    }

                    fprintf(stderr, "addr[%d]:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx\n",
                            i,
                            addr[i][0], addr[i][1],
                            addr[i][2], addr[i][3],
                            addr[i][4], addr[i][5]);
                }
                fprintf(stderr, "\n");
            }
            fclose(f_read);
        } else {
            fprintf(stderr, "rttSendCancelRangeRequest: Failed to open "
            "file /etc/wifi/rtt_range_cancel_params.txt  - "
            "use hard-coded defaults\n");

            num_devices = 3;
            memcpy(&addr[0], "012345", sizeof(mac_addr));
            memcpy(&addr[1], "678901", sizeof(mac_addr));
            memcpy(&addr[2], "333333", sizeof(mac_addr));
        }

        ret = wifi_rtt_range_cancel(id, wifiHandle_, num_devices, addr);
        fprintf(stderr, "rttSendCancelRangeRequest: Sending RTT Range "
            "Cancel request completed. Returned value: %d.\n", ret);

        if (ret)
            return;
    }

    int RttTestSuite::rttParseHex(unsigned char c)
    {
       if (c >= '0' && c <= '9')
          return c-'0';
       if (c >= 'a' && c <= 'f')
          return c-'a'+10;
       if (c >= 'A' && c <= 'F')
          return c-'A'+10;
       return 0;
    }

    int RttTestSuite::rttParseMacAddress(const char* arg, u8* addr)
    {
       if (strlen(arg) != 17)
       {
          fprintf(stderr, "Invalid mac address %s\n", arg);
          fprintf(stderr, "expected format xx:xx:xx:xx:xx:xx\n");
          return -1;
       }

       addr[0] = rttParseHex(arg[0]) << 4 | rttParseHex(arg[1]);
       addr[1] = rttParseHex(arg[3]) << 4 | rttParseHex(arg[4]);
       addr[2] = rttParseHex(arg[6]) << 4 | rttParseHex(arg[7]);
       addr[3] = rttParseHex(arg[9]) << 4 | rttParseHex(arg[10]);
       addr[4] = rttParseHex(arg[12]) << 4 | rttParseHex(arg[13]);
       addr[5] = rttParseHex(arg[15]) << 4 | rttParseHex(arg[16]);
       return 0;
    }

}
