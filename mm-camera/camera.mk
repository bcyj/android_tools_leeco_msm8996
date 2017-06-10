# sources and intermediate files are separated
# Path settings

QCAM_DIR  := $(SRCDIR)
STILL_DIR := $(SRCDIR)/../mm-still

ifeq "$(MSM_VERSION)" "8650"
QCAM_TARGET = 8k
else
QCAM_TARGET = 7k
endif

CFLAGS   := $(QCT_CFLAGS)
CFLAGS   += -Werror

CPPFLAGS := $(QCT_CPPFLAGS)

CPPFLAGS += -DHERE='fprintf(stderr, "%s %s(%d)\n", __FILE__, __FUNCTION__, __LINE__)'
CPPFLAGS += -DFEATURE_QDSP_RTOS
CPPFLAGS += -DTRACE_ARM_DSP
CPPFLAGS += -DMSM7600
#CPPFLAGS += -Dsize_t="unsigned int"
CPPFLAGS += -Wall

CPPFLAGS += -I$(KERNEL_DIR)/include
CPPFLAGS += -I$(KERNEL_OBJDIR)/include
CPPFLAGS += -I$(KERNEL_OBJDIR)/include2
CPPFLAGS += -I$(KERNEL_DIR)/arch/arm/include
CPPFLAGS += -I$(SYSCORE_DIR)/include
CPPFLAGS += -I$(SRCDIR)/../common/inc

CPPFLAGS += -include stddef.h #for size_t
CPPFLAGS += -include camera_defs_i.h


###############################################################################
# Build the target library
###############################################################################

TARGET_INC += -I$(QCAM_DIR)/common
TARGET_INC += -I$(QCAM_DIR)/targets/tgtcommon
TARGET_INC += -I$(QCAM_DIR)/targets/tgtcommon/chromatix/0204
TARGET_INC += -I$(QCAM_DIR)/targets/tgtcommon/config
TARGET_INC += -I$(QCAM_DIR)/targets/tgtcommon/effects
TARGET_INC += -I$(QCAM_DIR)/targets/tgtcommon/flash
TARGET_INC += -I$(QCAM_DIR)/targets/tgtcommon/flash/xenon
TARGET_INC += -I$(QCAM_DIR)/targets/tgtcommon/isp3a
TARGET_INC += -I$(QCAM_DIR)/targets/tgtcommon/isp3a/aec
TARGET_INC += -I$(QCAM_DIR)/targets/tgtcommon/isp3a/af
TARGET_INC += -I$(QCAM_DIR)/targets/tgtcommon/isp3a/awb
TARGET_INC += -I$(QCAM_DIR)/targets/tgtcommon/postproc
TARGET_INC += -I$(QCAM_DIR)/targets/tgtcommon/postproc/hjr
TARGET_INC += -I$(QCAM_DIR)/targets/tgtcommon/postproc/afd
TARGET_INC += -I$(QCAM_DIR)/targets/tgtcommon/sensor
TARGET_INC += -I$(QCAM_DIR)/targets/tgtcommon/sensor/mt9d112
TARGET_INC += -I$(QCAM_DIR)/targets/tgtcommon/sensor/mt9p012
TARGET_INC += -I$(QCAM_DIR)/targets/tgtcommon/sensor/mt9p012_km
TARGET_INC += -I$(QCAM_DIR)/targets/tgtcommon/sensor/mt9t013
TARGET_INC += -I$(QCAM_DIR)/targets/tgtcommon/sensor/s5k3e2fx
TARGET_INC += -I$(QCAM_DIR)/targets/tgtcommon/sensor/vb6801
TARGET_INC += -I$(QCAM_DIR)/targets/tgtcommon/sensor/vx6953
TARGET_INC += -I$(QCAM_DIR)/targets/tgtcommon/sensor/ov9726
TARGET_INC += -I$(QCAM_DIR)/targets/tgtcommon/sensor/ov5640
TARGET_INC += -I$(QCAM_DIR)/targets/tgtcommon/sensor/ov5647
TARGET_INC += -I$(QCAM_DIR)/targets/tgtcommon/sensor/ov7692
TARGET_INC += -I$(QCAM_DIR)/targets/tgtcommon/zoom
TARGET_INC += -I$(QCAM_DIR)/targets/$(QCAM_TARGET)/$(MSM_VERSION)
TARGET_INC += -I$(QCAM_DIR)/targets/$(QCAM_TARGET)/$(MSM_VERSION)/lpm
TARGET_INC += -I$(QCAM_DIR)/apps/test
TARGET_INC += -I$(STILL_DIR)/ipl/inc
ifeq ($(QCAM_TARGET),vfe31)
TARGET_INC += -I$(QCAM_DIR)/targets/$(QCAM_TARGET)/common/dis
TARGET_INC += -I$(QCAM_DIR)/targets/$(QCAM_TARGET)/common/vpe1
endif

