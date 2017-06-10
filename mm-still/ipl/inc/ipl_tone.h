#ifndef IPL_TONE_H
#define IPL_TONE_H


/*===========================================================================

    I M A G E  P R O C E S S I N G   L I B R A R Y    H E A D E R    F I L E

DESCRIPTION
  This header file contains all the definitions necessary to interface
  with the image processing library.

REFERENCES
  IPL ISOD: XX-XXXXX-XX

Copyright (c) 2008 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

$Header: //depot/asic/sandbox/users/ninadp/android/mm-camera/qcamera/ipl/ipl_tone.h#1 $
===========================================================================*/





// Some global variables define in ipl_tone.h

// these are temporary variables used by the inline funtions.
extern uint8 gipl_tluma, gipl_tcb, gipl_tcr, gipl_subCr;

// the return value of th function is put in gipl_match. 1 means it is
// skin,red,or whatever color, and 0 means not.
extern uint8 gipl_match;



#define TONE_MAX_REGIONS  1

extern ipl_rect_type gipl_regions[1];
extern int gipl_count[TONE_MAX_REGIONS][TONE_MAX_REGIONS];
extern int gipl_maxReg;
extern int gipl_bestReg;




extern uint8 sTable_v1_flash[1][1][1];
extern uint8 sTable_v1_mixed[1][1][1];
extern uint8 sTable_v2_mixed[1][1][1];

extern uint8 rTable[1][1][1];




/*
 *
 * Draw gipl_count's (blocks) or gipl_regions (regions) contents
 *
 */
API_EXTERN ipl_status_type ipl_draw_skin
(
  ipl_image_type* input_img_ptr,     // input image  
  ipl_image_type* out_img_ptr,       // if NULL, will draw over input image 
  ipl_rect_type* crop,    // what area to draw, must be that used when skin
                          // content was being computed by ipl_find_skin
  int dicex,              // must match dicex, dicey used when finding skin
  int dicey,
  int mode                // 2 means draw blocks, 3 means draw regions 
);





/* <EJECT> */
/*===========================================================================

FUNCTION ipl_find_skin

DESCRIPTION

  This function will return the location of skin blocks in an image
  that have a good chance of being a face

  It is imperative that the input image have gone through the entire
  color pipeline including gamma and AWB before we try to detect skin.

  The input should be YCrCb 4:2:0 line pack or frame pack
  The output should be YCrCb 4:2:0 line pack or frame pack

DEPENDENCIES

  None

ARGUMENTS IN

  input_img_ptr   points to the input image

  crop       area of image to look for skin

  dicex      number of blocks in x-direction 
  dicey      number of blocks in y-direction 

  thresh     threshold used to determin if a block has enough skin

  regionalize should we return regions of skin blocks

  color      what color (skin) to look for


RETURN VALUE
  IPL_SUCCESS   is operation was succesful
  IPL_FAILURE   otherwise

SIDE EFFECTS

  gipl_count[dicex][dicey] will hold the % of skin pixels in dicex * dicey 
  blocks. E.g. if dicex and dicey are 4 and 4, then gipl_count[0][0] through
  gipl_coutn[4][4] will have meaningful values.

===========================================================================*/
API_EXTERN ipl_status_type ipl_find_skin
(
  ipl_image_type* input_img_ptr,      
  ipl_rect_type* crop,
  int dicex,
  int dicey,
  int thresh,
  int regionalize,
  int color 
);


#endif 
