#ifndef CNE_PARCEL_H
#define CNE_PARCEL_H

/*==============================================================================
  FILE:         CneParcel.h

  OVERVIEW:     Parcel CNE messages

  DEPENDENCIES: android::parcel

                Copyright (c) 2011-2014 Qualcomm Technologies, Inc.
                All Rights Reserved.
                Qualcomm Technologies Confidential and Proprietary
==============================================================================*/


/*==============================================================================
  EDIT HISTORY FOR MODULE

  when        who     what, where, why
  ----------  ---     ----------------------------------------------------------
  2011-07-27  jnb     First revision.
==============================================================================*/


/*------------------------------------------------------------------------------
 * Include Files
 * ---------------------------------------------------------------------------*/

#include <binder/Parcel.h>
#include <utils/String16.h>
#include <string>
#include <vector>
#include "CneDefs.h"

/*------------------------------------------------------------------------------
 * CLASS         CneParcel
 *
 * DESCRIPTION   Parcel and unparcel data for CnE
 *----------------------------------------------------------------------------*/
class CneParcel {

public:

  static void writeStringToParcel(android::Parcel &p, const char *s);

  /*----------------------------------------------------------------------------
   * FUNCTION      parcel
   *
   * DESCRIPTION   parcel a std::vector<int>
   *               params:
   *                 data - the data to be parceled
   *                 p - an empty parcel to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  p will contained the parceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  static void parcel(std::vector<int> const& data, android::Parcel &p);

  /*----------------------------------------------------------------------------
   * FUNCTION      parcel
   *
   * DESCRIPTION   parcel a std::string
   *               params:
   *                 data - the data to be parceled
   *                 p - an empty parcel to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  p will contained the parceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  static void parcel(std::string const& data, android::Parcel &p);

  /*----------------------------------------------------------------------------
   * FUNCTION      parcel
   *
   * DESCRIPTION   parcel a CneVendorType
   *               params:
   *                 data - the data to be parceled
   *                 p - an empty parcel to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  p will contained the parceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  static void parcel(CneVendorType const& data, android::Parcel &p);

  /*----------------------------------------------------------------------------
   * FUNCTION      parcel
   *
   * DESCRIPTION   parcel a CneRatInfoType
   *               params:
   *                 data - the data to be parceled
   *                 p - an empty parcel to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  p will contained the parceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  static void parcel(CneRatInfoType const& data, android::Parcel &p);

  /*----------------------------------------------------------------------------
   * FUNCTION      parcel
   *
   * DESCRIPTION   parcel a CneIcdStartMsgType
   *               params:
   *                 data - the data to be parceled
   *                 p - an empty parcel to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  p will contained the parceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  static void parcel(CneIcdStartMsgType const& data, android::Parcel &p);

  /*----------------------------------------------------------------------------
   * FUNCTION      parcel
   *
   * DESCRIPTION   parcel a cne_get_compatible_nws_evt_rsp_data_type
   *               params:
   *                 data - the data to be parceled
   *                 p - an empty parcel to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  p will contained the parceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  static void parcel(cne_get_compatible_nws_evt_rsp_data_type const& data, android::Parcel &p);

  /*----------------------------------------------------------------------------
   * FUNCTION      parcel
   *
   * DESCRIPTION   parcel a cne_pref_rat_avail_evt_data_type
   *               params:
   *                 data - the data to be parceled
   *                 p - an empty parcel to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  p will contained the parceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  static void parcel(cne_pref_rat_avail_evt_data_type const& data, android::Parcel &p);

  /*----------------------------------------------------------------------------
   * FUNCTION      parcel
   *
   * DESCRIPTION   parcel a cne_reg_role_rsp_evt_data_type
   *               params:
   *                 data - the data to be parceled
   *                 p - an empty parcel to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  p will contained the parceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  static void parcel(cne_reg_role_rsp_evt_data_type const& data, android::Parcel &p);

  /*----------------------------------------------------------------------------
   * FUNCTION      parcel
   *
   * DESCRIPTION   parcel a CneNsrmBlockedUidType
   *               params:
   *                 data - the data to be parceled
   *                 p - an empty parcel to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  p will contained the parceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  static void parcel(CneNsrmBlockedUidType const& data, android::Parcel &p);

  /*----------------------------------------------------------------------------
   * FUNCTION      parcel
   *
   * DESCRIPTION   parcel a CneAtpParentAppInfoMsg_t
   *               params:
   *                 data - the data to be parceled
   *                 p - an empty parcel to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  p will contained the parceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  static void parcel(CneAtpParentAppInfoMsg_t const& data, android::Parcel &p);

  /*----------------------------------------------------------------------------
   * FUNCTION      parcel
   *
   * DESCRIPTION   parcel a CnePolicyUpdateRespType
   *               params:
   *                 data - the data to be parceled
   *                 p - an empty parcel to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  p will contained the parceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  static void parcel(CnePolicyUpdateRespType const& data, android::Parcel &p);

  /*----------------------------------------------------------------------------
   * FUNCTION      parcel
   *
   * DESCRIPTION   parcel a CneDisallowedAPType
   *               params:
   *                 data - the data to be parceled
   *                 p - an empty parcel to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  p will contained the parceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  static void parcel(CneDisallowedAPType const& data, android::Parcel &p);

  /*----------------------------------------------------------------------------
   * FUNCTION      parcel
   *
   * DESCRIPTION   parcel a CneBQEActiveProbeMsgType
   *               params:
   *                 data - the data to be parceled
   *                 p - an empty parcel to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  p will contained the parceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  static void parcel(CneBQEActiveProbeMsgType const& data, android::Parcel &p);

  /*----------------------------------------------------------------------------
   * FUNCTION      parcel
   *
   * DESCRIPTION   parcel a CneAppInfoMsgDataType
   *               params:
   *                 data - the data to be parceled
   *                 p - an empty parcel to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  p will contained the parceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  static void parcel(CneAppInfoMsgDataType const& data, android::Parcel &p);

  /*----------------------------------------------------------------------------
   * FUNCTION      parcel
   *
   * DESCRIPTION   parcel a cne_set_default_route_req_data_type
   *               params:
   *                 data - the data to be parceled
   *                 p - an empty parcel to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  p will contained the parceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  static void parcel(cne_set_default_route_req_data_type const& data, android::Parcel &p);

/*----------------------------------------------------------------------------
 * FUNCTION      parcel
 *
 * DESCRIPTION   parcel a CneDnsPriorityType
 *               params:
 *                 data - the data to be parceled
 *                 p - an empty parcel to be populated
 *
 * DEPENDENCIES  parcel
 *
 * RETURN VALUE  p will contained the parceled data
 *
 * SIDE EFFECTS  none
 *--------------------------------------------------------------------------*/
static void parcel(CneDnsPriorityType const& data, android::Parcel &p);

