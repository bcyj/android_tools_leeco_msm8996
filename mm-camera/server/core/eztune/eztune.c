/**********************************************************************
* Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.*
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <string.h>

#include "camera_dbg.h"
#include "eztune_items.h"
#include "eztune.h"

/*#define member_of(ptr, type, member) ({                      \
  const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
  (type *)( (char *)__mptr - offsetof(type,member) );})*/

extern eztune_item_t *eztune_items;
static uint16_t num_to_start = 0;
int eztune_item_counter = 0;
static my_list_t  apply_clist;
static int8_t eztune_status = 0;

int8_t eztune_get_status (void)
{
  return eztune_status;
}

static void eztune_set_status (int ez_enable)
{
  eztune_status = ez_enable;
}

/*===========================================================================
 * FUNCTION     - eztune_get_list_process -
 *
 * DESCRIPTION:
 * ==========================================================================*/
static int32_t eztune_get_list_process (void *ez, int client_socket)
{
  int rc;
  int offset_buflen = 3;
  uint16_t i = 0;
  uint16_t total_item = EZT_PARMS_MAX;
  uint8_t ack = 1;
  char output_buf[EZTUNE_MAX_SEND];

  memset(output_buf, 0, sizeof(output_buf));
  rc = write(client_socket, &ack, 1);
  CDBG_EZ("ez = 0x%x\n", (uint32_t)ez);
  CDBG_EZ("offset_buflen = %d\n", offset_buflen);
  CDBG_EZ("num_to_start = %d\n", num_to_start);

  do {
    eztune_item_t item = eztune_get_item(num_to_start);
    CDBG_EZ("item.name = %s\n", item.name);
    CDBG_EZ("item.id = %d\n", item.id);

    if ((offset_buflen + 8 + strlen(item.name) + 1) > EZTUNE_MAX_SEND) {
      CDBG_EZ("for next buffer items!!!!");
      memcpy(output_buf, "\x01", 1);
      break;
    }
    memcpy(output_buf + offset_buflen, &num_to_start, 2);
    offset_buflen += 2;
    CDBG_EZ("offset_buflen_item.index = %d\n", offset_buflen);

    if (item.offset) {
      memcpy(output_buf + offset_buflen, &item.size, 2);
    } else {
      memcpy(output_buf + offset_buflen, "\x00\x00", 2);
    }
    offset_buflen += 2;
    CDBG_EZ("offset_buflen_item.offset = %d\n", offset_buflen);

    if (item.reg_flag == EZT_WRITE_FLAG) {
      memcpy(output_buf + offset_buflen, "\x00\x00\x00\x00", 4);
    }else if (item.reg_flag == EZT_READ_FLAG || item.reg_flag == EZT_3A_FLAG) {
      memcpy(output_buf + offset_buflen, "\x01\x00\x00\x00", 4);
    } else if (item.reg_flag == EZT_CHROMATIX_FLAG) {
      memcpy(output_buf + offset_buflen, "\x40\x00\x00\x00", 4);
    } else if (item.reg_flag == EZT_AUTOFOCUS_FLAG) {
      memcpy(output_buf + offset_buflen, "\x00\x04\x00\x00", 4);
    } else if (item.reg_flag == (EZT_AUTOFOCUS_FLAG | EZT_READ_FLAG)) {
      memcpy(output_buf + offset_buflen, "\x01\x04\x00\x00", 4);
    }

    offset_buflen += 4;
    CDBG_EZ("offset_buflen_item.type = %d\n", offset_buflen);

    strlcpy(output_buf + offset_buflen, item.name, strlen(item.name) + 1);
    offset_buflen = offset_buflen + strlen(item.name) + 1;
    CDBG_EZ("offset_buflen_item.name = %d\n", offset_buflen);

    num_to_start += 1;
    i++;

    if (total_item <= num_to_start) {
      memcpy(output_buf, "\x00", 1);
      num_to_start = 0;
      break;
    }
  } while (1);

  memcpy(output_buf + 1, &i, 2);

  rc = write(client_socket, output_buf, EZTUNE_MAX_SEND);
  CDBG_EZ("%s: %d\n", __func__, rc);
  CDBG_EZ("eztune_get_list_process: num_to_start = %d\n",
    num_to_start);

  return rc;
}/*end of eztune_get_list_process*/

