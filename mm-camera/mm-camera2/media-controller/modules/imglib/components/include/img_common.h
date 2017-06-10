/***************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
****************************************************************************/

#ifndef __IMG_COMMON_H__
#define __IMG_COMMON_H__

#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <linux/msm_ion.h>

/**
 * CONSTANTS and MACROS
 **/
#define MAX_PLANE_CNT 3
#define MAX_FRAME_CNT 2
#define GAMMA_TABLE_ENTRIES 64
#define RNR_LUT_SIZE 164

#define IMG_MAX_INPUT_FRAME 8
#define IMG_MAX_OUTPUT_FRAME 1
#define IMG_MAX_META_FRAME 8

#undef TRUE
#undef FALSE
#undef MIN
#undef MAX

#define TRUE 1
#define FALSE 0

#define MIN(a,b) ((a)>(b)?(b):(a))
#define MAX(a,b) ((a)<(b)?(b):(a))

#define MIN2(a,b)      ((a<b)?a:b)
#define MIN4(a,b,c,d)  (MIN2(MIN2(a,b),MIN2(c,d)))
#define MAX2(a,b)      ((a>b)?a:b)
#define MAX4(a,b,c,d)  (MAX2(MAX2(a,b),MAX2(c,d)))
#define CLIP(x, lower, upper)  {x = ((x < lower) ? lower : \
                               ((x > upper) ? upper : x)); }

/** BILINEAR_INTERPOLATION
 *
 *   Bilinear interpolation
 **/
#ifndef BILINEAR_INTERPOLATION
#define BILINEAR_INTERPOLATION(v1, v2, ratio) ((v1) + ((ratio) * ((v2) - (v1))))
#endif

/** FLOAT_TO_Q:
 *
 *   convert from float to integer
 **/
#define FLOAT_TO_Q(exp, f) \
  ((int32_t)((f*(1<<(exp))) + ((f<0) ? -0.5 : 0.5)))

/** Round:
 *
 *   Round the value
 **/
#ifndef Round
#define Round(x) (int)(x + sign(x)*0.5)
#endif

/**
 *   indices for semiplanar frame
 **/
#define IY 0
#define IC 1

/**
 *   chroma indices for planar frame
 **/
#define IC1 1
#define IC2 2

/* utility functions to get frame info */
/** IMG_ADDR
 *   @p: pointer to the frame
 *
 *   Returns the Y address from the frame
 **/
#define IMG_ADDR(p) ((p)->frame[0].plane[0].addr)

/** IMG_WIDTH
 *   @p: pointer to the frame
 *
 *   Returns the Y plane width
 **/
#define IMG_WIDTH(p) ((p)->frame[0].plane[0].width)

/** IMG_HEIGHT
 *   @p: pointer to the frame
 *
 *   Returns the Y plane height
 **/
#define IMG_HEIGHT(p) ((p)->frame[0].plane[0].height)

/** IMG_Y_LEN
 *   @p: pointer to the frame
 *
 *   Returns the length of Y plane
 **/
#define IMG_Y_LEN(p) ((p)->frame[0].plane[0].length)

/** IMG_FD
 *   @p: pointer to the frame
 *
 *   Returns the fd of the frame
 **/
#define IMG_FD(p) ((p)->frame[0].plane[0].fd)

/** IMG_FRAME_LEN
 *   @p: pointer to the frame
 *
 *   Returns the fd of the frame
 **/
#define IMG_FRAME_LEN(p) ({ \
  int i = 0, len = 0;; \
  for (i = 0; i < (p)->frame[0].plane_cnt; i++) { \
    len += (p)->frame[0].plane[i].length; \
  } \
  len; \
})

