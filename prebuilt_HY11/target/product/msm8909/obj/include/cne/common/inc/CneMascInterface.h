#ifndef CNE_MASC_INTERFACE_H
#define CNE_MASC_INTERFACE_H

/*=============================================================================
               Copyright (c) 2014 Qualcomm Technologies, Inc.
               All Rights Reserved.
               Qualcomm Technologies Confidential and Proprietary
=============================================================================*/

class CneAtp;
class ICneMasc;
class ICneMascReportMgr;
class AtpQmiClientStub;

class CneMascInterface
{
  public:
    CneMascInterface(){}
    virtual ~CneMascInterface(){}
    virtual void createMasc(CneCom &com, CneTimer &timer)=0;
    virtual ICneMasc* getCneMasc()=0;
    virtual ICneMascReportMgr* getCneMascReportMgr()=0;
};
#endif
