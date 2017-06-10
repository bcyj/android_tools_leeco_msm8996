/*============================================================================

  Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include "chromatix.h"
#include "eeprom.h"
#include "sensor_common.h"

#define OTP_MESH_HWROLLOFF_SIZE 63
#define MESH_HWROLLOFF_SIZE 130

#define AF_OFFSET_L3 0x0
#define AF_OFFSET_L2 (AF_OFFSET_L3+8)
#define AF_OFFSET_L1 (AF_OFFSET_L2+8)
#define AF_OFFSET_L0 (AF_OFFSET_L1+8)

#define WB_OFFSET_L3 (AF_OFFSET_L0+8)
#define WB_OFFSET_L2 (WB_OFFSET_L3+7)
#define WB_OFFSET_L1 (WB_OFFSET_L2+7)
#define WB_OFFSET_L0 (WB_OFFSET_L1+7)

#define LSC_OFFSET (WB_OFFSET_L0+7)
#define LSC_R_OFFSET (LSC_OFFSET)
#define LSC_GR_OFFSET (LSC_R_OFFSET+2)
#define LSC_B_OFFSET (LSC_OFFSET+252)
#define LSC_GB_OFFSET (LSC_B_OFFSET+2)
#define AF_OFFSET AF_OFFSET_L3
#define WB_OFFSET WB_OFFSET_L3

#define PAGE_EMPTY 0
#define PAGE_NOT_EMPTY 1
#define MAX_EMPTY_BYTES 7
#define AWBLSC_VALID_BIT 6

#define QVALUE 1024.0
/* Data conversion macros */
#define ABS(x)  (((x) > 0) ? (x):-(x))
#define SIGN(x) (((x) > 0) ? (1):-(1))
#define MIN(x, y) (((x) > (y)) ? (y):(x))
#define MAX(x, y) (((x) > (y)) ? (x):(y))
#define CLIP(z, x, y) MAX(MIN((z), (y)), (x))

/* defining pixel patterns */
#define RGGB_PATTERN  0
#define GRBG_PATTERN  1
#define BGGR_PATTERN  2
#define GBRG_PATTERN  3

#define YCBYCR422_PATTERN   4
#define YCRYCB422_PATTERN   5
#define CBYCRY422_PATTERN   6
#define CRYCBY422_PATTERN   7

#define MESH_ROLLOFF_HORIZONTAL_GRIDS  12
#define MESH_ROLLOFF_VERTICAL_GRIDS    9

#define SENSOR_FULL_SIZE_WIDTH 4208
#define SENSOR_FULL_SIZE_HEIGHT 3120

#define max(x, y) (((x) > (y)) ? (x):(y))
#define min(x, y) (((x) < (y)) ? (x):(y))
#define CUBIC_F(fs, fc0, fc1, fc2, fc3) {\
  double fs3, fs2;\
  fs2 = fs * fs; \
  fs3 = fs * fs2; \
  fc0 = 0.5 * (-fs3 + (2.0 * fs2) - fs); \
  fc1 = 0.5 * ((3.0 * fs3) - (5.0 * fs2) + 2.0); \
  fc2 = 0.5 * ((-3.0 * fs3) + (4.0 * fs2) + fs); \
  fc3 = 0.5 * (fs3 - fs2); \
}

/* Temp data */
static float otp_r[OTP_MESH_HWROLLOFF_SIZE], otp_gr[OTP_MESH_HWROLLOFF_SIZE];
static float otp_gb[OTP_MESH_HWROLLOFF_SIZE], otp_b[OTP_MESH_HWROLLOFF_SIZE];
static float mesh_r[MESH_HWROLLOFF_SIZE], mesh_gr[MESH_HWROLLOFF_SIZE];
static float mesh_gb[MESH_HWROLLOFF_SIZE], mesh_b[MESH_HWROLLOFF_SIZE];
static uint8_t bLscAwbValid;

/** sonyimx135_get_calibration_items:
 *    @e_ctrl: address of pointer to
 *                   chromatix struct
 *
 * Loads data structure for enabling / disabling parameters that can be
 * calibrated
 *
 * Return:
 * void
 **/
