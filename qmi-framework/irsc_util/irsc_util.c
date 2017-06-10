/****************************************************************************
  @file    irsc_util.c
  @brief   The IPC Router Security Configuration Tool.

  DESCRIPTION
  This module parses the security configuration at system initialization
  and populates the IPC Router  with the configuration.

  ---------------------------------------------------------------------------
  Copyright (c) 2013-2015 QUALCOMM Technologies Incorporated. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "msm_ipc.h"

#ifdef USE_GLIB
#include <glib.h>
#define strlcpy g_strlcpy
#endif

#define FILE_NAME 50
#define MAX_LINE_LEN 100
#define MAX_BUF_LEN 200
#define MAX_GROUPS 255
#define DIGIT_TYPE 0x01
#define BUFFER_START 0xaa
#define BUFFER_END 0xff
#define MIN_NUM_ELE 3

#if defined DEBUG_ANDROID

#define LOG_TAG "irsc_util"
#include <utils/Log.h>
#include "common_log.h"

#define IRSC_DEBUG(x...) SLOGI(x)
#define IRSC_ERR(x...) LOGE(x)

#else

#define IRSC_DEBUG(x...) printf(x)
#define IRSC_ERR(x...) fprintf(stderr, x);

#endif

struct format_rule {
   char delimeter;
};

struct security_rule_info {
   int num_group_info;
   uint32_t service_id;
   uint32_t instance_id;
   unsigned reserved;
   gid_t group_id[MAX_GROUPS];
};

struct config_security_rules_args {
   uint32_t num_entries;
   struct security_rule_info info[0];
};

struct irsc_tool {
   char sec_cfg_f_name[FILE_NAME];
   size_t file_name_len;
   FILE *sec_cfg_fp;
   struct config_security_rules_args *sec_info;
   struct format_rule fmt;
   enum irsc_tool_err (*parser)(void *);
   enum irsc_tool_err (*feed_sec_config)(void *);
};

enum irsc_tool_err {
   IRSC_NO_ERR,
   IRSC_INVALID_ARG,
   IRSC_INVALID_FORMAT,
   IRSC_INVALID_FILE,
   IRSC_READ_ERR,
   IRSC_IOCTL_ERR,
   IRSC_NO_MEM
};

/**
 * get_int_val - Calculate base 10 value.
 * @in_buf: Pointer to buffer.
 * @len: Length of buffer.
 * @val: Out pointer, will contain base 10 value after successful completion.
 * @Return: Returns the error values specified in enum irsc_tool_err.
 *
 * This function calculates the base 10 value from the bytes contained in
 * the in_buff.
 */
static enum irsc_tool_err get_int_val
(
uint8_t *in_buf,
uint32_t len,
uint32_t *val
)
{
   uint32_t i, res = 0, tmp;
   uint8_t *buf;

   if(!in_buf)
      return IRSC_INVALID_ARG;

   buf = in_buf;

   for( i = 1; i <= len; i++) {
      tmp = *buf;
      tmp -= '0';
      res = res*10 + tmp;
      buf++;
   }

   *val = res;
   return IRSC_NO_ERR;
}

/**
 * line_parser - Parse the line provided.
 * @line: Pointer to line buffer.
 * @delimit: Character used to separate the fields in the line.
 * @in_buf: Out pointer, will contain the parsed output.
 * @len: Length of the in_buf.
 * @num_ele: Out pointer, will contain number of fields in the line
 * on successful completion.
 * @Return: Returns the error values specified in enum irsc_tool_err.
 *
 * This function allows parsing of a line from the security configuration file.
 * The function basically extracts the field separated by the delimiter, and
 * stores them in buffer pointed to by in_buf. The format of the buffer is:
 * -------------- ----- -------- -----  -----------  ------------
 *  BUFFER_START |TYPE | LENGTH |VALUE| ...     | BUFFER_END |
 * --------------------------------------------------------------
 *  Currently only DIGIT_TYPE is supported. This function
 *  can be modified to support other types as well.
 */
static enum irsc_tool_err line_parser
(
char *line,
char delimit,
uint8_t *in_buf,
uint32_t len,
uint32_t *num_ele
)
{
   char *to = NULL,*frm = NULL;
   uint8_t ele_len = 0, line_len = 0;
   uint8_t *buf;
   uint32_t n_ele = 0;
   if(!line || !in_buf || len == 0 || !num_ele)
      return IRSC_INVALID_ARG;

   /* Parsing the line and looking for digits and delimiter */
   to = frm = line;
   buf = in_buf;
   *buf = BUFFER_START;
   buf++;
   while(1)
   {
      if(isdigit(*to)) {
         ele_len++;
      } else if((*to == delimit || *to == '\n' || *to == '\r' || *to == '\0')
                && ele_len > 0) {
         *buf = DIGIT_TYPE;
         buf++;
         *buf = ele_len;
         buf++;
         memcpy(buf, frm, ele_len);
         if((to + 1) != NULL)
            frm = to + 1;
         if(line_len < len) {
            buf += ele_len;
         } else {
            IRSC_ERR("%s Line Buffer length exceeded!\n", __func__);
         }
         ele_len = 0;
         n_ele++;
         if (*to == '\n' || *to == '\r' || *to == '\0')
             break;
      } else {
           n_ele = 0;
           break;
      }
      to++;
      line_len++;
   }
   *num_ele = n_ele;
   *buf = BUFFER_END;
   return IRSC_NO_ERR;
}

