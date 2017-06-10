
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "qmi.h"
#include "qmi_wds_srvc.h"
#include "qmi_nas_srvc.h"
#include "qmi_qos_srvc.h"

static const char* 
                qmi_link_to_conn_id_map[3] = 
{
    QMI_PORT_RMNET_1,
    QMI_PORT_RMNET_2,
    QMI_PORT_RMNET_3
};

static const char*
qmi_get_conn_id_for_link (int link)
{
    /* Verify that link id is valid */
    if (link >= 0 && link < 3) {
        /* Return qmi connection id for the link */
        return qmi_link_to_conn_id_map[link];
    }
    return NULL;
}

/*qmi message library handle*/
int qmi_handle = QMI_INVALID_CLIENT_HANDLE;

int main(int argc, char *argv[])
{
  int wds_handles[QMI_MAX_CONNECTIONS][20]; 
  int qos_handles[QMI_MAX_CONNECTIONS][20]; 
  int nas_handles[QMI_MAX_CONNECTIONS][20]; 
  int qmi_err_code;
  int i, j, main_ret, rc;
  (void)argc; (void)argv;

  main_ret = 0;

  qmi_handle = qmi_init (NULL,NULL);

  if (qmi_handle < 0)
  {
    fprintf (stderr,"Unable to acquire qmi handle \n");
    fprintf (stderr, "Test failed!!\n");
    return -1;
  }

  /* Initialize connection to first QMI control connection (corresponds to SMD_DATA_5 for data transfer */
  if ((rc=qmi_connection_init (QMI_PORT_RMNET_1, &qmi_err_code)) < 0)
  {
    fprintf (stderr,"Unable to open QMI RMNET_1 port rc=%d, qmi_err_code=%x\n",rc,qmi_err_code);
    return -1;
  }
  if ((rc=qmi_connection_init (QMI_PORT_RMNET_2, &qmi_err_code)) < 0)
  {
    fprintf (stderr,"Unable to open QMI RMNET_3 port rc=%d, qmi_err_code=%x\n",rc,qmi_err_code);
    return -1;
  }
  if ((rc=qmi_connection_init (QMI_PORT_RMNET_3, &qmi_err_code)) < 0)
  {
    fprintf (stderr,"Unable to open QMI RMNET_3 port rc=%d, qmi_err_code=%x\n",rc,qmi_err_code);
    return -1;
  }
 
  for (j = 0; j < 3; j++)
  {
    for (i = 0; i < 9; i++) 
    {
      if ((wds_handles[j][i] = qmi_wds_srvc_init_client (qmi_get_conn_id_for_link(j),NULL,NULL,&qmi_err_code)) < 0)
      {
        fprintf (stderr,"Allocated failed, allocated %d WDS handles for port %d\n",i - 1,j);
        break;
      }
      else
      {
        fprintf (stderr,"Open WDS client handle for conn=%d, count = %d, handle= %x \n",j,i,wds_handles[j][i]);
      }
    }

    
    for (i = 0; i < 9; i++) 
    {
      if ((qos_handles[j][i] = qmi_qos_srvc_init_client (qmi_get_conn_id_for_link(j),NULL,NULL,&qmi_err_code)) < 0)
      {
        fprintf (stderr,"Allocated failed, allocated %d QOS handles for port %d\n",i - 1,j);
        break;
      }
      else
      {
        fprintf (stderr,"Open QOS client handle for conn=%d, count = %d, handle= %x \n",j,i,qos_handles[j][i]);
      }
    }

    for (i = 0; i < 9; i++) 
    {
      if ((nas_handles[j][i] = qmi_nas_srvc_init_client (qmi_get_conn_id_for_link(j),NULL,NULL,&qmi_err_code)) < 0)
      {
        fprintf (stderr,"Allocated failed, allocated %d NAS handles for port %d\n",i - 1,j);
        break;
      }
      else
      {
        fprintf (stderr,"Open NAS client handle for conn=%d, count = %d, handle= %x \n",j,i,nas_handles[j][i]);
      }
    }
  }


  fprintf (stderr,"Pausing....\n");

  getchar();


  qmi_release(qmi_handle);
  return main_ret;
  
}




  