/** Imaging values error values
*    IMG_SUCCESS - success
*    IMG_ERR_GENERAL - any generic errors which cannot be defined
*    IMG_ERR_NO_MEMORY - memory failure ION or heap
*    IMG_ERR_NOT_SUPPORTED -  mode or operation not supported
*    IMG_ERR_INVALID_INPUT - input passed by the user is invalid
*    IMG_ERR_INVALID_OPERATION - operation sequence is invalid
*    IMG_ERR_TIMEOUT - operation timed out
*    IMG_ERR_NOT_FOUND - object is not found
*    IMG_GET_FRAME_FAILED - get frame failed
*    IMG_ERR_OUT_OF_BOUNDS - input to the function is out of
*                           bounds
**/
#define IMG_SUCCESS                   0
#define IMG_ERR_GENERAL              -1
#define IMG_ERR_NO_MEMORY            -2
#define IMG_ERR_NOT_SUPPORTED        -3
#define IMG_ERR_INVALID_INPUT        -4
#define IMG_ERR_INVALID_OPERATION    -5
#define IMG_ERR_TIMEOUT              -5
#define IMG_ERR_NOT_FOUND            -6
#define IMG_GET_FRAME_FAILED         -7
#define IMG_ERR_OUT_OF_BOUNDS        -8
#define IMG_ERR_BUSY                 -9

/** SUBSAMPLE_TABLE
*    @in: input table
*    @in_size: input table size
*    @out: output table
*    @out_size: output table size
*    @QN: number of bits to shift while generating output tables
*
*    Macro to subsample the tables
**/
#define SUBSAMPLE_TABLE(in, in_size, out, out_size, QN) ({ \
  int i, j = 0, inc = (in_size)/(out_size); \
  for (i = 0, j = 0; j < (out_size) && i < (in_size); j++, i += inc) \
    out[j] = ((int32_t)in[i] << QN); \
})

/** IMG_ERROR
*    @v: status value
*
*    Returns true if the status is error
**/
#define IMG_ERROR(v) ((v) != IMG_SUCCESS)

/** IMG_SUCCEEDED
*    @v: status value
*
*    Returns true if the status is success
**/
#define IMG_SUCCEEDED(v) ((v) == IMG_SUCCESS)

/** IMG_LENGTH
*    @size: image size structure
*
*    Returns the length of the frame
**/
#define IMG_LENGTH(size) (size.width * size.height)

/** IMG_CEIL_FL1
*    @x: image to be converted
*
*    Ceil the image to one decimal point.
*    For eq:- 1.12 will be converted to 1.2
**/
#define IMG_CEIL_FL1(x) ((((int)((x) * 10 + .9))/10))

/** IMG_TRANSLATE2
*    @v: value to be converted
*    @s: scale factor
*    @o: offset
*
*    Translate the scale factors w.r.t cordiantes and offset
**/
#define IMG_TRANSLATE2(v, s, o) ((v) * (s) + (o))

/** IMG_TRANSLATE
*    @v: value to be converted
*    @s: scale factor
*    @o: offset
*
*    Translate the scale factors w.r.t cordiantes and offset
**/
#define IMG_TRANSLATE(v, s, o) (((v) - (o)) * s)

/** IMG_DUMP_TO_FILE:
 *  @filename: file name
 *  @p_addr: address of the buffer
 *  @len: buffer length
 *
 *  dump the image to the file
 **/
#define IMG_DUMP_TO_FILE(filename, p_addr, len) ({ \
  int rc = 0; \
  FILE *fp = fopen(filename, "w+"); \
  if (fp) { \
    rc = fwrite(p_addr, 1, len, fp); \
    IDBG_ERROR("%s:%d] written size %d", __func__, __LINE__, len); \
    fclose(fp); \
  } else { \
    IDBG_ERROR("%s:%d] open %s failed", __func__, __LINE__, filename); \
  } \
})


/** IMG_PRINT_RECT:
   *  @p: img rect
   *
   *  prints the crop region
   **/
#define IMG_PRINT_RECT(p) ({ \
  IDBG_MED("%s:%d] crop info (%d %d %d %d)", __func__, __LINE__, \
    (p)->pos.x, \
    (p)->pos.y, \
    (p)->size.width, \
    (p)->size.height); \
})

/** IMG_RECT_IS_VALID:
   *  @p: img rect
   *  @w: width of the main image
   *  @h: height of the main image
   *
   *  check if the region is valid
   **/
#define IMG_RECT_IS_VALID(p, w, h) (((p)->pos.x >= 0) && ((p)->pos.y >= 0) && \
  ((p)->size.width > 0) && ((p)->size.height > 0) && \
  (((p)->pos.x + (p)->size.width) < w) && \
  (((p)->pos.y + (p)->size.height) < h))

/** IMG_F_EQUAL:
 *  @a: floating point input
 *  @b: floating point input
 *
 *  checks if the floating point numbers are equal
 **/
