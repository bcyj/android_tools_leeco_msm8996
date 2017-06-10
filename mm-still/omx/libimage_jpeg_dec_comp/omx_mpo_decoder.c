/*============================================================================
Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "omx_component.h"

#include "mpod.h"

static mpod_obj_t mpo_decoder;
static mpod_src_t mpod_source;

int mpoDecode(omx_jpegd_comp * comp, omx_jpegd_input_buffer *inputBuffer);
int mpodStop(omx_jpegd_comp *comp);
int decodeMPOImages(omx_jpegd_comp * comp, mpo_hdr_t *header);
int mpod_handle_output(omx_jpegd_comp *comp, mpod_dst_t * mpod_dest,
                       mpod_output_buf_t *mpod_dest_buffer, omx_jpeg_decoded_image_type image_type);
static int decoded_image_count =0;



/*----------------------------------------------------------------------------
* Function : mpo_decoder_engine_create
*
* Description: Decode the MPO image
------------------------------------------------------------------------------*/
void configure_mpo_decoder(omx_jpeg_decoder_engine *p_obj){
  OMX_DBG_INFO("%s: E", __func__);
  p_obj->decode = &mpoDecode;
  //p_obj->configure_output_buffer = &configure_MPO_OutputBuffer;
  p_obj->stop = &mpodStop;
  OMX_DBG_INFO("%s: X", __func__);
}

/*--------------------------------------------------------------------------
* Function : configure_MPOD_OutputBuffer
*
* Description: This function configures the output buffer to be used
*              during decoding by the decoder.
*
---------------------------------------------------------------------------
int configure_MPO_OutputBuffer(omx_jpegd_comp * comp){

    OMX_DBG_INFO("%s: E", __func__);

    omx_jpegd_output_buffer * buffer =
            OMX_MM_MALLOC(sizeof(omx_jpegd_output_buffer));
    OMX_MM_ZERO(buffer, omx_jpegd_output_buffer);
     OMX_BUFFERHEADERTYPE * bufferHeader = OMX_MM_MALLOC(
            sizeof(OMX_BUFFERHEADERTYPE));
    OMX_MM_ZERO(bufferHeader, OMX_BUFFERHEADERTYPE);
    bufferHeader->nSize = sizeof(OMX_BUFFERHEADERTYPE);
    bufferHeader->nVersion = version;
    bufferHeader->nFilledLen = 0;
    bufferHeader->nOffset = 0;
    bufferHeader->pOutputPortPrivate = comp->outPort;
    bufferHeader->pPlatformPrivate = buffer;
    buffer->outputHeader = bufferHeader;
    buffer->fd = -1;
    buffer->offset = 0;

    comp->outputBuffer = buffer;
    comp->outPort->bPopulated = OMX_TRUE;

    return 0;
}*/