/**
 * file_parser - Parse the security configuration file.
 * @fp: File pointer.
 * @arg: Out Pointer, Will hold configuration data, if arg is valid(not NULL).
 * @delimiter: Character used to separate the fields.
 * @num_entries: Out Pointer, Will hold the number of configuration specified
 * in the file. Each configuration corresponds to service_id and instance_id
 * combination.
 * @Return: Returns the error values specified in enum irsc_tool_err.
 *
 * This function returns the security configuration data in the arg output
 * pointer on successful completion. If arg is NULL then it will parse
 * the file and provide the number of entries in the num_entries output
 * pointer.The function basically extracts the information from the formatted
 * buffer as returned by the line_parser function. The information is extracted
 * based on predetermined security configuration format as shown below
 * <service_id>:<instance_id>:<group_id_1>:<group_id_2>:...:<group_id_n>.
 * Any changes in the configuration format in future can be handled by
 * manipulating the parse_state variable.
 *
 */
static enum irsc_tool_err file_parser
(
FILE *fp,
struct config_security_rules_args *arg,
char delimeter,
uint32_t *num_entries
)
{
   uint32_t  entries = 0, parse_state;
   enum irsc_tool_err err;
   uint8_t *buf_ptr, buf[MAX_BUF_LEN], len;
   uint32_t num_ele_line = 0;
   char line[MAX_LINE_LEN];

   if(!fp || !num_entries)
      return IRSC_INVALID_ARG;


   while(fgets(line, MAX_LINE_LEN, fp))
   {
      err = line_parser(line, delimeter, buf, MAX_BUF_LEN,
                      &num_ele_line);
      IRSC_DEBUG("%s Number of element :%d\n", __func__, num_ele_line);
      if(num_ele_line < MIN_NUM_ELE) {
         IRSC_DEBUG("%s Invalid Format or Comment - '%s'\n", __func__, line);
         continue;
      }

      if(err == IRSC_NO_ERR) {
         buf_ptr = buf;
         parse_state = 0;
         while(*buf_ptr != BUFFER_END) {
            if(*buf_ptr == DIGIT_TYPE) {
               buf_ptr++;
               len = *buf_ptr;
               buf_ptr++;
               if(arg != NULL) {
                  if(parse_state == 0)
                     get_int_val(buf_ptr,len, &arg->info[entries].service_id);

                  if(parse_state == 1)
                     get_int_val(buf_ptr,len, &arg->info[entries].instance_id);

                  if( parse_state > 1)
                     get_int_val(buf_ptr,len,
                                &arg->info[entries].group_id[parse_state - 2]);
               }
               buf_ptr += len;
               parse_state++;
            } else {
               buf_ptr++;
            }
         }
         if(arg) {
            arg->info[entries].num_group_info = num_ele_line - 2;
         }
         num_ele_line = 0;
         entries++;
      }
   }

   *num_entries = entries;
   return IRSC_NO_ERR;
}

/**
 * irsc_parser - Extract the information from security configuration file.
 * @irsc_obj: IRSC object.
 * @Return: Returns the error values specified in enum irsc_tool_err.
 */
static  enum irsc_tool_err irsc_parser(void *irsc_obj)
{
   uint32_t num_entries = 0, n = 0;
   enum irsc_tool_err err;
   struct config_security_rules_args* sec_args;
   struct irsc_tool *irsc_d;


   if(!irsc_obj)
      return IRSC_INVALID_ARG;

   irsc_d = (struct irsc_tool *)irsc_obj;

   if(irsc_d->file_name_len > FILE_NAME - 1) {
      IRSC_ERR("File name too long %d !< %d\n",
               irsc_d->file_name_len, FILE_NAME - 1);
      err = IRSC_INVALID_FILE;
      goto bail;
   }

   IRSC_DEBUG("Trying to open sec config file\n");
   irsc_d->sec_cfg_fp = fopen(irsc_d->sec_cfg_f_name, "r");
   if(irsc_d->sec_cfg_fp == NULL) {
      IRSC_ERR("Failed to open file:%s\n", irsc_d->sec_cfg_f_name);
      err = IRSC_INVALID_FILE;
      goto bail;
   }

   /* Figure out the number of entries */
   err = file_parser(irsc_d->sec_cfg_fp, NULL,irsc_d->fmt.delimeter,
          &num_entries);

