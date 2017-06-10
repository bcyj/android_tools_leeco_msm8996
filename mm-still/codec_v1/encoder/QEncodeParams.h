/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QIENCODEPARAMS_H__
#define __QIENCODEPARAMS_H__

#include "QIParams.h"
#include "QICrop.h"
#include "QISize.h"

#define Q_TABLE_CNT 2
#define H_TABLE_CNT 4

/*===========================================================================
 * Class: QIEncodeParams
 *
 * Description: This class represents the encode parameters
 *
 * Notes: none
 *==========================================================================*/
class QIEncodeParams :  public QIBase {

public:

  /** QIEncodeParams:
   *
   *  constructor
   **/
  QIEncodeParams();

  /** QIEncodeParams:
   *  @aInput: input size
   *  @aOutput: output size
   *  @aQuality: image quality
   *  @aCrop: image crop info
   *
   *  constructor
   **/
  QIEncodeParams(QISize &aInput, QISize &aOutput, uint32_t aQuality,
    QICrop &aCrop);

  /** ~QIEncodeParams:
   *
   *  virtual destructor
   **/
  virtual ~QIEncodeParams();

  /** setDefaultTables:
   *  @aQuality: image quality
   *
   *  set default quant and huff tables
   **/
  int setDefaultTables(uint32_t aQuality);

  /** setHuffTables:
   *  @aHuffTable: huffman table object
   *
   *  set specific huffman tables
   *  arg should be a single dim array of H_TABLE_CNT
   **/
  void setHuffTables(QIHuffTable *aHuffTable);

  /** setQuantTables:
   *  @aQuantTable: quantization table object
   *
   *  set specific quant tables
   *  arg should be a single dim array of Q_TABLE_CNT
   **/
  void setQuantTables(QIQuantTable *aQuantTable);

  /** QuantTable:
   *  @aType: quantization table type
   *
   *  get quantization table
   **/
  QIQuantTable *QuantTable(QIQuantTable::QTableType aType);

  /** HuffTable:
   *  @aType: huffman table type
   *
   *  get huffman table
   **/
  QIHuffTable *HuffTable(QIHuffTable::QHuffTableType aType);

  /** Quality:
   *
   *  returns image quality 1-100
   **/
  inline uint32_t Quality()
  {
    return mQuality;
  }

  /** setQuality:
   *  @aQuality: image quality
   *
   *  sets image quality 1-100
   **/
  inline void setQuality(uint32_t aQuality)
  {
    mQuality = aQuality;
  }

  /** OutputSize:
   *
   *  returns output image size
   **/
  inline QISize &OutputSize()
  {
    return mOutputSize;
  }

  /** setOutputSize:
   *  @aOutputSize: output image size
   *
   *  set output image size
   **/
  inline void setOutputSize(QISize &aOutputSize)
  {
    mOutputSize = aOutputSize;
  }

  /** InputSize:
   *
   *  returns input image size
   **/
  inline QISize &InputSize()
  {
    return mInputSize;
  }

  /** setInputSize:
   *  @aInputSize: image size
   *
   *  set input image size
   **/
  inline void setInputSize(QISize &aInputSize)
  {
    mInputSize = aInputSize;
  }

  /** Crop:
   *  @aInputSize: image size
   *
   *  returns crop window for encoding
   **/
  inline QICrop &Crop()
  {
    return mCrop;
  }

  /** setCrop:
   *  @aCrop: image cropping
   *
   *  sets crop window for encoding
   **/
  inline void setCrop(QICrop &aCrop)
  {
    mCrop = aCrop;
  }

  /** operator=:
   *  @aOther: encoder parameters
   *
   *  overloaded assignment operator
   **/
  QIEncodeParams& operator=(const QIEncodeParams& aOther);

  /** RestartInterval:
   *
   *  returns restart interval for encoding
   **/
  inline uint32_t RestartInterval()
  {
    return mRestartInterval;
  }

  /** setRestartInterval:
   *  @aInterval: restart interval
   *
   *  sets restart interval for encoding
   **/
  inline void setRestartInterval (uint32_t aInterval)
  {
    mRestartInterval = aInterval;
  }

  /** Rotation:
   *
   *  returns the image rotation
   **/
  inline uint32_t Rotation()
  {
    return mRotation;
  }

  /** setRotation:
   *  @aRotation: image rotation
   *
   *  sets the image rotation
   **/
  inline void setRotation(uint32_t aRotation)
  {
    mRotation = aRotation;
  }

  /** Number of planes:
   *
   *  returns the number of planes
   **/
  inline uint32_t NumOfComponents()
  {
    return mNumOfComponents;
  }

  /** setNumOfPlanes:
   *  @aNumOfPlanes: Number of planes in the image
   *
   *  sets the number of planes in the input image
   **/
  inline void setNumOfComponents(uint32_t aNumOfComponents)
  {
    mNumOfComponents = aNumOfComponents;
  }
  /** setHiSpeed:
   *  @aHiSpeed: enable hi speed
   *
   *  turn the HI speed HW encoding on/off
   **/
  inline void setHiSpeed(bool aHiSpeed)
  {
    mUseHiSpeed = aHiSpeed;
  }

  /** HiSpeed:
   *
   *  get the hi speed setting
   **/
  inline bool HiSpeed()
  {
    return mUseHiSpeed;
  }

private:

  /** mQuality:
   *
   *  image quality
   **/
  uint32_t mQuality;

  /** mQuantTable:
   *
   *  quantization table
   **/
  QIQuantTable *mQuantTable;

  /** mHuffTable:
   *
   *  huffman table
   **/
  QIHuffTable *mHuffTable;

  /** mCrop:
   *
   *  image cropping
   **/
  QICrop mCrop;

  /** mOutputSize:
   *
   *  output image size
   **/
  QISize mOutputSize;

  /** mInputSize:
   *
   *  input image size
   **/
  QISize mInputSize;

  /** mRestartInterval:
   *
   *  restart interval
   **/
  uint32_t mRestartInterval;

  /** mRotation:
   *
   *  image rotation
   **/
  uint32_t mRotation;

  /** mNumOfPlanes:
   *
   *  Number of planes
   *
  **/
  uint16_t mNumOfComponents;

  /** mUseHiSpeed:
   *
   *  Use jpeg turbo clock
   **/
  bool mUseHiSpeed;
};

#endif //__QIENCODEPARAMS_H__
