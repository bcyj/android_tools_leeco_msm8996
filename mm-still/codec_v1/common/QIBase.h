/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QIBASE_H__
#define __QIBASE_H__

/*===========================================================================
 * Class: QIBase
 *
 * Description: This class represents the base class of all objects which
 *              are heap allocated or used by the utility classes
 *
 *
 * Notes: none
 *==========================================================================*/
class QIBase {

public:

  /** QIBase:
   *
   *  base class contructor
   **/
  QIBase();

  /** ~QIBase:
   *
   *  base class destructor
   **/
  virtual ~QIBase();

private:

  /** mType:
   *
   * type of the object incase of dynamic cast
   **/
  int mType;
};

#endif //__QIBASE_H__