void sonyimx135_get_calibration_items( void *e_ctrl )
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  eeprom_calib_items_t *e_items = &(ectrl->eeprom_data.items);
  e_items->is_afc = TRUE;

  if(TRUE == bLscAwbValid){
    e_items->is_wbc = TRUE;
    e_items->is_lsc = TRUE;
    SHIGH("WBC and LSC Available and loaded");
  } else {
    e_items->is_wbc = FALSE;
    e_items->is_lsc = FALSE;
    SHIGH("WBC and LSC UNavailable and not loaded");
  }
  e_items->is_dpc = FALSE;
}

/** sonyimx135_check_empty_page:
 *    @buff: address of page buffer
 *
 * Checks if the page has non zero data
 *
 * Return:
 * uint8_t :  PAGE_EMPTY / PAGE_NOT_EMPTY
 **/
uint8_t sonyimx135_check_empty_page( uint8_t *buff )
{
  uint8_t retval = PAGE_EMPTY;
  int i=0;

  for(i=0; i < MAX_EMPTY_BYTES; i++){
    if( buff[i] != 0 )
    {
      retval = PAGE_NOT_EMPTY;
      break;
    }
  }
  return retval;
}

/** sonyimx135_check_awblsc_valid:
 *    @buff: address of page buffer
 **
 * Checks if the page has non zero data and also validates if the AWB & LSC
 * data can be used for sensor calibration
 *
 * Return:
 * uint8_t :  PAGE_EMPTY / PAGE_NOT_EMPTY
 **/
uint8_t sonyimx135_check_awblsc_valid( uint8_t *buff )
{
  uint8_t retval = PAGE_EMPTY;
  int i=0;

  retval = sonyimx135_check_empty_page(buff );
  if(PAGE_EMPTY != retval){
    retval = PAGE_EMPTY;
    if (buff[AWBLSC_VALID_BIT] == 0xFF){
      retval = PAGE_NOT_EMPTY;
    }
  }
  return retval;
}

/** sonyimx135_format_afdata_internal:
 *    @e_ctrl: address of pointer to
 *                   chromatix struct
 *    @AF_START_OFFSET: start offset of page in eeprom memory
 *
 * Format Auto Focus calibration data for AF calibration
 *
 * Return:
 * void
 **/
void  sonyimx135_format_afdata_internal( sensor_eeprom_data_t *e_ctrl,
        uint32_t AF_START_OFFSET )
{
  e_ctrl->eeprom_data.afc.starting_dac =
    (uint16_t) (((e_ctrl->eeprom_params.buffer[AF_START_OFFSET]) << 8) |
    (e_ctrl->eeprom_params.buffer[AF_START_OFFSET + 1]));

  e_ctrl->eeprom_data.afc.infinity_dac =
    (uint16_t)(((e_ctrl->eeprom_params.buffer[AF_START_OFFSET + 2]) << 8) |
    (e_ctrl->eeprom_params.buffer[AF_START_OFFSET + 3]));

  e_ctrl->eeprom_data.afc.macro_dac =
    (uint16_t)(((e_ctrl->eeprom_params.buffer[AF_START_OFFSET + 4]) << 8) |
    (e_ctrl->eeprom_params.buffer[AF_START_OFFSET + 5]));

  SLOW("AF Starting DAC = %d", e_ctrl->eeprom_data.afc.starting_dac);
  SLOW("AF Macro DAC = %d", e_ctrl->eeprom_data.afc.macro_dac);
  SLOW("AF Infinity DAC = %d", e_ctrl->eeprom_data.afc.infinity_dac);
}

/** sonyimx135_format_afdata_internal:
 *    @e_ctrl: address of pointer to
 *                   chromatix struct
 *
 *  Checks for non empty page to calibrate sensor for Auto Focus
 *  This function is called by sonyimx135_format_afdata
 *
 * Return:
 * void
 **/
