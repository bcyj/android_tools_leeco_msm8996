#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "qmi.h"
#include "qmi_eap_srvc.h"

/*qmi message library handle*/
int qmi_handle = QMI_INVALID_CLIENT_HANDLE;

void
print_aka_data
(
  const unsigned char * aka_data,
  int aka_data_len
)
{
  int i;

  if (aka_data != NULL)
  {
    for (i = 0; i < aka_data_len; i++)
    {
      fprintf(stderr, "%0x", aka_data[i]);
    }
  }
  else
  {
    fprintf(stderr, "print_aka_data() called "
                    "but nothing to print \n");
  }
}

void
qmi_eap_indication
(
  int                           user_handle,
  qmi_service_id_type           service_id,
  void                          *user_data,
  qmi_eap_indication_id_type    ind_id,
  qmi_eap_indication_data_type  *ind_data
)
{
 int len = 0, i = 0;
 fprintf(stderr,"Call Back function called \n");

 if (ind_id == QMI_EAP_AKA_RESULT_IND_MSG)
 {
   fprintf(stderr, "aka result = %ld \n",
           ind_data->aka_result.aka_handle);
   fprintf(stderr, "aka status = %d \n",
           ind_data->aka_result.status);
   fprintf(stderr, "auth result = %d \n",
           ind_data->auth_result);
   fprintf(stderr, "aka result mask = %d \n",
           ind_data->aka_result.param_mask);
   fprintf(stderr, "auth params data len = %d \n",
           ind_data->aka_result.v1_or_v2_auth_params.aka_data_len);

   if (ind_data->aka_result.param_mask & QMI_EAP_AKA_V1_OR_V2_AUTH_RESP_PARAMS)
   {
     len = ind_data->aka_result.v1_or_v2_auth_params.aka_data_len;

     if (len > 0)
     {
       fprintf(stderr, "\n aka data = ");
       print_aka_data(ind_data->aka_result.v1_or_v2_auth_params.aka_data,
                      len);
     }

     len = ind_data->aka_result.v1_or_v2_auth_params.digest_len;

     if (len > 0)
     {
       fprintf(stderr, "\n digest data = ");
       print_aka_data(ind_data->aka_result.v1_or_v2_auth_params.digest,
                      len);
     }
   }

 }
 else
 {
   fprintf(stderr, "Not interested in this indiacation %d \n", ind_id);
 }
 user_handle = user_handle;
 service_id = service_id;
 user_data = user_data;
}

int
main
(
  int argc,
  char *argv[]
)
{
  int return_code = 0;
  int qmi_client_handle;
  int qmi_err_code;
  int rc;
  unsigned long aka_handle;
  qmi_eap_initiate_aka_algorithm_type aka_info;
#define RAND_LEN    16
  char rand[RAND_LEN] = {0x23,0x55,0x3c,0xbe,0x96,0x37,0xa8,0x9d,0x21,0x8a,0xe6,0x4d,0xae,0x47,0xbf,0x36};
#define AUTH_LEN    16
  char auth[AUTH_LEN] = {0x24,0x54,0x2a,0x28,0x7c,0xb4,0x30,0x31,0xf0,0x8c,0xeb,0x89,0x88,0x98,0xb9,0xf6};


  memset(&aka_info, 0,  sizeof(aka_info));

  qmi_handle = qmi_init(NULL, NULL);

  if (qmi_handle < 0)
  {
    fprintf (stderr,"Unable to acquire qmi handle \n");
    fprintf (stderr, "Test failed!!\n");
    return -1;
  }

  if ((rc = qmi_connection_init (QMI_PORT_RMNET_1, &qmi_err_code)) < 0)
  {
    fprintf (stderr,"Unable to open QMI RMNET_1 port rc=%d, "
                    "qmi_err_code=%x",rc,qmi_err_code);
    return_code = -1;
    goto exit0;
  }

  fprintf (stdout,"Starting EAP service.... \n");

  if ((qmi_client_handle = qmi_eap_srvc_init_client (QMI_PORT_RMNET_1,
                                                     qmi_eap_indication,
                                                     NULL,
                                                     &qmi_err_code)) < 0)
  {
    fprintf (stderr,"Unable to start EAP service on port #1 "
                    "rc = %d, qmi_err_code=%d\n",
             qmi_client_handle,qmi_err_code);
    getchar();
    return_code = -1;
    goto exit3;
  }

  fprintf (stdout,"Started EAP service.... \n");

  aka_info.aka_version = QMI_EAP_AKA_ALGORITHM_VERSION_2;

  aka_info.param_mask = QMI_EAP_AKA_V1_OR_V2_AUTH_PARAMS;

  aka_info.v1_or_v2_auth_params.rand_len = 16;

  memcpy(aka_info.v1_or_v2_auth_params.rand,
         rand,
         RAND_LEN);

  aka_info.v1_or_v2_auth_params.auth_len = 16;

  memcpy(aka_info.v1_or_v2_auth_params.auth,
         auth,
         AUTH_LEN);

   if ( (rc = qmi_eap_auth_initiate_aka_algorithm (qmi_client_handle,
                                                   &aka_info,
                                                   &aka_handle,
                                                   &qmi_err_code)) < 0)
   {
     fprintf (stderr,"Unable to initiate AKA algorithm rc = %d,"
                     " qmi_err_code=%d \n",
              rc, qmi_err_code);
     getchar();
     return_code = -1;
     goto exit3;
   }
   fprintf (stdout," Successfully initiated the AKA algorithm \n");
   fprintf (stdout," Aka handle is %ld \n", aka_handle);

   fprintf (stdout," Hit any Key at any time to exit the test application,"
                   " or Wait for call back information to be printed \n");
   getchar();

   argc = argc;
   argv = argv;

exit3:
  if (  (rc = qmi_eap_srvc_release_client (qmi_client_handle,
                                           &qmi_err_code)) < 0)
  {
    fprintf  (stderr,"Unable to release QMI client handle for port #1, "
                     "rc=%d, qmi_err_code=%d\n",rc,qmi_err_code);
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


