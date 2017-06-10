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
  int i;

  const char * server_ip = "127.0.0.1";
  const char * server_port = "9008";
  int num_trials = 20000;
  int opt;
  extern char *optarg;

  while ((opt = getopt(argc, argv, "a:p:n:h")) != -1) {
    switch(opt) {
      case 'a':
        server_ip = optarg;
        break;

      case 'p':
        server_port = optarg;
        break;

      case 'n':
        num_trials = atoi(optarg);
        break;

      case 'h':
      default:
        printf("usage: %s [-a <ip address>] [-p <port>]\n", argv[0]);
        printf("-a:  server ip address. By default it is 127.0.0.1\n");
        printf("-p:  server port number. By default it is 9009\n");
        printf("-h:  print this help\n\n");
        printf("run adb port forwarding for android target:\n");
        printf("    adb forward tcp:9009 tcp:9009\n");
        return 0;
    }
  }


  for (i = 0; i < num_trials; i++) {
    struct sockaddr_in si_other;
    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd < 0) {
      printf("socket failed!\n");
      return -1;
    }

    memset((char *) &si_other, 0, sizeof(si_other));

    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(atoi(server_port));
    if (inet_aton(server_ip, &si_other.sin_addr) < 0 ) {
      printf("inet_aton failed!\n");
      return -1;
    }


    char buf[100];
    strncpy(buf, "how are you gentlemen", 100);
    if (sendto(fd, buf, 100, 0, (const struct sockaddr *)&si_other, sizeof(si_other)) < 0 ) {
      printf("sendto failed! %s\n", strerror(errno));
      return -1;
    }

    printf("done!\n");
    close(fd);


    sleep(1);
  }
  return 0;
}

