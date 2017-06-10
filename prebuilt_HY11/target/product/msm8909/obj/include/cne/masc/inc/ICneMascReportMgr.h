#ifndef ICNE_MASC_REPORT_MGR_H
#define ICNE_MASC_REPORT_MGR_H

/*==============================================================================
  FILE:         ICneMascReportMgr.h

  OVERVIEW:     Responsible for report managment, sending reports, receiving
                ACK/NAKs etc..

  DEPENDENCIES:

                Copyright (c) 2014 Qualcomm Technologies, Inc.
                All Rights Reserved.
                Qualcomm Technologies Confidential and Proprietary
==============================================================================*/


/*------------------------------------------------------------------------------
 * Include Files
 * ---------------------------------------------------------------------------*/
#include "CneCom.h"
#include "CneQmi.h"
#include "SocketWrapperClient.h"

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/
typedef struct CneAtpFilterReport_s
{
  int fd;
  atp_send_filter_report_req_msg_v01 report;
} CneAtpFilterReport_t;

typedef struct CneAtpNestedFilterInfo_s
{
  int fd;
  CneAtpFilter_t filter;
} CneAtpNestedFilterInfo_t;

typedef enum CneAtpEvent_e
{
  CNE_MASC_IPC_RSP_EVENT
} CneAtpEvent_t;

typedef struct CneAtpEventData_s
{
  int fd;
  SwimNimsRetCodeType_t retCode;
} CneAtpEventData_t;

typedef atp_send_filter_report_ind_msg_v01 CneAtpFilterReportRsp_t;
/* report id and report map */
typedef multimap<uint32_t,CneAtpFilterReport_t> CneAtpFilterReportsMap_t;
/* fd and report id map */
typedef multimap<int, uint32_t> CneAtpReportIdFdMap_t;
/* cookie and nested filter info */
typedef map<int, CneAtpNestedFilterInfo_t> CneAtpNestedFiltersMap_t;

/*------------------------------------------------------------------------------
 * CLASS         CneReportMgr
 *
 * DESCRIPTION  Responsible for report managment, sending reports, receiving
                ACK/NAKs etc..
 *----------------------------------------------------------------------------*/
class ICneMascReportMgr : public EventDispatcher< CneAtpEvent_t >
{

public:
  ICneMascReportMgr(){}
  virtual ~ICneMascReportMgr(){}
  virtual CneRetType reportFilter(int fd, CneAtpFilter_t& filter) = 0;
  virtual CneRetType removeFilters(int fd) = 0;
};

#endif /* define ICNE_MASC_REPORT_MGR_H */
