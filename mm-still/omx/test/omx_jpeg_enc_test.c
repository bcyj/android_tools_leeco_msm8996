/*============================================================================

 Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <sys/types.h>
#include <fcntl.h>
#include <linux/android_pmem.h>
#include <linux/ioctl.h>
#include <stdio.h>
#include <stdlib.h>

/* KHRONOS header files */
#include "OMX_Types.h"
#include "OMX_Index.h"
#include "OMX_Core.h"
#include "OMX_Component.h"

#include "omx_debug.h"
#include "omx_jpeg_ext.h"
#include "os_pmem.h"

#define CEILING16(X) (((X) + 0x000F) & 0xFFF0)
#define PAD_TO_WORD(a)(((a)+3)&~3)
#define CMP_SIZE 1024

OMX_CALLBACKTYPE callbacks;
OMX_INDEXTYPE type;
omx_jpeg_exif_info_tag tag;
OMX_CONFIG_ROTATIONTYPE rotType;
omx_jpeg_thumbnail thumbnail;
OMX_CONFIG_RECTTYPE recttype;
OMX_BUFFERHEADERTYPE* pInBuffers = NULL;
OMX_BUFFERHEADERTYPE* pInBuffers1 = NULL;
OMX_BUFFERHEADERTYPE* pOutBuffers = NULL;
OMX_INDEXTYPE buffer_offset;
omx_jpeg_buffer_offset bufferoffset;
OMX_INDEXTYPE user_preferences;
omx_jpeg_user_preferences userpreferences;
static omx_jpeg_thumbnail_quality thumbnailQuality;
static OMX_INDEXTYPE thumbnailQualityType;
int encoding =0;

pthread_mutex_t lock;
pthread_cond_t cond;
int expectedEvent = 0;
int expectedValue1 = 0;
int expectedValue2 = 0;
uint8_t * buffer1 = 0;
uint8_t * buffer2 = 0;
char *output_file;
char *reference_file;

void invokeDeinit(OMX_HANDLETYPE pHandle );

const char color_formats[8][13] =
{
  "YCRCBLP_H2V2",
  "YCBCRLP_H2V2",
  "YCRCBLP_H2V1",
  "YCBCRLP_H2V1",
  "YCRCBLP_H1V2",
  "YCBCRLP_H1V2",
  "YCRCBLP_H1V1",
  "YCBCRLP_H1V1",
};

const char *preference_str[] =
{
  "Hardware accelerated preferred",
  "Hardware accelerated only",
  "Software based preferred",
  "Software based only",
};

typedef struct {
  char                *file_name;
  uint32_t             width;
  uint32_t             height;
  uint32_t             quality;
  omx_jpeg_color_format format;

} input_image_args_t;

typedef struct {
  uint32_t    input_width;
  uint32_t    input_height;
  uint32_t    h_offset;
  uint32_t    v_offset;
  uint32_t    output_width;
  uint32_t    output_height;
  uint8_t     enable;
}image_scale_cfg_t;

/*Input data structure*/
typedef struct {
  input_image_args_t   main;
  input_image_args_t   thumbnail;
  char *               output_file;
  int16_t              rotation;
  omx_jpeg_preference  preference;
  uint8_t              encode_thumbnail;
  uint16_t             back_to_back_count;
  uint32_t             target_filesize;
  uint32_t             output_buf_size;
  uint32_t             abort_time;
  const char*          reference_file;
  uint8_t              use_pmem;
  image_scale_cfg_t    main_scale_cfg;
  image_scale_cfg_t    tn_scale_cfg;

} test_args_t;

/*===========================================================================
 * FUNCTION    - readFile -
 *
 * DESCRIPTION:
 *==========================================================================*/
void readFile(const char * filename, uint8_t* buffer,int w, int h)
{
  int fd = open(filename, O_RDONLY, 0777);
  int s = (w*h*3)/2;
  buffer[0] = 100;
  buffer[100] = 11;
  buffer[1000] = 10101;
  int i ;
  for (i =0; i<(s/1000);i++) {
    int r = read(fd, buffer+(1000*i), 1000);
  }
  close(fd);
}
/*===========================================================================
 * FUNCTION    - matchReferenceFile -
 *
 * DESCRIPTION: Match the gold standard reference file to
 *              the output file
 *==========================================================================*/