TARGET_SRC  = $(QCAM_DIR)/common/cam_mmap.c
TARGET_SRC += $(QCAM_DIR)/targets/tgtcommon/config/config.c
TARGET_SRC += $(QCAM_DIR)/targets/tgtcommon/config/config_proc.c
TARGET_SRC += $(QCAM_DIR)/targets/tgtcommon/config/config_proc_ctrlcmd.c
TARGET_SRC += $(QCAM_DIR)/targets/tgtcommon/effects/effects.c
TARGET_SRC += $(QCAM_DIR)/targets/tgtcommon/flash/led.c
TARGET_SRC += $(QCAM_DIR)/targets/tgtcommon/isp3a/isp3a.c
TARGET_SRC += $(QCAM_DIR)/targets/tgtcommon/isp3a/aec/aec.c
TARGET_SRC += $(QCAM_DIR)/targets/tgtcommon/isp3a/af/af.c
TARGET_SRC += $(QCAM_DIR)/targets/tgtcommon/isp3a/af/af_core.c
TARGET_SRC += $(QCAM_DIR)/targets/tgtcommon/isp3a/awb/awb.c
TARGET_SRC += $(QCAM_DIR)/targets/tgtcommon/isp3a/awb/awb_core.c
TARGET_SRC += $(QCAM_DIR)/targets/tgtcommon/postproc/postproc.c
TARGET_SRC += $(QCAM_DIR)/targets/tgtcommon/postproc/hjr/hjr.c
TARGET_SRC += $(QCAM_DIR)/targets/tgtcommon/postproc/afd/afd.c
TARGET_SRC += $(QCAM_DIR)/targets/tgtcommon/sensor/sensor.c
TARGET_SRC += $(QCAM_DIR)/targets/tgtcommon/sensor/mt9d112/mt9d112.c
TARGET_SRC += $(QCAM_DIR)/targets/tgtcommon/sensor/mt9p012/mt9p012.c
TARGET_SRC += $(QCAM_DIR)/targets/tgtcommon/sensor/mt9p012_km/mt9p012_km.c
TARGET_SRC += $(QCAM_DIR)/targets/tgtcommon/sensor/mt9t013/mt9t013.c
TARGET_SRC += $(QCAM_DIR)/targets/tgtcommon/sensor/s5k3e2fx/s5k3e2fx_u.c
TARGET_SRC += $(QCAM_DIR)/targets/tgtcommon/sensor/vb6801/vb6801.c
TARGET_SRC += $(QCAM_DIR)/targets/tgtcommon/sensor/vx6953/vx6953_u.c
TARGET_SRC += $(QCAM_DIR)/targets/tgtcommon/sensor/ov9726/ov9726_u.c
TARGET_SRC += $(QCAM_DIR)/targets/tgtcommon/sensor/ov5640/ov5640_u.c
TARGET_SRC += $(QCAM_DIR)/targets/tgtcommon/sensor/ov5647/ov5647_u.c
TARGET_SRC += $(QCAM_DIR)/targets/tgtcommon/sensor/ov7692/ov7692_u.c
TARGET_SRC += $(QCAM_DIR)/targets/tgtcommon/zoom/zoom.c
TARGET_SRC += $(QCAM_DIR)/targets/$(QCAM_TARGET)/$(MSM_VERSION)/vfe.c
TARGET_SRC += $(QCAM_DIR)/targets/$(QCAM_TARGET)/$(MSM_VERSION)/vfe_preview.c
TARGET_SRC += $(QCAM_DIR)/targets/$(QCAM_TARGET)/$(MSM_VERSION)/vfe_video.c
TARGET_SRC += $(QCAM_DIR)/targets/$(QCAM_TARGET)/$(MSM_VERSION)/vfe_proc_msg_evt.c
TARGET_SRC += $(QCAM_DIR)/targets/$(QCAM_TARGET)/$(MSM_VERSION)/vfe_snapshot.c
TARGET_SRC += $(QCAM_DIR)/targets/$(QCAM_TARGET)/$(MSM_VERSION)/vfe_util.c
TARGET_SRC += $(QCAM_DIR)/targets/tgtcommon/flash/xenon/strobe_flash.c

