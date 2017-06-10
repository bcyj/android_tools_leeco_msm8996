
# libmmcamera_server_assembly.a
noinst_LTLIBRARIES := libmmcamera_server_assembly.la

libmmcamera_server_assembly_la_CFLAGS := $(DEBUG_CFLAGS)
libmmcamera_server_assembly_la_CFLAGS += -DMSM_CAMERA_GCC
libmmcamera_server_assembly_la_CFLAGS += -DLINUX_ENABLED

libmmcamera_server_assembly_la_SOURCES := frameproc/hdr/hdrcolorgamma_armv7_gcc.S
libmmcamera_server_assembly_la_SOURCES += frameproc/hdr/hdrmotioncorrection_armv7_gcc.S
libmmcamera_server_assembly_la_SOURCES += frameproc/hdr/hdrtonemap_armv7_gcc.S

if WAVELET_DENOISE
libmmcamera_server_assembly_la_SOURCES += frameproc/wavelet_denoise/edge_weighting_armv7_gcc.S
libmmcamera_server_assembly_la_SOURCES += frameproc/wavelet_denoise/idwt_filters_armv7_gcc.S
libmmcamera_server_assembly_la_SOURCES += frameproc/wavelet_denoise/dwt_filters_armv7_gcc.S
endif

include ../override_mem_alloc_dealloc.am
libmmcamera_server_assembly_la_DEPENDENCIES := $(call override_mem_alloc_dealloc,$(libmmcamera_server_assembly_la_OBJECTS))

# libmmcamera_target.a
noinst_LTLIBRARIES += libmmcamera_target.la

libmmcamera_target_la_CFLAGS := $(DEBUG_CFLAGS)
libmmcamera_target_la_CFLAGS += -DAMSS_VERSION=$(AMSS_VERSION)
libmmcamera_target_la_CFLAGS += -DMSM_CAMERA_GCC
libmmcamera_target_la_CFLAGS += -DLINUX_ENABLED
libmmcamera_target_la_CFLAGS += -Dstrlcpy=g_strlcpy
libmmcamera_target_la_CFLAGS += -fPIC
libmmcamera_target_la_CFLAGS += -D_V4L2_BASED_CAM_
if VFE2X
libmmcamera_target_la_CFLAGS += -DVFE_2X
endif
if VFE31
libmmcamera_target_la_CFLAGS += -DVFE_31
if MSM7630
libmmcamera_target_la_CFLAGS += -DVFE_31_7x30
else
libmmcamera_target_la_CFLAGS += -DVFE_31_8x60
endif
endif
if VFE32
libmmcamera_target_la_CFLAGS += -DVFE_32
libmmcamera_target_la_CFLAGS += -DFEATURE_GYRO
endif
libmmcamera_target_la_CFLAGS += -DIPL_DEBUG_STANDALONE
libmmcamera_target_la_CFLAGS += -DCHROMATIX_VERSION_$(CHROMATIX_VERSION)
if FACE_PROC
libmmcamera_target_la_CFLAGS += -DMM_CAMERA_FD
endif
#remove when snapshot is supported
libmmcamera_target_la_CFLAGS += -DDISABLE_JPEG_ENCODING

#Enable for parallel sensors device
#libmmcamera_target_la_CFLAGS += -DPARALLEL_SENSOR
#Enable for Non-ffa devices
#libmmcamera_target_la_CFLAGS += -DOVERRIDE_SENSOR_SIZE


libmmcamera_target_la_CFLAGS += -I ../common 
libmmcamera_target_la_CFLAGS += -I core/mctl -I core/config
libmmcamera_target_la_CFLAGS += -I hardware/vfe/vfe_interface
libmmcamera_target_la_CFLAGS += -I hardware/sensor
libmmcamera_target_la_CFLAGS += -I hardware/sensor/vx6953
libmmcamera_target_la_CFLAGS += -I hardware/sensor/s5k4ecgx
libmmcamera_target_la_CFLAGS += -I hardware/sensor/mt9e013
libmmcamera_target_la_CFLAGS += -I hardware/sensor/ov9726
libmmcamera_target_la_CFLAGS += -I hardware/sensor/ov7692
libmmcamera_target_la_CFLAGS += -I hardware/sensor/ov5647
libmmcamera_target_la_CFLAGS += -I hardware/sensor/imx074
libmmcamera_target_la_CFLAGS += -I hardware/sensor/imx135
libmmcamera_target_la_CFLAGS += -I hardware/camif
libmmcamera_target_la_CFLAGS += -I hardware/axi

