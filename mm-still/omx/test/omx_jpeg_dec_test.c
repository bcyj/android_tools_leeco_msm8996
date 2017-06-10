/*============================================================================

 Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#include "stdio.h"
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/types.h>
#include "OMX_Types.h"
#include "OMX_Index.h"
#include "OMX_Core.h"
#include "OMX_Component.h"
#include "omx_debug.h"
#include "omx_jpeg_ext.h"
#include <linux/android_pmem.h>
#include <linux/ioctl.h>
#include <jpeg_buffer_private.h>
#include "os_pmem.h"
#include <jpegerr.h>

#define INPUT_PORT 0
#define OUTPUT_PORT 1
#define CMP_SIZE 1024

static OMX_BUFFERHEADERTYPE* pInBuffers;

OMX_CALLBACKTYPE callbacks;
OMX_CONFIG_ROTATIONTYPE rotType;
OMX_CONFIG_RECTTYPE recttype;
OMX_INDEXTYPE user_preferences;
omx_jpeg_user_preferences userpreferences;
OMX_INDEXTYPE region_index;
omx_jpeg_region region;
OMX_INDEXTYPE type;
omx_jpeg_thumbnail thumbnail;
OMX_INDEXTYPE imagetype;
omx_jpeg_type decoder_type;


static pthread_mutex_t lock;
static pthread_cond_t cond;
static int expectedEvent = 0;
static int expectedValue1 = 0;
static int expectedValue2 = 0;
int decoding =0;
char *output_file;
char *thumbnail_file;
int decoded_image_count;

void invokeDeinit(OMX_HANDLETYPE pHandle );
int matchReferenceFile(char *reference_file);

const char color_formats[11][13] =
{
    "YCRCBLP_H2V2",
    "YCBCRLP_H2V2",
    "YCRCBLP_H2V1",
    "YCBCRLP_H2V1",
    "YCRCBLP_H1V2",
    "YCBCRLP_H1V2",
    "YCRCBLP_H1V1",
    "YCBCRLP_H1V1",
    "RGB565",
    "RGB888",
    "RGBa",
};

const char *preference_str[] =
{
    "Hardware accelerated preferred",
    "Hardware accelerated only",
    "Software based preferred",
    "Software based only",
};

typedef struct
{
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;

} rectangle_t;


typedef struct
{
    char                 *input_file;
    char                 *output_file;
    char                 *thumbnail_file;
    omx_jpeg_image_type decoder_type;
    uint32_t              width;
    uint32_t              height;
    omx_jpeg_preference   preference;
//    uint32_t              abort_time;
//    uint32_t              back_to_back_count;
    char                  *reference_file;
    uint8_t               thumbnail_flag;
    uint32_t              stride;
    int32_t               rotation;
    rectangle_t           region;
    omx_jpeg_color_format format;

} test_args_t;


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
    if (decoding) {
      fprintf(stderr,"JPEG Failure!!\n");
      OMX_DBG_ERROR("Jpeg Failure: No FTB Done \n");
      invokeDeinit(pHandle);
      pthread_cond_destroy(&cond);
      pthread_mutex_destroy(&lock);
    }
  }
  return 0;
}

/*===========================================================================
 * FUNCTION    - append_filename -
 *
 * DESCRIPTION: Append filename with an ID to make it unique
 *==========================================================================*/
void append_filename(char *original_name, char **appended_name, int append_id)
{
    // Look for the last occurence of '/' and then last occurence of '.'
    char *s = strrchr(original_name, '/');
    if (s){
        s = strrchr(s, '.');
    }
    else{
        s = strrchr(original_name, '.');
    }

    // Allocate space for appended file name, reserve 5 bytes for additional appending id
    *appended_name = (char *)malloc(5 + strlen(original_name));
    if(!(*appended_name))
        return;
    // Copy original name to appended name
    snprintf(*appended_name, 5 + strlen(original_name), "%s", original_name);
    if (s){
        // If the original file_name contains an file extension, then insert
        // the append_id between the main file name and the file extension
        // eg: test.yuv ==> test_02.yuv
        snprintf(*appended_name + (s - original_name), 5 + strlen(original_name) - (s - original_name), "_%.2d%s", append_id, s);
    }
    else{
        // eg: test ==> test_02
        snprintf(*appended_name, 5 + strlen(original_name) - (s - original_name), "%s_%.2d", *appended_name, append_id);
    }
}


