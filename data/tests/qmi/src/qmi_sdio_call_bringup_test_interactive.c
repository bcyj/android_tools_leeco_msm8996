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


static char * qmi_sdio_port_names[9] =
{ /* left blank intentionally */"",
  QMI_PORT_RMNET_SDIO_0,
  QMI_PORT_RMNET_SDIO_1,
  QMI_PORT_RMNET_SDIO_2,
  QMI_PORT_RMNET_SDIO_3,
  QMI_PORT_RMNET_SDIO_4,
  QMI_PORT_RMNET_SDIO_5,
  QMI_PORT_RMNET_SDIO_6,
  QMI_PORT_RMNET_SDIO_7
};

int main(int argc, char *argv[])
{
  int return_code = 0;
  int qmi_client_handle = 0;
  int qmi_err_code;
  int rc;
  qmi_wds_start_nw_if_params_type start_nw_params;
  qmi_wds_call_end_reason_type    call_end_reason;
  (void)argc; (void)argv;

  call_end_reason.call_end_reason_verbose.verbose_reason  
    = QMI_WDS_VERBOSE_CE_INVALID;
  char ch;
  int index=0;
  char * qmi_port_name = "";

  qmi_handle = qmi_init (NULL,NULL);

  if (qmi_handle < 0)
  {
    fprintf (stderr,"Unable to acquire qmi handle \n");
    fprintf (stderr, "Test failed!!\n");
    return -1;
  }

  do
  {

    do
    {
      return_code = -1;
      qmi_client_handle = 0;

      fprintf(stdout, "enter qmi sdio port number (1..8)");

      ch = (char)getchar();

      if ((ch < '1') || (ch > '8'))
      {
        fprintf(stderr, "invalid input");
        break;
      }

      index = ch - '0';

      qmi_port_name = qmi_sdio_port_names[index];

      /* Initialize connection to first QMI control connection (corresponds to SMD_DATA_5 for data transfer */
      if ((rc=qmi_connection_init (qmi_port_name, &qmi_err_code)) < 0)
      {
        fprintf (stderr,"Unable to open %s port rc=%d, qmi_err_code=%x",qmi_port_name,rc,qmi_err_code);
        break;
      }

      /* Initialize connection to second QMI control connection (corresponds to SMD_DATA_6 for data transfer */

      fprintf (stdout,"Starting WDS services.... \n");

      /* Bring up WDS service for first port */
      if ((qmi_client_handle = qmi_wds_srvc_init_client (qmi_port_name,NULL,NULL,&qmi_err_code)) < 0)
      {
        fprintf (stderr,"Unable to start WDS service on port #1 rc = %d, qmi_err_code=%d\n",qmi_client_handle,qmi_err_code);
        return_code = -1;
        break;
      }

      fprintf (stdout, "sucessfully opened all QMI WDS client\n");

      /* Start N/W interface #1 synchronously, use start_nw_params to specify 
      ** the PDP profile index you want so associate with the call
      */
      start_nw_params.params_mask = 0;
      if (  (rc = qmi_wds_start_nw_if (qmi_client_handle,
                                       &start_nw_params,
                                       NULL,
                                       NULL,
                                       &call_end_reason,
                                       &qmi_err_code)) < 0)
      {
        fprintf (stderr,"unable to start NW IF #1, rc=%d, qmi_err_code = %x\n",rc,qmi_err_code);
        fprintf (stderr,"Call End Reason Code is = %d \n",
                 call_end_reason.call_end_reason_verbose.verbose_reason);
        break;
      }
      else
      {
        fprintf (stdout,"Successfully started NW I/F #1\n");
      }


      fprintf (stdout,"\nPress 'q' to bring I/F down\n");


      while (1)
      {
        ch = (char)getchar();
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

    if(qmi_client_handle > 0)
    {
      if (  (rc=qmi_wds_srvc_release_client (qmi_client_handle,&qmi_err_code)) < 0)
      {
        fprintf  (stderr,"Unable to release QMI client handle for port %s, rc=%d, qmi_err_code=%d\n",qmi_port_name,rc,qmi_err_code);
      }
    }

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