if MSM7X27A
libmmcamera_target_la_CFLAGS += -I hardware/camif/DSP_CAMIF
libmmcamera_target_la_CFLAGS += -I hardware/axi/DSP_AXI
libmmcamera_target_la_CFLAGS += -I hardware/vfe/vfe_modules/color_proc
libmmcamera_target_la_CFLAGS += -I hardware/vfe/vfe_modules/active_crop
libmmcamera_target_la_CFLAGS += -I hardware/sensor/s5k4e1
else
libmmcamera_target_la_CFLAGS += -I hardware/camif/ARM_CAMIF
libmmcamera_target_la_CFLAGS += -I hardware/axi/ARM_AXI
endif
libmmcamera_target_la_CFLAGS += -I hardware/actuator
libmmcamera_target_la_CFLAGS += -I hardware/actuator/$(CHROMATIX_VERSION)
libmmcamera_target_la_CFLAGS += -I hardware/actuator/$(CHROMATIX_VERSION)/7627a
libmmcamera_target_la_CFLAGS += -I hardware/actuator/$(CHROMATIX_VERSION)/8960
libmmcamera_target_la_CFLAGS += -I hardware/c2d_client
libmmcamera_target_la_CFLAGS += -I hardware/eeprom
libmmcamera_target_la_CFLAGS += -I hardware/flash
libmmcamera_target_la_CFLAGS += -I hardware/flash/led
libmmcamera_target_la_CFLAGS += -I hardware/flash/xenon
libmmcamera_target_la_CFLAGS += -I hardware/vfe/$(VFE_VERS)/$(MSM_VERSION)
libmmcamera_target_la_CFLAGS += -I hardware/vfe/$(VFE_VERS)/common
libmmcamera_target_la_CFLAGS += -I hardware/vfe/$(VFE_VERS)/common/eztune
libmmcamera_target_la_CFLAGS += -I hardware/vfe/$(VFE_VERS)/test
libmmcamera_target_la_CFLAGS += -I hardware/vfe/common
libmmcamera_target_la_CFLAGS += -I hardware/vfe/vfe_modules/fovcrop -I hardware/vfe/vfe_modules/abf  -I hardware/vfe/vfe_modules/asf  -I hardware/vfe/vfe_modules/black_level  -I hardware/vfe/vfe_modules/bpc  -I hardware/vfe/vfe_modules/chroma_enhan  -I hardware/vfe/vfe_modules/chroma_subsample  -I hardware/vfe/vfe_modules/chroma_suppress  -I hardware/vfe/vfe_modules/clamp  -I hardware/vfe/vfe_modules/clf  -I hardware/vfe/vfe_modules/colorcorrect  -I hardware/vfe/vfe_modules/demosaic  -I hardware/vfe/vfe_modules/demux  -I hardware/vfe/vfe_modules/fovcrop  -I hardware/vfe/vfe_modules/frame_skip  -I hardware/vfe/vfe_modules/gamma  -I hardware/vfe/vfe_modules/linearization  -I hardware/vfe/vfe_modules/luma_adaptation  -I hardware/vfe/vfe_modules/mce  -I hardware/vfe/vfe_modules/rolloff -I hardware/vfe/vfe_modules/rolloff/mlro_to_plro -I hardware/vfe/vfe_modules/scaler  -I hardware/vfe/vfe_modules/sce  -I hardware/vfe/vfe_modules/stats -I hardware/vfe/vfe_modules/wb 
libmmcamera_target_la_CFLAGS += -I hardware/vpe1
libmmcamera_target_la_CFLAGS += -I features/bestshot
libmmcamera_target_la_CFLAGS += -I features/dis
libmmcamera_target_la_CFLAGS += -I features/effects
libmmcamera_target_la_CFLAGS += -I features/hdr
libmmcamera_target_la_CFLAGS += -I features/live_snapshot
libmmcamera_target_la_CFLAGS += -I frameproc/hdr
libmmcamera_target_la_CFLAGS += -I features/stereocam
libmmcamera_target_la_CFLAGS += -I features/zoom
libmmcamera_target_la_CFLAGS += -I core/eztune
libmmcamera_target_la_CFLAGS += -I core/dsps
libmmcamera_target_la_CFLAGS += -I frameproc
libmmcamera_target_la_CFLAGS += -I statsproc
libmmcamera_target_la_CFLAGS += -I chromatix/$(CHROMATIX_VERSION)
#for jpeg
libmmcamera_target_la_CFLAGS += -I ../../mm-still/ipl/inc
libmmcamera_target_la_CFLAGS += -I ../../mm-still/jpeg/inc