int matchReferenceFile()
{
  FILE *fout, *fref;
  char *buf1;
  char *buf2;
  int bytes_read1, bytes_read2;

  buf1 = malloc(CMP_SIZE);
  if (!buf1) {
    fprintf(stderr, "encoder_test: failed to malloc for buffer for file comparison\n");
    return 1;
  }

  buf2 = malloc(CMP_SIZE);
  if (!buf2) {
    fprintf(stderr, "encoder_test: failed to malloc for buffer for file comparison\n");
    free(buf1);
    return 1;
  }
  fprintf(stderr, "encoder_test: comparing %s with %s...\n",
          output_file, reference_file);
  fref = fopen(reference_file, "rb");
  if (!fref) {
    fprintf(stderr, "encoder_test: error opening reference file: %s\n", reference_file);
    free(buf1);
    free(buf2);
    return 1;
  }

  fout = fopen(output_file, "rb");
  if (!fout) {
    fprintf(stderr, "encoder_test: error opening reference file: %s\n", output_file);
    free(buf1);
    free(buf2);
    return 1;
  }
  for (;;) {
    bytes_read1 = (int)fread(buf1, 1, CMP_SIZE, fref);
    bytes_read2 = (int)fread(buf2, 1, CMP_SIZE, fout);
    if (bytes_read1 != bytes_read2 || memcmp(buf1, buf2, bytes_read1)) {
      fprintf(stderr, "encoder_test: the two files differ.\n");
      free(buf1);
      free(buf2);
      return 1;
    }
    if (feof(fref) && feof(fout)) {
      fprintf(stderr, "encoder_test: output matches the reference file.\n");
      break;
    }
    if (feof(fref) || feof(fout) || ferror(fref) || ferror(fout)) {
      fprintf(stderr, "encoder_test: the two files differ.\n");
      free(buf1);
      free(buf2);
      return 1;
    }
  }
  free(buf1);
  free(buf2);
  return 0;
}

/*===========================================================================
 * FUNCTION    - handleError -
 *
 * DESCRIPTION: Handle Jpeg failure and other errors
 *==========================================================================*/
OMX_ERRORTYPE handleError(OMX_IN OMX_EVENTTYPE eEvent, OMX_IN OMX_U32 error,
                          OMX_HANDLETYPE pHandle)
{
  OMX_DBG_ERROR("%s", __func__);
  if (error == OMX_EVENT_JPEG_ERROR) {
    if (encoding) {
      fprintf(stderr,"JPEG Failure!!\n");
      OMX_DBG_ERROR("Jpeg Failure: No FTB Done \n");
      invokeDeinit(pHandle);
      pthread_cond_destroy(&cond);
      pthread_mutex_destroy(&lock);
    }
  } else if (error == OMX_EVENT_THUMBNAIL_DROPPED) {
    if (encoding) {
      OMX_DBG_ERROR("handleError :(OMX_EVENT_THUMBNAIL_DROPPED\n");
    }
  }
  return 0;
}
/*===========================================================================
 * FUNCTION    - use_padded_buffer -
 *
 * DESCRIPTION: Check if padding is needed depending on the inout image
 *==========================================================================*/

static int use_padded_buffer(test_args_t *p_args)
{
  int use_padding = 0;
  if (((p_args->main.format == OMX_YCRCBLP_H2V2)
       || (p_args->main.format == OMX_YCBCRLP_H2V2))
      && (((omx_jpeg_preference)p_args->preference ==
           OMX_JPEG_PREF_HW_ACCELERATED_ONLY)
          || ((omx_jpeg_preference)p_args->preference ==
              OMX_JPEG_PREF_HW_ACCELERATED_PREFERRED))) {
    uint32_t actual_size = p_args->main.height*p_args->main.width;
    uint32_t padded_size = CEILING16(p_args->main.width) *
      CEILING16(p_args->main.height);
    if (actual_size != padded_size) {
      use_padding = 1;
    }
  }
  return use_padding;
}
/*===========================================================================
 * FUNCTION    - etbdone -
 *
 * DESCRIPTION: Handle the OMX_ETB_DONE event
 *==========================================================================*/
OMX_ERRORTYPE etbdone(OMX_OUT OMX_HANDLETYPE hComponent,
                      OMX_OUT OMX_PTR pAppData,
                      OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer)
{
  OMX_DBG_INFO("%s", __func__);
  pthread_mutex_lock(&lock);
  expectedEvent = OMX_EVENT_ETB_DONE;
  expectedValue1 = 0;
  expectedValue2 = 0;
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&lock);
  return 0;
}

/*===========================================================================
 * FUNCTION    - ftbdone -
 *
 * DESCRIPTION: Handle the OMX_EVENT_FTB_DONE event
 *==========================================================================*/
OMX_ERRORTYPE ftbdone(OMX_OUT OMX_HANDLETYPE hComponent,
                      OMX_OUT OMX_PTR pAppData,
                      OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer)
{
  OMX_DBG_INFO("%s: E", __func__);
  /*write to file*/
  int fd = open(output_file, O_RDWR|O_CREAT, 0777);
  write(fd, pBuffer->pBuffer,
        pBuffer->nFilledLen);
  close(fd);

  pthread_mutex_lock(&lock);
  expectedEvent = OMX_EVENT_FTB_DONE;
  expectedValue1 = 0;
  expectedValue2 = 0;
  invokeDeinit(hComponent);
  if (reference_file) {
    matchReferenceFile();
  }
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&lock);

  return 0;
}

/*===========================================================================
 * FUNCTION    - eventHandler -
 *
 * DESCRIPTION: EventHandler to handle generic events
 *==========================================================================*/