static void sonyimx135_format_afdata( sensor_eeprom_data_t *e_ctrl )
{
  SLOW("Enter");

  if ( PAGE_NOT_EMPTY ==  sonyimx135_check_empty_page(
    &e_ctrl->eeprom_params.buffer[AF_OFFSET_L3]) ){

      SLOW( "Loading AF_OFFSET_L3" );
      sonyimx135_format_afdata_internal( e_ctrl, AF_OFFSET_L3 );

  } else if ( PAGE_NOT_EMPTY == sonyimx135_check_empty_page(
    &e_ctrl->eeprom_params.buffer[AF_OFFSET_L2]) ){

      SLOW( "Loading AF_OFFSET_L2" );
      sonyimx135_format_afdata_internal(e_ctrl, AF_OFFSET_L2);

  } else if ( PAGE_NOT_EMPTY == sonyimx135_check_empty_page(
    &e_ctrl->eeprom_params.buffer[AF_OFFSET_L1]) ){

      SLOW("Loading AF_OFFSET_L1");
      sonyimx135_format_afdata_internal(e_ctrl, AF_OFFSET_L1);

  } else if ( PAGE_NOT_EMPTY == sonyimx135_check_empty_page(
    &e_ctrl->eeprom_params.buffer[AF_OFFSET_L0])){

      SLOW("Loading AF_OFFSET_L0");
      sonyimx135_format_afdata_internal(e_ctrl, AF_OFFSET_L0);

  }
  SLOW("Exit");
}

/** sonyimx135_format_wbdata_internal:
 *    @e_ctrl: address of pointer to
 *                   chromatix struct
 *    @WB_START_OFFSET: start offset of page in eeprom memory
 *
 * Reads out White Balance calibration data from eeprom and calibrates sensor
 *  This function is called by sonyimx135_format_wbdata
 *
 * Return:
 * void
 **/
void sonyimx135_format_wbdata_internal( sensor_eeprom_data_t *e_ctrl,
        uint32_t WB_START_OFFSET )
{
  int index;
  float awb_r_over_gr, awb_b_over_gr, awb_gb_over_gr;
  bLscAwbValid = TRUE;

  awb_r_over_gr =(float)((float)
    ((uint16_t)((e_ctrl->eeprom_params.buffer[WB_START_OFFSET] << 8) |
    (e_ctrl->eeprom_params.buffer[WB_START_OFFSET+ 1])) ) / ((float) QVALUE));

  awb_b_over_gr = (float)((float)
    ((uint16_t)((e_ctrl->eeprom_params.buffer[WB_START_OFFSET + 2] << 8) |
    (e_ctrl->eeprom_params.buffer[WB_START_OFFSET+ 3])) ) / ((float) QVALUE));

  awb_gb_over_gr = (float)((float)
    ((uint16_t)((e_ctrl->eeprom_params.buffer[WB_START_OFFSET + 4] << 8) |
    (e_ctrl->eeprom_params.buffer[WB_START_OFFSET+ 5])) ));


  if ( awb_gb_over_gr != 0 ) {

    e_ctrl->eeprom_data.wbc.gr_over_gb = ((float) ((float) (QVALUE) /
      (float)awb_gb_over_gr));

  } else {

    e_ctrl->eeprom_data.wbc.gr_over_gb = 1.0f;

  }

  for ( index = 0; index < AGW_AWB_MAX_LIGHT; index++ ) {

    e_ctrl->eeprom_data.wbc.r_over_g[index] =
      (float) (awb_r_over_gr);

    e_ctrl->eeprom_data.wbc.b_over_g[index] =
      (float) (awb_b_over_gr);

  }

  SHIGH("r_over_g \tb_over_g\tgr_over_gb");

  for ( index = 0; index < AGW_AWB_MAX_LIGHT; index++ ) {
    SHIGH("R_G: %f\t\t B_G: %f\t\t GR_GB: %f",
      e_ctrl->eeprom_data.wbc.r_over_g[index],
      e_ctrl->eeprom_data.wbc.b_over_g[index],
      e_ctrl->eeprom_data.wbc.gr_over_gb);
  }
}

