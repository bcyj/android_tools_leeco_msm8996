#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "qmi.h"
#include "qmi_uim_srvc.h"

static char file_path[4];
static char write_data[4];

/*qmi message library handle*/
int qmi_handle = QMI_INVALID_CLIENT_HANDLE;

static void qmi_uim_test_print_card_status_rsp
(
  qmi_uim_rsp_data_type *rsp_data_ptr
)
{
  int i, j, k;
  /* Print the response data */
  fprintf (stdout, "\nResponse data for qmi_uim_get_card_status():\n");

  fprintf (stdout, "  rsp_id = %d", rsp_data_ptr->rsp_id);
  fprintf (stdout, "  sys_err_code = %d, qmi_err_code = %x\n\n", 
           rsp_data_ptr->sys_err_code, 
           rsp_data_ptr->qmi_err_code);
  fprintf (stdout, "  get_card_status_rsp.card_status.index_gw_pri_prov = %d \n", 
           rsp_data_ptr->rsp_data.get_card_status_rsp.card_status.index_gw_pri_prov);
  fprintf (stdout, "  get_card_status_rsp.card_status.index_1x_pri_prov = %d \n", 
           rsp_data_ptr->rsp_data.get_card_status_rsp.card_status.index_1x_pri_prov);
  fprintf (stdout, "  get_card_status_rsp.card_status.index_gw_sec_prov = %d \n", 
           rsp_data_ptr->rsp_data.get_card_status_rsp.card_status.index_gw_sec_prov);
  fprintf (stdout, "  get_card_status_rsp.card_status.index_1x_sec_prov = %d \n", 
           rsp_data_ptr->rsp_data.get_card_status_rsp.card_status.index_1x_sec_prov);
  fprintf (stdout, "  get_card_status_rsp.card_status.num_slots = %d \n", 
           rsp_data_ptr->rsp_data.get_card_status_rsp.card_status.num_slots);  

  /* Print out each card details */
  for(i=0; i<rsp_data_ptr->rsp_data.get_card_status_rsp.card_status.num_slots; i++)
  {
    fprintf (stdout, "  get_card_status_rsp.card_status.card[i].card_state = %d \n", 
             rsp_data_ptr->rsp_data.get_card_status_rsp.card_status.card[i].card_state);
    fprintf (stdout, "  get_card_status_rsp.card_status.card[i].upin_state = %d \n", 
             rsp_data_ptr->rsp_data.get_card_status_rsp.card_status.card[i].upin_state);
    fprintf (stdout, "  get_card_status_rsp.card_status.card[i].upin_num_retries = %d \n", 
             rsp_data_ptr->rsp_data.get_card_status_rsp.card_status.card[i].upin_num_retries);
    fprintf (stdout, "  get_card_status_rsp.card_status.card[i].upuk_num_retries = %d \n", 
             rsp_data_ptr->rsp_data.get_card_status_rsp.card_status.card[i].upuk_num_retries);
    fprintf (stdout, "  get_card_status_rsp.card_status.card[i].card_error = %d \n", 
             rsp_data_ptr->rsp_data.get_card_status_rsp.card_status.card[i].card_error);
    fprintf (stdout, "  get_card_status_rsp.card_status.card[i].num_app = %d \n", 
             rsp_data_ptr->rsp_data.get_card_status_rsp.card_status.card[i].num_app);

    /* Print out each card app details */
    for(j=0; j<rsp_data_ptr->rsp_data.get_card_status_rsp.card_status.card[i].num_app; j++)
    {
      fprintf (stdout, "  get_card_status_rsp.card_status.card[i].application[j].app_type = %d \n", 
               rsp_data_ptr->rsp_data.get_card_status_rsp.card_status.card[i].application[j].app_type);
      fprintf (stdout, "  get_card_status_rsp.card_status.card[i].application[j].app_state = %d \n", 
               rsp_data_ptr->rsp_data.get_card_status_rsp.card_status.card[i].application[j].app_state);
      fprintf (stdout, "  get_card_status_rsp.card_status.card[i].application[j].perso_state = %d \n", 
               rsp_data_ptr->rsp_data.get_card_status_rsp.card_status.card[i].application[j].perso_state);
      fprintf (stdout, "  get_card_status_rsp.card_status.card[i].application[j].perso_feature = %d \n", 
               rsp_data_ptr->rsp_data.get_card_status_rsp.card_status.card[i].application[j].perso_feature);
      fprintf (stdout, "  get_card_status_rsp.card_status.card[i].application[j].perso_retries = %d \n", 
               rsp_data_ptr->rsp_data.get_card_status_rsp.card_status.card[i].application[j].perso_retries);
      fprintf (stdout, "  get_card_status_rsp.card_status.card[i].application[j].perso_unblock_retries = %d \n", 
               rsp_data_ptr->rsp_data.get_card_status_rsp.card_status.card[i].application[j].perso_unblock_retries);
      fprintf (stdout, "  get_card_status_rsp.card_status.card[i].application[j].aid_len = %d \n", 
               rsp_data_ptr->rsp_data.get_card_status_rsp.card_status.card[i].application[j].aid_len);

      fprintf (stdout, "  get_card_status_rsp.card_status.card[i].application[i].aid_value = ");
      for(k=0; k<rsp_data_ptr->rsp_data.get_card_status_rsp.card_status.card[i].application[j].aid_len; k++)
      {
        fprintf (stdout, " 0x%X", 
                 rsp_data_ptr->rsp_data.get_card_status_rsp.card_status.card[i].application[j].aid_value[k]);
      }

      fprintf (stdout, "  get_card_status_rsp.card_status.card[i].application[j].univ_pin = %d \n", 
               rsp_data_ptr->rsp_data.get_card_status_rsp.card_status.card[i].application[j].univ_pin);
      fprintf (stdout, "  get_card_status_rsp.card_status.card[i].application[j].pin1_state = %d \n", 
               rsp_data_ptr->rsp_data.get_card_status_rsp.card_status.card[i].application[j].pin1_state);
      fprintf (stdout, "  get_card_status_rsp.card_status.card[i].application[j].pin1_num_retries = %d \n", 
               rsp_data_ptr->rsp_data.get_card_status_rsp.card_status.card[i].application[j].pin1_num_retries);
      fprintf (stdout, "  get_card_status_rsp.card_status.card[i].application[j].puk1_num_retries = %d \n", 
               rsp_data_ptr->rsp_data.get_card_status_rsp.card_status.card[i].application[j].puk1_num_retries);
      fprintf (stdout, "  get_card_status_rsp.card_status.card[i].application[j].pin2_state = %d \n", 
               rsp_data_ptr->rsp_data.get_card_status_rsp.card_status.card[i].application[j].pin2_state);
      fprintf (stdout, "  get_card_status_rsp.card_status.card[i].application[j].pin2_num_retries = %d \n", 
               rsp_data_ptr->rsp_data.get_card_status_rsp.card_status.card[i].application[j].pin2_num_retries);
      fprintf (stdout, "  get_card_status_rsp.card_status.card[i].application[j].puk2_num_retries = %d \n", 
               rsp_data_ptr->rsp_data.get_card_status_rsp.card_status.card[i].application[j].puk2_num_retries);
    }
  }  
}


