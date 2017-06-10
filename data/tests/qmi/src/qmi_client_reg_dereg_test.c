
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <cutils/properties.h>
#include "qmi.h"
#include "qmi_wds_srvc.h"
#include "qmi_nas_srvc.h"
#include "qmi_qos_srvc.h"

typedef enum {
  QMI_TRANSPORT_MIN,
  QMI_TRANSPORT_SMD = QMI_TRANSPORT_MIN,
  QMI_TRANSPORT_BAM = QMI_TRANSPORT_MIN,
  QMI_TRANSPORT_SDIO,
  QMI_TRANSPORT_USB,
  QMI_TRANSPORT_MAX
} qmi_transport_type;

struct {
  qmi_transport_type  type;
  char  name[32];
} qmi_transports_names[] =
{
  { QMI_TRANSPORT_SMD,  "QMI_TRANSPORT_SMD/BAM" },
  { QMI_TRANSPORT_SDIO, "QMI_TRANSPORT_SDIO"    },
  { QMI_TRANSPORT_USB,  "QMI_TRANSPORT_USB"     }
};

#define QMI_MAX_PORTS (8)

static const char* 
qmi_link_to_conn_id_map[QMI_TRANSPORT_MAX][QMI_MAX_PORTS] = 
{
  /* SMD/BAM transport */
  { QMI_PORT_RMNET_0,
    QMI_PORT_RMNET_1,
    QMI_PORT_RMNET_2,
    QMI_PORT_RMNET_3,
    QMI_PORT_RMNET_4,
    QMI_PORT_RMNET_5,
    QMI_PORT_RMNET_6,
    QMI_PORT_RMNET_7
  },
  /* SDIO transport */
  {
    QMI_PORT_RMNET_SDIO_0,
    QMI_PORT_RMNET_SDIO_1,
    QMI_PORT_RMNET_SDIO_2,
    QMI_PORT_RMNET_SDIO_3,
    QMI_PORT_RMNET_SDIO_4,
    QMI_PORT_RMNET_SDIO_5,
    QMI_PORT_RMNET_SDIO_6,
    QMI_PORT_RMNET_SDIO_7
  },
  /* USB transport */
  {
    QMI_PORT_RMNET_USB_0,
    QMI_PORT_RMNET_USB_1,
    QMI_PORT_RMNET_USB_2,
    QMI_PORT_RMNET_USB_3,
    QMI_PORT_RMNET_USB_4,
    QMI_PORT_RMNET_USB_5,
    QMI_PORT_RMNET_USB_6,
    QMI_PORT_RMNET_USB_7
  }
};


#define QMI_PROPERTY_BASEBAND    "ro.baseband"
#define QMI_PROPERTY_BASEBAND_SIZE   10
#define QMI_BASEBAND_VALUE_MSM       "msm"
#define QMI_BASEBAND_VALUE_APQ       "apq"
#define QMI_BASEBAND_VALUE_SVLTE1    "svlte1"
#define QMI_BASEBAND_VALUE_SVLTE2A   "svlte2a"
#define QMI_BASEBAND_VALUE_CSFB      "csfb"
#define QMI_BASEBAND_VALUE_MDMUSB    "mdm"
#define QMI_BASEBAND_VALUE_UNDEFINED "undefined"



static const char*
qmi_get_conn_id_for_link (int transport, int link)
{
  /* Verify that link id is valid */
  if( (transport >= QMI_TRANSPORT_MIN && transport < QMI_TRANSPORT_MAX) && 
      (link >= 0 && link < QMI_MAX_PORTS) ) {
    /* Return qmi connection id for the link */
    return qmi_link_to_conn_id_map[transport][link];
  }
  return NULL;
}