/*===========================================================================
 * FUNCTION     - eztune_add_to_list -
 *
 * DESCRIPTION:
 * ==========================================================================*/
int eztune_add_to_list (eztune_set_val_t item)
{
  my_list_t * tmp;
  tmp = ( my_list_t*)malloc(sizeof(my_list_t));
  if (!tmp) {
    perror("malloc");
    exit(1);
  }

  tmp->data.item_num = item.item_num;
  tmp->data.table_index = item.table_index;
  tmp->data.value_string = (char *) malloc(strlen (item.value_string)+1);
  if (!tmp->data.value_string) {
    free(tmp);
    perror("malloc error ");
    exit(1);
  }
  strlcpy(tmp->data.value_string, item.value_string,
    strlen(item.value_string) + 1);

  add_node( &(tmp->list), &(apply_clist.list) );
  CDBG_EZ("eztune_add_to_list: LIST HEad2 = 0x%x\n",
    (uint32_t)(&apply_clist.list));
  CDBG_EZ("eztune_add_to_list: Added element to list and index is %d,\
    item_num=%d\n", tmp->data.table_index, tmp->data.item_num );
  CDBG_EZ("eztune_add_to_list: list pointer is tmp->list=0x%x, tmp is 0x%x\n",
    (uint32_t)(&(tmp->list)), (uint32_t)(tmp));

  usleep(1000);
  eztune_item_counter++;
  return TRUE;
}/* end of eztune_add_to_list*/

/*===========================================================================
 * FUNCTION     - eztune_print_list -
 *
 * DESCRIPTION:
 * ==========================================================================*/
void eztune_print_list ()
{
  struct ezlist *pos;
  struct ezlist *clist = &(apply_clist.list);
  my_list_t *tmp;
  CDBG_EZ("eztune: printList\n");
  for (pos = clist->next ; pos != clist ; pos = pos->next ) {
    tmp= member_of(pos, my_list_t , list);
    CDBG_EZ("eztune: item_num = %d,table_index = %d,value is %s\n",
      tmp->data.item_num, tmp->data.table_index,tmp->data.value_string);
  }
}/* end of eztune_print_list*/

/*===========================================================================
 * FUNCTION     - eztune_delete_list -
 *
 * DESCRIPTION:
 * ==========================================================================*/
void eztune_delete_list ()
{
  struct ezlist * pos, *q;
  struct ezlist *clist = &(apply_clist.list);
  my_list_t * tmp;
  CDBG_EZ("eztune: delete_list\n");
  /*list_for_each_safe(pos, q, &apply_clist.list)*/
  for (pos =(clist)->next, q =pos->next; pos != (clist);
    pos = q, q = pos->next) {
    tmp= member_of(pos, my_list_t,list);
    /*
     * CDBG_EZ("eztune_apply_changes: Freeing item_num = %d,
     *    Table_index = %d,value is %s\n",
     *    tmp->data.item_num, tmp->data.table_index,
     *    tmp->data.value_string);
     */
    del_node(pos);
    free(tmp->data.value_string) ;
    free(tmp);
  }
  eztune_item_counter = 0;
}/*end of eztune_delete_list*/

/*===========================================================================
 * FUNCTION     - eztune_set_parms_process -
 *
 * DESCRIPTION:
 * ==========================================================================*/