   if(err == IRSC_NO_ERR) {
      sec_args = (struct config_security_rules_args *)
             malloc(sizeof(*sec_args) +
             num_entries * sizeof(struct security_rule_info));

      if(!sec_args) {
         err =  IRSC_NO_MEM;
         fclose(irsc_d->sec_cfg_fp);
         goto bail;
      }

      sec_args->num_entries = num_entries;
      /* Reposition the file pointer to the begining */
      fseek(irsc_d->sec_cfg_fp,0,SEEK_SET);
      err = file_parser(irsc_d->sec_cfg_fp, sec_args,
             irsc_d->fmt.delimeter, &n);
      if(err == IRSC_NO_ERR)
         irsc_d->sec_info =  sec_args;
   }

   fclose(irsc_d->sec_cfg_fp);

bail:
   return err;

}


/**
 * irsc_feed_config - Feed the security configuration to the IPC Router.
 * @irsc_obj: IRSC object.
 * @Return: Returns the error values specified in enum irsc_tool_err.
 */
static enum irsc_tool_err irsc_feed_config(void *irsc_obj)
{
   int fd;
   uint32_t i, j;
   enum irsc_tool_err err =  IRSC_NO_ERR;
   struct config_sec_rules_args *arg;
   struct irsc_tool *irsc_d;

   if(!irsc_obj)
      return IRSC_INVALID_ARG;

   irsc_d = (struct irsc_tool *)irsc_obj;
   IRSC_DEBUG("Feed the sec config info to the router\n");

   fd = socket(AF_MSM_IPC, SOCK_DGRAM, 0);

   if (!irsc_d->sec_info || !irsc_d->sec_info->num_entries) {
       arg = calloc(1, (sizeof(*arg) + 1 * sizeof(uint32_t)));
       if (!arg) {
           IRSC_ERR("Calloc failure, Config feeding error\n");
           close(fd);
           return IRSC_NO_MEM;
       }
       if (ioctl(fd, IPC_ROUTER_IOCTL_CONFIG_SEC_RULES, arg) < 0) {
           IRSC_DEBUG("Absent/Invalid config,Default rules apply\n");
       }
       free(arg);
       close(fd);
       return IRSC_INVALID_FILE;
   }

   IRSC_DEBUG("Num of entries:%d\n", irsc_d->sec_info->num_entries);

   for( i = 0; i < irsc_d->sec_info->num_entries; i++) {
      if(!irsc_d->sec_info->info[i].num_group_info)
         continue;


      arg = malloc(sizeof(*arg) +
            irsc_d->sec_info->info[i].num_group_info *
            sizeof(uint32_t));
      if (!arg) {
          IRSC_ERR("Malloc failure, couldn't feed entry:%d\n", i);
          close(fd);
          return IRSC_NO_MEM;
      }

      arg->service_id = irsc_d->sec_info->info[i].service_id;
      arg->instance_id = irsc_d->sec_info->info[i].instance_id;
      arg->num_group_info = irsc_d->sec_info->info[i].num_group_info;
      for( j = 0; j < irsc_d->sec_info->info[i].num_group_info;
                         j++) {
          arg->group_id[j] = irsc_d->sec_info->info[i].group_id[j];
      }
      if (ioctl(fd, IPC_ROUTER_IOCTL_CONFIG_SEC_RULES, arg) < 0) {

         IRSC_ERR("%s: Security config ioctl failed\n", __func__);
         free(arg);
         err = IRSC_IOCTL_ERR;
         break;
      }
      free(arg);
   }
   close(fd);

   return err;
}

/**
 * create_irsc_tool - Create the IRSC object.
 * @name: Security configuration file name.
 * @delimiter: Character used to separate the entries on a single line.
 * @Return: IRSC object on success, NULL otherwise.
 */
static  struct irsc_tool*  create_irsc_tool(char *name, char delimeter)
{
   struct irsc_tool* irsc_d;
   size_t name_len = 0;
   irsc_d =  calloc(1, sizeof(struct irsc_tool));

   if(irsc_d) {
      if(name) {
         name_len = strlcpy(irsc_d->sec_cfg_f_name,name,
                            sizeof(irsc_d->sec_cfg_f_name));
      }
      irsc_d->file_name_len = name_len;
      irsc_d->fmt.delimeter = delimeter;
      irsc_d->parser = irsc_parser;
      irsc_d->feed_sec_config = irsc_feed_config;
      IRSC_DEBUG("irsc tool created:%p\n", irsc_d);
   } else {
       IRSC_ERR("Calloc failure, Tool creation error\n");
   }

   return irsc_d;
}

/**
 * delete_irsc_tool - Delete the IRSC  object.
 *
 * @d: IRSC object.
 */
static void delete_irsc_tool(struct irsc_tool* d)
{
   if(!d)
      return;
   free(d->sec_info);
   free(d);
}

int main(int argc, char *argv[])
{

   struct irsc_tool* d = create_irsc_tool(argv[1],':');
   IRSC_DEBUG("Starting irsc tool\n");

   if(d) {
      d->parser(d);
      /* Feed the info to router */
      d->feed_sec_config(d);
   }

   delete_irsc_tool(d);
   return 0;
}