  /*----------------------------------------------------------------------------
   * FUNCTION      parcel
   *
   * DESCRIPTION   parcel a CneBQEPostParamsMsgType
   *               params:
   *                 data - the data to be parceled
   *                 p - an empty parcel to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  p will contained the parceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  static void parcel(CneBQEPostParamsMsgType const& data, android::Parcel &p);

  /*----------------------------------------------------------------------------
  * FUNCTION      parcel
  *
  * DESCRIPTION   parcel a CneFeatureInfoType
  *               params:
  *                 data - the data to be parceled
  *                 p - an empty parcel to be populated
  *
  * DEPENDENCIES  parcel
  *
  * RETURN VALUE  p will contained the parceled data
  *
  * SIDE EFFECTS  none
  *--------------------------------------------------------------------------*/
  static void parcel(CneFeatureInfoType const& data, android::Parcel &p);

 /*----------------------------------------------------------------------------
  * FUNCTION      parcel
  *
  * DESCRIPTION   parcel a CneFeatureInfoType
  *               params:
  *                 data - the data to be parceled
  *                 p - an empty parcel to be populated
  *
  * DEPENDENCIES  parcel
  *
  * RETURN VALUE  p will contained the parceled data
  *
  * SIDE EFFECTS  none
  *--------------------------------------------------------------------------*/
  static void parcel(CneFeatureRespType const& data, android::Parcel &p);

