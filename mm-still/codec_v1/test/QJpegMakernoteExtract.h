/*****************************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

/** MKN_ERR
 *
 * Error macro
 */
#ifdef _ANDROID_
#define MKN_ERR(fmt, args...) (QIDBG_ERROR(fmt, ##args))
#else
#define MKN_ERR(fmt, args...) (fprintf(stderr, "[%s:%d]" fmt "\n", __func__, __LINE__, ##args))
#endif
/*===========================================================================
 * Class: QJpegMakernoteExtractor
 *
 * Description: This class provides functionality to extract and decrypt
 * makernote data
 *
 * Notes: none
 *==========================================================================*/
class QJpegMakernoteExtractor : public QImageReaderObserver {
public:
  /** setKeyFile
   *  @aKeyFile: Decryption key file name
   *
   *  Set name of decryption key file
   **/
  void setKeyFile(char *aKeyFile);
  /** setJpegFile
   *  @aJpegFile: image file name
   *
   *  Set name of image file
   **/
  void setJpegFile(char *aJpegFile);
  /** Extract
   *  @aOutBuf: Pointer to output buffer
   *
   *  Decrypt and extract makernote data to *aOutBuf
   **/
  int Extract(char **aOutBuf);
  /** Init
   *
   *  Initialise
   **/
  int Init();
  /** QJpegMakernoteExtractor
   *
   * Constructor
   */
  QJpegMakernoteExtractor();
  /** ~QJpegMakernoteExtractor
   *
   * Destructor
   */
  ~QJpegMakernoteExtractor();
private:
  /** ReadComplete
   *  @aBuffer: Image buffer parsed
   *
   *  Callback issued when the Image buffer is parsed successfully
   *  by the component
   **/
  void ReadComplete(QIBuffer &aBuffer);

  /** ReadFragmentDone
   *  @aBuffer: Image buffer parsed
   *
   *  Callback issued when the Image buffer passed to the parser
   *  is read.
   *  Note that if this callback is issued, the client should call
   *  addBuffer to send more buffers
   **/
  void ReadFragmentDone(QIBuffer &aBuffer);

  /** ReadError
   *  @aErrorType: Error type
   *
   *  Callback issued when error is occured during parsing
   **/
  void ReadError(ErrorType aErrorType);

  /** loadKey
   *
   * Load decryption key from file
   */
  int loadKey();

  /** loadImage
   *
   * Load image from file
   */
  int loadImage();

  /** parseExif
   *
   * Parse EXIF data
   */
  int parseExif();

  /** decryptMakernote
   *
   * Decrypt the makernote data
   */
  int decryptMakernote();

  /** mKeyFile
   *
   * Key file name
   */
  char *mKeyFile;

  /** mJpegFile
   *
   * Image file name
   */
  char *mJpgFile;

  /** mInBuffer
   *
   * Input buffer
   */
  QIBuffer *mInBuffer;

  /** mKey
   *
   * Decryption key
   */
  char *mKey;

  /** mParser
   *
   * Exif parser instance
   */
  QExifParser *mParser;

  /** mJpgHeader
   *
   * Jpeg header
   */
  jpeg_header_t *mJpgHeader;

  /** mCrypto
   *
   * Decryptor class instance
   */
  QCrypt *mCrypto;

  /** mDecData
   *
   * Pointer to decrypted data.
   */
  char *mDecData;

};
