/* Copyright (c) 2009 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Copyright (c) 2012 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include "loc_eng_dmn_conn.h"
#include "gpsone_glue_msg.h"

#include "test_dbg.h"

typedef void (*ThreadStart) (void *);
ThreadStart g_pfnThreadStart;

void *my_thread_fn (void *arg)
{
    g_pfnThreadStart (arg);
    return NULL;
}


pthread_t test_gps_create_thread_cb (const char *name, void (*start) (void *),
                                void *arg)
{
    pthread_t thread_id;
    TEST_DBG("%s", name);
    g_pfnThreadStart = start;

    if (0 > pthread_create (&thread_id, NULL, my_thread_fn, arg)) {
        TEST_DBG("error creating thread");
    } else {
        TEST_DBG("created thread");
    }

    return thread_id;
}

/*
 */
int main(int argc, char * argv[])
{
    int result;
    const char * loc_api_q_path = GPSONE_LOC_API_Q_PATH;
    const char * ctrl_q_path = GPSONE_LOC_API_RESP_Q_PATH;
    int opt;
    extern char *optarg;

    while ((opt = getopt(argc, argv, "q:hc:")) != -1) {
      switch(opt) {
        case 'q':
          loc_api_q_path = optarg;
          break;

        case 'c':
          ctrl_q_path = optarg;
          break;

        case 'h':
        default:
          TEST_DBG("usage: %s [-q </tmp/gpsone_loc_api_q>]\n", argv[0]);
          TEST_DBG("-q:  gpsone loc api message queue path. By default it is /tmp/gpsone_loc_api_q\n");
          TEST_DBG("-c:  gpsone daemon control queue path. By default it is /tmp/gpsone_ctrl_q\n");
          TEST_DBG("-h:  print this help\n\n");
          return 0;
      }
    }
    result = loc_eng_dmn_conn_loc_api_server_launch(test_gps_create_thread_cb, loc_api_q_path, ctrl_q_path, NULL);

    /* loop forever, don't unblock */
    /* loc_eng_dmn_conn_loc_api_server_unblock(); */

    loc_eng_dmn_conn_loc_api_server_join();

    return result;
}