OMX_ERRORTYPE eventHandler( OMX_IN OMX_HANDLETYPE hComponent,
                            OMX_IN OMX_PTR pAppData, OMX_IN OMX_EVENTTYPE eEvent,
                            OMX_IN OMX_U32 nData1, OMX_IN OMX_U32 nData2,
                            OMX_IN OMX_PTR pEventData)
{
  OMX_DBG_ERROR("%s: %d %d %d", __func__, eEvent, nData1, nData2);
  pthread_mutex_lock(&lock);
  expectedEvent = eEvent;
  expectedValue1 = nData1;
  expectedValue2 = nData2;
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&lock);
  if ((nData1== OMX_EVENT_JPEG_ERROR)||(nData1== OMX_EVENT_THUMBNAIL_DROPPED))
    handleError(eEvent,nData1,hComponent);

  return 0;
}

/*===========================================================================
 * FUNCTION    - waitForEvent -
 *
 * DESCRIPTION:
 *==========================================================================*/
void waitForEvent(int event, int value1, int value2 )
{
  OMX_DBG_INFO("%s:E", __func__);
  pthread_mutex_lock(&lock);
  while (! (expectedEvent == event &&
            expectedValue1 == value1 && expectedValue2 == value2)) {
    pthread_cond_wait(&cond, &lock);
  }
  pthread_mutex_unlock(&lock);
  OMX_DBG_INFO("%s:X", __func__);
}

/*===========================================================================
 * FUNCTION    - get_buffer_offset -
 *
 * DESCRIPTION: Get the buffer offset depending on padding
 *==========================================================================*/
int8_t get_buffer_offset(uint32_t width, uint32_t height,
                         int* p_y_offset, int* p_cbcr_offset,
                         int* p_buf_size, int usePadding, int rotation)
{
  OMX_DBG_ERROR("jpeg_encoder_get_buffer_offset");
  if ((NULL == p_y_offset) || (NULL == p_cbcr_offset)) {
    return -1;
  }

  if (usePadding) {
    int cbcr_offset = 0;
    uint32_t actual_size = width*height;
    uint32_t padded_size = width * CEILING16(height);
    *p_y_offset = 0;
    *p_cbcr_offset = padded_size;
    if ((rotation == 90) || (rotation == 180)) {
      *p_y_offset += padded_size - actual_size;
      *p_cbcr_offset += ((padded_size - actual_size) >> 1);
    }
    *p_buf_size = padded_size * 3/2;
  } else {
    *p_y_offset = 0;
    *p_cbcr_offset = PAD_TO_WORD(width*height);
    *p_buf_size = *p_cbcr_offset * 3/2;
  }
  return 0;
}

/*===========================================================================
 * FUNCTION    - print_usage -
 *
 * DESCRIPTION: Print usage of the test client
 *==========================================================================*/