libmmcamera_target_la_SOURCES := ../common/cam_mmap.c
libmmcamera_target_la_SOURCES += ../common/stacktrace.c

#add core framework
libmmcamera_target_la_SOURCES += core/qcamsvr.c
libmmcamera_target_la_SOURCES += core/config/config.c
libmmcamera_target_la_SOURCES += core/config/config_pp.c
libmmcamera_target_la_SOURCES += core/config/config_proc.c
libmmcamera_target_la_SOURCES += core/config/config_proc_ctrlcmd.c
libmmcamera_target_la_SOURCES += core/mctl/mctl_stats.c
libmmcamera_target_la_SOURCES += core/mctl/mctl.c
libmmcamera_target_la_SOURCES += core/mctl/mctl_pp.c
libmmcamera_target_la_SOURCES += core/mctl/mctl_pp_c2d_cmd.c
libmmcamera_target_la_SOURCES += core/mctl/mctl_pp_dest.c
libmmcamera_target_la_SOURCES += core/mctl/mctl_pp_node.c
libmmcamera_target_la_SOURCES += core/mctl/mctl_pp_src.c
libmmcamera_target_la_SOURCES += core/mctl/mctl_pp_vpe_cmd.c
libmmcamera_target_la_SOURCES += core/mctl/mctl_ez.c
libmmcamera_target_la_SOURCES += core/mctl/mctl_af.c
libmmcamera_target_la_SOURCES += core/mctl/mctl_proc.c
libmmcamera_target_la_SOURCES += core/dsps/dsps.c
libmmcamera_target_la_SOURCES += core/dsps/dsps_interface.c
libmmcamera_target_la_SOURCES += core/eztune/eztune.c
libmmcamera_target_la_SOURCES += core/eztune/eztune_items_0208.c
libmmcamera_target_la_SOURCES += core/eztune/eztune_preview.c


#add features
#libmmcamera_target_la_SOURCES += features/stereocam/stereocam_analysis.c
#libmmcamera_target_la_SOURCES += features/stereocam/stereocam.c
#libmmcamera_target_la_SOURCES += features/stereocam/stereocam_dispatch.c
libmmcamera_target_la_SOURCES += features/bestshot/bestshot.c
libmmcamera_target_la_SOURCES += features/effects/effects.c
libmmcamera_target_la_SOURCES += features/zoom/zoom.c
libmmcamera_target_la_SOURCES += features/hdr/hdr.c
libmmcamera_target_la_SOURCES += features/live_snapshot/live_snapshot.c

#add vpe/ccp
libmmcamera_target_la_SOURCES += hardware/vpe1/vpe_proc.c
libmmcamera_target_la_SOURCES += hardware/vpe1/vpe.c

#add camif
libmmcamera_target_la_SOURCES += hardware/camif/camif_interface.c
if MSM7X27A
libmmcamera_target_la_SOURCES += hardware/camif/DSP_CAMIF/camif.c
else
libmmcamera_target_la_SOURCES += hardware/camif/ARM_CAMIF/camif.c
endif

