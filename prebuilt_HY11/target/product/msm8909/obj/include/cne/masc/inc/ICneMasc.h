#ifndef I_CNE_MASC_H
#define I_CNE_MASC_H

/*==============================================================================
  FILE:         ICneMasc.h

  OVERVIEW:     This is the main interface for rest of the software modules to
                interact with ATP.

  DEPENDENCIES:

                Copyright (c) 2014 Qualcomm Technologies, Inc.
                All Rights Reserved.
                Qualcomm Technologies Confidential and Proprietary
==============================================================================*/

/*------------------------------------------------------------------------------
 * Include Files
 * ---------------------------------------------------------------------------*/
#include "SocketWrapperClient.h"

class ICneMascReportMgr;

/*------------------------------------------------------------------------------
 * CLASS         ICneMasc
 *
 * DESCRIPTION   This is the main interface for rest of the software modules to
                 interact with ATP.
 *----------------------------------------------------------------------------*/
class ICneMasc {

public:
  ICneMasc() {}
  virtual ~ICneMasc(){}
  virtual CneRetType reportFilter(int fd, CneAtpFilter_t filter) = 0;
  virtual ICneMascReportMgr* getAtpReportMgr() = 0;
};

#endif /* define I_CNE_MASC_H */
