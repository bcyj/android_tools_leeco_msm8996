/******************************************************************************NS

  @file    sap_authorize.h
  @brief

  DESCRIPTION

******************************************************************************/
/*===========================================================================

  Copyright (c) 2011 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/
#ifndef _SAP_AUTHORIZE_AJ_H_
#define _SAP_AUTHORIZE_AJ_H_

#ifdef __cplusplus
extern "C" {
#endif

int init_alljoyn(int);
int call_authorize(char *addr);
int updateStateChanged(const char* state);
void cleanup_alljoyn();

#ifdef __cplusplus
}
#endif

#endif/*_SAP_AUTHORIZE_AJ_H_*/