#define IMG_F_EQUAL(a, b) (fabs(a-b) < 1e-4)

/** IMG_SWAP
 *  @a: input a
 *  @b: input b
 *
 *  Swaps the input values
 **/
#define IMG_SWAP(a, b) ({typeof(a) c; c=a; a=b; b=c;})

/** IMG_ALIGN
 *  @a: input a Value
 *  @b: input b Alignment
 *
 *  Align (a) value with (b) Alignment
 **/
#define IMG_ALIGN(a, b) (((a) + (b)) & ~((typeof(a))(b) - 1))

/** IMG_ARRAY_SIZE:
 *    @a: array to be processed
 *
 * Returns number of elements in array
 **/
#define IMG_ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

/**sigma_lut_in
  * Default sigma table for nornal lighting conditions
**/
extern float sigma_lut_in[RNR_LUT_SIZE];

/** IMG_RETURN_IF_NULL
 *   @p: pointer to be checked
 *
 *   Returns if pointer is null
 **/
#define IMG_RETURN_IF_NULL(ret, p) {if (!p) {\
  IDBG_ERROR("%s:%d Null pointer detected %s %p\n",\
    __func__, __LINE__, #p, p);\
  ret;\
}}

/** img_plane_type_t
*    PLANE_Y: Y plane
*    PLANE_CB_CR: C plane for pseudo planar formats
*    PLANE_CB: Cb plane for planar format
*    PLANE_CR: Cr plane for planar format
*    PLANE_Y_CB_CR: YCbCr plane for interleaved format
*
*    Plane type
**/
typedef enum {
  PLANE_Y,
  PLANE_CB_CR,
  PLANE_CB,
  PLANE_CR,
  PLANE_Y_CB_CR
} img_plane_type_t;

/** img_subsampling_t
*    IMG_H4V4 - h4v4 subsampling
*    IMG_H4V4 - h4v2 subsampling
*    IMG_H2V2 - h2v2 subsampling (4:2:0)
*    IMG_H2V1 - h2v1 subsampling (4:2:2)
*    IMG_H1V2 - h1v2 subsampling (4:2:2)
*    IMG_H1V1 - h1v1 subsampling (4:4:4)
*    IMG_HV_MAX - Invalid
*
*    Image subsampling type
**/
typedef enum {
  IMG_H4V4,
  IMG_H4V2,
  IMG_H2V2,
  IMG_H2V1,
  IMG_H1V2,
  IMG_H1V1,
  IMG_HV_MAX,
} img_subsampling_t;

/** img_frame_info_t
*    @width: width of the frame
*    @height: height of the frame
*    @ss: subsampling for the frame
*    @analysis: flag to indicate if this is a analysis frame
*
*    Returns true if the status is success
**/
typedef struct {
  int width;
  int height;
  img_subsampling_t ss;
  int analysis;
} img_frame_info_t;

/** img_plane_t
*    @plane_type: type of the plane
*    @addr: address of the plane
*    @stride: stride of the plane
*    @length: length of the plane
*    @fd: fd of the plane
*    @height: height of the plane
*    @width: width of the plane
*    @offset: offset of the valid data within the plane
*    @scanline: scanline of the plane
*
*    Represents each plane of the frame
**/
typedef struct {
  img_plane_type_t plane_type;
  uint8_t *addr;
  int stride;
  int length;
  int fd;
  int height;
  int width;
  int offset;
  int scanline;
} img_plane_t;

/** img_sub_frame_t
*    @plane_cnt: number of planes
*    @plane: array of planes
*
*    Represents each image sub frame.
**/
typedef struct {
  int plane_cnt;
  img_plane_t plane[MAX_PLANE_CNT];
} img_sub_frame_t;

/** img_frame_t
*    @timestamp: timestamp of the frame
*    @plane: array of planes
*    @frame_cnt: frame count, 1 for 2D, 2 for 3D
*    @idx: unique ID of the frame
*    @frame_id: Frame id
*    @info: frame information
*    @private_data: private data associated with the client
*    @ref_count: ref count of the buffer
*
*    Represents a frame (2D or 3D frame). 2D contains only one
*    sub frame where as 3D has 2 sub frames (left/right or
*    top/bottom)
**/
typedef struct {
  uint64_t timestamp;
  img_sub_frame_t frame[MAX_FRAME_CNT];
  int frame_cnt;
  uint32_t idx;
  uint32_t frame_id;
  img_frame_info_t info;
  void *private_data;
  int ref_count;
} img_frame_t;

