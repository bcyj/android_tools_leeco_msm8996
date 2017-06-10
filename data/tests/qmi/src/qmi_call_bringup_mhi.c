/*!
  @file
  qmi_call_bringup_mhi.c

  @brief
  This test call bring-up over MHI channel.
*/

/*===========================================================================
  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "qmi.h"
#include "qmi_wds_srvc.h"

/*qmi message library handle*/
int qmi_handle = QMI_INVALID_CLIENT_HANDLE;

/* Device names from qmi_platform.config.h */
#define QMI_DEV_NAME_LEN (16)
static char qmi_port_name_tbl[][QMI_DEV_NAME_LEN] =
{
  QMI_PORT_RMNET_MHI_0,
  QMI_PORT_RMNET_MHI_1
};
#define QMI_PORT_NAME_TBL_SIZE (sizeof(qmi_port_name_tbl)/sizeof(qmi_port_name_tbl[0]))

int set_connection( char **qmi_port_name )
{
  char ch;
  unsigned int c, i;
  int ret = -1;

  fprintf(stdout, "Choose device from table:\n");
  for(i=0; i<QMI_PORT_NAME_TBL_SIZE; i++)
  {
    fprintf(stdout, "  %c. %s\n", (i+'a'), qmi_port_name_tbl[i]);
  }

  fprintf(stdout, "Choice: ");
  while (1)
  {
    ch = getchar();
    c = ch - 'a';
    if( c < QMI_PORT_NAME_TBL_SIZE )
    {
      *qmi_port_name = qmi_port_name_tbl[c];
      ret = 0;
      break;
    }
  }
  return ret;
}

int main(int argc, char *argv[])
{
  int return_code = 0;
  int qmi_client_handle = -1;
  int qmi_err_code;
  int rc;
  qmi_wds_start_nw_if_params_type start_nw_params;
  qmi_wds_call_end_reason_type    call_end_reason;
  char ch;
  char *qmi_port_name = QMI_PORT_RMNET_MHI_0;  // default connection
  (void)argc; (void)argv;

  call_end_reason.call_end_reason_verbose.verbose_reason
    = QMI_WDS_VERBOSE_CE_INVALID;

  qmi_handle = qmi_init(NULL, NULL);
  if (qmi_handle < 0)
  {
    fprintf (stderr,"Unable to acquire qmi handle \n");
    fprintf (stderr, "Test failed!!\n");
    return -1;
  }
  do
  {
LOOP:
    do
    {
      return_code = -1;
      qmi_client_handle = -1;

      fprintf(stdout, "Enter '1' to bring up call, "
                      "'c' to specify connection [current=%s], "
                      "'e' to exit\n", qmi_port_name);

      while (1)
      {
        ch = getchar();
        if ((ch == '1') || (ch == 'c')|| (ch == 'e'))
        {
                break;
        }
      }

      if (ch == 'e')
      {
        return 0;
      }

      else if (ch == 'c')
      {
        if( 0>set_connection( &qmi_port_name ) )
        {
            fprintf (stderr,"Unable to assign connection\n" );
            break;
        }
        goto LOOP;  // Display menu again
      }

      else if (ch == '1')
      {
        if (qmi_client_handle < 0)
        {
          fprintf (stdout,"Opening port %s\n",qmi_port_name);
          if ((rc=qmi_connection_init (  qmi_port_name, &qmi_err_code)) < 0)
          {
            fprintf (stderr,"Unable to open %s port rc=%d, qmi_err_code=%x",qmi_port_name,rc,qmi_err_code);
            break;
          }
           fprintf (stdout,"Opening port %s is successful!\n",qmi_port_name);

          /* Bring up WDS service for first port */
          if ((qmi_client_handle = qmi_wds_srvc_init_client (qmi_port_name,NULL,NULL,&qmi_err_code)) < 0)
          {
            fprintf (stderr,"Unable to start WDS service on port %s, rc = %d, qmi_err_code=%d\n",qmi_port_name,qmi_client_handle,qmi_err_code);
            return_code = -1;
            break;
          }
          fprintf (stdout,"Starting WDS on port %s is successful!\n",qmi_port_name);
        }
      }

      else
      {
        fprintf (stdout,"Bad input =%c\n", ch);
        break;
      }

      /* Start N/W interface synchronously, use start_nw_params to
       * specify the PDP profile index you want so associate with the
       * call.  By default, APN will be device name to be unique per
       * connection. */
      start_nw_params.params_mask = QMI_WDS_START_NW_APN_NAME_PARAM;
      memcpy(start_nw_params.apn_name,qmi_port_name,sizeof(start_nw_params.apn_name));
      if (  (rc = qmi_wds_start_nw_if (qmi_client_handle,
                                       &start_nw_params,
                                       NULL,
                                       NULL,
                                       &call_end_reason,
                                       &qmi_err_code)) < 0)
      {
        fprintf (stderr,"Unable to start NW IF #1, on port %s rc=%d, "
                        "qmi_err_code = %x, call_end_reason=%d\n",
                 qmi_port_name,rc,qmi_err_code,
                 call_end_reason.call_end_reason_verbose.verbose_reason);
        break;
      }
      else
      {
        fprintf (stdout,"Successfully started NW I/F #1 on port %s\n",qmi_port_name);
      }


      fprintf (stdout,"\nPress 'q' to bring I/F down\n");


      while (1)
      {
        ch = getchar();
        if ((ch == 'q') || (ch == 'Q'))
        {
                break;
        }
      }

      fprintf (stdout,"Stopping all network interfaces\n");


      if (  (rc = qmi_wds_stop_nw_if (qmi_client_handle,NULL,NULL,&qmi_err_code)) < 0)
      {
        fprintf (stderr,"unable to stop NW IF, rc=%d, qmi_err_code = %x\n",rc,qmi_err_code);
        break;
      }

      fprintf (stdout,"Interfaces sucessfully stopped, closing clients\n");

      return_code = 0;

    } while (0);

    if (return_code == 0)
    {
      fprintf (stdout, "Test on port %s ran sucessfully!!\n", qmi_port_name);
    }
    else
    {
      fprintf (stderr, "Test failed for port %s!!\n", qmi_port_name);
    }
  } while (1);

  sleep(2);
  qmi_release(qmi_handle);
  return return_code;
}

