CPPFLAGS = $(QCT_CPPFLAGS)
CFLAGS   = $(QCT_CFLAGS)

#CPU      := -mcpu=arm1136j-s

#CPPFLAGS += $(CPU)
CPPFLAGS += -O3
CPPFLAGS += -Werror

CPPFLAGS += -I$(SRCDIR)/jpeg/inc
CPPFLAGS += -I$(SRCDIR)/jpeg/src
CPPFLAGS += -I$(SRCDIR)/jpeg/src/os
CPPFLAGS += -I$(KERNEL_DIR)/include
CPPFLAGS += -I$(KERNEL_OBJDIR)/include
CPPFLAGS += -I$(KERNEL_OBJDIR)/include2

CPPFLAGS += -DFEATURE_QDSP_RTOS
CPPFLAGS += -DTRACE_ARM_DSP
CPPFLAGS += -DMSM7600
CPPFLAGS += -g3
CPPFLAGS += -D_DEBUG

ifeq "$(findstring qsd8650,$(PTXDIST_PLATFORMCONFIG))" "qsd8650"
CPPFLAGS += -DARM_ARCH_7A
endif

all: libmm-jpeg.so.$(LIBVER) mm-jpeg-dec-test mm-jpeg-enc-test

###############################################################################
# jpeg library
###############################################################################

vpath %.c $(SRCDIR)/jpeg/src
vpath %.c $(SRCDIR)/jpeg/src/os
vpath %.cpp $(SRCDIR)/jpeg/src
ifeq "$(findstring qsd8650,$(PTXDIST_PLATFORMCONFIG))" "qsd8650"
vpath %.a $(SRCDIR)/jpeg/lib/os/armv7
else
vpath %.a $(SRCDIR)/jpeg/lib/os/armv6
endif

SRCS  = jpege.c
SRCS += jpegd.c
SRCS += jpege_engine_sw.c
SRCS += jpege_engine_sw_fetch_dct.c
SRCS += jpege_engine_sw_huff.c
SRCS += jpege_engine_sw_upscale.c
SRCS += jpege_engine_bs.c
SRCS += jpege_engine_q5.c
SRCS += jpegd_engine_utils.c
SRCS += jpegd_engine_sw.c
SRCS += jpegd_engine_sw_progressive.c
SRCS += jpegd_engine_sw_utils.c
SRCS += jpegd_engine_sw_idct.c
SRCS += jpegd_engine_sw_huff.c
SRCS += jpegd_engine_q5.c
SRCS += jpeg_q5_helper_sp.c
SRCS += jpeg_writer.c
SRCS += jpeg_file_size_control.c
SRCS += jpeg_reader.c
SRCS += jpeg_buffer.c
SRCS += jpeg_header.c
SRCS += jpeg_debug.c
SRCS += jpeg_postprocessor.c
SRCS += jpeg_postprocess_config.c
SRCS += jpeg_postprocess_dm.c
SRCS += jpeg_postprocess_cc.c
SRCS += jpeg_postprocess_ds.c
SRCS += jpeg_postprocess_yuv2rgb.c
SRCS += exif.c
SRCS += exif_defaults.c
SRCS += os_thread_sp.c
SRCS += os_timer_sp.c
SRCS += os_pmem_sp.c

# Add platform-specific source files
ifeq ($(call is-board-platform-in-list,$(QSD8K_BOARD_PLATFORMS)),true)
SRCS += jpege_englist_sw_only.c
SRCS += jpegd_englist_sw_only.c
else
SRCS += jpege_englist_q5_sw.c
SRCS += jpegd_englist_q5_sw.c
endif

LIBS := libmmjpeg-enc-rvct.a
LIBS += libmmjpeg-dec-rvct.a

libmm-jpeg.so.$(LIBVER): $(SRCS) $(LIBS)
	$(CC) $(CPPFLAGS) $(QCT_CFLAGS_SO) $(QCT_LDFLAGS_SO) -Wl,-soname,libmm-jpeg.so.$(LIBMAJOR) -o $@ $^ -lpthread

###############################################################################
# Test Target - Jpeg Decoder
###############################################################################

vpath %.c $(SRCDIR)/jpeg/test

CPPFLAGS += -DSCREEN_DUMP_SUPPORTED

MM_JPEG_DEC_TEST_LDLIBS  = -lpthread
MM_JPEG_DEC_TEST_LDLIBS += -lrt
MM_JPEG_DEC_TEST_LDLIBS += -lm
MM_JPEG_DEC_TEST_LDLIBS += libmm-jpeg.so.$(LIBVER)

MM_JPEG_DEC_TEST_SRCS := decoder_test.c

mm-jpeg-dec-test: $(MM_JPEG_DEC_TEST_SRCS) libmm-jpeg.so.$(LIBVER)
	$(CC) $(CPPFLAGS) $(LDFLAGS) -o $@ $^ $(MM_JPEG_DEC_TEST_LDLIBS)

###############################################################################
# Test Target - Jpeg Encoder
###############################################################################

vpath %.c $(SRCDIR)/jpeg/test

MM_JPEG_ENC_TEST_LDLIBS  = -lpthread
MM_JPEG_ENC_TEST_LDLIBS += -lrt
MM_JPEG_ENC_TEST_LDLIBS += -lm
MM_JPEG_ENC_TEST_LDLIBS += libmm-jpeg.so.$(LIBVER)

MM_JPEG_ENC_TEST_SRCS := encoder_test.c

mm-jpeg-enc-test: $(MM_JPEG_ENC_TEST_SRCS) libmm-jpeg.so.$(LIBVER)
	$(CC) $(CPPFLAGS) $(LDFLAGS) -o $@ $^ $(MM_JPEG_ENC_TEST_LDLIBS)

