/*===========================================================================
                           usf_epos_dynamic_lib_proxy.h

DESCRIPTION: Provide a dynamic library proxy for the pen lib.

INITIALIZATION AND SEQUENCING REQUIREMENTS: None

Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#ifndef __USF_EPOS_DYNAMIC_LIB_PROXY__
#define __USF_EPOS_DYNAMIC_LIB_PROXY__

/*----------------------------------------------------------------------------
Include files
----------------------------------------------------------------------------*/
#include "usf_dynamic_lib_proxy.h"
#include "usf_epos_defs.h"

/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
  Typedefs
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
  Classes
-----------------------------------------------------------------------------*/

class UsfEposDynamicLibProxy: public UsfDynamicLibProxy
{
private:

  /*============================================================================
    Function pointer typedefs
  ============================================================================*/

  typedef void (*get_dsp_version_fp_t)(char *OutDSPVersionString,
                                       unsigned char *OutDSPVersion);

  typedef void (*get_allocation_sizes_fp_t)(int32_t *OutPointMaxCountPerPen,
                                            int32_t *OutMaxPens,
                                            int32_t *OutSizeOfWorkspace);

  typedef int32_t (*init_dsp_fp_t)(EPoint *InPointBuffer,
                                void *InWorkspace,
                                void *InConfiguration,
                                int32_t ConfigureationLength);

  typedef int32_t(*get_points_fp_t)(int32_t *pPacket,
                                 void *InWorkspace,
                                 FeedbackInfo *OutFeedback);

  typedef void (*set_rotation_axis_fp_t)(void *InWorkspace,
                                         int32_t origin[3],
                                         int32_t direction[3],
                                         int32_t OffScreenZ);

  typedef void (*release_dsp_fp_t)(void *InWorkspace,
                                   void *OutConfiguration);

  typedef void (*agc_fp_t)(short *inPtr16,
                           int32_t *input512,
                           AGC_struct *pAGC,
                           int32_t ChannelNumber,
                           int32_t EnableEvents);

  typedef void (*init_agc_fp_t)(AGC_struct *pAGC);

  typedef void (*update_agc_fp_t)(AGC_struct *pAGC,
                                  short Gain);

  typedef int32_t (*command_fp_t)(void *InWorkspace,
                               int32_t *CommandBuffer);

  typedef void (*init_dump_callbacks_fp_t)(WriteDumpCallback WriteDumpFunc);

  typedef int32_t (*reset_dsp_fp_t)(EPoint *InPointBuffer,
                                 void *InWorkspace);

  typedef int32_t (*load_coeffs_fp_t)(void *InWorkspace,
                                   void *InConfiguration,
                                   int32_t InConfigLength);

  typedef int (*get_persistent_data_fp_t)(void *InWorkspace,
                                          int32_t *Buffer,
                                          int Length);

  typedef void (*set_dsp_trace_callback_fp_t)(TraceCallback callback);

  typedef int32_t (*query_epoint_fp_t)(void *InWorkspace,
                                    EPointType eptype,
                                    int32_t pointnum,
                                    int32_t *result,
                                    int32_t buflen);

  /*============================================================================
    Function pointers
  ============================================================================*/

  get_dsp_version_fp_t          m_get_dsp_version;
  get_allocation_sizes_fp_t     m_get_allocation_sizes;
  init_dsp_fp_t                 m_init_dsp;
  get_points_fp_t               m_get_points;
  set_rotation_axis_fp_t        m_set_rotation_axis;
  release_dsp_fp_t              m_release_dsp;
  agc_fp_t                      m_agc;
  init_agc_fp_t                 m_init_agc;
  update_agc_fp_t               m_update_agc;
  command_fp_t                  m_command;
  init_dump_callbacks_fp_t      m_init_dump_callbacks;
  reset_dsp_fp_t                m_reset_dsp;
  load_coeffs_fp_t              m_load_coeffs;
  get_persistent_data_fp_t      m_get_persistent_data;
  set_dsp_trace_callback_fp_t   m_set_dsp_trace_callback;
  query_epoint_fp_t             m_query_epoint;

protected:
  /*============================================================================
    FUNCTION:  load_all_methods
  ============================================================================*/
  /**
   * Loads all the methods from the library.
   *
   * @return bool - true success
   *                false failure
   */
  bool load_all_methods();

public:

  /*============================================================================
    FUNCTION:  expand_epoints
  ============================================================================*/
  /**
   * Converts the EPoints array to the wrapped array to support
   * the old pen lib API by assigning zero to all the non-existing
   * fields.
   *
   * @return bool - true success
   *                false failure
   */
  bool extend_epoints(void *in_workspace,
                      int32_t num_points,
                      EPoint *in_epoints,
                      usf_extended_epoint_t *out_wrapped_epoints);

  /*============================================================================
    Library method wrapper functions
  ============================================================================*/

  void get_dsp_version(char *OutDSPVersionString,
                       unsigned char *OutDSPVersion);

  void get_allocation_sizes(int32_t *OutPointMaxCountPerPen,
                            int32_t *OutMaxPens,
                            int32_t *OutSizeOfWorkspace);

  int32_t init_dsp(EPoint *InPointBuffer,
                void *InWorkspace,
                void *InConfiguration,
                int32_t ConfigureationLength);

  int32_t get_points(int32_t *pPacket,
                  void *InWorkspace,
                  FeedbackInfo *OutFeedback);

  void set_rotation_axis(void *InWorkspace,
                         int32_t origin[3],
                         int32_t direction[3],
                         int32_t OffScreenZ);

  void release_dsp(void *InWorkspace,
                   void *OutConfiguration);

  void agc(short *inPtr16,
           int32_t *input512,
           AGC_struct *pAGC,
           int32_t ChannelNumber,
           int32_t EnableEvents);

  void init_agc(AGC_struct *pAGC);

  void update_agc(AGC_struct *pAGC,
                  short Gain);

  int32_t command(void *InWorkspace,
               int32_t *CommandBuffer);

  void init_dump_callbacks(WriteDumpCallback WriteDumpFunc);

  int32_t reset_dsp(EPoint *InPointBuffer,
                 void *InWorkspace);

  int32_t load_coeffs(void *InWorkspace,
                   void *InConfiguration,
                   int32_t InConfigLength);

  int get_persistent_data(void *InWorkspace,
                          int32_t *Buffer,
                          int Length);

  void set_dsp_trace_callback(TraceCallback callback);

  int32_t query_epoint(void *InWorkspace,
                    EPointType eptype,
                    int32_t pointnum,
                    int32_t *result,
                    int32_t buflen,
                    int32_t *return_code);

};

#endif //__USF_EPOS_DYNAMIC_LIB_PROXY__