static void qmi_uim_test_print_read_transparent_rsp
(
  qmi_uim_rsp_data_type *rsp_data_ptr
)
{
  int i;
  fprintf (stdout, "\nResponse data for qmi_uim_read_transparent():\n");

  fprintf (stdout, "  rsp_id = %d", rsp_data_ptr->rsp_id);
  fprintf (stdout, "  sys_err_code = %d, qmi_err_code = %x\n", 
           rsp_data_ptr->sys_err_code, 
           rsp_data_ptr->qmi_err_code);

  fprintf (stdout, "  read_transparent_rsp.sw1 = %d \n", 
           rsp_data_ptr->rsp_data.read_transparent_rsp.sw1);
  fprintf (stdout, "  read_transparent_rsp.sw2 = %d \n", 
           rsp_data_ptr->rsp_data.read_transparent_rsp.sw2);
  fprintf (stdout, "  read_transparent_rsp.content.data_len = %d \n", 
           rsp_data_ptr->rsp_data.read_transparent_rsp.content.data_len);
  fprintf (stdout, "  read_transparent_rsp.content.data_ptr = \n");
  for(i=0; i<rsp_data_ptr->rsp_data.read_transparent_rsp.content.data_len; i++)
  {
    fprintf (stdout, " 0x%X", rsp_data_ptr->rsp_data.read_transparent_rsp.content.data_ptr[i]);
  }  
}