static int32_t eztune_set_parms_process (void *ez, int client_socket)
{
  int i;
  int rc = 0;
  eztune_set_val_t item;
  uint16_t num_items = *(uint16_t *)ez;
  char output_buf[EZTUNE_MAX_SEND];
  uint8_t ack = 1;

  ez = (uint8_t *)ez + sizeof(uint16_t);
  CDBG_EZ("eztune_set_parms_process: num_items = %d\n", num_items);

  rc = write(client_socket, &ack, 1);

  memset(output_buf, 0, EZTUNE_MAX_SEND);

  CDBG_EZ("eztune_set_parms_process: LIST HEad = 0x%x\n",
    (uint32_t)(&apply_clist.list));

  for (i=0 ; i<num_items ; ++i) {
    int rc = 0;

    item.item_num = *(uint16_t *)ez;
    ez = (uint8_t *)ez + sizeof(uint16_t);
    item.table_index = *(uint16_t *)ez;
    ez = (uint8_t *)ez + sizeof(uint16_t);
    item.value_string = (char *) malloc(strlen (ez)+1);
    if (!item.value_string) {
      CDBG_EZ(" %s malloc \n ", __func__);
      exit(1);
    }
    strlcpy(item.value_string, ez, strlen(ez) + 1);
    ez = (uint8_t *)ez + strlen (ez) + 1;
    rc = eztune_add_to_list(item);
    free(item.value_string);
  }

  if (i ==  num_items) {
    memcpy(output_buf , "\x01", 1);
  }
  rc = write(client_socket, output_buf, EZTUNE_MAX_SEND);
  return rc;
}/* end of eztune_set_parms_process*/

/*===========================================================================
 * FUNCTION     - eztune_apply_changes -
 *
 * DESCRIPTION:
 * ==========================================================================*/
static int32_t eztune_apply_changes (eztune_t *ezctrl)
{
  struct ezlist * pos;
  struct ezlist *clist = &(apply_clist.list);
  my_list_t * tmp;
  int32_t cnt = 0;
  eztune_item_t temp;
  eztune_item_type_t ez_type = EZTUNE_TYPE_INVALID;
//  isp3a_set_data_t isp3a_set_parm;
//  isp3a_set_parm.type = ISP3A_RELOAD_CHROMATIX;
  CDBG_EZ("eztune_apply_changes: LIST HEad = 0x%x\n",
    (uint32_t)(&apply_clist.list));
#if 0
  vfe_ctrl_t *vfeCtrl = config_get_ctrl(vfe_ctrl_t, vfeCtrl);
  sensor_ctrl_t *sensorCtrl = config_get_ctrl(sensor_ctrl_t, sensorCtrl);
  isp3a_ctrl_t *isp3aCtrl = config_get_ctrl(isp3a_ctrl_t, isp3aCtrl);
#endif
  uint8_t update_default_gamma = FALSE, update_lowlight_gamma = FALSE;
  uint8_t update_outdoor_gamma = FALSE, update_backlight_gamma=FALSE;
  uint8_t update_rolloff = FALSE;

  /*This parameter is added to track the isp3a parameters changes from eztune_change_item()
     and set those corresponding changes in to the 3A stats while applying the changes and
     reseting the chromatix */

  for ( pos = clist->next ; pos != clist ; pos = pos->next ) {
    /*list_for_each(pos,&apply_clist.list)*/

    /* at this point: pos->next points to the next item's 'list' variable and
     * pos->prev points to the previous item's 'list' variable. Here item is
     * of type struct kool_list. But we need to access the item itself not the
     * variable 'list' in the item! macro list_entry() does just that. See "How
     * does this work?" below for an explanation of how this is done.
     */
    tmp= member_of(pos, my_list_t , list);

    temp=eztune_get_item( tmp->data.item_num);
    /* given a pointer to struct ezlist1, type of data structure it is part of,
     * and it's name (struct ezlist1's name in the data structure) it returns a
     * pointer to the data structure in which the pointer is part of.
     * For example, in the above line list_entry() will return a pointer to the
     * struct kool_list item it is embedded in!
     */
    CDBG_EZ("eztune_apply_changes: list pointer is tmp->list=0x%x,\
      list is 0x%x tmp is 0x%x\n", (uint32_t)(&(tmp->list)),
      (uint32_t)pos, (uint32_t)(tmp));
    CDBG_EZ("eztune_apply_changes: Got element at index  %d\n",
      tmp->data.table_index);
    /*usleep(1000);*/
    eztune_change_item(tmp, ezctrl);
    if(temp.type == EZT_T_CHROMATIX)
      ez_type = EZT_T_CHROMATIX;

  }

  CDBG_EZ("eztune_apply_changes: Applied all the changes \n");
  if(ez_type == EZT_T_CHROMATIX) {
    mctl_eztune_set_vfe(
      VFE_MODULE_ALL, SET_RELOAD_CHROMATIX, 0);
    mctl_eztune_set_3A(EZ_RELOAD_CHROMATIX, 1);
  }
  eztune_delete_list();
  CDBG_EZ("eztune_apply_changes: Deleted the list\n");

  return TRUE;
}/* end of eztune_apply_changes*/