/** sonyimx135_format_wbdata:
 *    @e_ctrl: address of pointer to
 *                   chromatix struct
 *
 *  Checks for non empty page to calibrate sensor for Auto Focus
 *
 * Return:
 * void
 **/
static void sonyimx135_format_wbdata(sensor_eeprom_data_t *e_ctrl)
{
  SLOW("Enter sonyimx135_format_wbdata");
  bLscAwbValid = FALSE; /* Reset value before every read */
  if ( PAGE_NOT_EMPTY ==  sonyimx135_check_awblsc_valid(
    &e_ctrl->eeprom_params.buffer[WB_OFFSET_L3]) ) {

    SLOW("Loading WB_OFFSET_L3");
    sonyimx135_format_wbdata_internal(e_ctrl, WB_OFFSET_L3);

  } else if ( PAGE_NOT_EMPTY == sonyimx135_check_awblsc_valid(
    &e_ctrl->eeprom_params.buffer[WB_OFFSET_L2])){

    SLOW("Loading WB_OFFSET_L2");
    sonyimx135_format_wbdata_internal(e_ctrl, WB_OFFSET_L2);

  } else if ( PAGE_NOT_EMPTY == sonyimx135_check_awblsc_valid(
    &e_ctrl->eeprom_params.buffer[WB_OFFSET_L1])){

    SLOW("Loading WB_OFFSET_L1");
    sonyimx135_format_wbdata_internal(e_ctrl, WB_OFFSET_L1);

  } else if ( PAGE_NOT_EMPTY == sonyimx135_check_awblsc_valid(
    &e_ctrl->eeprom_params.buffer[WB_OFFSET_L0])){

    SLOW("Loading WB_OFFSET_L0");
    sonyimx135_format_wbdata_internal(e_ctrl, WB_OFFSET_L0);

  }

  SLOW("Exit");
}

/** sonyimx135_print_otp_matrix:
 *    @paramlist: address of pointer to
 *                   chromatix struct
 *
 * Prints out debug logs
 *
 * This function executes in module sensor context
 *
 * Return:
 * void
 **/
void sonyimx135_print_otp_matrix(float* paramlist)
{
  int x =0;

  for( x=0; x < OTP_MESH_HWROLLOFF_SIZE; x = x+9 ){
    SLOW("%.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f",
      paramlist[x], paramlist[x+1],paramlist[x+2], paramlist[x+3],
      paramlist[x+4], paramlist[x+5], paramlist[x+6], paramlist[x+7],
      paramlist[x+8]);
  }
}

/** sonyimx135_print_matrix:
 *    @paramlist: address of pointer to
 *                   chromatix struct
 *
 * Prints out debug logs
 *
 * This function executes in module sensor context
 *
 * Return:
 * void
 **/
void sonyimx135_print_matrix(float* paramlist)
{
    int j =0;

  for(j=0; j < MESH_HWROLLOFF_SIZE; j = j+17){
    SLOW("%.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f,%.1f, %.1f, %.1f, \
%.1f, %.1f, %.1f, %.1f, %.1f, %.1f",
      paramlist[j], paramlist[j+1], paramlist[j+2], paramlist[j+3],
      paramlist[j+4], paramlist[j+5], paramlist[j+6], paramlist[j+7],
      paramlist[j+8], paramlist[j+9], paramlist[j+10], paramlist[j+11],
      paramlist[j+12], paramlist[j+13], paramlist[j+14], paramlist[j+15],
      paramlist[j+16]);
  }
}

/** interp_grid_optimization:
 *    @raw_width: raw width of image
 *    @raw_height: raw height of image
 *    @deltah: delta height
 *    @deltav: delta width
 *    @subgridh: subgrid height
 *    @subgridv: subgrid width
 *
 *  This function performs inter pixel grid optimization while rescaling
 *  This function is called by mesh_rolloff_V4_UpScaleOTPMesh_9x7to10x13
 *
 * Return:
 * void
 **/