void print_usage()
{
  fprintf(stderr, "Usage: program_name [options] [-I <input file>] [-O <output file] [-W <width>] [-H <height>] [-F <format>]\n");
  fprintf(stderr, "Mandatory options:\n");
  fprintf(stderr, "  -I FILE\t\tPath to the input file.\n");
  fprintf(stderr, "  -O FILE\t\tPath for the output file.\n");
  fprintf(stderr, "  -W WIDTH\t\tInput image width.\n");
  fprintf(stderr, "  -H HEIGHT\t\tInput image height.\n");
  fprintf(stderr, "  -F FORMAT\t\tInput image format:\n");
  fprintf(stderr, "\t\t\t\tYCRCBLP_H2V2 (0), YCBCRLP_H2V2 (1), YCRCBLP_H2V1 (2), YCCRLP_H2V1 (3)\n");
  fprintf(stderr, "\t\t\t\tYCRCBLP_H1V2 (4), YCBCRLP_H1V2 (5), YCRCBLP_H1V1 (6), YCCRLP_H1V1 (7)\n");
  fprintf(stderr, "\t\t\t\tBITSTREAM_H2V2 (12), BITSTREAM_H2V1 (14), BITSTREAM_H1V2(16), BITSTREAM_H1V1 (18)\n");
  fprintf(stderr, "Optional:\n");
  fprintf(stderr, "  -r ROTATION\t\tRotation (clockwise) in degrees\n");
  fprintf(stderr, "  -t\t\t\tEncode thumbnail image as well. If turned on, the fourarguments below should be supplied.\n");
  fprintf(stderr, "  -i FILE\t\tPath to the input file for thumbnail.\n");
  fprintf(stderr, "  -w WIDTH\t\tThumbnail image width.\n");
  fprintf(stderr, "  -h HEIGHT\t\tThumbnail image height.\n");
  fprintf(stderr, "  -f FORMAT\t\tThumbnail image format. (Permissible values are lsted above)\n");
  fprintf(stderr, "  -Q QUALITY\t\tQuality factor for main image (1-100).\n");
  fprintf(stderr, "  -q QUALITY\t\tQuality factor for thumbnail (1-100).\n");
  fprintf(stderr, "  -p PREFERENCE\t\tPreference on which encoder to use (Software-ased or Hardware-accelerated).\n");
  fprintf(stderr, "               \t\t\tHW preferred (0-default), HW only (1), SW peferred (2), SW only (3)\n");
  fprintf(stderr, "  -P PMEM\t\tEnable use of PMEM.\n");
  fprintf(stderr, "  -c COUNT\t\tRe-encode back-to-back 'COUNT' times. Default is 1\n");
  fprintf(stderr, "  -u\t\t\tEnable scale for Main Image. Make sure the scale paramters are supplied.\n");
  fprintf(stderr, "  -m SCALE_I_WIDTH\tMain input width.\n");
  fprintf(stderr, "  -n SCALE_I_HEIGHT\tMain input height.\n");
  fprintf(stderr, "  -x SCALE_H_OFFSET\tMain horizontal offset.\n");
  fprintf(stderr, "  -y SCALE_V_OFFSET\tMain vertical offset.\n");
  fprintf(stderr, "  -M SCALE_O_WIDTH\tMain output width.\n");
  fprintf(stderr, "  -N SCALE_O_HEIGHT\tMain output height.\n");
  fprintf(stderr, "  -U\t\t\tEnable scale for Thumbnail. Make sure the scale parameers are supplied.\n");
  fprintf(stderr, "  -j SCALE_I_WIDTH\tThumbnail input width.\n");
  fprintf(stderr, "  -k SCALE_I_HEIGHT\tThumbnail input height.\n");
  fprintf(stderr, "  -X SCALE_H_OFFSET\tThumbnail horizontal offset.\n");
  fprintf(stderr, "  -Y SCALE_V_OFFSET\tThumbnail vertical offset.\n");
  fprintf(stderr, "  -J SCALE_O_WIDTH\tThumbnail output width.\n");
  fprintf(stderr, "  -K SCALE_O_HEIGHT\tThumbnail output height.\n");
  //  fprintf(stderr, "  -S FILE_SIZE_CONTROL\t\tTarget file size in Bytes.\n");
  //  fprintf(stderr, "  -B OUTPUT_BUFFER_SIZE\t\tOutput buffer size in KBytes.\n")
  //  fprintf(stderr, "  -e CONCURRENT_COUNT\tRun COUNT tests in concurrent threads(test for multiple instances of encoder within same process).\n");
  //  fprintf(stderr, "  -a ABORT_TIME\t\tThe duration before an abort is issued (i milliseconds).\n");
  fprintf(stderr, "  -g GOLDEN_REFERENCE\tThe reference file to be matched against he output.\n");
  //  fprintf(stderr, "  -R RESTART_INTERVAL\tRestart Interval in number of MCUs\n";
  //  fprintf(stderr, "  -Z NOWRITE_TO_OUTPUT\tDisable write to the output file.\n";

  fprintf(stderr, "\n");
}

/*===========================================================================
 * FUNCTION    -getInput-
 *
 * DESCRIPTION: Get User Input
 *==========================================================================*/
