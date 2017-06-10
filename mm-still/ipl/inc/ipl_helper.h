#ifndef IPL_HELPER_H
#define IPL_HELPER_H

/*===========================================================================

    I M A G E  P R O C E S S I N G   L I B R A R Y    H E A D E R    F I L E

DESCRIPTION
  This header file contains all the definitions necessary to interface
  with the image processing library.

REFERENCES
  IPL ISOD: XX-XXXXX-XX

Copyright (c) 2008 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential

$Header: //depot/asic/sandbox/users/ninadp/android/mm-camera/qcamera/ipl/ipl_helper.h#1 $
===========================================================================*/


/*===========================================================================
                      EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.


when       who     what, where, why
--------   ---     ----------------------------------------------------------

===========================================================================*/


extern const uint8  rgb565_table[];
extern const uint32 r666[];
extern const uint32 g666[];
extern const uint32 b666[];
extern const uint16 r444[];
extern const uint16 g444[];
extern const uint16 b444[];


/*===========================================================================
                        CONVERT SPECIFIC DATA
===========================================================================*/
#define IPL_SCALEBITS 16
#define IPL_ONE_HALF ((long) 1 << (IPL_SCALEBITS-1))
#define IPL_FIX(x) ((long) ((x) * (1L<<IPL_SCALEBITS) + 0.5))

// extern const int32 ipl_crr[];
// extern const int32 ipl_cbb[];
// extern const int32 ipl_crg[];
// extern const int32 ipl_cbg[];


#define sys_malloc(a) malloc(a)
#define sys_free(a)   free(a)

extern uint8 ipl_temp_buffer[];
extern uint8 ipl_temp_buffer_inuse;

extern uint8 ipl_temp_line_buffer[];
extern uint8 ipl_temp_line_buffer_inuse;

extern ipl_image_type mimage;



/* <EJECT> */
/*===========================================================================
                            FUNCTIONAL TABLES 
===========================================================================*/
#define IPL_DEBUG     0
#define IPL_DEBUG_2   0

#define IPLMSG(str) if (IPL_DEBUG) printf("%s\n", str);
#define IPL_MIN(a,b) ((a<b)? a:b);
#define IPL_MAX(a,b) ((a>b)? a:b);
#define IPL_ABS(x) ((x<0)? -x:x)

/* CLIPIT is define in camif */
#define CLIPIT(a) ( ((a<0)||(a>255)) \
                 ? ((a<0)?0:255) \
                 :a);

#define CLIPIT255(a) ((a>255)?255:a);

#define pack_rgb565(r,g,b) ((uint16)(((rgb565_table[r]&0xF8)<<8)+ \
                                     ((rgb565_table[g+256]&0xFC)<<3)+ \
                                     ((rgb565_table[b]&0xF8)>>3)));

#define pack_rgb666(r,g,b) ((uint32)((((uint32)rgb565_table[r+256])<<10)+ \
                                     (((uint32)rgb565_table[g+256])<<4)+ \
                                     (((uint32)rgb565_table[b+256]>>2))));

#define pack_rgb444(r,g,b) ((uint16)((((uint16)r444[r]))+ \
                                      ((uint16)g444[g])+ \
                                      ((uint16)b444[b])));

#define pack_ycbcr(y,cb,cr) ((uint32)((y << 16) + \
                                    (cb << 8) + \
                                    (cr)));



#define max2(a,b) ((a > b)? a : b);
#define min2(a,b) ((a < b)? a : b);


/*===========================================================================

FUNCTION ipl_unpack_rgb565

DESCRIPTION
  This function takes 16 bits rgb565 and unpacks into 24 bit r g b

DEPENDENCIES
  None

ARGUMENTS IN
  in is a word input rgb565

ARGUMENTS OUT
  r - address of R value
  g - address of G value
  b - address of B value


RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type unpack_rgb565
(
  unsigned short in,
  unsigned char* r,
  unsigned char* g,
  unsigned char* b
);


/* <EJECT> */
/*===========================================================================

FUNCTION unpack_rgb444

DESCRIPTION
  This function takes 16 bits rgb444 and unpacks into 24 bit r g b

DEPENDENCIES
  None

ARGUMENTS IN
  in is a word input rgb444

ARGUMENTS OUT
  r, g and b of input as char

RETURN VALUE
  None

SIDE EFFECTS
  None

MODIFIED
  04/09/04  Created

===========================================================================*/
extern ipl_status_type unpack_rgb444
(
  unsigned short in, 
  unsigned char* r, 
  unsigned char* g,
  unsigned char* b
);




/* <EJECT> */
/*===========================================================================

FUNCTION unpack_rgb666

DESCRIPTION
  This function takes 32 bits rgb666 and unpacks into 24 bit r g b

DEPENDENCIES
  None

ARGUMENTS IN
  in is a 32 bit word (4 byte) input rgb666

ARGUMENTS OUT
  r, g and b of input as char

RETURN VALUE
  None

SIDE EFFECTS
  None

MODIFIED
  04/09/04  Created

===========================================================================*/
extern ipl_status_type unpack_rgb666
(
  unsigned long in, 
  unsigned char* r, 
  unsigned char* g,
  unsigned char* b
);