/*===========================================================================
 * FUNCTION    - handleOutput -
 *
 * DESCRIPTION: Handle the decoded output and write it to a file
 *==========================================================================*/
OMX_ERRORTYPE handleOutput(OMX_IN OMX_EVENTTYPE eEvent,int buff_size, uint8_t *pBuffer,
                           int isThumbnail){
   char *filename;
   int fd = 0;

   OMX_DBG_INFO("%s: E", __func__);
   OMX_DBG_INFO("%s: buffer size is %d", __func__,buff_size);

   pthread_mutex_lock(&lock);
   if(isThumbnail) {
        if(decoded_image_count){
           append_filename(thumbnail_file, &filename, decoded_image_count);
        }
        else{
            filename = thumbnail_file;
        }
   }
   else{
         if(decoded_image_count){
            append_filename(output_file, &filename, decoded_image_count);
            decoded_image_count++;
         }
         else{
            filename = output_file;
         }
         decoded_image_count++;
    }

   OMX_DBG_INFO("Handleoutput: acquiredlock: before writing output\n");
   if(filename)
      fd = open(filename, O_RDWR|O_CREAT, 0777);
   if(fd) {
      OMX_DBG_INFO("Handleoutput:Before write filesname= %s\n",filename);
     write(fd, pBuffer,buff_size);

     close(fd);
     pthread_cond_signal(&cond);
     pthread_mutex_unlock(&lock);
   }
   else{
     fprintf(stderr,"Error opening output file %s",filename);
     close(fd);
     pthread_cond_signal(&cond);
     pthread_mutex_unlock(&lock);
     free(filename);
     return 1;
   }
   if(filename)
     free(filename);
   return 0;
}

/*===========================================================================
 * FUNCTION    - etbdone -
 *
 * DESCRIPTION: Handle the OMX_ETB_DONE event
 *==========================================================================*/
OMX_ERRORTYPE etbdone(OMX_OUT OMX_HANDLETYPE hComponent,
        OMX_OUT OMX_PTR pAppData,
        OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer){

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
        OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer){

  char *filename;
  int fd = 0;
  OMX_DBG_INFO("%s: E", __func__);

  if(decoded_image_count){
      append_filename(output_file, &filename, decoded_image_count);
   }
  else {
       filename = output_file;
  }
  decoded_image_count++;

  //write to file
  if(filename) {
    OMX_DBG_ERROR("%s,File name is null",__func__);
    fd = open(filename, O_RDWR|O_CREAT, 0777);
  }
  if(fd){
  write(fd, pBuffer->pBuffer,pBuffer->nFilledLen);
  close(fd);
  }
 /* if (reference_file) {
    matchReferenceFile();
  }*/
  pthread_mutex_lock(&lock);
  expectedEvent = OMX_EVENT_FTB_DONE;
  expectedValue1 = 0;
  expectedValue2 = 0;
  invokeDeinit(hComponent);
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&lock);
  free(filename);
  return 0;

}

/*===========================================================================
**Callback - EventHandler
**Used to notify the IL client when an event of interest occurs within the component.
**The OMX_EVENTTYPE enumeration defines the set of OpenMAX IL events.
 *==========================================================================*/
