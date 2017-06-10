/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <linux/socket.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>


int main(int argc, char * argv[])
{
  struct sockaddr_in si_me, si_other;
  int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  int slen = sizeof(si_other);
  if (fd < 0 ) {
    printf("socket failed!\n");
    return -1;
  }

  memset((char *) &si_me, 0, sizeof(si_me));
  si_me.sin_family = AF_INET;
  si_me.sin_port = htons(9010);
  si_me.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(fd, &si_me, sizeof(si_me)) < 0 ) {
    printf("bind failed!\n");
    return -1;
  }

  char buf[100];
  if (recvfrom(fd, buf, 100, 0, &si_other, &slen) < 0) {
    printf("recvfrom failed! %s\n", strerror(errno));
    return -1;
  }
  printf("we received something: %s\n", buf);
  close(fd);
  return 0;
}