void interp_grid_optimization(int raw_width, int raw_height, int *scale_cubic,
      int *deltah, int *deltav, int *subgridh, int *subgridv)
{
  int level,scale, w, h, sgh, sgv, gh, gv, dh, dv, nx, ny;
  int e,temp;

  nx = MESH_ROLLOFF_HORIZONTAL_GRIDS;
  ny = MESH_ROLLOFF_VERTICAL_GRIDS;

  w = raw_width>>1;  /* per-channel image width */
  h = raw_height>>1;  /* per-channel image height */

  level= 3 +1; /* Initial bicubic level level as 1 more than maximum 3 */

  do{
    level--;
    sgh = (w + nx - 1) / nx;  /* Ceil */
    sgh = (sgh + (1 << level) - 1) >> level;  /* Ceil */
    gh = sgh << level;     /* Bayer grid width */
    dh = gh * nx - w; /* two-side overhead */

    sgv = (h + ny - 1) / ny;  /* Ceil */
    sgv = (sgv + (1 << level) - 1) >> level;   /* Ceil */
    gv = sgv << level;     /* Bayer grid height */
    dv = gv * ny - h; /* two-side overhead */
  } while ( (level > 0) && ((sgh < 9) || (sgv <9) || (dh >= gh) ||
            (dv >= gv)||(gh-(dh+1)/2<16) ||(gv-(dv+1)/2<9)) );

  *scale_cubic = 1 << level;
  *deltah = (dh + 1)>>1;
  *deltav = (dv + 1)>>1;
  *subgridh = sgh;
  *subgridv = sgv;
}

/** mesh_rolloff_V4_UpScaleOTPMesh_9x7to10x13:
 *    @MeshIn: Input Mesh  9x7
 *    @MeshOut: Output Mesh   13x10
 *    @width: Sensor width
 *    @height: Sensor height
 *    @dh: Horizontal offset of OTP grid from top-left corner of image
 *    @dv: Vertical offset of OTP grid from top-left corner of image
 *
 *  This function resamples the 9x7 OTP data into the rolloff 13x10 grid
 *  This function assumed that 9x7 OTP data is unifrom
 *  This function is called by sonyimx135_format_lensshading
 *
 * Return:
 * void
 **/