OMX_ERRORTYPE eventHandler( OMX_IN OMX_HANDLETYPE hComponent,
     OMX_IN OMX_PTR pAppData, OMX_IN OMX_EVENTTYPE eEvent,
     OMX_IN OMX_U32 nData1, OMX_IN OMX_U32 nData2,
     OMX_IN OMX_PTR pEventData){
    OMX_DBG_ERROR("event handler %d %ld %ld", eEvent, nData1, nData2);
    pthread_mutex_lock(&lock);
    expectedEvent = eEvent;
    expectedValue1 = nData1;
    expectedValue2 = nData2;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&lock);
  if (nData1== OMX_EVENT_JPEG_ERROR)
    handleError(eEvent,nData1,hComponent);
  else if(eEvent == OMX_EVENT_THUMBNAIL_IMAGE)
    handleOutput(eEvent,nData1,pEventData,1);
  else if(eEvent == OMX_EVENT_MAIN_IMAGE)
    handleOutput(eEvent,nData1,pEventData,0);

    return 0;
}

/*===========================================================================
* FUNCTION    -WaitForEvent-
* Wait till the completion of an event
*
*==========================================================================*/
void waitForEvent(int event, int value1, int value2 ){
  OMX_DBG_INFO("%s:E", __func__);
    pthread_mutex_lock(&lock);
    while(! (expectedEvent == event &&
            expectedValue1 == value1 && expectedValue2 == value2)){
        pthread_cond_wait(&cond, &lock);
    }
    pthread_mutex_unlock(&lock);
  OMX_DBG_INFO("%s:X", __func__);

}

/*===========================================================================
 * FUNCTION    - matchReferenceFile -
 *
 * DESCRIPTION: Match the gold standard reference file to
 *              the output file
 *==========================================================================*/
int matchReferenceFile(char *reference_file)
{
  char *buf1;
  char *buf2;
  FILE *fref;
  FILE *fout;
  int bytes_read1, bytes_read2;

  fprintf(stderr, "decoder_test: comparing %s with %s...\n",
                output_file, reference_file);
  buf1 = malloc(CMP_SIZE);
  if (!buf1) {
     fprintf(stderr, "decoder_test: failed to malloc for buffer for file comparison\n");
     return -1;
  }
  buf2 = malloc(CMP_SIZE);
  if (!buf2) {
     fprintf(stderr, "decoder_test: failed to malloc for buffer for file comparison\n");
     free(buf1);
     return -1;
  }
  fref = fopen(reference_file, "rb");
  if (!fref){
     fprintf(stderr, "decoder_test: error opening reference file: %s\n", reference_file);
     free(buf1);
     free(buf2);
     return -1;
  }
  fout = fopen(output_file, "rb");
  if (!fout) {
     fprintf(stderr, "decoder_test: failed to malloc for buffer for file comparison\n");
     free(buf1);
     free(buf2);
     return -1;
  }
  for (;;)
  {
     bytes_read1 = (int)fread(buf1, 1, CMP_SIZE, fref);
     bytes_read2 = (int)fread(buf2, 1, CMP_SIZE, fout);
     if (bytes_read1 != bytes_read2 || memcmp(buf1, buf2, bytes_read1))
     {
        fprintf(stderr, "decoder_test: the two files differ.\n");
        free(buf1);
        free(buf2);
        return -1;
     }
     if (feof(fref) && feof(fout))
     {
        fprintf(stderr, "decoder_test: output matches the reference file.\n");
        break;
     }
     if (feof(fref) || feof(fout) || ferror(fref) || ferror(fout))
     {
        fprintf(stderr, "decoder_test: the two files differ.\n");
        free(buf1);
        free(buf2);
        return -1;
     }
  }
  free(buf1);
  free(buf2);
  return 0;
}

/*===========================================================================
 * FUNCTION    - print_usage -
 *
 * DESCRIPTION: Print usage of the test client
 *==========================================================================*/
