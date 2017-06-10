/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#include "QIParams.h"


/* Static constants */
/*  default Luma Qtable */
const uint16_t DEFAULT_QTABLE_0[64] = {
  16, 11, 10, 16, 24, 40, 51, 61,
  12, 12, 14, 19, 26, 58, 60, 55,
  14, 13, 16, 24, 40, 57, 69, 56,
  14, 17, 22, 29, 51, 87, 80, 62,
  18, 22, 37, 56, 68, 109, 103, 77,
  24, 35, 55, 64, 81, 104, 113, 92,
  49, 64, 78, 87, 103, 121, 120, 101,
  72, 92, 95, 98, 112, 100, 103, 99
};

/*  default Chroma Qtable */
const uint16_t DEFAULT_QTABLE_1[64] = {
  17, 18, 24, 47, 99, 99, 99, 99,
  18, 21, 26, 66, 99, 99, 99, 99,
  24, 26, 56, 99, 99, 99, 99, 99,
  47, 66, 99, 99, 99, 99, 99, 99,
  99, 99, 99, 99, 99, 99, 99, 99,
  99, 99, 99, 99, 99, 99, 99, 99,
  99, 99, 99, 99, 99, 99, 99, 99,
  99, 99, 99, 99, 99, 99, 99, 99
};

const uint32_t nZigzagTable[64] = {
  0, 1, 8, 16, 9, 2, 3, 10,
  17, 24, 32, 25, 18, 11, 4, 5,
  12, 19, 26, 33, 40, 48, 41, 34,
  27, 20, 13, 6, 7, 14, 21, 28,
  35, 42, 49, 56, 57, 50, 43, 36,
  29, 22, 15, 23, 30, 37, 44, 51,
  58, 59, 52, 45, 38, 31, 39, 46,
  53, 60, 61, 54, 47, 55, 62, 63
};

/*  default Huffman Tables */
const QIHuffTable::HuffTable gStandardLumaDCHuffTable = {
  /* bits (0-based) */
  { 0, 0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
  /* values */
  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 },
};

const QIHuffTable::HuffTable gStandardChromaDCHuffTable = {
  /* bits (0-based) */
  { 0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 },
  /* values */
  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 },
};

const QIHuffTable::HuffTable gStandardLumaACHuffTable = {
  /* bits (0-based) */
  { 0, 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d },
  /* values */
  { 0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
    0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
    0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
    0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
    0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
    0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
    0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
    0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
    0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
    0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
    0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
    0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
    0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
    0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
    0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
    0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
    0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
    0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa },
};


const QIHuffTable::HuffTable gStandardChromaACHuffTable = {
  /* bits (0-based) */
  { 0, 0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77 },
  /* values */
  { 0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
    0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
    0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
    0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
    0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
    0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
    0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
    0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
    0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
    0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
    0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
    0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
    0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
    0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
    0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
    0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
    0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
    0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa },
};


/*===========================================================================
 * Function: QIQuantTable
 *
 * Description: QIQuantTable constructor
 *
 * Input parameters:
 *   aType - type of quantization table
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIQuantTable::QIQuantTable(QTableType aType)
{
  mType = aType;
}

/*===========================================================================
 * Function: QIQuantTable
 *
 * Description: QIQuantTable default constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIQuantTable::QIQuantTable()
{
  mType = QTABLE_LUMA;
}

/*===========================================================================
 * Function: ~QIQuantTable
 *
 * Description: QIQuantTable destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIQuantTable::~QIQuantTable()
{
}

/*===========================================================================
 * Function: setDefaultTable
 *
 * Description: Sets the standard quantization tables based on table type
 *
 * Input parameters:
 *   aQuality - quality of the image
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QIQuantTable::setDefaultTable(uint32_t aQuality)
{
  const uint16_t *lTable = (mType == QTABLE_LUMA) ?
    DEFAULT_QTABLE_0 : DEFAULT_QTABLE_1;
  int i;
  double lScaleFactor;


  aQuality = QICLAMP(aQuality, 1, 98);

  if (50 == aQuality) {
    memcpy(mTable, lTable, QUANT_SIZE * sizeof(uint16_t));
    return;
  }

  if (aQuality > 50) {
    lScaleFactor = 50.0 / (float)(100 - aQuality);
  } else {
    lScaleFactor = (float)aQuality / 50.0;
  }
  // Scale quant entries
  for (i = 0; i < QUANT_SIZE; i++) {
    // Compute new value based on input percent
    // and on the 50% table (low)
    // Add 0.5 after the divide to round up fractional divides to be
    // more conservative.
    mTable[i] = (uint16_t) (((double)lTable[i] / lScaleFactor) + 0.5);

    // Clamp
    mTable[i] = QICLAMP(mTable[i], 1, 255);
  }
}

/*===========================================================================
 * Function: setTable
 *
 * Description: Sets the quantization table provided by the user
 *
 * Input parameters:
 *   aTable - pointer to the table
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_INVALID_INPUT
 *
 * Notes: none
 *==========================================================================*/
int QIQuantTable::setTable(uint16_t *aTable)
{
  if (NULL == aTable) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_INVALID_INPUT;
  }
  memcpy(mTable, aTable, QUANT_SIZE * sizeof(uint16_t));
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: QIHuffTable
 *
 * Description: QIHuffTable constructor
 *
 * Input parameters:
 *   aType - type of the huffman table
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIHuffTable::QIHuffTable(QHuffTableType aType)
{
  mType = aType;
  memset(&mTable, 0x0, sizeof(HuffTable));
}

/*===========================================================================
 * Function: QIHuffTable
 *
 * Description: QIHuffTable constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIHuffTable::QIHuffTable()
{
  mType = HTABLE_LUMA_AC;
}

/*===========================================================================
 * Function: ~QIHuffTable
 *
 * Description: QIHuffTable destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIHuffTable::~QIHuffTable()
{
}

/*===========================================================================
 * Function: setDefaultTable
 *
 * Description: sets the standard huffman tables
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QIHuffTable::setDefaultTable()
{
  switch (mType) {
  case HTABLE_CHROMA_AC:
    mTable = gStandardChromaACHuffTable;
    break;
  case HTABLE_CHROMA_DC:
    mTable = gStandardChromaDCHuffTable;
    break;
  case HTABLE_LUMA_AC:
    mTable = gStandardLumaACHuffTable;
    break;
  default:
  case HTABLE_LUMA_DC:
    mTable = gStandardLumaDCHuffTable;
    break;
  }
}

/*===========================================================================
 * Function: setDefaultTable
 *
 * Description: sets the huffman table provided by the user
 *
 * Input parameters:
 *   aHuffTable - huffman table
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_INVALID_INPUT
 *
 * Notes: none
 *==========================================================================*/
int QIHuffTable::setTable(QIHuffTable::HuffTable *aHuffTable)
{
  if (NULL == aHuffTable) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_INVALID_INPUT;
  }
  mTable = *aHuffTable;
  return QI_SUCCESS;
}
