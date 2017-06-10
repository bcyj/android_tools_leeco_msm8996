#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "qmi.h"
#include "qmi_wds_srvc.h"


int qmi_client_handle;

/*qmi message library handle*/
int qmi_handle = QMI_INVALID_CLIENT_HANDLE;


int main(int argc, char *argv[])
{
  int return_code = 0;
  int qmi_client_handle;
  int qmi_err_code;
  int rc;
  qmi_wds_start_nw_if_params_type start_nw_params;
  qmi_wds_call_end_reason_type    call_end_reason;
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
   
  /* Initialize connection to first QMI control connection (corresponds to SMD_DATA_5 for data transfer */
  if ((rc=qmi_connection_init (QMI_PORT_RMNET_4, &qmi_err_code)) < 0)
  {
    fprintf (stderr,"Unable to open QMI RMNET_4 port rc=%d, qmi_err_code=%x",rc,qmi_err_code);
    return_code = -1;
    goto exit0;
  }

  /* Initialize connection to second QMI control connection (corresponds to SMD_DATA_6 for data transfer */

  fprintf (stdout,"Starting WDS services.... \n");

  /* Bring up WDS service for first port */
  if ((qmi_client_handle = qmi_wds_srvc_init_client (QMI_PORT_RMNET_4,NULL,NULL,&qmi_err_code)) < 0)
  {
    fprintf (stderr,"Unable to start WDS service on port #1 rc = %d, qmi_err_code=%d\n",qmi_client_handle,qmi_err_code);
    return_code = -1;
    goto exit3;
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
    return_code = -1;
    goto exit3;
  }
  else
  {
    fprintf (stdout,"Sucessfully started NW IF #1\n");
  }


  fprintf (stdout,"Network interface started... will stop in 3 seconds\n");

  sleep (3);

  fprintf (stdout,"Stopping all network interfaces\n");


  if (  (rc = qmi_wds_stop_nw_if (qmi_client_handle,NULL,NULL,&qmi_err_code)) < 0)
  {
    fprintf (stderr,"unable to stop NW IF, rc=%d, qmi_err_code = %x\n",rc,qmi_err_code);
    return_code = -1;
    goto exit3;

  } 

  fprintf (stdout,"Interfaces sucessfully stopped, closing clients\n");

exit3: 
  if (  (rc=qmi_wds_srvc_release_client (qmi_client_handle,&qmi_err_code)) < 0)
  {
    fprintf  (stderr,"Unable to release QMI client handle for port #1, rc=%d, qmi_err_code=%d\n",rc,qmi_err_code);
    return_code = -1;
  }

exit0:
  if (return_code == 0)
  {
    fprintf (stdout, "Test ran sucessfully!!\n");
  }
  else
  {
    fprintf (stderr, "Test failed!!\n");
  }
  sleep(2);	  
  qmi_release(qmi_handle);	
  return return_code;
}


