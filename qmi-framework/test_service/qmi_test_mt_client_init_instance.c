/******************************************************************************
  ---------------------------------------------------------------------------
  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
*******************************************************************************/
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "qmi_cci_target_ext.h"
#include "qmi_client.h"
#include "qmi_client_instance_defs.h"
#include "qmi_idl_lib.h"
#include "test_service_v01.h"

#define DEFAULT_NUM_THREADS 2
#define DEFAULT_NUM_ITERATIONS 10

static int use_qmuxd;
static int num_threads = DEFAULT_NUM_THREADS;
static int instance_id;
static int init_timeout;
static int num_iterations = DEFAULT_NUM_ITERATIONS;

struct thread_params {
  pthread_t tid;
  int thread_index;
  int num_iterations;
  int instance_id;
  int timeout_ms;
};

static struct thread_params input_params;
static qmi_client_os_params os_params;

static void usage(const char *fname)
{
  printf("\n*************************************************************\n");
  printf("Usage: %s PARAMS can be:\n"
          "  -q,          Use this option to enable QMUX\n"
          "  -h,          Help Option to print Usage\n"
          "  -t <value>,  number of threads to be launched\n"
          "  -i <value>,  Instance ID of the Test QMI service\n"
          "  -n <value>,  Number to iterations of client init\n"
          "  -d <value>,  Time in ms to wait for the service \n", fname);
  printf("\n*************************************************************\n");
}

static void parse_command(int argc, char *const argv[])
{
  int command;

  struct option longopts[] = {
        {"use_qmux", no_argument, NULL, 'q'},
        {"num_threads", required_argument, NULL, 't'},
        {"instance ID", required_argument, NULL, 'i'},
        {"num_iterations", required_argument, NULL, 'n'},
        {"help", no_argument, NULL, 'h'},
        {"init_timeout", required_argument, NULL, 'd'},
        {NULL, 0, NULL, 0},
  };

  while ((command = getopt_long(argc, argv, "qht:i:n:d:", longopts,
                                NULL)) != -1) {
    switch (command) {
      case 'q':
        use_qmuxd = 1;
        break;
      case 'h':
        usage(argv[0]);
        exit(0);
      case 't':
        num_threads = atoi(optarg);
        break;
      case 'i':
        instance_id = atoi(optarg);
        break;
      case 'n':
        num_iterations = atoi(optarg);
        break;
      case 'd':
        init_timeout = atoi(optarg);
        break;
      default:
        usage(argv[0]);
        exit(-1);
    }
  }
}

static void *test_thread(void *arg)
{
  struct thread_params *tp = (struct test_thread *)arg;
  int sleep_time_us;
  int i;
  int rc;
  qmi_client_type clnt;
  qmi_idl_service_object_type test_service_object = test_get_service_object_v01();

  if (tp->timeout_ms)
  {
    sleep_time_us = (rand() % tp->timeout_ms) * 1000;
    usleep(sleep_time_us);
  }

  for (i = 0; i < tp->num_iterations; i++)
  {
    printf("%s: TID %d, INDEX %d, iteration %d Start\n",
           __func__, tp->tid, tp->thread_index, i);
    rc = qmi_client_init_instance(test_service_object, tp->instance_id, NULL,
           NULL, &os_params, tp->timeout_ms, &clnt);
    if (rc == QMI_NO_ERR)
    {
      printf("%s: TID %d, INDEX %d, iteration %d success\n",
             __func__, tp->tid, tp->thread_index, i);
      qmi_client_release(clnt);
    }
    else {
      printf("%s: TID %d, INDEX %d, iteration %d, rc %d\n",
             __func__, tp->tid, tp->thread_index, i, rc);
    }
  }
  printf("%s: TID %d, INDEX %d Exit\n", __func__, tp->tid, tp->thread_index);
  return NULL;
}

int main(int argc, char **argv)
{
  struct thread_params *tp, *temp_tp;
  int i;
  int rc;

  printf("To show usage: %s -h", argv[0]);
  parse_command(argc, argv);

  if (!use_qmuxd)
  {
    qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_INSTANCE_0);
    qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0);
    qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_SMUX_INSTANCE_0);
    qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_MHI_INSTANCE_0);
  }

  tp = (struct thread_params *)calloc(num_threads, sizeof(*tp));
  if (!tp)
  {
    printf("%s: Cannot allocate thread params\n", __func__);
    return -1;
  }

  temp_tp = tp;
  for (i = 0; i < num_threads; i++)
  {
    temp_tp->thread_index = i;
    temp_tp->num_iterations = num_iterations;
    temp_tp->instance_id = instance_id;
    temp_tp->timeout_ms = init_timeout;
    temp_tp++;
  }

  temp_tp = tp;
  for (i = 0; i < num_threads; i++)
  {
    rc = pthread_create(&temp_tp->tid, NULL, test_thread, (void *)temp_tp);
    if (rc < 0)
    {
       printf("Error creating thread at %d iteration\n", i);
       exit(1);
    }
    temp_tp++;
  }

  temp_tp = tp;
  for (i = 0; i < num_threads; i++)
  {
    printf("Waiting for TID %d, Index %d to complete\n",
           temp_tp->tid, i);
    pthread_join(temp_tp->tid, &rc);
    temp_tp++;
  }
  printf("PASS\n");
  free(tp);
  return 0;
}