void mesh_rolloff_V4_UpScaleOTPMesh_9x7to10x13(float *MeshIn,  float *MeshOut,
      int width, int height, int Dh, int Dv)
{
  float cxm, cx0, cx1, cx2, cym, cy0, cy1, cy2;
  float am, a0, a1, a2, bm, b0, b1, b2;
  float tx , ty;
  int ix, iy;
  int i, j;

  /* Initialize the roll-off mesh grid */
  int level,scale, w, h, sgh, sgv, gh, gv, dh, dv;
  int sign = 0;
  int MESH_H = 12; /* the rolloff block number (horizontal). */
  int MESH_V = 9;  /* the rolloff block number (vertical). */
  int Nx = 9;    /* the OTP grid number (horizontal) */
  int Ny = 7;    /* the OTP grid number (vertical) */
  int Gh, Gv;
  float *Extend_Mesh;

  interp_grid_optimization(width, height, &scale, &dh, &dv, &sgh, &sgv);
  gh = sgh * scale;
  gv = sgv * scale;

  /* outer extend the mesh data 1 block by keeping the same slope */
  Dh = Dh/2; /* convert into the size in per-channel image */
  Dv = Dv/2;
  Gh = (width/2-2*Dh)/(Nx-1);  /* OTP block size (horizontal) */
  Gv = (height/2-2*Dv)/(Ny-1);  /* OTP block size (vertical) */

  Extend_Mesh = (float*) malloc( sizeof(float) * (Nx+2)*(Ny+2));
  if (Extend_Mesh == NULL) {
    SERR("%s: malloc failed",__func__);
    return;
  }
  for (i=1; i<Ny+1; i++)
    for (j=1; j<Nx+1; j++)
      Extend_Mesh[i*(Nx+2)+j] = MeshIn[(i-1)*Nx+j-1];


  Extend_Mesh[0*(Nx+2)+0] = Extend_Mesh[1*(Nx+2)+1]*2- Extend_Mesh[2*(Nx+2)+2];
  Extend_Mesh[(Ny+1)*(Nx+2)+0] =
    Extend_Mesh[(Ny)*(Nx+2)+1]*2- Extend_Mesh[(Ny-1)*(Nx+2)+2];
  Extend_Mesh[(Ny+1)*(Nx+2)+Nx+1] =
    Extend_Mesh[(Ny)*(Nx+2)+Nx]*2- Extend_Mesh[(Ny-1)*(Nx+2)+Nx-1];
  Extend_Mesh[0*(Nx+2)+Nx+1] =
    Extend_Mesh[1*(Nx+2)+Nx]*2- Extend_Mesh[2*(Nx+2)+Nx-1];

  for (i=1; i<Ny+1; i++){
    Extend_Mesh[i*(Nx+2)+0] = Extend_Mesh[i*(Nx+2)+1]*2 -
      Extend_Mesh[i*(Nx+2)+2];
    Extend_Mesh[i*(Nx+2)+Nx+1] = Extend_Mesh[i*(Nx+2)+Nx]*2 -
      Extend_Mesh[i*(Nx+2)+Nx-1];
  }

  for (j=1; j<Nx+1; j++){
    Extend_Mesh[0*(Nx+2)+j] = Extend_Mesh[1*(Nx+2)+j]*2 -
      Extend_Mesh[2*(Nx+2)+j];
    Extend_Mesh[(Ny+1)*(Nx+2)+j] = Extend_Mesh[(Ny)*(Nx+2)+j]*2 -
      Extend_Mesh[(Ny-1)*(Nx+2)+j];
  }

  /*  resample Extended Mesh data onto the roll-off mesh grid */
  for (i = 0; i < (MESH_V + 1); i++) {
    for (j = 0; j < (MESH_H + 1); j++) {
      tx =  (float)((double)(j*gh-dh -Dh + Gh)/ (double)(Gh));
      ix = floor(tx);
      tx -= (double)ix;

      ty =  (float)((double)(i*gv-dv -Dv + Gv)/(double)(Gv));
      iy = floor(ty);
      ty -= (double)iy;

      if (i == 0 || j == 0  || i == MESH_V|| j == MESH_H){
      /* for boundary points, use bilinear interpolation */
          b1 = (1 - tx)* Extend_Mesh[iy    *(Nx+2) + ix] +
            tx* Extend_Mesh[iy    *(Nx+2) + ix+1];
          b2 = (1 - tx)* Extend_Mesh[(iy+1)*(Nx+2) + ix] +
            tx* Extend_Mesh[(iy+1)*(Nx+2) + ix+1];
          MeshOut[(i * (MESH_H + 1)) + j] = (float)((1 - ty)*b1 + ty*b2);
      } else { /* for nonboundary points, use bicubic interpolation */
              /*get x direction coeff and y direction coeff*/
          CUBIC_F(tx, cxm, cx0, cx1, cx2);
          CUBIC_F(ty, cym, cy0, cy1, cy2);

          am = Extend_Mesh[(iy-1) *(Nx+2)+ (ix-1)];
          a0 = Extend_Mesh[(iy-1) *(Nx+2)+ (ix  )];
          a1 = Extend_Mesh[(iy-1) *(Nx+2)+ (ix+1)];
          a2 = Extend_Mesh[(iy-1) *(Nx+2)+ (ix+2)];
          bm = ((cxm * am) + (cx0 * a0) + (cx1 * a1) + (cx2 * a2));

          am = Extend_Mesh[(iy  ) *(Nx+2)+ (ix-1)];
          a0 = Extend_Mesh[(iy  ) *(Nx+2)+ (ix  )];
          a1 = Extend_Mesh[(iy  ) *(Nx+2)+ (ix+1)];
          a2 = Extend_Mesh[(iy  ) *(Nx+2)+ (ix+2)];
          b0 = ((cxm * am) + (cx0 * a0) + (cx1 * a1) + (cx2 * a2));

          am = Extend_Mesh[(iy+1) *(Nx+2)+ (ix-1)];
          a0 = Extend_Mesh[(iy+1) *(Nx+2)+ (ix  )];
          a1 = Extend_Mesh[(iy+1) *(Nx+2)+ (ix+1)];
          a2 = Extend_Mesh[(iy+1) *(Nx+2)+ (ix+2)];
          b1 = ((cxm * am) + (cx0 * a0) + (cx1 * a1) + (cx2 * a2));

          am = Extend_Mesh[(iy+2) *(Nx+2)+ (ix-1)];
          a0 = Extend_Mesh[(iy+2) *(Nx+2)+ (ix  )];
          a1 = Extend_Mesh[(iy+2) *(Nx+2)+ (ix+1)];
          a2 = Extend_Mesh[(iy+2) *(Nx+2)+ (ix+2)];
          b2 = ((cxm * am) + (cx0 * a0) + (cx1 * a1) + (cx2 * a2));

          MeshOut[(i * (MESH_H + 1)) + j] =  (float)
            ((cym * bm) + (cy0 * b0) + (cy1 * b1) + (cy2 * b2));
      } /* else */
    }/*for (j = 0; j < (MESH_H + 1); j++) */
  }/* for (i = 0; i < (MESH_V + 1); i++) */

  /* free memory */
  free(Extend_Mesh);
}