ifeq ($(MSM_VERSION),7x0x)
TARGET_SRC += $(QCAM_DIR)/targets/$(QCAM_TARGET)/$(MSM_VERSION)/lpm/lpm.c
endif

ifeq ($(QCAM_TARGET),vfe31)
TARGET_SRC += $(QCAM_DIR)/targets/$(QCAM_TARGET)/$(MSM_VERSION)/vfe_proc_stats.c
TARGET_SRC += $(QCAM_DIR)/targets/$(QCAM_TARGET)/common/dis/dis.c
TARGET_SRC += $(QCAM_DIR)/targets/$(QCAM_TARGET)/common/vpe1/vpe.c
TARGET_SRC += $(QCAM_DIR)/targets/$(QCAM_TARGET)/common/vpe1/vpe_proc.c
endif

all: libmm-camera-tgt.so.$(LIBVER)

libmm-camera-tgt.so.$(LIBVER): $(TARGET_SRC)
	$(CC) $(CPPFLAGS) $(TARGET_INC) $(QCT_CFLAGS_SO) $(QCT_LDFLAGS_SO) -Wl,-soname,libmm-camera-tgt.so.$(LIBMAJOR) -o $@ $^ -lpthread

###############################################################################
# Build the Apps library
###############################################################################

QCAM_INC := -I$(QCAM_DIR)/common
QCAM_INC += -I$(QCAM_DIR)/apps/appslib
QCAM_INC += -I$(STILL_DIR)/jpeg
QCAM_INC += -I$(STILL_DIR)/jpeg/inc

QCAM_SRC := $(QCAM_DIR)/apps/appslib/camframe.c
QCAM_SRC += $(QCAM_DIR)/apps/appslib/jpeg_encoder.c
QCAM_SRC += $(QCAM_DIR)/common/cam_mmap.c

all: libmm-camera.so.$(LIBVER)

libmm-camera.so.$(LIBVER): $(QCAM_SRC)
	$(CC) $(CPPFLAGS) $(QCAM_INC) $(QCT_CFLAGS_SO) $(QCT_LDFLAGS_SO) -Wl,-soname,libmm-camera.so.$(LIBMAJOR) -o $@ $^ -lpthread

###############################################################################
# Test Target
###############################################################################

mm-camera-test: libmm-camera.so.$(LIBVER) libmm-camera-tgt.so.$(LIBVER)

vpath %.c $(QCAM_TEST)

MM_CAMERA_TEST_LDLIBS += -lpthread -lm -lrt -ldl
MM_CAMERA_TEST_LDLIBS += libmm-camera.so.$(LIBVER)
MM_CAMERA_TEST_LDLIBS += libmm-camera-tgt.so.$(LIBVER)
MM_CAMERA_TEST_LDLIBS += $(SYSROOT_DIR)/lib/libmm-ipl.so
MM_CAMERA_TEST_LDLIBS += $(SYSROOT_DIR)/lib/libmm-jpeg.so

TEST_INC := -I$(QCAM_DIR)/apps/appslib
TEST_INC += -I$(QCAM_DIR)/common
TEST_INC += -I$(STILL_DIR)/jpeg
TEST_INC += -I$(STILL_DIR)/jpeg/inc
TEST_INC += -I$(QCAM_DIR)/common/inc

TEST_SRC := $(QCAM_DIR)/apps/test/main.c
TEST_SRC += $(QCAM_DIR)/apps/test/native_cam.c
TEST_SRC += $(QCAM_DIR)/apps/test/video_cam.c
TEST_SRC += $(QCAM_DIR)/apps/test/camera_testsuite.c
TEST_SRC += $(QCAM_DIR)/apps/test/testsuite_server.c
TEST_SRC += $(QCAM_DIR)/apps/test/testsuite_utilities.c
TEST_SRC += $(QCAM_DIR)/apps/appslib/camaf_ctrl.c
#TEST_SRC += $(QCAM_DIR)/common/cam_mmap.c

TESTSUITE_CLIENT_SRC = $(QCAM_DIR)/apps/test/testsuite_client.c
TESTSUITE_CLIENT_SRC += $(QCAM_DIR)/apps/test/testsuite_utilities.c

all: mm-camera-test mm-qcamera-testsuite-client

mm-camera-test: $(TEST_SRC)
	$(CC) $(CPPFLAGS) $(TEST_INC) $(LDFLAGS) -o $@ $^ $(MM_CAMERA_TEST_LDLIBS)

mm-qcamera-testsuite-client: $(TESTSUITE_CLIENT_SRC)
	$(CC) $(TESTSUITE_CLIENT_INC) -o $@ $^

