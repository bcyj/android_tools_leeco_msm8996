/******************************************************************************
  @file:  xtra_config_api.h
  @brief:

  DESCRIPTION

  public api of xtra configuration

  -----------------------------------------------------------------------------
  Copyright (c) 2013 Qualcomm Technology Incoporated.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------
 ******************************************************************************/


#ifndef XTRA_CONFIG_API_H_
#define XTRA_CONFIG_API_H_

struct Xtra_System_Config;
//config function
void xtra_set_config(struct Xtra_System_Config *pConfig);
void Xtra_ConfigSetDefaults();

#endif