int getInput(int argc, char *argv[], test_args_t  *test_args){
  int c;
  fprintf(stderr, "=============================================================\n");
  fprintf(stderr, "Encoder test\n");
  fprintf(stderr, "=============================================================\n");

  while ((c = getopt(argc, argv, "I:O:W:H:F:Q:r:ti:w:h:f:q:p:c:um:n:x:y:M:N:Uj:k:X::J:K:g:PZ")) != -1) {
    switch (c) {
    case 'O':
      test_args->output_file = optarg;
      fprintf(stderr, "%-25s%s\n", "Output image path", test_args->output_file);
      break;
    case 'I':
      test_args->main.file_name = optarg;
      fprintf(stderr, "%-25s%s\n", "Input image path", test_args->main.file_name);
      break;
    case 'i':
      test_args->thumbnail.file_name = optarg;
      fprintf(stderr, "%-25s%s\n", "Thumbnail image path",
              test_args->thumbnail.file_name);
      break;
    case 'Q':
      test_args->main.quality = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Main image quality", test_args->main.quality);
      break;
    case 'q':
      test_args->thumbnail.quality = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Thumbnail quality", test_args->thumbnail.quality);
      break;
    case 'W':
      test_args->main.width = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Input width", test_args->main.width);
      break;
    case 'w':
      test_args->thumbnail.width = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Thumbnail width", test_args->thumbnail.width);
      break;
    case 'H':
      test_args->main.height = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Input height", test_args->main.height);
      break;
    case 'h':
      test_args->thumbnail.height = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Thumbnail height",
              test_args->thumbnail.height);
      break;
    case 'r':
      test_args->rotation = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Rotation", test_args->rotation);
      break;
    case 't':
      test_args->encode_thumbnail = 1;
      fprintf(stderr, "%-25s%s\n", "Encode thumbnail", "true");
      break;
    case 'F':
      test_args->main.format = atoi(optarg);
      if (!test_args->main.format) {
        fprintf(stderr, "Invalid main image Input format\n");
        print_usage();
        return 1;
      }
      fprintf(stderr, "%-25s%s\n", "Input format",
              color_formats[(uint8_t)test_args->main.format]);
      break;
    case 'f':
      test_args->thumbnail.format = atoi(optarg);
      if (!test_args->thumbnail.format) {
        fprintf(stderr, "Invalid Thumbnail image Input format\n");
        print_usage();
        return 1;
      }
      fprintf(stderr, "%-25s%s\n", "Thumbnail format",
              color_formats[(uint8_t)test_args->thumbnail.format]);
      break;
    case 'p':
      test_args->preference = atoi(optarg);
      if ((test_args->preference < 0) ||
        (test_args->preference >= OMX_JPEG_PREF_MAX)) {
        fprintf(stderr, "Invalid Encoding preference\n");
        print_usage();
        return 1;
      }
      fprintf(stderr, "%-25s%s\n", "Encoder preference",
              preference_str[test_args->preference]);
      break;
    case 'c':
      test_args->back_to_back_count = atoi(optarg);
      if (test_args->back_to_back_count == 0) {
        fprintf(stderr, "Invalid count: -c %d\n", test_args->back_to_back_count);
        return 1;
      }
      break;
    case 'u':
      test_args->main_scale_cfg.enable = 1;
      fprintf(stderr, "%-25s%s\n", "scale enabled for main image", "true");
      break;
    case 'm':
      test_args->main_scale_cfg.input_width = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "main image scale input width",
              test_args->main_scale_cfg.input_width);
      break;
    case 'n':
      test_args->main_scale_cfg.input_height = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "main image scale input height",
              test_args->main_scale_cfg.input_height);
      break;
    case 'M':
      test_args->main_scale_cfg.output_width = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "main image scale output width",
              test_args->main_scale_cfg.output_width);
      break;
    case 'N':
      test_args->main_scale_cfg.output_height = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "main image scale output height",
              test_args->main_scale_cfg.output_height);
      break;
    case 'x':
      test_args->main_scale_cfg.h_offset = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "main image scale h offset",
              test_args->main_scale_cfg.h_offset);
      break;
    case 'y':
      test_args->main_scale_cfg.v_offset = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "main image scale v offset",
              test_args->main_scale_cfg.v_offset);
      break;
    case 'U':
      test_args->tn_scale_cfg.enable = 1;
      fprintf(stderr, "%-25s%s\n", "scale enabled for thumbnail", "true");
      break;
    case 'j':
      test_args->tn_scale_cfg.input_width = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "thumbnail scale input width",
               test_args->tn_scale_cfg.input_width);
      break;
    case 'k':
      test_args->tn_scale_cfg.input_height = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "thumbnail scale input height",
              test_args->tn_scale_cfg.input_height);
      break;
    case 'J':
      test_args->tn_scale_cfg.output_width = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "thumbnail scale output width",
               test_args->tn_scale_cfg.output_width);
      break;
    case 'K':
      test_args->tn_scale_cfg.output_height = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "thumbnail scale output height",
               test_args->tn_scale_cfg.output_height);
      break;
    case 'X':
      test_args->tn_scale_cfg.h_offset = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "thumbnail scale h offset",
               test_args->tn_scale_cfg.h_offset);
      break;
    case 'Y':
      test_args->tn_scale_cfg.v_offset = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "thumbnail scale v offset",
              test_args->tn_scale_cfg.v_offset);
      break;
    case 'g':
      test_args->reference_file = optarg;
      fprintf(stderr, "%-25s%s\n", "Reference file",
              test_args->reference_file);
      break;
    case 'P':
      test_args->use_pmem = 1;
      fprintf(stderr, "%-25s%s\n", "Use PMEM", "true");
      break;
    }
  }
  if (!test_args->main.file_name || !test_args->output_file ||
      !test_args->main.width ||
      !test_args->main.height ||
      test_args->main.format == 8) {
    fprintf(stderr, "Missing required arguments.\n");
    print_usage();
    return 1;
  }
  if (test_args->encode_thumbnail &&
      (!test_args->thumbnail.file_name ||
       !test_args->thumbnail.width ||
       !test_args->thumbnail.height ||
       test_args->thumbnail.format == 8)) {
    fprintf(stderr, "Missing thumbnail arguments.\n");
    print_usage();
    return 1;
  }
  return 0;
}

/*===========================================================================
 * FUNCTION    - main -
 *
 * DESCRIPTION:
 *==========================================================================*/
