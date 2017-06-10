/* signaltest.c
 *
 * Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

sigset_t sigSet;
pthread_t tid1, tid2;

typedef struct _threadInfo threadInfo_t;

struct _threadInfo {
  int threadNum;
  pthread_t tid;
};


static void print_thread(void *addr, int size)
{
  unsigned char *base = addr;
  int i;

  for (i = 0; i < size; i++) {
     printf("%02x", base[i]);
  }

  return;
}


void *thread_run(void *a)
{
  int sig;
  int err;
  threadInfo_t myInfo;
  sigset_t sigs;
  sigset_t oldSigSet;
  sigfillset(&sigs);

	pthread_t myTid;

  pthread_sigmask(SIG_BLOCK, &sigs, &oldSigSet);

  myInfo = *(threadInfo_t *)(a);
  myTid  = pthread_self();

  do {
    if (myInfo.threadNum == 1) {

      printf("threadNum %d before sigwait ...\n", myInfo.threadNum);
      err = sigwait(&sigSet, &sig);
      printf("\nI am assigned as:\n");
      print_thread(&(myInfo.tid), sizeof(pthread_t));
      printf("\nMyself is:\n");
      print_thread(&(myTid), sizeof(pthread_t));
      printf("\n");

      if (myInfo.tid == pthread_self()) {
        printf("!!! This is thread !!!\n");
      } else {
        printf("!!! Wrong !!!\n");
      }
      printf("threadNum %d after sigwait, err = %d, sig = %d\n", myInfo.threadNum, err, sig);
    }

    if (myInfo.threadNum == 2) {
#if 0
      sleep(1);
      printf("test2, thread2 send %d to thread1 \n", SIGRTMIN + 5);
      pthread_kill(tid1, SIGRTMIN + 5);
      sleep(1);
      printf("test3, thread2 send %d to thread1 \n", SIGRTMIN + 15);
      pthread_kill(tid1, SIGRTMIN + 15);
#endif /* if 0 */
    }
	} while (1);
}

int main(int argc, char * argv[])
{
  sigemptyset(&sigSet);
  sigaddset(&sigSet, SIGRTMIN);
  sigaddset(&sigSet, SIGRTMIN+5);
  sigaddset(&sigSet, SIGRTMIN+15);

  threadInfo_t info[2];

  pthread_sigmask(SIG_BLOCK, &sigSet, 0);

  info[0].threadNum = 1;
  if (pthread_create(&tid1, NULL, thread_run, &info[0])) {
    printf("\n ERROR creating thread 1");
    exit(1);
  }
  info[0].tid = tid1;

  info[1].threadNum = 2;
  if (pthread_create(&tid2, NULL, thread_run, &info[1])) {
    printf("\n ERROR creating thread 2");
    exit(1);
  }
  info[1].tid = tid2;

  while(1) {
    sleep(1);
    printf("test1, main send %d to thread1 \n", SIGRTMIN);
    pthread_kill(tid1, SIGRTMIN);
  }

  pthread_exit(NULL);
}