/*===========================================================================
 * FUNCTION     - eztune_misc_cmds_process -
 *
 * DESCRIPTION:
 * ==========================================================================*/
static int32_t eztune_misc_cmds_process (void *ez, eztune_t *ezctrl)
{
  int rc = 0;
  uint8_t message_id = *(uint8_t *)ez;
  char out_buffer[128];
  uint8_t ack = 1;
  int client_socket = ezctrl->clientsocket_id;
  memset(out_buffer, 0, sizeof(out_buffer));

  CDBG_EZ("eztune_misc_cmds_process: message_id = %d\n",message_id);

  rc = write(client_socket, &ack, 1);
  CDBG_EZ("EZTUNE_CMDS_PROCESS: ack: %d\n", rc);

  switch (message_id) {
    case EZTUNE_MISC_GET_VERSION:
      strlcpy(out_buffer, "2.1.0", strlen("2.1.0") + 1);
      rc = write(client_socket, &out_buffer, 128);
      CDBG_EZ("EZTUNE_MISC_GET_VERSION: rc = %d\n", rc);
      break;

    case EZTUNE_MISC_APPLY_CHANGES:
      rc = eztune_apply_changes(ezctrl);
      memset(out_buffer, 0, sizeof(out_buffer));
      CDBG_EZ("eztune_apply_changes: rc = %d\n", rc);

      if (rc <  0)
        memcpy(out_buffer,"\x01", 1);
      rc = write(client_socket, &out_buffer, 128);
      CDBG_EZ("EZTUNE_MISC_APPLY_CHANGES: rc = %d\n", rc);
      break;
    default:
      break;
  }
  return TRUE;
}