/** sonyimx135_format_lensshading:
 *    @e_ctrl: address of pointer to
 *                   chromatix struct
 *
 *  Loads lens shading data from the eeprom into the chromatix data
 *
 * Return:
 * void
 **/
static void sonyimx135_format_lensshading (void *e_ctrl)
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  uint8_t *a_r_gain, *a_gr_gain, *a_gb_gain, *a_b_gain;
  uint16_t i, j;
  uint16_t x,y;

  SLOW("Enter");
  a_r_gain = ( uint8_t *) &(ectrl->eeprom_params.buffer[LSC_R_OFFSET]);
  a_gr_gain = ( uint8_t *) &(ectrl->eeprom_params.buffer[LSC_GR_OFFSET]);
  a_gb_gain = ( uint8_t *) &(ectrl->eeprom_params.buffer[LSC_GB_OFFSET]);
  a_b_gain = ( uint8_t *) &(ectrl->eeprom_params.buffer[LSC_B_OFFSET]);

  /* Load data from eeprom */
  for (j = 0; j < OTP_MESH_HWROLLOFF_SIZE; j++) {
    otp_r[j] = (float) (((*a_r_gain) << 8) | (*(a_r_gain+1)));
    otp_gr[j] = (float) (((*a_gr_gain) << 8) | (*(a_gr_gain+1)));
    otp_gb[j] = (float) (((*a_gb_gain) << 8) | (*(a_gb_gain+1)));
    otp_b[j] = (float) (((*a_b_gain) << 8) | (*(a_b_gain+1)));

    a_r_gain += 4; /* R is interlaced with GR hence two increments */
    a_gr_gain += 4;
    a_gb_gain += 4; /* B is interlaced with GB hence two increments */
    a_b_gain += 4;
  }

  memset( mesh_r, 0, sizeof(mesh_r) );
  memset( mesh_gr, 0, sizeof(mesh_gr) );
  memset( mesh_gb, 0, sizeof(mesh_gb) );
  memset( mesh_b, 0, sizeof(mesh_b) );
  /* Convert 9x7 Grid to 13x10 grid for all params */
  mesh_rolloff_V4_UpScaleOTPMesh_9x7to10x13(otp_r, mesh_r,
    SENSOR_FULL_SIZE_WIDTH, SENSOR_FULL_SIZE_HEIGHT, 56, 24);

  mesh_rolloff_V4_UpScaleOTPMesh_9x7to10x13(otp_gr, mesh_gr,
    SENSOR_FULL_SIZE_WIDTH, SENSOR_FULL_SIZE_HEIGHT, 56, 24);

  mesh_rolloff_V4_UpScaleOTPMesh_9x7to10x13(otp_gb, mesh_gb,
    SENSOR_FULL_SIZE_WIDTH, SENSOR_FULL_SIZE_HEIGHT, 56, 24);

  mesh_rolloff_V4_UpScaleOTPMesh_9x7to10x13(otp_b, mesh_b,
    SENSOR_FULL_SIZE_WIDTH, SENSOR_FULL_SIZE_HEIGHT, 56, 24);


  for (j = 0; j < MESH_HWROLLOFF_SIZE; j++) {
    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].r_gain[j] =
      ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].r_gain[j] =
      ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].r_gain[j] =
      mesh_r[j];

    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].gr_gain[j] =
      ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].gr_gain[j] =
      ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].gr_gain[j] =
      mesh_gr[j];

    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].gb_gain[j] =
      ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].gb_gain[j] =
      ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].gb_gain[j] =
      mesh_gb[j];

    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].b_gain[j] =
      ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].b_gain[j] =
      ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].b_gain[j] =
      mesh_b[j];
  }

  ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].mesh_rolloff_table_size=
    MESH_HWROLLOFF_SIZE;
  ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].mesh_rolloff_table_size=
    MESH_HWROLLOFF_SIZE;
  ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].mesh_rolloff_table_size=
    MESH_HWROLLOFF_SIZE;

  SLOW("OTP R MATRIX") ;
  sonyimx135_print_otp_matrix(otp_r);

  SLOW("OTP GR MATRIX") ;
  sonyimx135_print_otp_matrix(otp_gr);

  SLOW("OTP GB MATRIX") ;
  sonyimx135_print_otp_matrix(otp_gb);

  SLOW("OTP B MATRIX") ;
  sonyimx135_print_otp_matrix(otp_b);

  SLOW("MESH R MATRIX");
  sonyimx135_print_matrix(ectrl->eeprom_data.lsc.lsc_calib[0].r_gain);

  SLOW("MESH GR MATRIX");
  sonyimx135_print_matrix(ectrl->eeprom_data.lsc.lsc_calib[0].gr_gain);

  SLOW("MESH GB MATRIX");
  sonyimx135_print_matrix(ectrl->eeprom_data.lsc.lsc_calib[0].gb_gain);

  SLOW("MESH B MATRIX");
  sonyimx135_print_matrix(ectrl->eeprom_data.lsc.lsc_calib[0].b_gain);

  SLOW("Exit");
}