/*-------------------------------------------------------------------------------------------------
* Function : mpod_output_produced_handler
*
* Description: JPEGD Output handler - consumes the output buffer
               Used only when tiling is enabled
---------------------------------------------------------------------------------------------------*/
int mpod_output_produced_handler(void *p_user_data,
                                 mpod_output_buf_t  *buffer,
                                 uint32_t            first_row_id,
                                 uint8_t             is_last_buffer)
{

 OMX_DBG_INFO("OMX MPO DECODER Component: In Output Produces handler\n");

 return 0;

}
/*-------------------------------------------------------------------------------------------------
* Function : mpoDecode
*
* Description: Decode the MPO image
---------------------------------------------------------------------------------------------------*/
int mpoDecode(omx_jpegd_comp * comp, omx_jpegd_input_buffer *inputBuffer){
    int rc =0;
    mpo_hdr_t header;
    uint32_t num_images, image_index =0;
    mp_entry_t  mp_entry[8];
    uint8_t use_pmem = true;

    OMX_DBG_ERROR("%s : E\n", __func__);

    comp->inputBuffer = inputBuffer;

    //Initialize the decoder
    rc = mpod_init(&mpo_decoder, jpegd_event_handler, mpod_output_produced_handler, comp);
    ONERROR(rc, errorHandler());

    OMX_DBG_INFO("%s :mpod init successful\n",__func__);
    //Set Source Information
    mpod_source.p_input_req_handler = &jpegd_input_req_handler;
    mpod_source.total_length = comp->inputBuffer->length;

    rc = jpeg_buffer_init(&mpod_source.buffers[0]);
    ONERROR(rc, errorHandler());

    rc = jpeg_buffer_init(&mpod_source.buffers[1]);
    ONERROR(rc, errorHandler());

   OMX_DBG_INFO("%s: mpod_source.buffers[0] is at %p\n",__func__,&mpod_source.buffers[0]);

    if (JPEG_SUCCEEDED(rc))
    {
       rc = jpeg_buffer_allocate(mpod_source.buffers[0], 0xA000, use_pmem);

    }
    if(JPEG_SUCCEEDED(rc))
    {
       rc = jpeg_buffer_allocate(mpod_source.buffers[1], 0xA000, use_pmem);
    }
    if (JPEG_FAILED(rc))
    {
       OMX_DBG_ERROR("%s : jpeg_buffer_allocate failed\n",__func__);
       jpeg_buffer_destroy(&mpod_source.buffers[0]);
       jpeg_buffer_destroy(&mpod_source.buffers[1]);
       return 1;
    }

    rc = mpod_set_source(mpo_decoder, &mpod_source);
    if (JPEG_FAILED(rc))
    {
        fprintf(stderr, "%s: mpod_set_source failed\n",__func__);
        return 1;
    }

   /****************************************************************************
     * Decode the first Individual Image in the MPO
    *****************************************************************************/

    // Parse header from the first individual image
    rc = mpod_read_first_header(mpo_decoder, &header);
    if (JPEG_FAILED(rc))
    {
        fprintf(stderr, "mpod_read_header_first failed\n");
        return 1;
    }

     // Print out the frame information
    fprintf(stderr, "First Image Main dimension: (%dx%d) subsampling: (%d)\n",
            header.main.width, header.main.height, (int)header.main.subsampling);
    OMX_DBG_INFO("%s :First Image Main dimension: (%dx%d) subsampling: (%d)\n",
        __func__,header.main.width, header.main.height, (int)header.main.subsampling);

    // Get the number of images in MPO
    rc = mpod_get_num_image(mpo_decoder, &num_images);
    if (JPEG_FAILED(rc))
    {
        fprintf(stderr, "OMX MPOD Component: mpod_get_num_image failed\n");
        return 1;
    }

    fprintf(stderr, "Number of images in MPO: %d\n",num_images);
//    comp->imageCount = num_images;
    comp->totalImageCount = num_images;


     // Extract MP_ENTRY values
    rc = mpod_get_mp_entry_value(mpo_decoder, mp_entry, num_images);
    if (JPEG_FAILED(rc))
    {
        fprintf(stderr, "OMX MPOD Component: mpod_get_mp_entry_value failed\n");
        return 1;
    }

    //Decode the first image
    decodeMPOImages(comp,&header);

     /**************************************************************************
    *  Decode the remaining images in MPO...
    **************************************************************************/
     for (image_index = 1; image_index < num_images; image_index++){

          // Move the starting pointer in the input buffer to the beginning of the new image
        rc = mpod_set_input_offset(mpo_decoder, mp_entry[image_index].data_offset);
        if (JPEG_FAILED(rc))
        {
            fprintf(stderr, "%s: mpod_reset_input_offset failed\n",__func__);
            return 1;
        }

        rc = mpod_read_header(mpo_decoder, &header);
        if (JPEG_FAILED(rc))
        {
            fprintf(stderr, "%s: mpo_read_header failed rc =%d\n",__func__,rc);
            return 1;
        }

         // Print out dimension and subsampling information
        fprintf(stderr, "Main Image dimension: (%dx%d) subsampling: (%d)\n",
                header.main.width, header.main.height, (int)header.main.subsampling);

        //Decoe the images
        decodeMPOImages(comp,&header);

    }
   return 0;
}