int32_t ez_read(int client_socket, char *recv_buffer, uint32_t length)
{
  uint32_t recv_len;
  recv_len = read(client_socket, recv_buffer, length);
  CDBG_EZ("recv_len = %d\n", recv_len);
  if ((recv_len !=length) ||(recv_len == 0)) {
    CDBG_EZ("%s: read fail on the connection. %d vs %d\n",
    __func__, length, recv_len);
    return -1;
  }
  return 0;
}
int32_t eztune_server_readwrite(eztune_t *ezctrl)
{
  int32_t rc;
  char recv_buffer[EZTUNE_MAX_RECV];
  int client_socket = ezctrl->clientsocket_id;
  eztune_protocol_t *p = ezctrl->protocol_ptr;

   rc = ez_read(client_socket, recv_buffer, p->next_recv_len);
   if (rc < 0)
     return rc;
    switch (p->next_recv_code) {
      case EZTUNE_RECV_COMMAND:
        p->current_cmd = *(uint16_t *)recv_buffer;
        p->next_recv_code = EZTUNE_RECV_PAYLOAD_SIZE;
        p->next_recv_len = sizeof(uint32_t);
        CDBG_EZ("EZTUNE_RECV_COMMAND\n");
        rc = ez_read(client_socket, recv_buffer, p->next_recv_len);
        if (rc < 0)
          return rc;

      case EZTUNE_RECV_PAYLOAD_SIZE:
        p->next_recv_code = EZTUNE_RECV_PAYLOAD;
        p->next_recv_len = *(uint32_t *)recv_buffer;
        CDBG_EZ("EZTUNE_RECV_PAYLOAD_SIZE\n");
        if (p->next_recv_len > EZTUNE_MAX_RECV)
          return -1;
        rc = ez_read(client_socket, recv_buffer, p->next_recv_len);
        if (rc < 0)
          return rc;

      case EZTUNE_RECV_PAYLOAD:
        p->recv_buf = malloc(p->next_recv_len);
        if (!p->recv_buf) {
          CDBG_EZ("%s:Error allocating memory for recv_buf\n",__func__);
          free(p);
          return -1;
        }
        memcpy(p->recv_buf, recv_buffer, p->next_recv_len);
        p->next_recv_code = EZTUNE_RECV_RESPONSE;
        p->next_recv_len = sizeof(uint32_t);
        CDBG_EZ("EZTUNE_RECV_PAYLOAD\n");
        rc = ez_read(client_socket, recv_buffer, p->next_recv_len);
        if (rc < 0)
          return rc;


      case EZTUNE_RECV_RESPONSE:
        p->next_recv_code = EZTUNE_RECV_COMMAND;
        p->next_recv_len = 2;
        CDBG_EZ("EZTUNE_RECV_RESPONSE\n");
        break;
      case EZTUNE_RECV_INVALID:
        CDBG_EZ("EZTUNE_RECV_INVALID\n");
        break;

      default:
        CDBG_EZ("p->next_recv_code: default\n");
    }

    switch (p->current_cmd) {
      case EZTUNE_GET_LIST:
        rc = eztune_get_list_process(p->recv_buf, client_socket);
        CDBG_EZ("EZTUNE_GET_LIST\n");
        break;

      case EZTUNE_GET_PARMS:
        rc = eztune_get_parms_process(p->recv_buf, ezctrl);
        CDBG_EZ("EZTUNE_GET_PARMS\n");
        break;

      case EZTUNE_SET_PARMS:
        rc = eztune_set_parms_process(p->recv_buf, client_socket);
        CDBG_EZ("EZTUNE_SET_PARMS\n");
        break;

      case EZTUNE_MISC_CMDS:
        rc = eztune_misc_cmds_process(p->recv_buf, ezctrl);
        CDBG_EZ("EZTUNE_MISC_CMDS\n");
        break;

      default:
        CDBG_EZ("p->current_cmd: default\n");
        break;
    }
  if (p->recv_buf) {
    free(p->recv_buf);
    p->recv_buf = NULL ;
  }
  CDBG_EZ("__debug %s ends \n", __func__);

  return rc;
}
/*===========================================================================
 * FUNCTION     - eztune_server_connect-
 *
 * DESCRIPTION:
 * ==========================================================================*/
int32_t eztune_server_connect(eztune_t *ezctrl)
{
  int32_t rc;
  int camfd = 0 ;
  int  test_count = 0;
  int  failure_count = 0;
  char *p_command_args;
  char *p_command_result;
  char *text;

  int16_t  result = 0;
  uint16_t recv;
  char     ack_1[6];
  uint16_t a = 1;
  uint32_t b = 1;
  int socket_flag;
  int client_socket = ezctrl->clientsocket_id;
  init_list(&apply_clist.list);

  CDBG_EZ("%s starts\n", __FUNCTION__);

  memcpy(ack_1, &a, 2);
  memcpy(ack_1+2, &b, 4);
  /* send echo back to client upon accept */
  if ((rc = write(client_socket, &ack_1, sizeof(ack_1))) < 0) {
    CDBG_EZ("eztune_server_run: write returns %d\n", rc);
    return rc;
  }
  ezctrl->protocol_ptr = malloc(sizeof(eztune_protocol_t));
  if (!ezctrl->protocol_ptr) {
    CDBG_EZ("malloc returns NULL\n");
    return -1;
  }

  ezctrl->protocol_ptr->current_cmd    = 0xFFFF;
  ezctrl->protocol_ptr->next_recv_code = EZTUNE_RECV_COMMAND;
  ezctrl->protocol_ptr->next_recv_len  = 2;
  ezctrl->protocol_ptr->recv_buf       = NULL;
  ezctrl->protocol_ptr->send_buf       = NULL;

  CDBG_EZ("%s end \n", __func__);

  return rc;
}

void eztune_init(eztune_t *ez_ctrl)
{
  if(NULL == ez_ctrl) {
    CDBG_EZ("%s null control passed\n", __func__);
    return;
  }
  ez_ctrl->fn_table.eztune_status = eztune_get_status;
}