static void qmi_uim_test_print_read_record_rsp
(
  qmi_uim_rsp_data_type *rsp_data_ptr
)
{
  int i;
  fprintf (stdout, "\nResponse data for qmi_uim_read_record():\n");

  fprintf (stdout, "  rsp_id = %d", rsp_data_ptr->rsp_id);
  fprintf (stdout, "  sys_err_code = %d, qmi_err_code = %x\n", 
           rsp_data_ptr->sys_err_code, 
           rsp_data_ptr->qmi_err_code);

  fprintf (stdout, "  read_record_rsp.sw1 = %d \n", 
           rsp_data_ptr->rsp_data.read_record_rsp.sw1);
  fprintf (stdout, "  read_record_rsp.sw2 = %d \n", 
           rsp_data_ptr->rsp_data.read_record_rsp.sw2);
  fprintf (stdout, "  read_record_rsp.content.data_len = %d \n", 
           rsp_data_ptr->rsp_data.read_record_rsp.content.data_len);
  fprintf (stdout, "  read_record_rsp.content.data_ptr = \n");
  for(i=0; i<rsp_data_ptr->rsp_data.read_record_rsp.content.data_len; i++)
  {
    fprintf (stdout, " 0x%X", rsp_data_ptr->rsp_data.read_record_rsp.content.data_ptr[i]);
  }
}


static void qmi_uim_test_print_write_transparent_rsp
(
  qmi_uim_rsp_data_type *rsp_data_ptr
)
{
  fprintf (stdout, "\nResponse data for qmi_uim_write_transparent():\n");

  fprintf (stdout, "  rsp_id = %d", rsp_data_ptr->rsp_id);
  fprintf (stdout, "  sys_err_code = %d, qmi_err_code = %x\n", 
           rsp_data_ptr->sys_err_code, 
           rsp_data_ptr->qmi_err_code);

  fprintf (stdout, "  write_transparent_rsp.sw1 = %d \n", 
           rsp_data_ptr->rsp_data.write_transparent_rsp.sw1);
  fprintf (stdout, "  write_transparent_rsp.sw2 = %d \n", 
           rsp_data_ptr->rsp_data.write_transparent_rsp.sw2);
}


static void qmi_uim_test_print_write_record_rsp
(
  qmi_uim_rsp_data_type *rsp_data_ptr
)
{
  fprintf (stdout, "\nResponse data for qmi_uim_write_record():\n");

  fprintf (stdout, "  rsp_id = %d", rsp_data_ptr->rsp_id);
  fprintf (stdout, "  sys_err_code = %d, qmi_err_code = %x\n", 
           rsp_data_ptr->sys_err_code, 
           rsp_data_ptr->qmi_err_code);

  fprintf (stdout, "  write_record_rsp.sw1 = %d \n", 
           rsp_data_ptr->rsp_data.write_record_rsp.sw1);
  fprintf (stdout, "  write_record_rsp.sw2 = %d \n", 
           rsp_data_ptr->rsp_data.write_record_rsp.sw2);
}