int decodeMPOImages(omx_jpegd_comp * comp, mpo_hdr_t *header){

    int rc=0;
    uint32_t output_width;
    uint32_t output_height;
    int output_buffers_count =1;
    uint32_t stride;
    uint8_t use_pmem = true;
    mpod_cfg_t mpod_config;
    mpod_dst_t mpod_dest, mpod_dest_thumb;
    mpod_output_buf_t mpod_dest_buffer,mpod_thumb_dest_buffer;

    omx_jpegd_output_buffer * outputBuffer = comp->outputBuffer;
     /*-------------------------------------------------------------------
      *                     Decode Thumbnail
      *-------------------------------------------------------------------*/

     //Check if the image has a thumbnail
     if(comp->thumbnailPresent){
          if (header->thumbnail.height != 0 && header->thumbnail.width != 0)
          {
            fprintf(stderr,"thumbnail dimension : (%dx%d)\n",header->thumbnail.width,
                     header->thumbnail.height);

           comp->totalImageCount++;
             // thumbnail output configuration
           mpod_dest_thumb.width = header->thumbnail.width;
           mpod_dest_thumb.height= header->thumbnail.height;
           mpod_dest_thumb.output_format = RGB888;
           mpod_dest_thumb.stride        = 0;

           mpod_dest_thumb.region.left   = 0;
           mpod_dest_thumb.region.top    = 0;
           mpod_dest_thumb.region.right  = 0;
           mpod_dest_thumb.region.bottom = 0;


           jpeg_buffer_init(&mpod_thumb_dest_buffer.data.rgb.rgb_buf);

            // Assign 0 to tile width and height
            // to indicate that no tiling is requested for thumbnail.
            mpod_thumb_dest_buffer.tile_width  = 0;
            mpod_thumb_dest_buffer.tile_height = 0;

            stride = mpod_dest_thumb.width * 3;

            //Output buffer allocation
            jpeg_buffer_allocate(mpod_thumb_dest_buffer.data.rgb.rgb_buf,stride* mpod_dest_thumb.height, use_pmem);

            // config decoder to decode thumbnail.
            memset(&mpod_config, 0, sizeof(mpod_cfg_t));

            mpod_config.preference = comp->preferences.preference;
            mpod_config.decode_from = JPEGD_DECODE_FROM_THUMB;
            mpod_config.rotation = 0;

            //Start decoding
             comp->decoding = true;
             rc = mpod_start(mpo_decoder,
                            &mpod_config,
                            &mpod_dest_thumb,
                            &mpod_thumb_dest_buffer,
                            1);

             if (JPEG_FAILED(rc))
            {
                fprintf(stderr, "decoder_test thumnail: mpod_start failed for thumbnail\n");
                return 1;
            }
            fprintf(stderr, "decoder_test thumbnail: mpod_start succeeded for thumbnail\n");

             //Wait until decoding is done or stopped due to error
            pthread_mutex_lock(&comp->mLock);
            while(comp->decoding)
            {
               pthread_cond_wait(&comp->cond,&comp->mLock);
            }
            pthread_mutex_unlock(&comp->mLock);


            if(comp->decode_success){
                 mpod_handle_output(comp,&mpod_dest_thumb,&mpod_thumb_dest_buffer,
                                    OMX_JPEGD_THUMBNAIL_IMAGE);

            }
           else{
                fprintf(stderr, "Decode failed for thumbnail\n");
                return 1;
           }

            // clean up buffers
         jpeg_buffer_destroy(&mpod_thumb_dest_buffer.data.rgb.rgb_buf);
       }
       else{

            fprintf(stderr, "decoder_test: Jpeg file does not contain thumbnail \n");
       }

     }

     /*-------------------------------------------------------------------------
      *               Decode Main Image
      *-------------------------------------------------------------------------*/
      mpod_dest.width         = comp->inPort->format.image.nFrameWidth ?
                                              comp->inPort->format.image.nFrameWidth :header->main.width;
      mpod_dest.height        = comp->inPort->format.image.nFrameHeight ?
                                              comp->inPort->format.image.nFrameHeight :header->main.height;
      mpod_dest.output_format = comp->preferences.color_format;
      mpod_dest.stride        = comp->inPort->format.image.nStride;
      mpod_dest.region.left   = comp->region.left;
      mpod_dest.region.top    = comp->region.top;
      mpod_dest.region.right  = comp->region.right;
      mpod_dest.region.bottom = comp->region.bottom;


      OMX_DBG_INFO("%s : rotation is %d\n",__func__,comp->rotation);
     // if region is defined, re-assign the output width/height
       output_width  = mpod_dest.width;
       output_height = mpod_dest.height;

       if(comp->region.right || comp->region.bottom){
          if(comp->rotation ==0 || comp->rotation == 180){

             output_width = MIN(mpod_dest.width,
                                (uint32_t)(mpod_dest.region.right-mpod_dest.region.left+1));
             output_height = MIN(mpod_dest.height,
                                (uint32_t)(mpod_dest.region.bottom - mpod_dest.region.top  + 1));

          }
          else if(comp->rotation == 90 || comp->rotation == 270){

               output_height = MIN(mpod_dest.height,
                                   (uint32_t)(mpod_dest.region.right-mpod_dest.region.left+1));
               output_width =  MIN(mpod_dest.width,
                                   (uint32_t)(mpod_dest.region.bottom - mpod_dest.region.top  + 1));
          }
          else{
               OMX_DBG_ERROR("Unsupported rotation cases\n");
               return 1;
          }

       }

      //For Formats YCRCBLP_H2V2,YCBCRLP_H2V2,YCRCBLP_H2V1 and YCBCRLP_H2V1
       if (mpod_dest.output_format == YCRCBLP_H2V2 || mpod_dest.output_format == YCBCRLP_H2V2 ||
           mpod_dest.output_format == YCRCBLP_H2V1 || mpod_dest.output_format == YCBCRLP_H2V1){
                jpeg_buffer_init(&mpod_dest_buffer.data.yuv.luma_buf);
                jpeg_buffer_init(&mpod_dest_buffer.data.yuv.chroma_buf);
        }
       else{
             jpeg_buffer_init(&mpod_dest_buffer.data.rgb.rgb_buf);

       }
       if (mpod_dest.stride == 0)
       {
           switch(mpod_dest.output_format){

           case OMX_YCRCBLP_H2V2:
           case OMX_YCBCRLP_H2V2:
           case OMX_YCRCBLP_H2V1:
           case OMX_YCBCRLP_H2V1:
                stride = output_width;
           break;

           case RGB565:
            stride = output_width * 2;
            break;

           case RGB888:
            stride = output_width * 3;
            break;

           case RGBa:
             stride = output_width * 4;
             break;

           default:
            return 1;
           }
        }
        else{
              stride = mpod_dest.stride;
        }

       comp->inPort->format.image.nStride = stride;


      //Tiling is not supported. Set width and height to zero
      mpod_dest_buffer.tile_width  = 0;
      mpod_dest_buffer.tile_height = 0;

      //Allocate buffer depending on the format
      switch (mpod_dest.output_format){
        case OMX_YCRCBLP_H2V2:
        case OMX_YCBCRLP_H2V2:
           OMX_DBG_INFO(" Stride is %d\n",stride);
           jpeg_buffer_allocate(mpod_dest_buffer.data.yuv.luma_buf,
                                stride * output_height * 3 / 2,
                                use_pmem);
           jpeg_buffer_attach_existing(mpod_dest_buffer.data.yuv.chroma_buf,
                                       mpod_dest_buffer.data.yuv.luma_buf,
                                       stride * output_height);
           break;

       case OMX_YCRCBLP_H2V1:
       case OMX_YCBCRLP_H2V1:
          jpeg_buffer_allocate(mpod_dest_buffer.data.yuv.luma_buf,
                               stride * output_height * 2, use_pmem);
          jpeg_buffer_attach_existing(mpod_dest_buffer.data.yuv.chroma_buf,
                                      mpod_dest_buffer.data.yuv.luma_buf,
                                      stride * output_height);
          break;

       case RGB565:
       case RGB888:
       case RGBa:
            // No tiling is requested, allocate the output buffer to hold the whole image to be dumped
            jpeg_buffer_allocate(mpod_dest_buffer.data.rgb.rgb_buf, stride * output_height, use_pmem);
        break;

      default:
        fprintf(stderr, "%s: unsupported output format\n",__func__);
        return 1;
    }

       // Set up configuration
    memset(&mpod_config, 0, sizeof(mpod_cfg_t));
    mpod_config.preference  = comp->preferences.preference;
    mpod_config.decode_from = JPEGD_DECODE_FROM_AUTO;
    mpod_config.rotation    = comp->rotation;

    //Start Decoding
    comp->decoding =true;
    rc = mpod_start(mpo_decoder,
                    &mpod_config,
                    &mpod_dest,
                    &mpod_dest_buffer,
                    output_buffers_count);

    if (JPEG_FAILED(rc))
    {
       fprintf(stderr, "OMX MPO Decoder Component: mpod_start failed\n");
       return 0;
    }

    fprintf(stderr, "OMX MPO Decoder Component: mpod_start succeeded\n");


    //Wait until decoding is done or stopped due to error
    pthread_mutex_lock(&comp->mLock);
    while(comp->decoding)
    {
      pthread_cond_wait(&comp->cond,&comp->mLock);
    }
    pthread_mutex_unlock(&comp->mLock);


    if(comp->decode_success){
       OMX_DBG_INFO("OMX MPO Decoder Component:Decode succesfull.. Writing out to output buffer\n");
       mpod_handle_output(comp, &mpod_dest, &mpod_dest_buffer,OMX_JPEGD_MAIN_IMAGE);

    }
    else{
        fprintf(stderr, "OMX MPO Decoder Component: Decode failed\n");
        return 1;
   }

  // Clean the output buffers each time a JPEG image has been decoded
   switch (mpod_dest.output_format){
    case YCRCBLP_H2V2:
    case YCBCRLP_H2V2:
    case YCRCBLP_H2V1:
    case YCBCRLP_H2V1:
        jpeg_buffer_destroy(&mpod_dest_buffer.data.yuv.luma_buf);
        jpeg_buffer_destroy(&mpod_dest_buffer.data.yuv.chroma_buf);
        break;
    case RGB565:
    case RGB888:
    case RGBa:
        jpeg_buffer_destroy(&mpod_dest_buffer.data.rgb.rgb_buf);
        break;
    default:
        break;
  }


 return 0;
}