int qmi_run_test_for_transport( int transport )
{
  int qmi_err_code;
  int rc = -1;
  int j;
  int wds_handles[QMI_MAX_PORTS]; 
  
  if( transport >= QMI_TRANSPORT_MIN && transport < QMI_TRANSPORT_MAX )
  {
    memset(wds_handles,0x0,sizeof(wds_handles));
    
    fprintf (stderr,"=== Running test for transport [%s] ===\n", qmi_transports_names[transport].name);
  
    /* Initialize QMI control connections */
    for (j = 0; j < QMI_MAX_PORTS; j++)
    {
      const char * port = qmi_get_conn_id_for_link(transport,j);
    
      if ((rc=qmi_connection_init( port, &qmi_err_code)) < 0)
      {
        fprintf (stderr,"qmi_connection_init failed for port[%s] rc[%d] qmi_err_code[0x%x]\n",port,rc,qmi_err_code);
        continue;
      }
    }
 
    for (j = 0; j < QMI_MAX_PORTS; j++)
    {
      const char * port = qmi_get_conn_id_for_link(transport,j);
    
      if ((wds_handles[j] = qmi_wds_srvc_init_client (port,NULL,NULL,&qmi_err_code)) < 0)
      {
        fprintf (stderr,"qmi_wds_srvc_init_client failed for port[%d] rc[%d] qmi_err_code[0x%x]\n",
                        j, wds_handles[j], qmi_err_code);
        continue;
      }
      else
      {
        fprintf (stderr,"Register WDS client handle for dev=%s conn=%d, handle=0x%08x\n",port,j,wds_handles[j]);
      }
    }

    for (j = 0; j < QMI_MAX_PORTS; j++)
    {
      if ((rc = qmi_wds_srvc_release_client (wds_handles[j],&qmi_err_code)) < 0)
      {
        fprintf (stderr,"qmi_wds_srvc_release_client failed for port[%d] rc[%d] qmi_err_code[0x%x]\n",
                        j,rc,qmi_err_code);
        continue;
      }
      else
      {
        fprintf (stderr,"Deregister WDS client handle for conn=%d, handle= 0x%08x\n",j,wds_handles[j]);
      }
    }
    rc = 0;
  }
  
  return rc;
}

/*qmi message library handle*/
int qmi_handle = QMI_INVALID_CLIENT_HANDLE;

int main(int argc, char *argv[])
{
  int main_ret;
  char buff[PROPERTY_VALUE_MAX];

  (void)argc; (void)argv;
  main_ret = 0;
  
  qmi_handle = qmi_init (NULL,NULL);

  if (qmi_handle < 0)
  {
    fprintf (stderr,"Unable to acquire qmi handle \n");
    fprintf (stderr, "Test failed!!\n");
    return -1;
  }

  /* Query platform property for target ID */
  (void)property_get( QMI_PROPERTY_BASEBAND, buff, QMI_BASEBAND_VALUE_UNDEFINED);
  fprintf (stderr,"Baseband property [%s]\n", buff);

  /* Execute test for specific transport based on target*/
  if( !strcmp(buff,QMI_BASEBAND_VALUE_UNDEFINED) ) {
    fprintf (stderr,"Aborting test!!\n");
    main_ret = -1;
  }
  else if( !strcmp(buff,QMI_BASEBAND_VALUE_MSM) ) {
    main_ret |= qmi_run_test_for_transport( QMI_TRANSPORT_SMD );
  }
  else if( !strcmp(buff,QMI_BASEBAND_VALUE_CSFB) ||
           !strcmp(buff,QMI_BASEBAND_VALUE_APQ) ) {
    main_ret |= qmi_run_test_for_transport( QMI_TRANSPORT_SDIO );
  }
  else if( !strcmp(buff,QMI_BASEBAND_VALUE_SVLTE1) ||
           !strcmp(buff,QMI_BASEBAND_VALUE_SVLTE2A) ) {
    main_ret |= qmi_run_test_for_transport( QMI_TRANSPORT_SMD );
    main_ret |= qmi_run_test_for_transport( QMI_TRANSPORT_SDIO );
  }
  else if( !strcmp(buff,QMI_BASEBAND_VALUE_MDMUSB) ) {
    main_ret |= qmi_run_test_for_transport( QMI_TRANSPORT_USB );
  }

  fprintf (stderr,"Pausing....\n");
  getchar();

  qmi_release(qmi_handle);

  fprintf(stderr, "TEST %s\n", (0==main_ret)? "PASSED" : "FAILED" );
  return main_ret;
}




  
