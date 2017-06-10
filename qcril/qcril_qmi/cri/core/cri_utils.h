/***************************************************************************************************
    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#ifndef CRI_UTILS
#define CRI_UTILS

#include "cri_core.h"

#define CRI_STORE_MAND_TLV(placeholder, value) {\
                            memcpy(&placeholder, &value, sizeof(placeholder)); }\

#define CRI_STORE_OPT_TLV(placeholder, value) {\
                          if(value ## _valid)\
                          {\
                                memcpy(&placeholder, &value, sizeof(placeholder));\
                                placeholder ## _valid = TRUE;\
                          } }\

// returns true if the memory is same.
#define CRI_MEMORY_COMPARISION(var1,var2,len) memcmp(var1,var2,len) ? FALSE:TRUE


















#endif