#add eeprom
libmmcamera_target_la_SOURCES += hardware/eeprom/eeprom.c
libmmcamera_target_la_SOURCES += hardware/eeprom/eeprom_interface.c

#add flash
libmmcamera_target_la_SOURCES += hardware/flash/xenon/strobe_flash.c
libmmcamera_target_la_SOURCES += hardware/flash/flash_led_interface.c
libmmcamera_target_la_SOURCES += hardware/flash/led/led.c
libmmcamera_target_la_SOURCES += hardware/flash/flash_strobe_interface.c

#add ax
libmmcamera_target_la_SOURCES += hardware/axi/axi_interface.c
if MSM7X27A
libmmcamera_target_la_SOURCES += hardware/axi/DSP_AXI/axi.c
else
libmmcamera_target_la_SOURCES += hardware/axi/ARM_AXI/axi.c
endif

#add actuator
libmmcamera_target_la_SOURCES += hardware/actuator/actuator.c
libmmcamera_target_la_SOURCES += hardware/actuator/actuator_interface.c

#add c2d
libmmcamera_target_la_SOURCES += hardware/c2d_client/c2d_client.c

#add sensor
libmmcamera_target_la_SOURCES += hardware/sensor/imx074/imx074_u.c
libmmcamera_target_la_SOURCES += hardware/sensor/imx135/imx135_u.c
libmmcamera_target_la_SOURCES += hardware/sensor/sensor_util.c
libmmcamera_target_la_SOURCES += hardware/sensor/sensor_interface.c
libmmcamera_target_la_SOURCES += hardware/sensor/ov2720/ov2720_u.c
libmmcamera_target_la_SOURCES += hardware/sensor/sensor.c
libmmcamera_target_la_SOURCES += hardware/sensor/vx6953/vx6953_u.c
libmmcamera_target_la_SOURCES += hardware/sensor/s5k4e1/s5k4e1_u.c
libmmcamera_target_la_SOURCES += hardware/sensor/mt9e013/mt9e013_u.c
libmmcamera_target_la_SOURCES += hardware/sensor/ov9726/ov9726_u.c
libmmcamera_target_la_SOURCES += hardware/sensor/ov7692/ov7692_u.c
libmmcamera_target_la_SOURCES += hardware/sensor/ov5647/ov5647_u.c

#add vfe
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_interface/vfe_interface.c

libmmcamera_target_la_SOURCES += hardware/vfe/$(VFE_VERS)/common/vfe.c

if VFE_TEST_VEC
libmmcamera_target_la_SOURCES += hardware/vfe/$(VFE_VERS)/test/vfe_test_vector.c
endif

libmmcamera_target_la_SOURCES += hardware/vfe/common/vfe_util_common.c

#add eztune vfe util
if VFE31
#no eztune_vfe_util for vfe31
else
libmmcamera_target_la_SOURCES += hardware/vfe/$(VFE_VERS)/common/eztune/eztune_vfe_util.c
endif

libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/fovcrop/fovcrop.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/demux/demux.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/wb/wb.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/clamp/clamp.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/colorcorrect/colorcorrect.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/chroma_enhan/chroma_enhan.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/chroma_subsample/chroma_subsample.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/abf/abf.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/gamma/gamma.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/asf/asf.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/rolloff/rolloff.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/rolloff/mesh_rolloff.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/rolloff/pca_rolloff.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/rolloff/mlro_to_plro/mlro.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/rolloff/mlro_to_plro/mlro_utils.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/rolloff/mlro_to_plro/matrix_utils.c

