/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#include "QINode.h"

/*===========================================================================
 * Function: QINode
 *
 * Description: QINode default constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QINode::QINode()
{
  mNext = NULL;
  mData = NULL;
}

/*===========================================================================
 * Function: QINode
 *
 * Description: QINode constructor
 *
 * Input parameters:
 *   aData - pointer to the node data
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QINode::QINode(QIBase *aData)
{
  mNext = NULL;
  mData = aData;
}

/*===========================================================================
 * Function: ~QINode
 *
 * Description: QINode destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QINode::~QINode()
{
}