int main(int argc, char* argv[])
{
  int pmem_fd = 0;
  int ion_fd = 0;
  struct ion_allocation_data alloc_data;
  struct ion_fd_data info_fd;

  int pmem_fd1 = 0;
  int ion_fd1 = 0;
  struct ion_allocation_data alloc_data1;
  struct ion_fd_data info_fd1;

  uint8_t * buffer = 0;
  uint8_t * buffer_thumb = 0;
  int w,h;
  int w_thumbnail, h_thumbnail;
  int size;
  int size_thumb;
  int i =0;
  int rc = 0,c =0;
  int t = 0;
  const char * filename;
  const char * filename_thumb;
  int usePadding =0;
  uint8_t use_pmem = 1;
  uint8_t* outBuffer;


  OMX_DBG_ERROR("IN OMX_JPEG_ENC_CLIENT main\n");
  /*Initialize the test argument structure*/
  test_args_t test_args;
  memset(&test_args, 0, sizeof(test_args));
  test_args.main.format = 8;
  test_args.main.quality = 75;
  test_args.thumbnail.format = 8;
  test_args.thumbnail.quality = 75;
  test_args.back_to_back_count = 1;
  test_args.preference = 0; /*HW-accelerated-preferred*/

  /*Get Command line input and fill the structure*/
  if (c =getInput(argc,argv,&test_args) != 0) {
    return 1;
  }
  filename = test_args.main.file_name;
  w = test_args.main.width;
  h = test_args.main.height;
  output_file = test_args.output_file;

  filename_thumb = test_args.thumbnail.file_name;
  w_thumbnail = test_args.thumbnail.width;
  h_thumbnail = test_args.thumbnail.height;

  reference_file =  test_args.reference_file;
  usePadding = use_padded_buffer(&test_args);
  if ((test_args.preference == OMX_JPEG_PREF_SOFTWARE_PREFERRED) ||
      ( test_args.preference ==OMX_JPEG_PREF_SOFTWARE_PREFERRED)) {
    use_pmem =  test_args.use_pmem;
  }

  callbacks.EmptyBufferDone = etbdone;
  callbacks.FillBufferDone = ftbdone;
  callbacks.EventHandler = eventHandler;

  pthread_mutex_init(&lock, NULL);
  pthread_cond_init(&cond, NULL);


  /*Get handle to OMX encoder component*/
  OMX_HANDLETYPE pHandle = NULL;
  OMX_ERRORTYPE ret = OMX_GetHandle(&pHandle,
                                    "OMX.qcom.image.jpeg.encoder",
                                    NULL,
                                    &callbacks);
  if (ret != OMX_ErrorNone) {
    OMX_DBG_ERROR("%s:L#%d: ret=%d\n", __func__, __LINE__, ret);
    return 0;
  }

  if (pHandle == NULL ||
      ((OMX_COMPONENTTYPE*)pHandle)->FillThisBuffer == NULL) {
    OMX_DBG_ERROR("%s:L#%d: pHandle is NULL\n", __func__, __LINE__);
    return 0;
  }

  OMX_PARAM_PORTDEFINITIONTYPE * inputPort =
  malloc(sizeof(OMX_PARAM_PORTDEFINITIONTYPE ));
  if(!inputPort) {
    OMX_DBG_ERROR("%s: Error in malloc for inputPort",__func__);
    return 0;
  }
  OMX_PARAM_PORTDEFINITIONTYPE * inputPort1 =
  malloc(sizeof(OMX_PARAM_PORTDEFINITIONTYPE ));
  if(!inputPort1) {
    OMX_MM_FREE(inputPort);
    OMX_DBG_ERROR("%s: Error in malloc for inputPort1 ",__func__);
    return 0;
  }
  OMX_PARAM_PORTDEFINITIONTYPE * outputPort =
  malloc(sizeof(OMX_PARAM_PORTDEFINITIONTYPE ));
  if(!outputPort) {
    OMX_MM_FREE(inputPort);
    OMX_MM_FREE(inputPort1);
    OMX_DBG_ERROR("%s: Error in malloc for outputPort ",__func__);
    return 0;
  }

  if(!inputPort) {
    return 0;
  }

  inputPort->nPortIndex = 0;
  inputPort1->nPortIndex = 2;
  outputPort->nPortIndex = 1;

  OMX_DBG_ERROR("function pointer is %p " ,
                ((OMX_COMPONENTTYPE*)pHandle)->GetParameter);
  OMX_GetParameter(pHandle, OMX_IndexParamPortDefinition, inputPort);
  OMX_GetParameter(pHandle, OMX_IndexParamPortDefinition, inputPort1);
  OMX_GetParameter(pHandle, OMX_IndexParamPortDefinition, outputPort);

 /*Set buffer offset*/
  bufferoffset.width = w;
  bufferoffset.height = h;
  get_buffer_offset( bufferoffset.width, bufferoffset.height,
                     &bufferoffset.yOffset,
                     &bufferoffset.cbcrOffset, &bufferoffset.totalSize,
                     usePadding,test_args.rotation);

  OMX_GetExtensionIndex(pHandle,"omx.qcom.jpeg.exttype.buffer_offset",
                        &buffer_offset);
  OMX_DBG_ERROR("Buffer offset: yOffset =%d, cbcrOffset =%d, totalSize = %d\n",
       bufferoffset.yOffset,
       bufferoffset.cbcrOffset,bufferoffset.totalSize);
  OMX_SetParameter(pHandle, buffer_offset, &bufferoffset);

  /*Set input port parameters*/
  inputPort->format.image.nFrameWidth = w;
  inputPort->format.image.nFrameHeight = h;
  inputPort->format.image.nStride = w;
  inputPort->format.image.nSliceHeight = h;
  inputPort->nBufferSize = bufferoffset.totalSize;
  OMX_SetParameter(pHandle, OMX_IndexParamPortDefinition, inputPort);
  OMX_GetParameter(pHandle, OMX_IndexParamPortDefinition, inputPort);
  size = inputPort->nBufferSize;

  inputPort1->format.image.nFrameWidth = w_thumbnail;
  inputPort1->format.image.nFrameHeight = h_thumbnail;
  inputPort1->format.image.nStride = w_thumbnail;
  inputPort1->format.image.nSliceHeight = h_thumbnail;
  OMX_SetParameter(pHandle, OMX_IndexParamPortDefinition, inputPort1);
  OMX_GetParameter(pHandle, OMX_IndexParamPortDefinition, inputPort1);

  bufferoffset.width = w;
  bufferoffset.height = h;


  size_thumb = inputPort1->nBufferSize;
  OMX_DBG_ERROR("%s Thumb size: %d\n",__func__,size_thumb);
  /*Allocate input buffer*/
  if (use_pmem) {

    alloc_data.len = size;
    alloc_data.align = 4096;
    alloc_data.flags = ION_FLAG_CACHED;
    alloc_data.heap_mask = 0x1 << ION_IOMMU_HEAP_ID;

  /*rc = os_pmem_fd_open(&pmem_fd);*/
    rc = os_pmem_fd_open(&pmem_fd, &ion_fd,
                         &alloc_data, &info_fd);
    rc = os_pmem_allocate(pmem_fd, size, &buffer);
    t = os_pmem_get_phy_addr(pmem_fd, &buffer1);

  } else {
    /*Allocate heap memory*/
    buffer = (uint8_t *)malloc(size);
    if (!buffer) {
      fprintf(stderr, "Cannot allocate memory to read input\n");
      return 1;
    }
  }

  readFile(filename, buffer, w, h);

  omx_jpeg_pmem_info * pmem_info = malloc(sizeof(omx_jpeg_pmem_info));
  if(!pmem_info) {
    OMX_DBG_ERROR("%s: Error in malloc for pmem_info",__func__);
    if(buffer) {
      free(buffer);
    }
    return 0;
  }
  pmem_info->fd = pmem_fd;
  pmem_info->offset = 0 ;
  OMX_DBG_ERROR("in use input buffer %p %d %d", buffer ,pmem_info->fd, size );

  /*For thumbnail buffer*/
  if (use_pmem) {

    alloc_data1.len = size_thumb;
    alloc_data1.align = 4096;
    alloc_data1.flags = ION_FLAG_CACHED;
    alloc_data1.heap_mask = 0x1 << ION_IOMMU_HEAP_ID;

    rc = os_pmem_fd_open(&pmem_fd1,&ion_fd1,
                         &alloc_data1, &info_fd1);
    rc = os_pmem_allocate(pmem_fd1, size_thumb, &buffer_thumb);
    t = os_pmem_get_phy_addr(pmem_fd1, &buffer2);
  } else {
    /*Allocate heap memory*/
    buffer_thumb = (uint8_t *)malloc(size_thumb);
    if (!buffer_thumb) {
      fprintf(stderr, "Cannot allocate memory to read input\n");
      OMX_MM_FREE(pmem_info);
      return 1;
    }
  }

  readFile(filename_thumb, buffer_thumb, w_thumbnail, h_thumbnail);


  omx_jpeg_pmem_info * pmem_info1 = malloc(sizeof(omx_jpeg_pmem_info));
  if(!pmem_info1) {
    OMX_DBG_ERROR("%s: Error in malloc for pmem_info",__func__);
    if(pmem_info)
      free(pmem_info);
    if(buffer_thumb)
      free(buffer_thumb);
    return 0;
  }
  pmem_info1->fd = pmem_fd1;
  pmem_info1->offset = 0 ;
  OMX_DBG_ERROR("in use input buffer %p %d %d", buffer_thumb ,pmem_info1->fd,
                size_thumb );
  /*Set color format and prefeence*/
  userpreferences.color_format = test_args.main.format;
  userpreferences.thumbnail_color_format = test_args.thumbnail.format;
  userpreferences.preference = test_args.preference;
  OMX_GetExtensionIndex(pHandle,"omx.qcom.jpeg.exttype.user_preferences",
                        &user_preferences);

  OMX_SetParameter(pHandle,user_preferences,&userpreferences);

  /*set exif indo*/
  OMX_GetExtensionIndex(pHandle, "omx.qcom.jpeg.exttype.exif", &type);
  tag.tag_id = EXIFTAGID_GPS_LONGITUDE_REF;
  tag.tag_entry.type = EXIF_ASCII;
  tag.tag_entry.count = 2;
  tag.tag_entry.copy = 1;
  tag.tag_entry.data._ascii = "se";
  OMX_SetParameter(pHandle, type, &tag);

  tag.tag_id = EXIFTAGID_GPS_LONGITUDE;
  tag.tag_entry.type = EXIF_RATIONAL;
  tag.tag_entry.count = 1;
  tag.tag_entry.copy = 1;
  tag.tag_entry.data._rat.num= 31;
  tag.tag_entry.data._rat.denom = 1;
  OMX_SetParameter(pHandle, type, &tag);

  /*Set thumbnail data*/
  if (test_args.encode_thumbnail) {
    OMX_GetExtensionIndex(pHandle, "omx.qcom.jpeg.exttype.thumbnail", &type);

    thumbnail.width = test_args.thumbnail.width;
    thumbnail.height = test_args.thumbnail.height;
    thumbnail.scaling = test_args.tn_scale_cfg.enable;
    thumbnail.cropWidth = test_args.tn_scale_cfg.input_width;
    thumbnail.cropHeight = test_args.tn_scale_cfg.input_height;
    thumbnail.left = test_args.tn_scale_cfg.h_offset;
    thumbnail.top = test_args.tn_scale_cfg.v_offset;
    OMX_SetParameter(pHandle, type, &thumbnail);
  }

  /*Set Thumbnail quality*/
  OMX_GetExtensionIndex(pHandle, "omx.qcom.jpeg.exttype.thumbnail_quality",
                        &thumbnailQualityType);
  OMX_GetParameter(pHandle, thumbnailQualityType, &thumbnailQuality);
  thumbnailQuality.nQFactor = test_args.thumbnail.quality;
  OMX_DBG_INFO("Setting thumbnail quality %d\n",test_args.thumbnail.quality);
  OMX_SetParameter(pHandle, thumbnailQualityType, &thumbnailQuality);

  /*Set Main image quality*/
  OMX_IMAGE_PARAM_QFACTORTYPE *mainImageQuality =
    malloc(sizeof(OMX_IMAGE_PARAM_QFACTORTYPE));
  if(!mainImageQuality) {
    if(pmem_info1)
      OMX_MM_FREE(pmem_info1);
    OMX_DBG_ERROR("%s: Error in malloc for mainImageQuality",__func__);
    return 0;
  }
  mainImageQuality->nPortIndex = 0;
  mainImageQuality->nQFactor = test_args.main.quality;
  OMX_DBG_INFO("Setting main image quality %d\n",test_args.main.quality);
  OMX_SetParameter(pHandle,OMX_IndexParamQFactor, mainImageQuality);

  /*Set rotation angle*/
  rotType.nPortIndex = 1;
  rotType.nRotation = test_args.rotation;
  OMX_SetConfig(pHandle, OMX_IndexConfigCommonRotate, &rotType);

  /*Set scaling parameters*/
  recttype.nLeft = test_args.main_scale_cfg.h_offset;
  recttype.nTop = test_args.main_scale_cfg.v_offset;
  recttype.nWidth = test_args.main_scale_cfg.input_width;
  recttype.nHeight = test_args.main_scale_cfg.input_height;
  recttype.nPortIndex = 1;

  OMX_SetConfig(pHandle, OMX_IndexConfigCommonOutputCrop, &recttype);

  OMX_SendCommand(pHandle, OMX_CommandStateSet, OMX_StateIdle, NULL);

  OMX_UseBuffer(pHandle, &pInBuffers, 0, pmem_info, size, buffer);
  OMX_UseBuffer(pHandle, &pInBuffers1, 2, pmem_info1, size_thumb, buffer_thumb);
  /*Allocate output buffer*/
  if (use_pmem) {
    rc = os_pmem_allocate(pmem_fd, size, &outBuffer );
  } else {
      /*Allocate heap memory*/
    outBuffer = (uint8_t *)malloc(size);
    if (!outBuffer) {
      fprintf(stderr, "Cannot allocate memory for the output buffer\n");
      return 1;
    }
  }

  OMX_UseBuffer(pHandle, &pOutBuffers, 1, NULL, size, outBuffer);
  waitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateIdle);
  encoding =1;
  OMX_SendCommand(pHandle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
  waitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateExecuting);
  OMX_EmptyThisBuffer(pHandle, pInBuffers);
  OMX_EmptyThisBuffer(pHandle, pInBuffers1);
  OMX_FillThisBuffer(pHandle, pOutBuffers);
  /*wait for etb*/
  waitForEvent(OMX_EVENT_ETB_DONE , 0, 0);
  /*wait for ftb*/
  waitForEvent(OMX_EVENT_FTB_DONE , 0, 0);

  fprintf(stderr,"JPEG Encode successfull\n");
  return 0;
}

/*===========================================================================
 * FUNCTION    - invokeDeinit -
 *
 * DESCRIPTION: Invoke Deinit on the omc component
 *==========================================================================*/
void invokeDeinit(OMX_HANDLETYPE pHandle ){

  if (encoding) {
    OMX_DBG_INFO("in invokeDeinit\n");
    encoding =0;
    OMX_SendCommand(pHandle, OMX_CommandStateSet, OMX_StateIdle, NULL);

    OMX_SendCommand(pHandle, OMX_CommandStateSet, OMX_StateLoaded, NULL);
    OMX_FreeBuffer(pHandle, 0, pInBuffers);
    OMX_FreeBuffer(pHandle, 0, pInBuffers1);
    OMX_FreeBuffer(pHandle, 1 , pOutBuffers);

    OMX_Deinit();
  }

}