/** sonyimx135_format_calibration_data:
 *    @e_ctrl: address of pointer to
 *                   chromatix struct
 *
 *  This function call all the sub function to read chromatix data and calibrate
 *  the sensor
 *
 * Return:
 * void
 **/
void sonyimx135_format_calibration_data(void *e_ctrl) {
  SLOW("Enter");

  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  uint8_t *data = ectrl->eeprom_params.buffer;

  sonyimx135_format_afdata(ectrl);
  sonyimx135_format_wbdata(ectrl);
  sonyimx135_format_lensshading(ectrl);

  SLOW("Exit");
}

/** sonyimx135_lib_func_ptr:
 *  This structure creates the function pointer for imx135 eeprom lib
 **/
static eeprom_lib_func_t sonyimx135_lib_func_ptr = {
    .get_calibration_items = sonyimx135_get_calibration_items,
    .format_calibration_data = sonyimx135_format_calibration_data,
    .do_af_calibration = eeprom_autofocus_calibration,
    .do_wbc_calibration = eeprom_whitebalance_calibration,
    .do_lsc_calibration = eeprom_lensshading_calibration,
    .do_dpc_calibration = NULL,
    .get_dpc_calibration_info = NULL,
    .get_raw_data = NULL,
};

/** sonyimx135_eeprom_open_lib:
 *    @e_ctrl: address of pointer to
 *                   chromatix struct
 *
 *  This function call all the sub function to read chromatix data and calibrate
 *  the sensor
 *
 * Return:
 * void* : Pinter to the sonyimx135 function table
 **/
void* sonyimx135_eeprom_open_lib(void) {
  return &sonyimx135_lib_func_ptr;
}
