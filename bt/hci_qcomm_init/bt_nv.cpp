/*============================================================================
  Copyright (c) 2010 - 2012, 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

  FILE:         bt_nv.cpp

  OVERVIEW:     This file implements functions to read and write the NV items into
                the /persistent file system.
                Through this code you can read and write the following format data.
                *** data format ***
                nvID    WriteProtect    Size    Data
                X       X(1 or 0)       X       X X X X X X

*==============================================================================
                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order. Please
  use ISO format for dates.

when        who  what, where, why
----------  ---  -----------------------------------------------------------
2010-02-15   ss  Intial version
2011-09-28  rrr  Moved the implementation to CPP, for having BD address being
                 programmed twice if previous BD address was random generated.
2012-02-16  rrr  Read/Write access to NV config file (i.e. /persist/bt_nv.bin)
                 is limited to "bluetooth" user only. And making bt_nv.bin as
                 hidden.
2012-02-22  rrr  Moved/modified macros to header file for reference across
                 source files.
============================================================================*/


/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
/* Necessary includes */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "comdef.h"
#include "bt_nv.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
  * -------------------------------------------------------------------------*/
#define NV_REF_CLOCK_SIZE     1
#define NV_REF_CLOCK_T_SIZE   1
#define NV_CMD_HDR_SIZE       3

#define SIZE_OF_BYTE          sizeof(char)
#define HC_VS_MAX_CMD_EVENT   260
#define NVM_PAYLOAD_MAXLENGTH 260
#define MAX_PAYLOAD_SIZE      NV_BD_ADDR_SIZE
#ifdef BTNV_DEBUG
#define BTNV_INFO(args...)    fprintf(stdout, ## args);
#define BTNV_ERR(args...)     fprintf(stderr, ## args);
#define BTNV_OS_ERR           fprintf(stderr, "err =%d msg = %s\n", errno, strerror(errno))
#else
#define BTNV_INFO(args...)
#define BTNV_ERR(args...)
#define BTNV_OS_ERR
#endif

typedef enum {
  NV_WRITE_ONCE_ENABLE = 1,
  NV_WRITE_ONCE_DISABLE = 0
}nv_persist_write_type;

typedef struct {
  nv_persist_items_enum_type item;     /* Item to access */
  nv_persist_write_type writeprotect;
  uint8 nSize;
  unsigned char pCmdBuffer[MAX_PAYLOAD_SIZE];    /* Pointer to read or write data */
} nv_persist_params;

typedef struct _nv_items_tag {
  nv_persist_items_enum_type type;
  uint8 size;
  nv_persist_write_type multi_time_wr;
}nv_items;

nv_items nvItems[3] =
{
  {NV_BD_ADDR_I,                 NV_BD_ADDR_SIZE,     NV_WRITE_ONCE_ENABLE},
  {NV_BT_SOC_REFCLOCK_TYPE_I,    NV_REF_CLOCK_SIZE,   NV_WRITE_ONCE_DISABLE},
  {NV_BT_SOC_CLK_SHARING_TYPE_I, NV_REF_CLOCK_T_SIZE, NV_WRITE_ONCE_DISABLE},
};
/*==============================================================
FUNCTION:  bt_nv_init
==============================================================*/
/**
* Intialize the required variables and check the path exists mentioned for writing the binary.
* TODO: Need to add verification of binary?
*
*
* @return  int - negative number on failure.
*
*/
int bt_nv_init()
{
  return 0;
}

/*==============================================================
FUNCTION:  bt_nv_deinit
==============================================================*/
/**
* DeIntialize the required variables.
*
*
*
* @return  int - negative number on failure.
*
*/
int bt_nv_deinit()
{
  return 0;
}