if VFE32
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/frame_skip/frame_skip.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/demosaic/demosaic_v3.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/bpc/bcc_v3.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/bpc/bpc_v3.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/linearization/linearization_v1.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/clf/clf.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/stats/bg_stats.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/stats/bf_stats.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/stats/bhist_stats.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/mce/mce.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/chroma_suppress/chroma_suppress.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/sce/sce.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/luma_adaptation/luma_adaptation.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/stats/rs_cs_stats.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/stats/ihist_stats.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/scaler/scaler.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/stats/aec_stats.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/stats/af_stats.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/stats/awb_stats.c
else 
if VFE2X
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/stats/aec_awb_7x27a.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/stats/af_7x27a.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/demosaic/demosaic_v3.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/black_level/black_level_v1.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/bpc/bpc_v3_7x27a.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/frame_skip/frame_skip_v1.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/active_crop/active_crop.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/color_proc/color_proc.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/scaler/scaler_v1.c
else
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/frame_skip/frame_skip.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/black_level/black_level.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/demosaic/demosaic.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/bpc/bpc.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/mce/mce.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/chroma_suppress/chroma_suppress.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/sce/sce.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/luma_adaptation/luma_adaptation.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/stats/rs_cs_stats.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/stats/ihist_stats.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/scaler/scaler.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/stats/aec_stats.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/stats/af_stats.c
libmmcamera_target_la_SOURCES += hardware/vfe/vfe_modules/stats/awb_stats.c
endif
endif

libmmcamera_target_la_LDFLAGS := -static

libmmcamera_target_la_LIBADD = -lglib-2.0
libmmcamera_target_la_LIBADD += -lm
libmmcamera_target_la_LIBADD += -lrt
libmmcamera_target_la_LIBADD += -lpthread
libmmcamera_target_la_LIBADD += -ldl

libmmcamera_target_la_DEPENDENCIES := $(call override_mem_alloc_dealloc,$(libmmcamera_target_la_OBJECTS))

#libmmcamera_statsproc31
lib_LTLIBRARIES := libmmcamera_statsproc31.la

libmmcamera_statsproc31_la_CFLAGS := $(DEBUG_CFLAGS)
libmmcamera_statsproc31_la_CFLAGS += -DAMSS_VERSION=$(AMSS_VERSION)
libmmcamera_statsproc31_la_CFLAGS += -DMSM_CAMERA_GCC
libmmcamera_statsproc31_la_CFLAGS += -DLINUX_ENABLED
libmmcamera_statsproc31_la_CFLAGS += -D_V4L2_BASED_CAM_
#remove when snapshot is supported
libmmcamera_statsproc31_la_CFLAGS += -DDISABLE_JPEG_ENCODING

libmmcamera_statsproc31_la_CFLAGS += -I ../common
libmmcamera_statsproc31_la_CFLAGS += -I statsproc/aec
libmmcamera_statsproc31_la_CFLAGS += -I statsproc
libmmcamera_statsproc31_la_CFLAGS += -I chromatix/$(CHROMATIX_VERSION)

libmmcamera_statsproc31_la_SOURCES := statsproc/stats_proc.c
libmmcamera_statsproc31_la_SOURCES += statsproc/aec/aec_interface.c
libmmcamera_statsproc31_la_SOURCES += statsproc/aec/aec.c
libmmcamera_statsproc31_la_SOURCES += statsproc/aec/aec_util.c
libmmcamera_statsproc31_la_SOURCES += statsproc/aec/aec_slow_conv.c
libmmcamera_statsproc31_la_SOURCES += statsproc/aec/aec_snapshot.c
libmmcamera_statsproc31_la_SOURCES += statsproc/awb/awb_interface.c
libmmcamera_statsproc31_la_SOURCES += statsproc/awb/awb.c
libmmcamera_statsproc31_la_SOURCES += statsproc/awb/awb_util.c
libmmcamera_statsproc31_la_SOURCES += statsproc/awb/awb_self_cal.c
libmmcamera_statsproc31_la_SOURCES += statsproc/awb/awb_agw.c
libmmcamera_statsproc31_la_SOURCES += statsproc/asd/asd_interface.c
libmmcamera_statsproc31_la_SOURCES += statsproc/asd/asd.c
libmmcamera_statsproc31_la_SOURCES += statsproc/af/af_interface.c
libmmcamera_statsproc31_la_SOURCES += statsproc/af/af_util.c
libmmcamera_statsproc31_la_SOURCES += statsproc/af/af.c
libmmcamera_statsproc31_la_SOURCES += statsproc/af/af_exhstv.c
libmmcamera_statsproc31_la_SOURCES += statsproc/af/af_hillclimb.c
libmmcamera_statsproc31_la_SOURCES += statsproc/af/af_slopepredictive.c
libmmcamera_statsproc31_la_SOURCES += statsproc/dis/dis_interface.c
libmmcamera_statsproc31_la_SOURCES += statsproc/dis/dis.c
libmmcamera_statsproc31_la_SOURCES += statsproc/afd/afd_interface.c
libmmcamera_statsproc31_la_SOURCES += statsproc/afd/afd.c

