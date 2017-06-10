/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QIPARAMS_H__
#define __QIPARAMS_H__

#include "QICommon.h"
#include "QIBase.h"

#define QUANT_SIZE 64
#define HUFF_BITS 17
#define HUFF_VALUES 256

/*===========================================================================
 * Class: QIQuantTable
 *
 * Description: This class represents the quantization table used by codecs
 *
 *
 * Notes: none
 *==========================================================================*/
class QIQuantTable :  public QIBase {

public:

  /** QTableType:
   *  QTABLE_LUMA - luma table
   *  QTABLE_CHROMA - chroma table
   *
   *  quant table type
   **/
  typedef enum QTableType {
    QTABLE_MIN,
    QTABLE_LUMA = QTABLE_MIN,
    QTABLE_CHROMA,
    QTABLE_MAX,
  } QTableType;

  /** QIQuantTable:
   *
   *  constructor
   **/
  QIQuantTable();

  /** QIQuantTable:
   *  @aType - quant table type
   *
   *  constructor
   **/
  QIQuantTable(QTableType aType);

  /** ~QIQuantTable:
   *
   *  destructor
   **/
  virtual ~QIQuantTable();

  /** setTable:
   *  @aTable - quant table
   *
   *  set the quant table
   **/
  int setTable(uint16_t *aTable);

  /** Table:
   *
   *  returns the quantization table
   **/
  inline uint16_t* Table()
  {
    return mTable;
  }

  /** setDefaultTable:
   *  @aQuality - quality of image
   *
   *  sets the default quantization table
   **/
  void setDefaultTable(uint32_t aQuality);

  /** setType:
   *  @aType - quantization table type
   *
   *  sets the quantization table type
   **/
  inline void setType(QTableType aType)
  {
    mType = aType;
  }

  /** Type:
   *
   *  returns the quantization table type
   **/
  inline QTableType Type()
  {
    return mType;
  }

private:

  /** mType:
   *
   *  quantization table type
   **/
  QTableType mType;

  /** mType:
   *
   *  quantization table
   **/
  uint16_t mTable[QUANT_SIZE];
};

/*===========================================================================
 * Class: QIHuffTable
 *
 * Description: This class represents the huffman table used by codecs
 *
 *
 * Notes: none
 *==========================================================================*/
class QIHuffTable :  public QIBase {

public:

  /** HuffTable:
   *  @mBits: huffman table bits
   *  @mValues: huffman table values
   *
   *  huffman table
   **/
  typedef struct HuffTable {
    uint8_t mBits[HUFF_BITS];
    uint8_t mValues[HUFF_VALUES];
  } HuffTable;

  /** QHuffTableType:
   *  HTABLE_LUMA_AC: luma AC table
   *  HTABLE_CHROMA_AC: chroma AC table
   *  HTABLE_LUMA_DC: luma DC table
   *  HTABLE_CHROMA_DC: chroma DC table
   *
   *  huffman table type
   **/
  typedef enum QHuffTableType {
    HTABLE_MIN,
    HTABLE_LUMA_AC = HTABLE_MIN,
    HTABLE_CHROMA_AC,
    HTABLE_LUMA_DC,
    HTABLE_CHROMA_DC,
    HTABLE_MAX,
  }QHuffTableType;

  /** QIHuffTable:
   *  @aType: huffman table type
   *
   *  constructor
   **/
  QIHuffTable(QHuffTableType aType);

  /** QIHuffTable:
   *
   *  constructor
   **/
  QIHuffTable();

  /** ~QIHuffTable:
   *
   *  virtual destructor
   **/
  virtual ~QIHuffTable();

  /** setTable:
   *  @aHuffTable: huffman table
   *
   *  set huffman table
   **/
  int setTable(HuffTable *aHuffTable);

  /** Table:
   *
   *  returns huffman table
   **/
  inline const HuffTable* Table()
  {
    return &mTable;
  }

  /** setDefaultTable:
   *
   *  set default huffman table
   **/
  void setDefaultTable();

  /** setType:
   *  @aType: huffman table type
   *
   *  sets the huffman table type
   **/
  inline void setType(QHuffTableType aType)
  {
    mType = aType;
  }

  /** Type:
   *
   *  returns the huffman table type
   **/
  inline QHuffTableType Type()
  {
    return mType;
  }

private:

  /** mType:
   *
   *  huffman table type
   **/
  QHuffTableType mType;

  /** mTable:
   *
   *  huffman table
   **/
  HuffTable mTable;
};

#endif //__QIPARAMS_H__
