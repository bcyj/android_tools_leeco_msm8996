/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#include "QEncodeParams.h"

#define DEFAULT_QUALITY 75

/*===========================================================================
 * Function: QIEncodeParams
 *
 * Description: QIEncodeParams constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIEncodeParams::QIEncodeParams()
{
  mInputSize = QISize(0, 0);
  mOutputSize = QISize(0, 0);
  mQuality = DEFAULT_QUALITY;
  mQuantTable = NULL;
  mHuffTable = NULL;
}

/*===========================================================================
 * Function: QIEncodeParams
 *
 * Description: QIEncodeParams constructor
 *
 * Input parameters:
 *   aInput - input dimension of the image
 *   aOutput - output dimension of the image after scaling
 *   aQuality - quality factor
 *   aCrop - crop region for the image
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIEncodeParams::QIEncodeParams(QISize &aInput, QISize &aOutput,
    uint32_t aQuality,
    QICrop &aCrop)
{
  mInputSize = aInput;
  mOutputSize = aOutput;
  mQuality = aQuality;
  mCrop = aCrop;
  mQuantTable = NULL;
  mHuffTable = NULL;
}

/*===========================================================================
 * Function: ~QIEncodeParams
 *
 * Description: QIEncodeParams destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIEncodeParams::~QIEncodeParams()
{
  if (mQuantTable) {
    delete[] mQuantTable;
    mQuantTable = NULL;
  }
  if (mHuffTable) {
    delete[] mHuffTable;
    mHuffTable = NULL;
  }
}

/*===========================================================================
 * Function: setDefaultTables
 *
 * Description: set the standard huffman and quantization tables
 *
 * Input parameters:
 *   aQuality - quality factor
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_NO_MEMORY
 *
 * Notes: none
 *==========================================================================*/
int QIEncodeParams::setDefaultTables(uint32_t aQuality)
{
  if (NULL == mQuantTable) {
    mQuantTable = new QIQuantTable[2];
    if (NULL == mQuantTable) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      return QI_ERR_NO_MEMORY;
    }
  }
  for (int l = QIQuantTable::QTABLE_MIN;
    l < QIQuantTable::QTABLE_MAX; l++) {
    mQuantTable[l].setType((QIQuantTable::QTableType)l);
    mQuantTable[l].setDefaultTable(aQuality);
  }

  if (NULL == mHuffTable) {
    mHuffTable = new QIHuffTable[4];
    if (NULL == mHuffTable) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      return QI_ERR_NO_MEMORY;
    }
  }
  for (int l = QIHuffTable::HTABLE_MIN;
    l < QIHuffTable::HTABLE_MAX; l++) {
    mHuffTable[l].setType((QIHuffTable::QHuffTableType)l);
    mHuffTable[l].setDefaultTable();
  }
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: setHuffTables
 *
 * Description: set the huffman tables provided by user
 *
 * Input parameters:
 *   aHuffTable - pointer to QIHuffTable object
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QIEncodeParams::setHuffTables(QIHuffTable *aHuffTable)
{
  mHuffTable = aHuffTable;
}

/*===========================================================================
 * Function: setQuantTables
 *
 * Description: set the quantization tables provided by user
 *
 * Input parameters:
 *   aHuffTable - pointer to QIQuantTable object
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QIEncodeParams::setQuantTables(QIQuantTable *aQuantTable)
{
  mQuantTable = aQuantTable;
}

/*===========================================================================
 * Function: QuantTable
 *
 * Description: gets the quantization tables give the type
 *
 * Input parameters:
 *   aType - type of quantization table to be fetched
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIQuantTable *QIEncodeParams::QuantTable(QIQuantTable::QTableType aType)
{
  if ((aType < QIQuantTable::QTABLE_MIN) ||
    (aType >= QIQuantTable::QTABLE_MAX)) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return NULL;
  }
  return &mQuantTable[aType];
}

/*===========================================================================
 * Function: HuffTable
 *
 * Description: gets the huffman tables give the type
 *
 * Input parameters:
 *   aType - type of huffman table to be fetched
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIHuffTable *QIEncodeParams::HuffTable(QIHuffTable::QHuffTableType aType)
{
  if ((aType < QIHuffTable::HTABLE_MIN) ||
    (aType >= QIHuffTable::HTABLE_MAX)) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return NULL;
  }
  return &mHuffTable[aType];
}

/*===========================================================================
 * Function: operator=
 *
 * Description: overload the assignment operator for the QIEncodeParams
 *
 * Input parameters:
 *   aOther - reference of the QIEncodeParams object to be copied from
 *
 * Return values:
 *   reference to the QIEncodeParams object
 *
 * Notes: none
 *==========================================================================*/
QIEncodeParams& QIEncodeParams::operator=(const QIEncodeParams& aOther)
{
  *this = aOther;

  mQuantTable = new QIQuantTable[2];
  if (NULL == mQuantTable) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return *this;
  }
  for (int l = QIQuantTable::QTABLE_MIN;
    l < QIQuantTable::QTABLE_MAX; l++) {
    mQuantTable[l] = aOther.mQuantTable[l];
  }

  mHuffTable = new QIHuffTable[4];
  if (NULL == mHuffTable) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return *this;
  }
  for (int l = QIHuffTable::HTABLE_MIN;
    l < QIHuffTable::HTABLE_MAX; l++) {
    mHuffTable[l] = aOther.mHuffTable[l];
  }
  return *this;
}