  /*----------------------------------------------------------------------------
   * FUNCTION      unparcel
   *
   * DESCRIPTION   unparcel a std::vector<int>
   *               params:
   *                 p - the parcel to be unparceled
   *                 data - the data structure to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  data will contained the unpraceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  static void unparcel(android::Parcel &p, std::vector<int> &data);

  /*----------------------------------------------------------------------------
   * FUNCTION      unparcel
   *
   * DESCRIPTION   unparcel a std::string
   *               params:
   *                 p - the parcel to be unparceled
   *                 data - the data structure to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  data will contained the unpraceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  static void unparcel(android::Parcel &p, std::string &data);

  /*----------------------------------------------------------------------------
   * FUNCTION      unparcel
   *
   * DESCRIPTION   unparcel a CneTetheringUpstreamInfo
   *               params:
   *                 p - the parcel to be unparceled
   *                 data - the data structure to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  data will contained the unpraceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  static void unparcel(android::Parcel &p, CneTetheringUpstreamInfo &data);

  /*----------------------------------------------------------------------------
   * FUNCTION      unparcel
   *
   * DESCRIPTION   unparcel a CneWlanInfoType
   *               params:
   *                 p - the parcel to be unparceled
   *                 data - the data structure to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  data will contained the unpraceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  static void unparcel(android::Parcel &p, CneWlanInfoType &data);

  /*----------------------------------------------------------------------------
   * FUNCTION      unparcel
   *
   * DESCRIPTION   unparcel a CneBrowserInfoMsgDataType
   *               params:
   *                 p - the parcel to be unparceled
   *                 data - the data structure to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  data will contained the unpraceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  static void unparcel(android::Parcel &p, CneBrowserAppType &data);

  /*----------------------------------------------------------------------------
   * FUNCTION      unparcel
   *
   * DESCRIPTION   unparcel a CneWwanInfoType
   *               params:
   *                 p - the parcel to be unparceled
   *                 data - the data structure to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  data will contained the unpraceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  static void unparcel(android::Parcel &p, CneWwanInfoType &data);

  /*----------------------------------------------------------------------------
   * FUNCTION      unparcel
   *
   * DESCRIPTION   unparcel a CneWlanScanResultsType
   *               params:
   *                 p - the parcel to be unparceled
   *                 data - the data structure to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  data will contained the unpraceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  static void unparcel(android::Parcel &p, CneWlanScanResultsType &data);

  /*----------------------------------------------------------------------------
   * FUNCTION      unparcel
   *
   * DESCRIPTION   unparcel a CneRatStatusType
   *               params:
   *                 p - the parcel to be unparceled
   *                 data - the data structure to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  data will contained the unpraceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  static void unparcel(android::Parcel &p, CneRatStatusType &data);

  /*----------------------------------------------------------------------------
   * FUNCTION      unparcel
   *
   * DESCRIPTION   unparcel a CneVendorType
   *               params:
   *                 p - the parcel to be unparceled
   *                 data - the data structure to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  data will contained the unpraceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  static void unparcel(android::Parcel &p, CneVendorType &data);

  /*----------------------------------------------------------------------------
   * FUNCTION      unparcel
   *
   * DESCRIPTION   unparcel a CneIcdResultCmdType
   *               params:
   *                 p - the parcel to be unparceled
   *                 data - the data structure to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  data will contained the unpraceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
 static void unparcel(android::Parcel &p, CneIcdResultCmdType &data);

  /*----------------------------------------------------------------------------
   * FUNCTION      unparcel
   *
   * DESCRIPTION   unparcel a CneIcdResultCmdType
   *               params:
   *                 p - the parcel to be unparceled
   *                 data - the data structure to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  data will contained the unpraceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
 static void unparcel(android::Parcel &p, CneIcdHttpResultCmdType &data);

  /*----------------------------------------------------------------------------
   * FUNCTION      unparcel
   *
   * DESCRIPTION   unparcel a CneAppInfoMsgDataType
   *               params:
   *                 p - the parcel to be unparceled
   *                 data - the data structure to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  data will contained the unpraceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
 static void unparcel(android::Parcel &p, CneAppInfoMsgDataType &data);

 /*----------------------------------------------------------------------------
   * FUNCTION      unparcel
   *
   * DESCRIPTION   unparcel a CneAtpParentAppInfoMsg_t
   *               params:
   *                 p - the parcel to be unparceled
   *                 data - the data structure to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  data will contained the unpraceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
 static void unparcel(android::Parcel &p, CneAtpParentAppInfoMsg_t &data);

  /*----------------------------------------------------------------------------
   * FUNCTION      unparcel
   *
   * DESCRIPTION   unparcel a CneJrttResultCmdType
   *               params:
   *                 p - the parcel to be unparceled
   *                 data - the data structure to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  data will contain the unparceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  static void unparcel(android::Parcel &p, CneJrttResultCmdType &data);

  /*----------------------------------------------------------------------------
   * FUNCTION      unparcel
   *
   * DESCRIPTION   unparcel a CneFeatureInfoType
   *               params:
   *                 p - the parcel to be unparceled
   *                 data - the data structure to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  data will contained the unpraceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
 static void unparcel(android::Parcel &p, CneFeatureInfoType &data);

  /*----------------------------------------------------------------------------
   * FUNCTION      unparcel
   *
   * DESCRIPTION   unparcel a CneFamType
   *               params:
   *                 p - the parcel to be unparceled
   *                 data - the data structure to be populated
   *
   * DEPENDENCIES  parcel
   *
   * RETURN VALUE  data will contain the unpraceled data
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
 static void unparcel(android::Parcel &p, CneWlanFamType &data);

private:
  // convert an android::String16 to a char*, char* must be freed
  static void from16to8(android::String16 const &str16, std::string &str8);
  static void from16to8(android::String16 const& str16, char* str8, int str8len);
};

#endif /* define CNE_PARCEL_H */
