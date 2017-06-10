/******************************************************************************
  @file    qmi_csi_target.c
  @brief   The QMI common service interface target specific module

  ---------------------------------------------------------------------------
  Copyright (c) 2012-2014 QUALCOMM Technologies, Inc.
  All Rights Reserved. QUALCOMM Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define QMI_FW_CONF_FILE "/etc/qmi_fw.conf"
#define MAX_LINE_LENGTH 80
#define QMI_CSI_DBG_CONF_STR "QMI_CSI_DEBUG_LEVEL="

#ifdef QMI_FW_SYSLOG
#define DEFAULT_DBG_LEVEL 4
#else
#define DEFAULT_DBG_LEVEL 5
#endif

unsigned int qmi_csi_debug_level = DEFAULT_DBG_LEVEL;

void qmi_csi_debug_init()
{
  char line[MAX_LINE_LENGTH];
  char debug_level[2];
  FILE *fp;

  fp = fopen(QMI_FW_CONF_FILE, "r");
  if(!fp)
    return;

  while(fgets(line, MAX_LINE_LENGTH, fp))
  {
    if(!strncmp(line, QMI_CSI_DBG_CONF_STR, strlen(QMI_CSI_DBG_CONF_STR)))
    {
      debug_level[0] = line[strlen(QMI_CSI_DBG_CONF_STR)];
      debug_level[1] = '\0';
      if(isdigit(debug_level[0]))
      {
        qmi_csi_debug_level = atoi(debug_level);
        break;
      }
    }
  }
  fclose(fp);
}
