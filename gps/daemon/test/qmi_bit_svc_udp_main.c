/*   Copyright (c) 2012 Qualcomm Atheros, Inc.
     All Rights Reserved.
     Qualcomm Atheros Confidential and Proprietary
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include "qmi_idl_lib.h"
#include "qmi_csi.h"
#include "qmi_csi_common.h"

extern qmi_csi_xport_ops_type udp_ops;
extern void *qmi_bit_register_service(qmi_csi_os_params *os_params);

/*=============================================================================
  FUNCTION qmi_start_bit_service
=============================================================================*/
void qmi_start_bit_service()
{
  qmi_csi_os_params os_params,os_params_in;
  fd_set fds;
  char buf[10];
  void *sp;
  printf("qmi_start_bit_service: starting qmiBITService\n");
  sp = qmi_bit_register_service(&os_params);

  if(!sp)
  {
    printf("Unable to register service!\n");
    exit(1);
  }

  /* This loop calls a blocking select to read from the UDP port the off target test is using */
  while(1)
  {
    fds = os_params.fds;
    FD_SET(STDIN_FILENO, &fds);
    select(os_params.max_fd+1, &fds, NULL, NULL, NULL);
    /* Test for user input of the ctrl+d character to terminate the server */
    if(FD_ISSET(STDIN_FILENO, &fds))
    {
      if(read(STDIN_FILENO, buf, sizeof(buf)) <= 0)
      {
        break;
      }
    }
    os_params_in.fds = fds;
    qmi_csi_handle_event(sp, &os_params_in);
  }
  qmi_csi_unregister(sp);
}

/*=============================================================================
  FUNCTION main
=============================================================================*/
int main(int argc, char **argv)
{
  /* The code in start_bit_service is the on-target intiialization code */
  qmi_start_bit_service();

  return 0;
}/* main */