/** img_size_t
*    @width: width of the image
*    @height: height of the image
*
*    Represents the image size
**/
typedef struct {
  int width;
  int height;
} img_size_t;

/** img_trans_info_t
 *   @h_scale: horizontal scale ratio to be applied on the
 *           result
 *   @v_scale: vertical scale ratio to be applied on the result.
*    @h_offset: horizontal offset
*    @v_offset: vertical offset
*
*    Translation information for the face cordinates
**/
typedef struct {
  float h_scale;
  float v_scale;
  int h_offset;
  int v_offset;
} img_trans_info_t;

/** img_pixel_t
*    @x: x cordinate of the pixel
*    @y: y cordinate of the pixel
*
*    Represents the image pixel
**/
typedef struct {
  int x;
  int y;
} img_pixel_t;

/** img_rect_t
*    @pos: position of the region
*    @size: size of the region
*
*    Represents the image region
**/
typedef struct {
  img_pixel_t pos;
  img_size_t size;
} img_rect_t;

/** img_gamma_t
*    @table: array of gamma values
*
*    Gamma table of size 64
**/
typedef struct {
  uint16_t table[GAMMA_TABLE_ENTRIES];
} img_gamma_t;


/** img_debug_info_t
*    @camera_dump_enabled: Flag indicating if dump
*        is enabled
*    @timestamp: Timestamp string when buffer was recieved
*
*   Debug Information
**/
typedef struct {
  uint8_t camera_dump_enabled;
  char timestamp[25];
} img_debug_info_t;

/** img_comp_mode_t
 * IMG_SYNC_MODE: The component will be executed in
 *   syncronous mode - per frame.
 *  IMG_ASYNC_MODE: The component will spawn a thread and will
 *  be executed asyncronously in the context of the component
 *  thread.
 *
 **/
typedef enum {
  IMG_SYNC_MODE,
  IMG_ASYNC_MODE,
} img_comp_mode_t;

/** img_caps_t
 *   @num_input: number of input buffers
 *   @num_output: number of output buffers
 *   @num_meta: number of meta buffers
 *   @inplace_algo: Flag to indicate whether the algorithm is
 *                inplace. If not output buffers needs to be
 *                obtained
 *
 *   Capabilities
 **/
typedef struct {
  int8_t num_input;
  int8_t num_output;
  int8_t num_meta;
  int8_t inplace_algo;
} img_caps_t;

/** img_init_params_t
 *    refocus: enable refocus encoding
 *
 *    Frameproc init params
 **/
typedef struct {
  int refocus_encode;
} img_init_params_t;

/** img_frame_ops_t
 *    get_frame: The function pointer to get the frame
 *    release_frame: The function pointer to release the frame
 *    @p_appdata: app data
 *
 *    Frame operations for intermediate buffer
 **/
typedef struct {
  int (*get_frame)(void *p_appdata, img_frame_t **pp_frame);
  int (*release_frame)(void *p_appdata, img_frame_t *p_frame,
    int is_dirty, int is_analysis,
    int is_bitstream, uint32_t size);
  void *p_appdata;
} img_frame_ops_t;

/** face_proc_scale_mn_v_info_t
*    @height: The possiblly cropped input height in whole in
*           pixels (N)
*    @output_height: The required output height in whole in
*                  pixels (M)
*    @step: The vertical accumulated step for a plane
*    @count: The vertical accumulated count for a plane
*    @index: The vertical index of line being accumulated
*    @p_v_accum_line: The intermediate vertical accumulated line
*                   for a plane
*
*    Used for downscaling image
*
**/
typedef struct {
  uint32_t height;
  uint32_t output_height;
  uint32_t step;
  uint32_t count;
  uint16_t *p_v_accum_line;
} img_scale_mn_v_info_t;

// M/N division table in Q10
static const uint16_t mn_division_table[] =
{
  1024,     // not used
  1024,     // 1/1
  512,     // 1/2
  341,     // 1/3
  256,     // 1/4
  205,     // 1/5
  171,     // 1/6
  146,     // 1/7
  128      // 1/8
};