static void qmi_uim_test_print_get_file_attr_rsp
(
  qmi_uim_rsp_data_type *rsp_data_ptr
)
{
  int i;
  fprintf (stdout, "\nResponse data for qmi_uim_get_file_attributes():\n");

  fprintf (stdout, "  rsp_id = %d", rsp_data_ptr->rsp_id);
  fprintf (stdout, "  sys_err_code = %d, qmi_err_code = %x\n", 
           rsp_data_ptr->sys_err_code, 
           rsp_data_ptr->qmi_err_code);

  fprintf (stdout, "  get_file_attributes_rsp.sw1 = %d \n", 
           rsp_data_ptr->rsp_data.get_file_attributes_rsp.sw1);
  fprintf (stdout, "  get_file_attributes_rsp.sw2 = %d \n", 
           rsp_data_ptr->rsp_data.get_file_attributes_rsp.sw2);

  fprintf (stdout, "  get_file_attributes_rsp.file_size = %d \n", 
           rsp_data_ptr->rsp_data.get_file_attributes_rsp.file_size);
  fprintf (stdout, "  get_file_attributes_rsp.file_id = %0X \n", 
           rsp_data_ptr->rsp_data.get_file_attributes_rsp.file_id);
  fprintf (stdout, "  get_file_attributes_rsp.file_type = %d \n", 
           rsp_data_ptr->rsp_data.get_file_attributes_rsp.file_type);
  fprintf (stdout, "  get_file_attributes_rsp.record_size = %d \n", 
           rsp_data_ptr->rsp_data.get_file_attributes_rsp.record_size);
  fprintf (stdout, "  get_file_attributes_rsp.record_count = %d \n", 
           rsp_data_ptr->rsp_data.get_file_attributes_rsp.record_count);

  fprintf (stdout, "  get_file_attributes_rsp.read_security.security_value = %d \n", 
           rsp_data_ptr->rsp_data.get_file_attributes_rsp.read_security.security_value);
  fprintf (stdout, "  get_file_attributes_rsp.read_security: pin1 = %d, pin2 = %d, upin = %d, adm = %d \n", 
           rsp_data_ptr->rsp_data.get_file_attributes_rsp.read_security.pin1,
           rsp_data_ptr->rsp_data.get_file_attributes_rsp.read_security.pin2,
           rsp_data_ptr->rsp_data.get_file_attributes_rsp.read_security.upin,
           rsp_data_ptr->rsp_data.get_file_attributes_rsp.read_security.adm);
  fprintf (stdout, "  get_file_attributes_rsp.write_security.security_value = %d \n", 
           rsp_data_ptr->rsp_data.get_file_attributes_rsp.write_security.security_value);
  fprintf (stdout, "  get_file_attributes_rsp.write_security: pin1 = %d, pin2 = %d, upin = %d, adm = %d \n", 
           rsp_data_ptr->rsp_data.get_file_attributes_rsp.write_security.pin1,
           rsp_data_ptr->rsp_data.get_file_attributes_rsp.write_security.pin2,
           rsp_data_ptr->rsp_data.get_file_attributes_rsp.write_security.upin,
           rsp_data_ptr->rsp_data.get_file_attributes_rsp.write_security.adm);
  fprintf (stdout, "  get_file_attributes_rsp.increase_security.security_value = %d \n", 
           rsp_data_ptr->rsp_data.get_file_attributes_rsp.increase_security.security_value);
  fprintf (stdout, "  get_file_attributes_rsp.increase_security: pin1 = %d, pin2 = %d, upin = %d, adm = %d \n", 
           rsp_data_ptr->rsp_data.get_file_attributes_rsp.increase_security.pin1,
           rsp_data_ptr->rsp_data.get_file_attributes_rsp.increase_security.pin2,
           rsp_data_ptr->rsp_data.get_file_attributes_rsp.increase_security.upin,
           rsp_data_ptr->rsp_data.get_file_attributes_rsp.increase_security.adm);
  fprintf (stdout, "  get_file_attributes_rsp.deactivate_security.security_value = %d \n", 
          rsp_data_ptr->rsp_data.get_file_attributes_rsp.deactivate_security.security_value);
  fprintf (stdout, "  get_file_attributes_rsp.deactivate_security: pin1 = %d, pin2 = %d, upin = %d, adm = %d \n", 
           rsp_data_ptr->rsp_data.get_file_attributes_rsp.deactivate_security.pin1,
           rsp_data_ptr->rsp_data.get_file_attributes_rsp.deactivate_security.pin2,
           rsp_data_ptr->rsp_data.get_file_attributes_rsp.deactivate_security.upin,
           rsp_data_ptr->rsp_data.get_file_attributes_rsp.deactivate_security.adm);
  fprintf (stdout, "  get_file_attributes_rsp.activate_security.security_value = %d \n", 
          rsp_data_ptr->rsp_data.get_file_attributes_rsp.activate_security.security_value);
  fprintf (stdout, "  get_file_attributes_rsp.activate_security: pin1 = %d, pin2 = %d, upin = %d, adm = %d \n", 
           rsp_data_ptr->rsp_data.get_file_attributes_rsp.activate_security.pin1,
           rsp_data_ptr->rsp_data.get_file_attributes_rsp.activate_security.pin2,
           rsp_data_ptr->rsp_data.get_file_attributes_rsp.activate_security.upin,
           rsp_data_ptr->rsp_data.get_file_attributes_rsp.activate_security.adm);

  fprintf (stdout, "  get_file_attributes_rsp.raw_value.data_len = %d \n", 
           rsp_data_ptr->rsp_data.get_file_attributes_rsp.raw_value.data_len);
  fprintf (stdout, "  get_file_attributes_rsp.raw_value.data_ptr = \n");
  for(i=0; i<rsp_data_ptr->rsp_data.get_file_attributes_rsp.raw_value.data_len; i++)
  {
    fprintf (stdout, " 0x%X", 
             rsp_data_ptr->rsp_data.get_file_attributes_rsp.raw_value.data_ptr[i]);
  }
}