void print_usage(void)
{
    fprintf(stderr, "Usage: program_name [options]\n");
    fprintf(stderr, "Mandatory options:\n");
    fprintf(stderr, "  -i FILE\t\tPath to the input file.\n");
    fprintf(stderr, "  -o FILE\t\tPath for the output file.\n");
    fprintf(stderr, "  -d IMAGE_TYPE\t\t JPEG(%d -Default), JPS (%d),MPO (%d)\n",OMX_JPEG,OMX_JPS,OMX_MPO);
    fprintf(stderr, "Optional:\n");
    fprintf(stderr, "  -t FILE\t\tPath for the thumbnail file\n");
    fprintf(stderr, "  -w WIDTH\t\tOutput width. (If unset, it will be automatically chosen based on the original dimension)\n");
    fprintf(stderr, "  -h HEIGHT\t\tOutput height. (If unset, it will be automatically chosen based on the original dimension)\n");
    fprintf(stderr, "  -f FORMAT\t\tOutput format: (YCRCBLP_H2V1 is not supported if the input is decoded using H2V2 subsampling.)\n");
    fprintf(stderr, "\t\t\t\tYCRCBLP_H2V2 (%d - Default), YCBCRLP_H2V2 (%d), YCRCBLP_H2V1 (%d), YCBCRLP_H2V1 (%d), RGB565 (%d), RGB888 (%d), RGBA (%d)\n",
                   OMX_YCRCBLP_H2V2, OMX_YCBCRLP_H2V2, OMX_YCRCBLP_H2V1, OMX_YCBCRLP_H2V1, OMX_RGB565, OMX_RGB888, OMX_RGBa);
    fprintf(stderr, "  -p PREFERENCE\t\tPreference on which decoder to use (Software-based or Hardware-accelerated).\n");
    fprintf(stderr, "               \t\t\tHW preferred (0), HW only (1), SW preferred (2), SW only (3)\n");
   // fprintf(stderr, "  -x FILE\t\tPath to the output text file which contains a dump of the EXIF tags found in the file.\n");
   // fprintf(stderr, "  -m FILE\t\tPath to the output text file which contains a dump of the MPO tags found in the file.\n");
   // fprintf(stderr, "  -a ABORT_TIME\t\tThe duration before an abort is issued (in milliseconds).\n");
   // fprintf(stderr, "  -c COUNT\t\tRe-decode back-to-back 'COUNT' times. Default is 1.\n");
    fprintf(stderr, "  -e CONCURRENT_COUNT\tRun COUNT tests in concurrent threads (test for multiple instances of encoder within same process).\n");
    fprintf(stderr, "  -g GOLDEN_REFERENCE\tThe reference file to be matched against the output.\n");
    fprintf(stderr, "  -s STRIDE\t\tSpecify the stride for the output.\n");
    fprintf(stderr, "  -r ROTATION\t\tSpecify the rotation degrees requested (90, 180, 270).\n");
    fprintf(stderr, "  -L REGION LEFT\t\tSpecify the region left coordinate (must be a even #).\n");
    fprintf(stderr, "  -T REGION TOP\t\tSpecify the region top coordinate (must be a even #).\n");
    fprintf(stderr, "  -R REGION RIGHT\t\tSpecify the region right coordinate.\n");
    fprintf(stderr, "  -B REGION BOTTOM\t\tSpecify the region bottom coordinate.\n");
   // fprintf(stderr, "  -l TILING ENABLED\t\tSpecify if the tiling is enabled. (1 to enalble, 0 to disable(default) )\tTiling is only supported when output format is rgb\n");
   // fprintf(stderr, "  -Z NOWRITE_TO_OUTPUT\tDisable write to the output file.\n");

    fprintf(stderr, "\n");
}

/*===========================================================================
 * FUNCTION    - getInput -
 *
 * DESCRIPTION: Get the user input from command line
 *==========================================================================*/