/** img_mmap_info_ion
*    @ion_fd: ION file instance
*    @virtual_addr: virtual address of the buffer
*    @bufsize: size of the buffer
*    @ion_info_fd: File instance for current buffer
*
*    Used for maping data
*
**/
typedef struct img_mmap_info_ion
{
    int               ion_fd;
    unsigned char*   *virtual_addr;
    unsigned int      bufsize;
    struct ion_fd_data ion_info_fd;
} img_mmap_info_ion;

/** img_cache_ops_type
*
*    Different cache operations
*
**/
typedef enum {
  CACHE_INVALIDATE,
  CACHE_CLEAN,
  CACHE_CLEAN_INVALIDATE,
} img_cache_ops_type;

/** img_get_subsampling_factor
*    @ss_type: subsampling type
*    @p_w_factor: pointer to the width subsampling factor
*    @p_h_factor: pointer to height subsampling factor
*
*    Get the width and height subsampling factors given the type
**/
int img_get_subsampling_factor(img_subsampling_t ss_type, float *p_w_factor,
  float *p_h_factor);

/** img_wait_for_completion
*    @p_cond: pointer to pthread condition
*    @p_mutex: pointer to pthread mutex
*    @ms: timeout value in milliseconds
*
*    This function waits until one of the condition is met
*    1. conditional variable is signalled
*    2. timeout happens
**/
int img_wait_for_completion(pthread_cond_t *p_cond, pthread_mutex_t *p_mutex,
  uint32_t ms);

/** img_image_copy:
 *  @out_buff: output buffer handler
 *  @in_buff: input buffer handler
 *
 * Function to copy image data from source to destination buffer
 *
 * Returns IMG_SUCCESS in case of success
 **/
int img_image_copy(img_frame_t *out_buff, img_frame_t *in_buff);

/**
 * Function: img_translate_cordinates
 *
 * Description: Translate the cordinates from one window
 *             dimension to another
 *
 * Input parameters:
 *   dim1 - dimension of 1st window
 *   dim2 - dimension of 2nd window
 *   p_in_region - pointer to the input region
 *   p_out_region - pointer to the output region
 *   zoom_factor - zoom factor
 *   p_zoom_tbl - zoom table
 *   num_entries - number of zoom table entries
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int img_translate_cordinates_zoom(img_size_t dim1, img_size_t dim2,
  img_rect_t *p_in_region, img_rect_t *p_out_region,
  double zoom_factor, const uint32_t *p_zoom_tbl,
  uint32_t num_entries);

/**
 * Function: img_translate_cordinates
 *
 * Description: Translate the region from one window
 *             dimension to another
 *
 * Input parameters:
 *   dim1 - dimension of 1st window
 *   dim2 - dimension of 2nd window
 *   p_in_region - pointer to the input region
 *   p_out_region - pointer to the output region
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_INVALID_INPUT
 *
 * Notes:  none
 **/
int img_translate_cordinates(img_size_t dim1, img_size_t dim2,
  img_rect_t *p_in_region, img_rect_t *p_out_region);

/**
 * Function: img_sw_scale_init_mn
 *
 * Description: init downscaling
 *
 * Input parameters:
 *   vInfo - contains width/height info for scaling
 *   pSrc - pointer to original img buffer
 *   srcWidth - original image width
 *   srcHeight - original image height
 *   srcStride - original image stride
 *   pDst - pointer to scaled image buffer
 *   dstWidth - desired width of schaled image
 *   dstHeight - desired height of scaled image
 *   dstStride - scaled image stride
 *
 * Return values: none
 *
 * Notes:  none
 **/
static void img_sw_scale_init_mn(img_scale_mn_v_info_t*  vInfo,
  uint8_t  *pSrc,
  uint32_t  srcWidth,
  uint32_t  srcHeight,
  uint32_t  srcStride,
  uint8_t  *pDst,
  uint32_t  dstWidth,
  uint32_t  dstHeight,
  uint32_t  dstStride);

