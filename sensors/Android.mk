SENSORS_DIR := $(call my-dir)

DSPS_API_PRESENT := $(shell if [ -d $(SENSORS_DIR)/dsps/api ] ; then echo true; fi)
LIBSENSORS_API_PRESENT := $(shell if [ -d $(SENSORS_DIR)/libsensors ] ; then echo true; fi)

include $(call all-subdir-makefiles)

LOCAL_PATH := $(SENSORS_DIR)

include $(CLEAR_VARS)

LOCAL_COPY_HEADERS_TO   := sensors/inc
LOCAL_COPY_HEADERS      :=

ifeq ($(strip $(DSPS_API_PRESENT)),true)
  LOCAL_COPY_HEADERS      += dsps/api/sensor1.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_common_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_debug_interface_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_diag_dsps_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_reg_api_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_reg_api_v02.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_time_api_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_time_api_v02.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_amd_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_bte_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_filtered_mag_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_fns_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_gravity_vector_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_mag_cal_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_qmd_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_quaternion_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_rotation_vector_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_rmd_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_sensor_thresh_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_vmd_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_smgr_api_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_smgr_common_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_orientation_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_basic_gestures_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_tap_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_facing_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_integ_angle_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_gyro_tap_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_gyro_tap2_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_gyrobuf_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_gyroint_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_ped_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_pam_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_cmc_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_cmc_v02.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_distance_bound_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_smd_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_game_rotation_vector_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_oem_1_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_common_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_tilt_detector_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_smgr_restricted_api_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_dpc_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_sam_event_gated_sensor_v01.h
  LOCAL_COPY_HEADERS      += dsps/api/sns_smgr_common_v01.h

endif # DSPS_API_PRESENT
ifeq ($(strip $(LIBSENSORS_API_PRESENT)),true)
  LOCAL_COPY_HEADERS      += libsensors/isenseutil.h
endif # LIBSENSORS_API_PRESENT

include $(BUILD_COPY_HEADERS)
