/*
    Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
*/

#define LOG_TAG "PerMgrApp"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>

#include <cutils/log.h>

#include "pm-service.h"

extern int pm_service_shutdown(void);

enum actions {
    NO,
    CONN,
    DISC,
    REG,
    UN,
    SLEEP,
    EXIT,
    CNT
};

enum usecase {
    RANDOM = 0,
    POWER,
    CRASH
};

const char *action_names[] = {
    [NO]   = "no action",
    [CONN] = "power",
    [DISC] = "disconnect",
    [REG]  = "registar",
    [UN]   = "unregister",
    [SLEEP]= "sleep",
    [EXIT] = "exit"
};


struct action_expect {
	enum actions  action;
	int           expect;
};

static const struct action_expect arbitrary[] =
{
  {REG, 0}, {UN, 0},
  {REG, 0}, {CONN, 0}, {DISC, 0}, {UN, 0},
  {REG, 0}, {DISC, -1}, {UN, 0},
  {REG, 0}, {CONN, 0}, {CONN, -1}, {DISC, 0}, {UN, 0},
  {REG, 0}, {CONN, 0}, {DISC, 0}, {UN, 0},
  {REG, 0}, {CONN, 0}, {DISC, 0}, {DISC, -1}, {CONN, 0}, {UN, 0},
  // Server will automatically drop voters count on unregister
  {REG, 0}, {CONN, 0}, {UN, 0},
  // actions without register
  {UN, -1}, {CONN, -1}, {DISC, -1}, {CONN, -1}, {UN, -1}, {DISC, -1},
  // Leak some references
  // {REG, 0}, {CONN, 0},
};

static const struct action_expect power[] =
{
  {REG, 0}, {CONN, 0}, {SLEEP, 0}
};

static const struct action_expect crash[] =
{
  {REG, 0}, {CONN, 0}, {EXIT, 0}
};

struct client_info {
    void            *id;        /* reference returned by registar() */
    int             number;
    const char      *device;    /* peripheral device name */
    enum pm_event   state;
    pthread_t       thread;
    unsigned        check;
    int             speed;
    int             ack;
    int             quiet;
    unsigned        steps;
    const struct action_expect *todo;
};

void app_event_notifier(void *client_cookie, enum pm_event event) {

    struct client_info *client = client_cookie;
    int ret;

    client->state = event;
    if (!client->ack)
        return;

    ret = pm_client_event_acknowledge(client->id, event);
    if (ret < 0 && !client->quiet) {
        ALOGE("%s event %d acknowledge fail", client->device, event);
    }
}

void app_commands(struct client_info *client) {

    const struct action_expect *todo;
    enum pm_event state;
    char app_name[16];
    int ret;

    todo = &client->todo[client->check];

    switch (todo->action) {
    default:
    case NO:
        ret = todo->expect;
        break;
    case CONN:
        ret = pm_client_connect(client->id);
        break;
    case DISC:
        ret = pm_client_disconnect(client->id);
        break;
    case REG:
        memset(app_name, 0, sizeof(app_name));
        snprintf(app_name, sizeof(app_name), "pm-app-%03d", client->number);
        ret = pm_client_register(app_event_notifier, client, client->device,
                                 app_name, &state, &client->id);
        break;
    case UN:
        ret = pm_client_unregister(client->id);
        break;
    case SLEEP:
        ret = todo->expect;
        sleep(86400); // 24H
        break;
    case EXIT:
        // Simulate crash: exit without unregister
        exit(1);
        break;
    }

    if (ret != todo->expect && !client->quiet) {
        ALOGE("%s client %p check %d action %s fail", client->device,
              client->id, client->check, action_names[todo->action]);
    }

    usleep(client->speed * 1000);

    client->check++;
    if (client->check >= client->steps)
        client->check = 0;
}

/* application exit condition */
static int g_pmapp_run;

static void take_signal(int s) {
    s = s; /* unused */
    g_pmapp_run = 0;
}

