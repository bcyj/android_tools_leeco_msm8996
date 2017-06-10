/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QINODE_H__
#define __QINODE_H__

#include "QIBase.h"

/*===========================================================================
 * Class: QINode
 *
 * Description: This class represents the node for base utilities
 *
 *
 * Notes: none
 *==========================================================================*/
class QINode : public QIBase {

public:

  /** QINode
   *
   *  constructor
   **/
  QINode();

  /** QINode
   *  @aData: data object of base class
   *
   *  constructor
   **/
  QINode(QIBase *aData);

  /** QINode
   *
   *  virtual destructor
   **/
  virtual ~QINode();

  /** Next
   *
   *  returns the next node of the current node
   **/
  inline QINode *Next()
  {
    return mNext;
  }

  /** Data
   *
   *  returns the QIBase object of the node
   **/
  inline QIBase *Data()
  {
    return mData;
  }

  /** setNext
   *  @aNext: pointer to the next node
   *
   *  sets the next node of the current node
   **/
  inline void setNext(QINode *aNext)
  {
    mNext = aNext;
  }

  /** setData
   *  @aData: pointer to the data (QIBase object)
   *
   *  sets the data of the node
   **/
  inline void setData(QIBase *aData)
  {
    mData = aData;
  }

private:

  /** mNext
   *
   *  pointer to the next node
   **/
  QINode *mNext;

  /** mData
   *
   *  pointer to the data
   **/
  QIBase *mData;
};

#endif //__QINODE_H__