int getInput(int argc, char *argv[], test_args_t  *test_args){

  int c;
  fprintf(stderr, "=============================================================\n");
  fprintf(stderr, "3D Decoder test\n");
  fprintf(stderr, "=============================================================\n");

   // Notice the below colons are not dividers.
    // A colon is attached to its left letter meaning a specific paramter is required.
    // If no parameter is required, no colon is needed, eg: "Z" w/o colon attached.
    while ((c = getopt(argc, argv, "i:o:d:t:w:h:f:p:x:m:a:g:c:e:s:r:L:T:R:B:l:Z")) != -1)
    {
        switch (c)
       {
          case 'i':
            test_args->input_file = optarg;
            fprintf(stderr, "%-25s%s\n", "Input file path", test_args->input_file);
            break;
        case 'o':
            test_args->output_file = optarg;
            fprintf(stderr, "%-25s%s\n", "Output file path", test_args->output_file);
            break;
        case 'd':
            test_args->decoder_type =atoi(optarg);
            if(test_args->decoder_type != OMX_JPEG &&
               test_args->decoder_type != OMX_JPS &&
               test_args->decoder_type != OMX_MPO) {
               fprintf(stderr,"Invalid Input Image format\n");
               return 1;
            }
            fprintf(stderr, "%-25s%d\n", "Decoder Type", test_args->decoder_type);
            break;
        case 't':
            test_args->thumbnail_file = optarg;
            test_args->thumbnail_flag = 1;
            fprintf(stderr, "%-25s%s\n", "Thumbnail file path", test_args->thumbnail_file);
            break;
        case 'w':
            test_args->width = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Output width", test_args->width);
            break;
        case 'h':
            test_args->height = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Output height", test_args->height);
            break;
        case 'f':
            test_args->format = atoi(optarg);
            if (test_args->format != YCRCBLP_H2V2 && test_args->format != YCBCRLP_H2V2 &&
                test_args->format != YCRCBLP_H2V1 && test_args->format != YCBCRLP_H2V1 &&
                test_args->format != RGB565 &&
                test_args->format != RGB888 &&
                test_args->format != RGBa)
            {
                fprintf(stderr, "Invalid output format.\n");
                return 1;
            }
            fprintf(stderr, "%-25s%s\n", "Output format", color_formats[test_args->format]);
            break;
       case 'p':
            test_args->preference = atoi(optarg);
            if (test_args->preference >= OMX_JPEG_PREF_MAX)
            {
                fprintf(stderr, "Invalid decoder preference\n");
                return 1;
            }
            fprintf(stderr, "%-25s%s\n", "Decoder preference", preference_str[test_args->preference]);
            break;
        case 'g':
            test_args->reference_file = optarg;
            fprintf(stderr, "%-25s%s\n", "Reference file", test_args->reference_file);
            break;
        case 's':
            test_args->stride = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Stride", test_args->stride);
            break;
        case 'r':
            test_args->rotation = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Rotation", test_args->rotation);
            break;
       case 'L':
            test_args->region.left = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Region Left", test_args->region.left);
            break;
        case 'T':
            test_args->region.top = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Region Top", test_args->region.top);
            break;
        case 'R':
            test_args->region.right = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Region Right", test_args->region.right);
            break;
        case 'B':
            test_args->region.bottom = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Region Bottom", test_args->region.bottom);
            break;

      }

    }

    // Double check all the required arguments are set
    if (!test_args->input_file || !test_args->output_file)
    {
        fprintf(stderr, "Missing required arguments.\n");
        print_usage();
        return -1;
    }

   return 0;

}

/*===========================================================================
 * FUNCTION    - main -
 *
 * DESCRIPTION:
 *==========================================================================*/