void *test_thread(void *arg)
{
    struct client_info *client = (struct client_info *) arg;

    while (g_pmapp_run) {
        app_commands(client);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    struct client_info *clients;
    struct sigaction sigact;
    const char *dev_names[PERIPH_MAX_SUPPORTED];
    int devs, cnt, wait, threads, idx, ret, speed,
        op, ack, quiet, offset, down, usecase;

    sigact.sa_handler = take_signal;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigaction(SIGINT, &sigact, NULL);

    wait = 60;      /* seconds */
    threads = 8;    /* number of clients per peripheral */
    speed = 100;    /* interval between PM API calls, ms */
    ack = 1;        /* do ack's by default */
    quiet = 0;
    down = 0;       /* shutdown service */
    usecase = RANDOM;

    static struct option options[] = {
        {"help",    no_argument,         NULL, 'h'},
        {"threads", required_argument,   NULL, 't'},
        {"wait",    required_argument,   NULL, 'w'},
        {"speed",   required_argument,   NULL, 's'},
        {"ack",     required_argument,   NULL, 'a'},
        {"quiet",   no_argument,         NULL, 'q'},
        {"down",    no_argument,         NULL, 'd'},
        {"usecase", no_argument,         NULL, 'u'},
      {0, 0, 0, 0}
    };

    while (1) {

        op = getopt_long(argc, argv, "?ht:w:s:a:qdu:", options, &idx);

        /* Detect the end of the options. */
        if (op == -1)
            break;

        switch (op) {
        case 'w':
            wait = atol(optarg);
            break;
        case 't':
            threads = atol(optarg);
            break;
        case 's':
            speed = atol(optarg);
            break;
        case 'a':
            ack = atol(optarg);
            break;
        case 'q':
            quiet = 1;
            break;
        case 'd':
            down = 1;
            break;
        case 'u':
            usecase = atol(optarg);
            break;
        case 'h':
        case '?':
        default:
            ALOGI("Usage: %s <parameters>"
                  "--wait|-w <seconds>      Time to execute tests (%d)"
                  "--threads|-t <number>    Number of clients (%d)"
                  "--speed|-s <ms>          Time between API calls (%d)"
                  "--ack|-a <1|0>           Do or don't ack event from server (1)"
                  "--quiet|-q               Run quietly (no)"
                  "--down|-d                Shutdown server (no)"
                  "--usecase|-u <0|1|2>     Usecase: arbitrary, sleep, crash (0)",
                  argv[0], wait, threads, speed);
            return -1;
        }
    }

    // Query available peripherals
    cnt = pm_show_peripherals(dev_names);
    if (cnt < 0) {
        ALOGE("Can't read peripheral names");
        return -1;
    }

    if (cnt == 0) {
        ALOGE("No peripherals found");
        return -1;
    }

    clients = calloc(cnt * threads, sizeof(*clients));
    if (!clients && (cnt * threads)) {
        ALOGE("%s", strerror(errno));
        return -1;
    }

    for (devs = 0; devs < cnt; devs++) {
        for (idx = 0; idx < threads; idx++) {

            offset = (devs * threads) + idx;

            clients[offset].device = dev_names[devs];
            clients[offset].speed = speed;
            clients[offset].ack = ack;
            clients[offset].quiet = quiet;
            clients[offset].number = offset;

            switch (usecase) {
                case RANDOM:
                    clients[offset].steps = sizeof(arbitrary) / sizeof(arbitrary[0]);
                    clients[offset].todo = arbitrary;
                    break;
                case POWER:
                    clients[offset].steps = sizeof(power) / sizeof(power[0]);
                    clients[offset].todo = power;
                    break;
                case CRASH:
                    clients[offset].steps = sizeof(crash) / sizeof(crash[0]);
                    clients[offset].todo = crash;
                    break;
            }

            ret = pthread_create(&clients[offset].thread, NULL,
                                 test_thread, &clients[offset]);
            if (ret) {
                ALOGE("Create %s", strerror(errno));
                return -1;
            }
        }
    }

    g_pmapp_run = 1;
    while(g_pmapp_run && wait--) {
        sleep(1);
    }
    g_pmapp_run = 0;
    // wait all threads to exit
    usleep(speed * 100 * 2);

    if (down) {
        ret = pm_service_shutdown();
        if (ret && !quiet) {
            ALOGE("Can't shutdown service");
        }
    }
    return 0;
}