static void qmi_uim_test_print_event_reg_rsp
(
  qmi_uim_rsp_data_type *rsp_data_ptr
)
{
  fprintf (stdout, "\nResponse data for qmi_uim_event_reg():\n");

  fprintf (stdout, "  rsp_id = %d", rsp_data_ptr->rsp_id);
  fprintf (stdout, "  sys_err_code = %d, qmi_err_code = %x\n", 
           rsp_data_ptr->sys_err_code, 
           rsp_data_ptr->qmi_err_code);

  fprintf (stdout, "  event_reg_rsp.event_mask = %lu \n", 
           rsp_data_ptr->rsp_data.event_reg_rsp.event_mask);
}


static int qmi_uim_test_get_file_attributes_send
(
  qmi_client_handle_type                      uim_client_handle,
  qmi_uim_get_file_attributes_params_type   * get_file_attributes_param,
  qmi_uim_rsp_data_type                     * rsp_data
)
{
  file_path[0] = 0x3F;
  file_path[1] = 0x00;
  file_path[2] = 0x7F;
  file_path[3] = 0xFF;

  fprintf (stdout, "\nResponse data for qmi_uim_test_get_file_attributes_send():\n");

  get_file_attributes_param->session_info.session_type = QMI_UIM_SESSION_TYPE_PRI_GW_PROV;
  get_file_attributes_param->session_info.aid.data_len = 0;
  get_file_attributes_param->session_info.aid.data_ptr = NULL;

  get_file_attributes_param->file_id.file_id = 0x6F07;   /* EF IMSI */
  get_file_attributes_param->file_id.path.data_len = 4;
  get_file_attributes_param->file_id.path.data_ptr = (unsigned char*)file_path;

  return qmi_uim_get_file_attributes (uim_client_handle,
                                      get_file_attributes_param,
                                      NULL, 
                                      NULL,
                                      rsp_data);
}


static int qmi_uim_test_read_transparent_send
(
  qmi_client_handle_type                      uim_client_handle,
  qmi_uim_read_transparent_params_type      * read_transparent_param,
  qmi_uim_rsp_data_type                     * rsp_data
)
{  
  file_path[0] = 0x3F;
  file_path[1] = 0x00;
  file_path[2] = 0x7F;
  file_path[3] = 0xFF;

  fprintf (stdout, "\nResponse data for qmi_uim_test_read_transparent_send():\n");

  read_transparent_param->session_info.session_type = QMI_UIM_SESSION_TYPE_PRI_GW_PROV;
  read_transparent_param->session_info.aid.data_len = 0;
  read_transparent_param->session_info.aid.data_ptr = NULL;

  read_transparent_param->file_id.file_id = 0x6F07;   /* EF IMSI */
  read_transparent_param->file_id.path.data_len = 4;
  read_transparent_param->file_id.path.data_ptr = (unsigned char*)file_path;
  read_transparent_param->offset = 0;
  read_transparent_param->length = 0;

  return qmi_uim_read_transparent (uim_client_handle,
                                   read_transparent_param,
                                   NULL, 
                                   NULL,
                                   rsp_data);
}


static int qmi_uim_test_read_record_send
(
  qmi_client_handle_type                      uim_client_handle,
  qmi_uim_read_record_params_type           * read_record_param,
  qmi_uim_rsp_data_type                     * rsp_data
)
{  
  file_path[0] = 0x3F;
  file_path[1] = 0x00;
  file_path[2] = 0x7F;
  file_path[3] = 0xFF;

  read_record_param->session_info.session_type = QMI_UIM_SESSION_TYPE_PRI_GW_PROV;
  read_record_param->session_info.aid.data_len = 0;
  read_record_param->session_info.aid.data_ptr = NULL;

  read_record_param->file_id.file_id = 0x6FC5;   /* EF PLMN Network Name */
  read_record_param->file_id.path.data_len = 4;
  read_record_param->file_id.path.data_ptr = (unsigned char*)file_path;
  read_record_param->record = 1;
  read_record_param->length = 0;

  return qmi_uim_read_record (uim_client_handle,
                              read_record_param,
                              NULL, 
                              NULL,
                              rsp_data);
}