int main(int argc, char* argv[]){

  int c;
  int file_size,rc=0,out_buff_size=0;
  FILE *fd;

  struct ion_allocation_data alloc_data;
  struct ion_fd_data info_fd;
  int ion_fd1;
  struct ion_allocation_data alloc_data1;
  struct ion_fd_data info_fd1;
  jpeg_buf_t *p_buf;
  int stride =0,width =0,height=0;

  int pmem_fd;
  int ion_fd;
  jpeg_buffer_t buffer;
  OMX_U8 * buffer_ptr = 0;
  uint8_t use_pmem = 1;

  OMX_DBG_ERROR("In OMX MPO Decoder test\n");

//Initialize the test argument structure
  test_args_t  test_args;
  memset(&test_args, 0, sizeof(test_args_t));
  test_args.preference = 0;
  test_args.format = 0;
  test_args.decoder_type = OMX_JPEG;

//Get Command line input and fill the structure
  if((c =getInput(argc,argv,&test_args)) != 0){
     return 1;

  }

  output_file = test_args.output_file;
  thumbnail_file = test_args.thumbnail_file;
  stride = test_args.stride;
  width = test_args.width;
  height = test_args.height;
  fprintf(stderr, "decoder_test: Input file to open %s\n",test_args.input_file);

  //Read the entire Input file into a buffer
  fd = fopen(test_args.input_file, "rb");
  if (!fd){
    fprintf(stderr, "decoder_test:Error Opening input file\n");
    return 1;
  }

  // Find out input file length
  fseek(fd, 0, SEEK_END);
  file_size = ftell(fd);
  fseek(fd, 0, SEEK_SET);
  OMX_DBG_ERROR("In OMX MPO Decoder test : input file size is %d\n",file_size);

  if (file_size > 0xffffffff){
    fprintf(stderr, "decoder_test: input file too large for decoder\n");
    fclose(fd);
    return 1;
  }

    rc = jpeg_buffer_init(&buffer);
    if(JPEG_SUCCEEDED(rc)){
      rc = jpeg_buffer_allocate(buffer,file_size,use_pmem);
      OMX_DBG_ERROR("In OMX MPO Decoder test :In os_pmem_allocate\n");
  }

  if (JPEG_FAILED(rc)){
      jpeg_buffer_destroy(&buffer);
      fprintf(stderr, "decoder_test: Buffer allocate failed to read input\n");
      return 1;
  }

  jpeg_buffer_get_addr(buffer, &buffer_ptr);
  OMX_DBG_ERROR("In OMX MPO Decoder test: os_pmem_get_phy_addr. buffer_ph_ is %p\n",buffer_ptr);
  OMX_DBG_ERROR("In OMX MPO Decoder test: buffer is %p\n",buffer);
  fread(buffer_ptr,1,file_size,fd);
  fclose(fd);


  callbacks.EmptyBufferDone = etbdone;
  callbacks.EventHandler = eventHandler;

  pthread_mutex_init(&lock, NULL);
  pthread_cond_init(&cond, NULL);

  OMX_HANDLETYPE pHandle = NULL;

  //get handle to the omx decoder component
  OMX_DBG_ERROR("Invoking decoder omx component\n");
  OMX_ERRORTYPE ret = OMX_GetHandle(&pHandle,
                         "OMX.qcom.image.jpeg.decoder",
                         NULL,
                         &callbacks);

  if (ret != OMX_ErrorNone) {
     OMX_DBG_ERROR("%s:L#%d: ret=%d\n", __func__, __LINE__, ret);
     return 0;
  }

  if(pHandle == NULL &&
    ((OMX_COMPONENTTYPE*)pHandle)->FillThisBuffer == NULL) {
     OMX_DBG_ERROR("%s:L#%d: pHandle is NULL\n", __func__, __LINE__);
     return 0;
  }

 //Define input and output ports
  OMX_PARAM_PORTDEFINITIONTYPE * inputPort =
           malloc(sizeof(OMX_PARAM_PORTDEFINITIONTYPE ));
  OMX_PARAM_PORTDEFINITIONTYPE * outputPort =
           malloc(sizeof(OMX_PARAM_PORTDEFINITIONTYPE ));

  inputPort->nPortIndex = 0;
  outputPort->nPortIndex = 1;

  //Getparameter on input and output port
  OMX_GetParameter(pHandle, OMX_IndexParamPortDefinition, inputPort);
  OMX_GetParameter(pHandle, OMX_IndexParamPortDefinition, outputPort);

  //Need to check the parameters
  inputPort->format.image.nFrameWidth = width;
  inputPort->format.image.nFrameHeight = height;
  inputPort->format.image.nStride = stride;
  inputPort->format.image.nSliceHeight = height;
  inputPort->nBufferSize = file_size;

  //Setparameter
  OMX_SetParameter(pHandle, OMX_IndexParamPortDefinition, inputPort);
  omx_jpeg_pmem_info * pmem_info = malloc(sizeof(omx_jpeg_pmem_info));
  pmem_info->fd = &buffer->pmem_fd;
  pmem_info->offset = 0 ;

  OMX_DBG_ERROR("in use input buffer %p %d %d", buffer ,pmem_info->fd, file_size );

  OMX_SetParameter(pHandle, OMX_IndexParamPortDefinition, inputPort);

  //Set Rotation Parameter
  rotType.nPortIndex = 1;
  rotType.nRotation = test_args.rotation;
  OMX_SetConfig(pHandle, OMX_IndexConfigCommonRotate, &rotType);

  //Set Image type
  decoder_type.image_type = test_args.decoder_type;
   OMX_DBG_ERROR("%s Before set parameter decoder type is %d\n",__func__, decoder_type.image_type);
  OMX_GetExtensionIndex(pHandle,"omx.qcom.jpeg.exttype.image_type",
                        &imagetype);
  OMX_SetParameter(pHandle,imagetype,&decoder_type);

  //Set color format and prefeence*/
  userpreferences.color_format = test_args.format;
  userpreferences.preference = test_args.preference;
  OMX_GetExtensionIndex(pHandle,"omx.qcom.jpeg.exttype.user_preferences",
                        &user_preferences);
  OMX_SetParameter(pHandle,user_preferences,&userpreferences);

  //Set the region parameters
  region.left = test_args.region.left;
  region.right = test_args.region.right;
  region.top = test_args.region.top;
  region.bottom = test_args.region.bottom;
  OMX_GetExtensionIndex(pHandle,"omx.qcom.jpeg.exttype.region",&region_index);
  OMX_SetParameter(pHandle,region_index,&region);

  //Set Thumbnail flag to true in the omx component
  if(test_args.thumbnail_flag){
       OMX_GetExtensionIndex(pHandle, "omx.qcom.jpeg.exttype.thumbnail", &type);
       OMX_SetParameter(pHandle, type, &thumbnail);
   }

  //Change state to Idle
  OMX_SendCommand(pHandle, OMX_CommandStateSet, OMX_StateIdle, NULL);
  //Assign buffers to the OMX component to be used as input buffers.
    OMX_UseBuffer(pHandle, &pInBuffers, 0, pmem_info, file_size, &buffer->ptr);

    waitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateIdle);
    decoding =1;
    OMX_SendCommand(pHandle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    waitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateExecuting);

    OMX_EmptyThisBuffer(pHandle, pInBuffers);

    //wait for etb
    waitForEvent(OMX_EVENT_ETB_DONE , 0, 0);
    //wait for event done
    waitForEvent(OMX_EVENT_DONE , 0, 0);

    if(test_args.reference_file){
         matchReferenceFile(test_args.reference_file);
    }

    fprintf(stderr,"Decode Successfull\n");
    return 0;

}

/*===========================================================================
 * FUNCTION    - invokeDeinit -
 *
 * DESCRIPTION: Invoke Deinit on the omc component
 *==========================================================================*/
void invokeDeinit(OMX_HANDLETYPE pHandle ){

  if (decoding) {
    OMX_DBG_INFO("in invokeDeinit\n");
    decoding =0;
    OMX_SendCommand(pHandle, OMX_CommandStateSet, OMX_StateIdle, NULL);
    OMX_SendCommand(pHandle, OMX_CommandStateSet, OMX_StateLoaded, NULL);
    OMX_FreeBuffer(pHandle, 0, pInBuffers);

    OMX_Deinit();
  }

}