/**
 * Function: img_sw_scale_mn_vscale_byte
 *
 * Description: init Vertical M/N scaling on an input lines,
 * which is one byte per pixel
 *
 * Input parameters:
 *   p_v_info - contains width/height info for scaling
 *   p_output_line
 *   output_width
 *   p_input_line
 *
 * Return values:
 *   0 - accumulating
 *   1 - outputting 1 line
 *
 * Notes:  none
 **/
int img_sw_scale_mn_vscale_byte(img_scale_mn_v_info_t *p_v_info,
  uint8_t *p_output_line,
  uint32_t output_width,
  uint8_t *p_input_line);

/**
 * Function: img_sw_scale_mn_hscale_byte
 *
 * Description: init horizontal scaling
 *
 * Input parameters:
 *   p_output_line
 *   output_width - M value
 *   p_input_line
 *   input_width - N value
 *
 * Return values: None
 *
 * Notes:  none
 **/
void img_sw_scale_mn_hscale_byte (uint8_t *p_output_line,
  uint32_t                          output_width,
  uint8_t                          *p_input_line,
  uint32_t                          input_width     );

/**
 * Function: img_sw_downscale
 *
 * Description: Image downscaling using MN method
 *
 * Input parameters:
 *   pSrc - pointer to original img buffer
 *   srcWidth - original image width
 *   srcHeight - original image height
 *   pDst - pointer to scaled image buffer
 *   dstWidth - desired width of schaled image
 *   dstHeight - desired height of scaled image
 *
 * Return values: none
 *
 * Notes:  none
 **/

void img_sw_downscale(uint8_t *src,uint32_t srcWidth, uint32_t srcHeight,
  uint8_t *dst, uint32_t dstWidth,uint32_t dstHeight);

/** img_image_stride_fill:
 *  @out_buff: output buffer handler
 *
 * Function to fill image stride with image data
 *
 * Returns IMG_SUCCESS in case of success
 **/
int img_image_stride_fill(img_frame_t *out_buff);

/** img_alloc_ion:
 *  @mapion_list: Ion structure list to memory blocks to be allocated
 *  @num: number of buffers to be allocated
 *  @ionheapid: ION heap ID
 *  @cached:
 *    TRUE: mappings of this buffer should be cached, ion will do cache
            maintenance when the buffer is mapped for dma
 *    FALSE: mappings of this buffer should not be cached
 *
 * Function to allocate a physically contiguous memory
 *
 * Returns IMG_SUCCESS in case of success
 **/
int img_alloc_ion(img_mmap_info_ion *mapion_list, int num, uint32_t ionheapid,
  int cached);

/** img_free_ion:
 *  @mapion_list: Ion structure list to the allocated memory blocks
 *  @num: number of buffers to be freed
 *
 * Free ion memory
 *
 *
 * Returns IMG_SUCCESS in case of success
 **/
int img_free_ion(img_mmap_info_ion* mapion_list, int num);

int img_cache_ops_external (void *p_buffer, size_t size, int offset, int fd,
  img_cache_ops_type type, int ion_device_fd);

/** img_dump_frame
 *    @img_frame: frame handler
 *    @number: number to be appended at the end of the file name
 *
 * Saves specified frame to folder /data/
 *
 * Returns None.
 **/
void img_dump_frame(img_frame_t *img_frame, char* file_name,
  uint32_t number);

/** img_perf_lock_handle_create
 *
 * Creates new performance handle
 *
 * Returns new performance handle
 **/
void* img_perf_handle_create();

/** img_perf_handle_destroy
 *    @p_perf: performance handle
 *
 * Destoyes performance handle
 *
 * Returns None.
 **/
void img_perf_handle_destroy(void* p_perf);

/** img_perf_lock_start
 *    @p_perf: performance handle
 *    @p_perf_lock_params: performance lock parameters
 *    @perf_lock_params_size: size of performance lock parameters
 *    @duration: duration
 *
 * Locks performance with specified parameters
 *
 * Returns new performance lock handle
 **/
void* img_perf_lock_start(void* p_perf, int32_t* p_perf_lock_params,
  size_t perf_lock_params_size, int32_t duration);

/** img_perf_lock_end
 *    @p_perf: performance handle
 *    @p_perf_lock: performance lock handle
 *
 * Locks performance with specified parameters
 *
 * Returns None.
 **/
void img_perf_lock_end(void* p_perf, void* p_perf_lock);

#endif //__IMG_COMMON_H__