static int qmi_uim_test_write_transparent_send
(
  qmi_client_handle_type                      uim_client_handle,
  qmi_uim_write_transparent_params_type     * write_transparent_param,
  qmi_uim_rsp_data_type                     * rsp_data
)
{  
  file_path[0] = 0x3F;
  file_path[1] = 0x00;
  file_path[2] = 0x7F;
  file_path[3] = 0xFF;

  write_data[0] = 0x12;
  write_data[1] = 0x13;

  write_transparent_param->session_info.session_type = QMI_UIM_SESSION_TYPE_PRI_GW_PROV;
  write_transparent_param->session_info.aid.data_len = 0;
  write_transparent_param->session_info.aid.data_ptr = NULL;

  write_transparent_param->file_id.file_id = 0x6F07;   /* EF IMSI */
  write_transparent_param->file_id.path.data_len = 4;
  write_transparent_param->file_id.path.data_ptr = (unsigned char*)file_path;
  write_transparent_param->offset = 0;
  write_transparent_param->data.data_len = 2;
  write_transparent_param->data.data_ptr = (unsigned char*)write_data;

  return qmi_uim_write_transparent (uim_client_handle,
                                   write_transparent_param,
                                   NULL, 
                                   NULL,
                                   rsp_data);
}


static int qmi_uim_test_write_record_send
(
  qmi_client_handle_type                      uim_client_handle,
  qmi_uim_write_record_params_type          * write_record_param,
  qmi_uim_rsp_data_type                     * rsp_data
)
{  
  file_path[0] = 0x3F;
  file_path[1] = 0x00;
  file_path[2] = 0x7F;
  file_path[3] = 0xFF;

  write_data[0] = 0x30;
  write_data[1] = 0x31;
  write_data[2] = 0x32;
  write_data[3] = 0x33;

  write_record_param->session_info.session_type = QMI_UIM_SESSION_TYPE_PRI_GW_PROV;
  write_record_param->session_info.aid.data_len = 0;
  write_record_param->session_info.aid.data_ptr = NULL;

  write_record_param->file_id.file_id = 0x6FC5;   /* EF PLMN Network Name */
  write_record_param->file_id.path.data_len = 4;
  write_record_param->file_id.path.data_ptr = (unsigned char*)file_path;

  write_record_param->record = 1;
  write_record_param->data.data_len = 4;
  write_record_param->data.data_ptr = (unsigned char*)write_data;

  return qmi_uim_write_record (uim_client_handle,
                              write_record_param,
                              NULL, 
                              NULL,
                              rsp_data);
}

static int qmi_uim_test_event_reg_send
(
  qmi_client_handle_type                      uim_client_handle,
  qmi_uim_event_reg_params_type             * event_reg_param,
  qmi_uim_rsp_data_type                     * rsp_data
)
{
  event_reg_param->card_status = QMI_UIM_TRUE;

  return qmi_uim_event_reg (uim_client_handle,
                            event_reg_param,
                            rsp_data);
}