libmmcamera_statsproc31_la_LDFLAGS := $(DEBUG_LDFLAGS)
libmmcamera_statsproc31_la_LDFLAGS += -shared

#libmmcamera_frameproc
lib_LTLIBRARIES += libmmcamera_frameproc.la

libmmcamera_frameproc_la_CFLAGS := $(DEBUG_CFLAGS)
libmmcamera_frameproc_la_CFLAGS += -DAMSS_VERSION=$(AMSS_VERSION)
libmmcamera_frameproc_la_CFLAGS += -DMSM_CAMERA_GCC
libmmcamera_frameproc_la_CFLAGS += -D_GNU_SOURCE
libmmcamera_frameproc_la_CFLAGS += -DLINUX_ENABLED
libmmcamera_frameproc_la_CFLAGS += -D_V4L2_BASED_CAM_
#remove when snapshot is supported
libmmcamera_frameproc_la_CFLAGS += -DDISABLE_JPEG_ENCODING
if WAVELET_DENOISE
libmmcamera_frameproc_la_CFLAGS += -DMM_CAMERA_NEON_ENABLED
endif
if FACE_PROC
libmmcamera_frameproc_la_CFLAGS += -DMM_CAMERA_FD
endif

libmmcamera_frameproc_la_CFLAGS += -I ../common -I chromatix/$(CHROMATIX_VERSION) -I frameproc
libmmcamera_frameproc_la_CFLAGS += -I frameproc/face_proc/engine
#for jpeg
libmmcamera_frameproc_la_CFLAGS += -I ../../mm-still/ipl/inc
libmmcamera_frameproc_la_CFLAGS += -I ../../mm-still/jpeg/inc

libmmcamera_frameproc_la_SOURCES := frameproc/hjr/hjr_interface.c
libmmcamera_frameproc_la_SOURCES += frameproc/hjr/hjr.c
libmmcamera_frameproc_la_SOURCES += frameproc/wavelet_denoise/wavelet_denoise_interface.c
libmmcamera_frameproc_la_SOURCES += frameproc/wavelet_denoise/wavelet_denoise.c
libmmcamera_frameproc_la_SOURCES += frameproc/hdr/hdr_interface.c
libmmcamera_frameproc_la_SOURCES += frameproc/hdr/hdr_util.c
libmmcamera_frameproc_la_SOURCES += frameproc/hdr/hdrsingleframe.c
libmmcamera_frameproc_la_SOURCES += frameproc/hdr/hdrtwoframe.c
if FACE_PROC
libmmcamera_frameproc_la_SOURCES += frameproc/frameproc.c
libmmcamera_frameproc_la_SOURCES += frameproc/face_proc/face_proc_interface.c
libmmcamera_frameproc_la_SOURCES += frameproc/face_proc/face_proc_util.c
libmmcamera_frameproc_la_SOURCES += frameproc/face_proc/engine/faceproc_engine.c
endif

libmmcamera_frameproc_la_LDFLAGS := $(DEBUG_LDFLAGS)
libmmcamera_frameproc_la_LDFLAGS += -shared
if WAVELET_DENOISE
libmmcamera_frameproc_la_LIBADD = libmmcamera_server_assembly.la
libmmcamera_frameproc_la_LIBADD += -ldl
endif

dirs :=

if ARM
dirs += chromatix/0208/libchromatix/chromatix_vx6953/preview
dirs += chromatix/0208/libchromatix/chromatix_vx6953/video
dirs += chromatix/0208/libchromatix/chromatix_s5k4e1/preview
dirs += chromatix/0208/libchromatix/chromatix_s5k4e1/video
endif

SUBDIRS = $(dirs)