/*==============================================================
FUNCTION:  bt_nv_write
==============================================================*/
/**
* Write the nparams number of items from nv_params structure to the persist NV file
* (PERSISTENCE_PATH/BT_NV_FILE_NAME)
*
*
* @return  int - negative number on failure.
*
*/
int bt_nv_write
(
  nv_persist_params *params,
  uint8 nparams
)
{
  int nFd;
  struct passwd *pwd;
  struct group  *grp;
  unsigned char cmd[HC_VS_MAX_CMD_EVENT];
  int nwrite = 0, i = 0,j = 0;
  char filename[NAME_MAX];

  snprintf(filename, NAME_MAX, "%s/%s",PERSISTENCE_PATH,BT_NV_FILE_NAME );
  BTNV_INFO("BT-NV-WRITE: Opening file '%s' for writing\n", filename);

  /* R/W access to user only for the file created below */
  umask(066);

  /* Open the firmware file*/
  nFd = open(filename, O_CREAT | O_RDWR, 0666);
  if(nFd < 0)
  {
     BTNV_ERR("BT-NV-WRITE: Failed to Open/create a '%s' file\n",filename);
     BTNV_OS_ERR;
     return -1;
  }

  /* Retrieve user id corresponding to "bluetooth" user */
  pwd = getpwnam("bluetooth");
  if (!pwd)
  {
      BTNV_ERR("BT-NV-WRITE: user not found in /etc/passwd\n");
      BTNV_OS_ERR;
      nwrite = -1;
      goto out;
  }

  /* Retrieve group id corresponding to "bluetooth" user */
  grp = getgrnam("bluetooth");
  if (!grp)
  {
      BTNV_ERR("BT-NV-WRITE: group not found in /etc/group\n");
      BTNV_OS_ERR;
      nwrite = -1;
      goto out;
  }

  /* Set user and group ownership as "bluetooth"
   * for firmware file created/opened above
   */
  if (fchown(nFd, pwd->pw_uid, grp->gr_gid) < 0)
  {
      BTNV_ERR("BT-NV-WRITE: fchown failed \n");
      BTNV_OS_ERR;
      nwrite = -1;
      goto out;
  }

  for(i = 0; i < nparams; i++)
  {
    if((void*)(&params[i]) == NULL)
    {
      BTNV_ERR("BT-NV-WRITE: params[%d]) is NULL \n", i);
      BTNV_OS_ERR;
      nwrite = -1;
      goto out;
    }
    else
    {
      BTNV_INFO("BT-NV-WRITE: params[%d] is valid\n",i);
    }
    cmd[0] = (uint8)params[i].item;
    cmd[1] = (uint8)params[i].writeprotect;
    cmd[2] = (uint8)params[i].nSize;

    BTNV_INFO("BT-NV-WRITE:****** params[i].writeprotect_0ld = %d new  = %d\n",
                    (uint8)params[i].writeprotect,(uint8)cmd[1]);

    memcpy(&cmd[NV_CMD_HDR_SIZE], params[i].pCmdBuffer, params[i].nSize);

    if((nwrite = write(nFd, cmd, (NV_CMD_HDR_SIZE+params[i].nSize))) <= 0)
    {
      BTNV_ERR("BT-NV-WRITE: Failed to write to binary file\n");
      BTNV_OS_ERR;
      nwrite = -1;
      goto out;
    }
    else
    {
      BTNV_INFO("BT-NV-WRITE: Written %d data",nwrite);
      for(j = 0; j < NV_CMD_HDR_SIZE + params[i].nSize;j++)
        BTNV_INFO("%02x  ",cmd[j]);
      BTNV_INFO("\n");
    }
  }
out:
  close(nFd);
  BTNV_INFO("BT-NV-WRITE: closed file pointer\n");
  return nwrite;
}
/*==============================================================
FUNCTION:  bt_nv_read
==============================================================*/
/**
* Read the item  from (PERSISTENCE_PATH/BT_NV_FILE_NAME) as
* mentioned in the  nv_params and return the value through
* the argument pointer.
*
* @return  int - negative number on failure.
*
*/
int bt_nv_read
(
 nv_persist_params *params_read
)
{
  int nFd;
  int nRead = 0;
  unsigned char payload[NVM_PAYLOAD_MAXLENGTH];
  unsigned char header[NV_CMD_HDR_SIZE];
  int nNvItem = 0, i = 0;
  char filename[NAME_MAX];

  snprintf(filename, NAME_MAX, "%s/%s",PERSISTENCE_PATH,BT_NV_FILE_NAME);
  BTNV_INFO("BT-NV-READ: Opening file '%s' for reading\n",filename);

  /* Open the firmware file */
  nFd = open(filename, O_RDONLY);
  if(nFd < 0)
  {
    BTNV_ERR("BT-NV-READ: Open of firmware file failed\n");
    BTNV_OS_ERR;
    return -1;
  }

  nNvItem = 0;
  while((nRead = read(nFd, header, NV_CMD_HDR_SIZE)) &&
         (nRead == 0 || nRead == NV_CMD_HDR_SIZE))
  {
    /*Third byte gives the length*/
    params_read[nNvItem].item = (nv_persist_items_enum_type)header[0];
    params_read[nNvItem].writeprotect = (nv_persist_write_type)header[1];
    params_read[nNvItem].nSize = header[2];

    BTNV_INFO("BT-NV-READ: Header = %02x %02x %02x\n",params_read[nNvItem].item,
                  params_read[nNvItem].writeprotect,
                  params_read[nNvItem].nSize);

    if(read(nFd, payload, params_read[nNvItem].nSize) ==
            params_read[nNvItem].nSize)
    {
      BTNV_INFO("BT-NV-READ: NV item %02x value:",params_read[nNvItem].item);
      for(i=0; i < params_read[nNvItem].nSize;i++)
      {
        params_read[nNvItem].pCmdBuffer[i] = payload[i];
        BTNV_INFO("%02x  ",params_read[nNvItem].pCmdBuffer[i]);
      }
      BTNV_INFO("\n");
    }
    else
    {
      BTNV_ERR("BT-NV-READ: fread failed !!! \n");
    }
    nNvItem++;
  }

  BTNV_INFO("BT-NV-READ: Total number of item Read = %d\n",nNvItem);
  close(nFd);
  return nNvItem;
}