/*===========================================================================

FUNCTION unpack_ycbcr

DESCRIPTION
  This function takes 32 bits and unpacks into 24 bit y cb cr

DEPENDENCIES
  None

ARGUMENTS IN
  in is a word input rgb565

ARGUMENTS OUT
  Y  - address of R value
  Cb - address of G value
  Cr - address of B value


RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type unpack_ycbcr
(
  unsigned int in,
  unsigned char * y,
  unsigned char * cb,
  unsigned char * cr
);


/* <EJECT> */
/*===========================================================================

FUNCTION min3

DESCRIPTION
  This function will find min of 3 numbers

DEPENDENCIES
  None

ARGUMENTS IN
  a,b,c are the 3 input numbers

RETURN VALUE
  It returns as output minimum of a,b,c

SIDE EFFECTS
  None

===========================================================================*/
extern int32 min3(int32 a, int32 b, int32 c);


/*===========================================================================

FUNCTION max3

DESCRIPTION
  This function will find max of 3 numbers

DEPENDENCIES
  None

ARGUMENTS IN
  a,b,c are the 3 input numbers

RETURN VALUE
  It returns as output maximum of a,b,c

SIDE EFFECTS
  None

===========================================================================*/
extern int32 max3(int32 a, int32 b, int32 c);




/* <EJECT> */
/*===========================================================================

FUNCTION ipl_memory_needed

DESCRIPTION
  This function returns the amount of memory the image pointed to by in
  requires.

DEPENDENCIES
  None

ARGUMENTS IN
  in    points to the input image
  isize points to amount of bytes neede by imgPtr
  csize points to amount of bytes neede by imgPtr

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_memory_needed
(
  const ipl_image_type * in, uint32 * isize, uint32 * csize
);



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_malloc

DESCRIPTION
  This function is used to get memory for a given ptr

DEPENDENCIES
  None

ARGUMENTS IN
  bytes       number of bytes to sys_malloc 

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN void * ipl_malloc
(
  size_t bytes
);




/* <EJECT> */
/*===========================================================================

FUNCTION ipl_sys_free

DESCRIPTION
  This function sys_frees the memory allocated by ipl_malloc

DEPENDENCIES
  None

ARGUMENTS IN
  in    points to the buffer to sys_free

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN void ipl_sys_free
(
  void * in
);



/* <EJECT> */
/*===========================================================================

FUNCTION ipl_malloc_img

DESCRIPTION
  This function is used to get memory for a given image of type ipl_image_type

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   points to the input image

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_malloc_img
(
  ipl_image_type * in
);




/* <EJECT> */
/*===========================================================================

FUNCTION ipl_free_img

DESCRIPTION
  This function is used to sys_free memory for a given image of type ipl_image_type

DEPENDENCIES
  None

ARGUMENTS IN
  input_img_ptr   points to the input image

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
API_EXTERN ipl_status_type ipl_free_img
(
  ipl_image_type * in
);



#ifdef IPL_DEBUG_PROFILE

/* <EJECT> */
/*===========================================================================

FUNCTION ipl_profile

DESCRIPTION
  This function is used to measure run time of various functions SURFS

DEPENDENCIES
  None

ARGUMENTS IN
  in      pointer to the input image
  out     pointer to the output image

RETURN VALUE
  IPL_SUCCESS    indicates operation was successful
  IPL_FAILURE    otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type ipl_profile
(
  ipl_image_type * in, 
  ipl_image_type * out, 
  ipl_rect_type  * icrop,
  ipl_rect_type  * ocrop,
  void * arg1,
  void * arg2
);

#endif




/*===========================================================================

FUNCTION   CREATE_COLOR_TABLES

DESCRIPTION
  Create color conversion tables for efficient YCbCr to RGB conversion.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

============================================================================*/
//void ipl_create_look_ups(void);


// clamp between 0 and 255
extern uint8 ipl_clamp(int32 i);





/* <EJECT> */
/*===========================================================================

FUNCTION ipl_debug_write_tile

DESCRIPTION
  Write image to disk

DEPENDENCIES
  None

ARGUMENTS IN
  in            points to the input image

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type
ipl_debug_write_tile
(
  const ipl_image_type *in, 
  const ipl_rect_type *crop, 
  const char * str
);


/* <EJECT> */
/*===========================================================================

FUNCTION ipl_debug_read_tile

DESCRIPTION
  Read image from disk 

DEPENDENCIES
  None

ARGUMENTS IN
  in            points to the input image

RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS
  None

===========================================================================*/
extern ipl_status_type
ipl_debug_read_tile
(
  const ipl_image_type *in, 
  const char * ifname
);





/*lint -restore */






































/*lint -restore */
#endif 