/*-------------------------------------------------------------------------------------------------
* Function : mpod_handle_output
*
* Description: Handle output priduced
---------------------------------------------------------------------------------------------------*/
int mpod_handle_output(omx_jpegd_comp *comp, mpod_dst_t * mpod_dest,
                       mpod_output_buf_t *mpod_dest_buffer, omx_jpeg_decoded_image_type image_type)
{

    uint32_t luma_buf_size, chroma_buf_size, full_buffer_size =0;
    uint8_t *buf_ptr;
    int stride;
    FILE *fout;
    omx_jpeg_queue_item item;
  //  uint8_t* outBuffer;

    OMX_DBG_INFO("OMX MPO Decoder Component:In mpod_handle_output\n");

    if(mpod_dest->stride ==0){
       switch(mpod_dest->output_format) {
         case YCRCBLP_H2V2:
         case YCBCRLP_H2V2:
         case YCRCBLP_H2V1:
         case YCBCRLP_H2V1:
              stride = mpod_dest->width;
              break;
        case RGB565:
             stride = mpod_dest->width * 2;
             break;
        case RGB888:
             stride = mpod_dest->width * 3;
             break;
        case RGBa:
             stride = mpod_dest->width * 4;
             break;
        default:
             return 1;
      }
    }
     else
    {
        stride = mpod_dest->stride;
    }

    switch(mpod_dest->output_format) {
       case YCRCBLP_H2V2:
       case YCBCRLP_H2V2:
               luma_buf_size = stride * mpod_dest->height;
               chroma_buf_size = stride * mpod_dest->height / 2;
               full_buffer_size = luma_buf_size + chroma_buf_size;
               OMX_DBG_INFO("%s Format YCRCBLP_H2V2/YCBCRLP_H2V2 luma_buf_size = %d "
                            "chroma_buf_size = %d\n",__func__,luma_buf_size,chroma_buf_size);
               //Allocate a buffer dynamically
               comp->outBuffer[decoded_image_count] = OMX_MM_MALLOC(full_buffer_size);
               if(comp->outBuffer[decoded_image_count]){
                    jpeg_buffer_get_addr(mpod_dest_buffer->data.yuv.luma_buf, &buf_ptr);
                    memcpy(comp->outBuffer[decoded_image_count], buf_ptr, luma_buf_size);

                   jpeg_buffer_get_addr(mpod_dest_buffer->data.yuv.chroma_buf,&buf_ptr);
                   memcpy(comp->outBuffer[decoded_image_count]+luma_buf_size, buf_ptr,
                          chroma_buf_size);
               }
               else{
                    OMX_DBG_ERROR("%s :Buffer allocation failed!!!!\n",__func__);
                    return 1;
               }
         break;
        case YCRCBLP_H2V1:
        case YCBCRLP_H2V1:
               luma_buf_size = stride * mpod_dest->height;
               chroma_buf_size = stride * mpod_dest->height ;
               full_buffer_size = luma_buf_size + chroma_buf_size;
               OMX_DBG_INFO("%s Format YCRCBLP_H2V1/YCBCRLP_H2V1 luma_buf_size = %d "
                            "chroma_buf_size = %d\n",__func__,luma_buf_size,chroma_buf_size);
               //Allocate a buffer dynamically
               comp->outBuffer[decoded_image_count] = OMX_MM_MALLOC(full_buffer_size);
               if(comp->outBuffer[decoded_image_count]){
                    jpeg_buffer_get_addr(mpod_dest_buffer->data.yuv.luma_buf, &buf_ptr);
                    memcpy(comp->outBuffer[decoded_image_count], buf_ptr, luma_buf_size);

                   jpeg_buffer_get_addr(mpod_dest_buffer->data.yuv.chroma_buf,&buf_ptr);
                   memcpy(comp->outBuffer[decoded_image_count]+luma_buf_size, buf_ptr,
                          chroma_buf_size);
               }
               else{
                    OMX_DBG_ERROR("%s :Buffer allocation failed!!!!\n",__func__);
                    return 1;
               }
         break;
        case RGB565:
        case RGB888:
        case RGBa:
             full_buffer_size = stride * mpod_dest->height;
             OMX_DBG_INFO("%s Format RGB565/RGB888/RGBa full_size =%d\n",
                          __func__,full_buffer_size);
             //Allocate a buffer dynamically
               comp->outBuffer[decoded_image_count] = OMX_MM_MALLOC(full_buffer_size);
               if(comp->outBuffer[decoded_image_count]){
                  jpeg_buffer_get_addr(mpod_dest_buffer->data.rgb.rgb_buf, &buf_ptr);
                  memcpy(comp->outBuffer[decoded_image_count], buf_ptr, full_buffer_size);
               }
               else{
                    OMX_DBG_ERROR("%s :Buffer allocation failed!!!!\n",__func__);
                    return 1;
               }
         break;
       default:
             break;
    }

      if(image_type == OMX_JPEGD_THUMBNAIL_IMAGE) {
         item.message = OMX_JPEG_MESSAGE_DECODED_IMAGE;
         item.args[0].iValue = OMX_EVENT_THUMBNAIL_IMAGE;
         item.args[1].iValue = full_buffer_size;
         item.args[1].pValue = comp->outBuffer[decoded_image_count];
         item.args[2].iValue = 0;
         postMessage(comp->queue, OMX_JPEG_QUEUE_COMMAND, &item);
      }
      else if(image_type == OMX_JPEGD_MAIN_IMAGE) {
         item.message = OMX_JPEG_MESSAGE_DECODED_IMAGE;
         item.args[0].iValue = OMX_EVENT_MAIN_IMAGE;
         item.args[1].iValue = full_buffer_size;
         item.args[1].pValue = comp->outBuffer[decoded_image_count];
         item.args[2].iValue = 0;
         postMessage(comp->queue, OMX_JPEG_QUEUE_COMMAND, &item);

       }

       decoded_image_count++;
       OMX_DBG_ERROR("%s : Image %d receirved from the decoder\n",__func__,decoded_image_count);

    if(decoded_image_count == comp->totalImageCount){
       jpegdInvokeStop(comp);
       item.message = OMX_JPEG_MESSAGE_EVENT;
       item.args[0].iValue = OMX_EVENT_DONE;
       item.args[1].iValue = 0;
       item.args[2].iValue = 0;
       postMessage(comp->queue, OMX_JPEG_QUEUE_COMMAND, &item);
    }
       return 0;
}



int mpodStop(omx_jpegd_comp *comp){

   int rc=0;
   OMX_DBG_INFO("OMX MPOD Component: In MpodStop\n");

   if(comp->decoding){
   rc = mpod_abort(mpo_decoder);;
   }

   ONERROR(rc, errorHandler());

   jpeg_buffer_destroy(&mpod_source.buffers[0]);
   jpeg_buffer_destroy(&mpod_source.buffers[1]);

   mpod_destroy(&mpo_decoder);
   comp->decoding = 0;
   OMX_DBG_INFO("%s :X\n",__func__);

   return 0;

}
