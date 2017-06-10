#ifndef CNE_UTILS_H
#define CNE_UTILS_H

/**----------------------------------------------------------------------------
  @file CneUtils.h

  This header file provides various utility routines.

-----------------------------------------------------------------------------*/

/*=============================================================================
               Copyright (c) 2009,2010 Qualcomm Technologies, Inc.
               All Rights Reserved.
               Qualcomm Technologies Confidential and Proprietary
=============================================================================*/

/*=============================================================================
  EDIT HISTORY FOR MODULE

  $Header: //depot/asic/sandbox/projects/cne/common/core/inc/CneUtils.h#1 $
  $DateTime: 2009/08/31 14:29:20 $
  $Author: syadagir $
  $Change: 1012856 $

  when        who  what, where, why
  ----------  ---  -------------------------------------------------------
  2009-07-15  ysk  First revision.

============================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include <map>
#include <string>
#include "CneDefs.h"

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Class Definitions
 * -------------------------------------------------------------------------*/

class CneUtils {
public:

    /**
     @brief Will return the Uint16 bit value pointed by the byte pointer
               passed in

     @param  bytePtr: byte pointer from which the next 16 bits will be refered.
     @see    None
     @return  Uint16
    */
  static uint16_t getUint16(const uint8_t* bytePtr);

    /**
     @brief Will return the Uint32 bit value pointed by the byte pointer
               passed in

     @param  bytePtr: byte pointer from which the next 32 bits will be refered.
     @see    None
     @return  Uint32
    */
  static uint32_t getUint32(const uint8_t* bytePtr);

  /**
   * @brief get a string representation of a CnE command
   *
   * @param[in] getCmd cne_cmd_enum_type
   *
   * @return char const*
   */
  static char const* getCneCmdStr(cne_cmd_enum_type getCmd);

  /**
   * @brief get a string representation of a CnE message
   *
   * @param[in] getMsg cne_msg_enum_type
   *
   * @return char const*
   */
  static char const* getCneMsgStr(cne_msg_enum_type getMsg);

  /**
   * @brief get a string representation of a network state
   *
   * @param[in] getState cne_network_state_enum_type
   *
   * @return char const*
   */
  static char const* getCneNwStateStr(cne_network_state_enum_type getState);

  /**
   * @brief get a string representation of an RAT type
   *
   * @param[in] getRat cne_rat_type
   *
   * @return char const*
   */
  static char const* getCneRatStr (cne_rat_type getRat);

  /**
   * @brief get a string representation of a RAT subtype
   *
   * @param[in] getRatSubType cne_rat_subtype
   *
   * @return char const*
   */
  static char const* getCneRatSubTypeStr(cne_rat_subtype getRatSubType);

  /**
   * @brief get a string representation of a CnE return type
   *
   * @param[in] getRetType CneRetType
   *
   * @return char const*
   */
  static char const* getCneRetTypeStr(CneRetType getRetType);

  /**
   * @brief get a string representation of a CnE Event
   *
   * @param[in] event CneEvent
   *
   * @return char const*
   */
  inline static char const* getCneEventStr(CneEvent event) {
    return getCneCmdStr(event);
  }

  // queries the kernel to get the appname belonging to a process
  static bool GetAppName( int pid, std::string &appname );

private:

  static bool isInitNeeded; // if true, init should be called
  static char const* const EMPTY_STRING; // sent when no string representation
                                         // is available

  // typedefs for map pairs
  typedef std::pair<cne_cmd_enum_type, char const* const> cmdPair;
  typedef std::pair<cne_msg_enum_type, char const* const> msgPair;
  typedef std::pair<cne_network_state_enum_type, char const* const> netStatePair;
  typedef std::pair<cne_rat_type, char const* const> ratTypePair;
  typedef std::pair<cne_rat_subtype, char const* const> ratSubtypePair;
  typedef std::pair<CneRetType, char const* const> retTypePair;

  // map enum -> string
  static std::map<cne_cmd_enum_type, char const* const> cmd;
  static std::map<cne_msg_enum_type, char const* const> msg;
  static std::map<cne_network_state_enum_type, char const* const> netState;
  static std::map<cne_rat_type, char const* const> ratType;
  static std::map<cne_rat_subtype, char const* const> ratSubtype;
  static std::map<CneRetType, char const* const> retType;

  // init maps
  static void init();
};

#endif /* CNE_UTILS_H */