int main(int argc, char *argv[])
{
  int rc, main_ret, qmi_err_code;
  qmi_client_handle_type    uim_client_handle;
  qmi_uim_rsp_data_type     rsp_data;
  (void)argc; (void)argv;

  /* Variables for each command API */
  qmi_uim_read_transparent_params_type      read_transparent_param;
  qmi_uim_read_record_params_type           read_record_param;
  qmi_uim_write_transparent_params_type     write_transparent_param;
  qmi_uim_write_record_params_type          write_record_param;
  qmi_uim_get_file_attributes_params_type   get_file_attributes_param;
  qmi_uim_set_pin_protection_params_type    set_pin_protection_param;
  qmi_uim_verify_pin_params_type            verify_pin_param;
  qmi_uim_unblock_pin_params_type           unblock_pin_param;
  qmi_uim_change_pin_params_type            change_pin_param;
  qmi_uim_depersonalization_params_type     depersonalization_param;
  qmi_uim_power_down_params_type            power_down_param;
  qmi_uim_power_up_params_type              power_up_param;
  qmi_uim_event_reg_params_type             event_reg_param;

  qmi_handle = qmi_init (NULL,NULL);

  if (qmi_handle < 0)
  {
    fprintf (stderr,"Unable to acquire qmi handle \n");
    fprintf (stderr, "Test failed!!\n");
    return -1;
  }

  /* Initialize connection to first QMI control connection (corresponds to SMD_DATA_5 for data transfer */
  if ((rc = qmi_connection_init (QMI_PORT_RMNET_4, &qmi_err_code)) < 0)
  {
    fprintf (stderr,"Unable to open QMI RMNET_4 port rc=%d, qmi_err_code=%x\n",
             rc,
             qmi_err_code);
    return -1;
  }

  fprintf (stderr,"Starting UIM service.... \n");

  /* Initialize UIM service */
  if ((uim_client_handle = qmi_uim_srvc_init_client (QMI_PORT_RMNET_4, 
                                                     NULL, 
                                                     NULL, 
                                                     &qmi_err_code)) < 0)
  {
    fprintf (stderr,"Unable to start UIM service rc = %d, qmi_err_code=%x\n", 
             uim_client_handle, 
             qmi_err_code);
    return -1;
  }
  else
  {
    fprintf (stderr,"Opened UIM Client. rc = %d, handle= %x \n", rc, uim_client_handle);
  }

  /* Initialize main return code */
  main_ret = 0;

  /* Test 6 - qmi_uim_get_file_attributes */
  if ((rc = qmi_uim_test_get_file_attributes_send(uim_client_handle,
                                                  &get_file_attributes_param,
                                                  &rsp_data)) < 0)
  {
    fprintf (stderr, "Error for qmi_uim_get_file_attributes(), rc = %d, qmi_err_code =%x\n", 
             rc, 
             rsp_data.qmi_err_code);
    main_ret = -1;    
    goto finish;
  }
  qmi_uim_test_print_get_file_attr_rsp(&rsp_data);
  if(rsp_data.rsp_data.get_file_attributes_rsp.raw_value.data_ptr)
  {
    free(rsp_data.rsp_data.get_file_attributes_rsp.raw_value.data_ptr);
  }

  /* Test 2 - qmi_uim_read_transparent */
  if ((rc = qmi_uim_test_read_transparent_send (uim_client_handle,
                                                &read_transparent_param,
                                                &rsp_data)) < 0)
  {
    fprintf (stderr, "Error for qmi_uim_read_transparent(), rc = %d, qmi_err_code =%x\n", 
             rc, 
             rsp_data.qmi_err_code);
    main_ret = -1;
    goto finish;
  }
  qmi_uim_test_print_read_transparent_rsp(&rsp_data);
  if(rsp_data.rsp_data.read_transparent_rsp.content.data_ptr)
  {
    free(rsp_data.rsp_data.read_transparent_rsp.content.data_ptr);
  }

  /* Test 3 - qmi_uim_read_record */
  if ((rc = qmi_uim_test_read_record_send (uim_client_handle,
                                           &read_record_param,
                                           &rsp_data)) < 0)
  {
    fprintf (stderr, "Error for qmi_uim_read_record(), rc = %d, qmi_err_code =%x\n", 
             rc, 
             rsp_data.qmi_err_code);
    main_ret = -1;
    goto finish;
  }
  qmi_uim_test_print_read_record_rsp(&rsp_data);
  if(rsp_data.rsp_data.read_record_rsp.content.data_ptr)
  {
    free(rsp_data.rsp_data.read_record_rsp.content.data_ptr);
  }


  /* Test 4 - qmi_uim_write_transparent */
  if ((rc = qmi_uim_test_write_transparent_send (uim_client_handle,
                                                 &write_transparent_param,
                                                 &rsp_data)) < 0)
  {
    fprintf (stderr, "Error for qmi_uim_write_tranparent(), rc = %d, qmi_err_code =%x\n", 
             rc, 
             rsp_data.qmi_err_code);
    main_ret = -1;
    goto finish;
  }
  qmi_uim_test_print_write_transparent_rsp(&rsp_data);


  /* Test 5 - qmi_uim_write_record */
  if ((rc = qmi_uim_test_write_record_send (uim_client_handle,
                                            &write_record_param,
                                            &rsp_data)) < 0)
  {
    fprintf (stderr, "Error for qmi_uim_write_record(), rc = %d, qmi_err_code =%x\n", 
             rc, 
             rsp_data.qmi_err_code);
    main_ret = -1;
    goto finish;
  }
  qmi_uim_test_print_write_record_rsp(&rsp_data);

  /* Test 1 - qmi_uim_get_card_status - Sync call */  
  if ((rc = qmi_uim_get_card_status (uim_client_handle, 
                                     NULL, 
                                     NULL,
                                     &rsp_data)) < 0)
  {
    fprintf (stderr, "Error for qmi_uim_get_card_status(), rc = %d, qmi_err_code =%x\n", 
             rc, 
             rsp_data.qmi_err_code);
    main_ret = -1;
    goto finish;
  }
  qmi_uim_test_print_card_status_rsp(&rsp_data);


  /* Test 14 - qmi_uim_event_reg */
  if ((rc = qmi_uim_test_event_reg_send (uim_client_handle,
                                         &event_reg_param,                              
                                         &rsp_data)) < 0)
  {
    fprintf (stderr, "Error for qmi_uim_event_reg(), rc = %d, qmi_err_code =%x\n", 
             rc, 
             rsp_data.qmi_err_code);
    main_ret = -1;
    goto finish;
  }
  qmi_uim_test_print_event_reg_rsp(&rsp_data);

  /* Test 7 - qmi_uim_set_pin_protection */
  if ((rc = qmi_uim_set_pin_protection (uim_client_handle,
                                        &set_pin_protection_param,
                                        NULL, 
                                        NULL,
                                        &rsp_data)) < 0)
  {
    fprintf (stderr, "Error for qmi_uim_set_pin_protection(), rc = %d, qmi_err_code =%x\n", 
             rc, 
             rsp_data.qmi_err_code);
    main_ret = -1;
    goto finish;
  }


  /* Test 8 - qmi_uim_verify_pin */
  if ((rc = qmi_uim_verify_pin (uim_client_handle,
                                &verify_pin_param,
                                NULL, 
                                NULL,
                                &rsp_data)) < 0)
  {
    fprintf (stderr, "Error for qmi_uim_verify_pin(), rc = %d, qmi_err_code =%x\n", 
             rc, 
             rsp_data.qmi_err_code);
    main_ret = -1;
    goto finish;
  }


  /* Test 9 - qmi_uim_unblock_pin */
  if ((rc = qmi_uim_unblock_pin (uim_client_handle,
                                &unblock_pin_param,
                                NULL, 
                                NULL,
                                &rsp_data)) < 0)
  {
    fprintf (stderr, "Error for qmi_uim_unblock_pin(), rc = %d, qmi_err_code =%x\n", 
             rc, 
             rsp_data.qmi_err_code);
    main_ret = -1;
    goto finish;
  }


  /* Test 10 - qmi_uim_change_pin */
  if ((rc = qmi_uim_change_pin (uim_client_handle,
                                &change_pin_param,
                                NULL, 
                                NULL,
                                &rsp_data)) < 0)
  {
    fprintf (stderr, "Error for qmi_uim_change_pin(), rc = %d, qmi_err_code =%x\n", 
             rc, 
             rsp_data.qmi_err_code);
    main_ret = -1;
    goto finish;
  }


  /* Test 11 - qmi_uim_depersonalization */
  if ((rc = qmi_uim_depersonalization (uim_client_handle,
                                       &depersonalization_param,
                                       NULL, 
                                       NULL,
                                       &rsp_data)) < 0)
  {
    fprintf (stderr, "Error for qmi_uim_depersonalization(), rc = %d, qmi_err_code =%x\n", 
             rc, 
             rsp_data.qmi_err_code);
    main_ret = -1;
    goto finish;
  }

  /* Test 12 - qmi_uim_power_down */
  if ((rc = qmi_uim_power_down (uim_client_handle,
                                &power_down_param,
                                NULL, 
                                NULL,
                                &rsp_data)) < 0)
  {
    fprintf (stderr, "Error for qmi_uim_power_down(), rc = %d, qmi_err_code =%x\n", 
             rc, 
             rsp_data.qmi_err_code);
    main_ret = -1;
    goto finish;
  }

  /* Test 13 - qmi_uim_power_up */
  if ((rc = qmi_uim_power_up (uim_client_handle,
                              &power_up_param,
                              NULL, 
                              NULL,
                              &rsp_data)) < 0)
  {
    fprintf (stderr, "Error for qmi_uim_power_up(), rc = %d, qmi_err_code =%x\n", 
             rc, 
             rsp_data.qmi_err_code);
    main_ret = -1;
    goto finish;
  }

  /* Test 15 - qmi_uim_refresh_register */

  /* Test 16 - qmi_uim_refresh_ok */

  /* Test 17 - qmi_uim_refresh_complete */

  /* Test 18 - qmi_uim_refresh_get_last_event */
  

finish:
  if ((rc = qmi_uim_srvc_release_client (uim_client_handle, &qmi_err_code)) < 0)
  {
    fprintf (stderr,"Unable to release UIM client handle, rc = %d, qmi_err_code = %x\n",
             rc,
             qmi_err_code);
  }
  else
  {
    fprintf (stdout,"Released UIM client\n");
  }

  if (main_ret == 0)
  {
    fprintf (stdout, "Test ran sucessfully!!\n");
  }
  else
  {
    fprintf (stderr, "Test failed!!\n");
  }
  qmi_release(qmi_handle);
  return main_ret;  
}