/*==============================================================
FUNCTION:  bt_nv_cmd
==============================================================*/
/**
*  This function is to do the read and write the NV item.
*
*  @see  nv_persist_func_enum_type -> For Read or Write (nv_persist_func_enum_type)
*    nv_persist_items_enum_type  -> NV ID number (refer the nv_persist_items_enum_type enum)
*    nv_persist_item_type    -> Structure which contains the variables to read or write.
*    bIsRandom -> Used only in case of BD address NV item being written.
*      bIsRandom = 1 implies that BD address programmed is random generated
*      bIsRandom = 0 (default) implies it to be user programmed
*
*  @return  Returns the nagative value on failure.
*
*  @sideeffects Undetermined.
*/
int bt_nv_cmd(nv_persist_func_enum_type nvReadWriteFunc,  nv_persist_items_enum_type nvitem,
                nv_persist_item_type *my_nv_item, int bIsRandom)
{
  nv_persist_params params_temp[NV_BT_ITEM_MAX];
  nv_persist_stat_enum_type status = NV_SUCCESS;
  int item = -1, check = -1;
  int i =0,numitems =0;

  if((numitems = bt_nv_read(params_temp)) == -1)
  {
    if(nvReadWriteFunc == NV_READ_F)
    {
      BTNV_ERR("BT-NV-CMD: Read from NV persist failed\n");
      BTNV_OS_ERR;
      return NV_FAILURE;
    }
    numitems = 0;
  }

  BTNV_INFO("BT-NV-CMD: numitems = %d\n",numitems);

  if(nvitem <= NV_BT_ITEM_MIN || nvitem >= NV_BT_ITEM_MAX)
  {
    BTNV_ERR("BT-NV-CMD: Invalid NV item %d\n",nvitem);
    BTNV_OS_ERR;
    return NV_FAILURE;
  }

  for(i = 0; i <numitems;i++)
  {
    if( nvitem == params_temp[i].item)
    {
      item = i;
      BTNV_INFO("BT-NV-CMD: Found NV item '%d' at '%d'\n",nvitem,item);
      break;
    }
  }

  switch(nvReadWriteFunc)
  {
    case NV_READ_F: //read from file
      BTNV_INFO("BT-NV-CMD: Read Command Execution\n");
      if(item == -1)
      {
        BTNV_ERR("BT-NV-CMD: NV item %d not found\n",nvitem);
        BTNV_OS_ERR;
        return NV_FAILURE;
      }
      if(nvitem == NV_BD_ADDR_I)
      {
        memcpy((*my_nv_item).bd_addr, params_temp[item].pCmdBuffer, NV_BD_ADDR_SIZE );
        BTNV_INFO("BT-NV-CMD: NV_READ_F: BD Addr: %02x %02x %02x %02x %02x %02x\n",
            params_temp[item].pCmdBuffer[0],params_temp[item].pCmdBuffer[1],
            params_temp[item].pCmdBuffer[2],params_temp[item].pCmdBuffer[3],
            params_temp[item].pCmdBuffer[4],params_temp[item].pCmdBuffer[5]);
      }
      else if(nvitem == NV_BT_SOC_REFCLOCK_TYPE_I)
      {
        (*my_nv_item).bt_soc_refclock_type = params_temp[item].pCmdBuffer[0];
        BTNV_INFO("BT-NV-CMD: NV_READ_F: REFCLOCK_TYPE: %02x (0 -> 32MHz, 1-> 19P2MHz)\n",
                params_temp[item].pCmdBuffer[0]);
      }
      else if(nvitem == NV_BT_SOC_CLK_SHARING_TYPE_I)
      {
        (*my_nv_item).bt_soc_clk_sharing_type = params_temp[item].pCmdBuffer[0];
        BTNV_INFO("BT-NV-CMD: NV_READ_F: CLOCK_SHARING_TYPE: %02x (0 -> Disabled, 1-> Enabled)\n",
              params_temp[item].pCmdBuffer[0]);
      }
      break;

    case NV_WRITE_F: //write to file
      BTNV_INFO("BT-NV-CMD: Write Command Execution\n");
      if((item != -1) && params_temp[item].writeprotect)
      {
        BTNV_INFO("BT-NV-CMD: NV item %d can be written only once\n",params_temp[item].item);
        status = NV_READONLY;
      }
      else
      {
        if(item == -1)
        {
          BTNV_INFO("BT-NV-CMD: NV item %d not found\n",nvitem);
          item = numitems;
          numitems++;
        }
        params_temp[item].item = (nv_persist_items_enum_type)nvitem;
        params_temp[item].writeprotect = nvItems[nvitem - 1].multi_time_wr;
        params_temp[item].nSize = (uint8)nvItems[nvitem -1].size;
        if(nvitem == NV_BD_ADDR_I)
        {
            /* Overridden. Since in case of BD address it can be either
               generated or user defined.
               If random generated then BD address to be persistent across target
               reboots but can be reprogrammed once by the user.
               In case of user defined its persistent across target reboots and
               can't be reprogrammed again by the user. */
            params_temp[item].writeprotect = (bIsRandom) ? NV_WRITE_ONCE_DISABLE:
                                                       nvItems[nvitem - 1].multi_time_wr;
            params_temp[item].pCmdBuffer[0] = my_nv_item->bd_addr[0];
            params_temp[item].pCmdBuffer[1] = my_nv_item->bd_addr[1];
            params_temp[item].pCmdBuffer[2] = my_nv_item->bd_addr[2];
            params_temp[item].pCmdBuffer[3] = my_nv_item->bd_addr[3];
            params_temp[item].pCmdBuffer[4] = my_nv_item->bd_addr[4];
            params_temp[item].pCmdBuffer[5] = my_nv_item->bd_addr[5];
            BTNV_INFO("BT-NV-CMD: NV_WRITE_F: BD Addr: %02x %02x %02x %02x %02x %02x\n",
                  params_temp[item].pCmdBuffer[0],params_temp[item].pCmdBuffer[1],
                  params_temp[item].pCmdBuffer[2],params_temp[item].pCmdBuffer[3],
                  params_temp[item].pCmdBuffer[4],params_temp[item].pCmdBuffer[5]);
        }
        else if(nvitem == NV_BT_SOC_REFCLOCK_TYPE_I)
        {
          params_temp[item].pCmdBuffer[0] = (*my_nv_item).bt_soc_refclock_type;
          BTNV_INFO("BT-NV-CMD: NV_WRITE_F: REFCLOCK_TYPE: %02x (0 -> 32MHz, 1-> 19P2MHz)\n",
                   params_temp[item].pCmdBuffer[0]);
        }
        else if(nvitem == NV_BT_SOC_CLK_SHARING_TYPE_I){
          params_temp[item].pCmdBuffer[0] = (*my_nv_item).bt_soc_clk_sharing_type;
          BTNV_INFO("BT-NV-CMD: NV_WRITE_F: CLK_SHARING_TYPE: %02x (0 ->Disable, 1->Enable )\n",
                  params_temp[item].pCmdBuffer[0]);
        }
      }

      BTNV_INFO("BT-NV-CMD:  bt_nv_write numitems: %d.... item = %d\n",numitems,item);
      if(numitems < NV_BT_ITEM_MAX)
      {
        if(bt_nv_write(params_temp, numitems) == -1)
        {
          BTNV_ERR("BT-NV-CMD: Write failed\n");
          BTNV_OS_ERR;
          return NV_FAILURE;
        }
      }
      else
      {
        BTNV_ERR("BT-NV-CMD: numitems exceeds NV_BT_ITEM_MAX\n");
        BTNV_OS_ERR;
        return NV_FAILURE;
      }
      break;

    default:
        status = NV_BADCMD;
        BTNV_INFO("BT-NV-CMD: Oops default cmd option\n");
        break;
  }
  return status;
}
