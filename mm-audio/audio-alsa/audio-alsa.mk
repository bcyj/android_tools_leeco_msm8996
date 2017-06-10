# ---------------------------------------------------------------------------------
#				MM-OMX-ADEC_AAC
# ---------------------------------------------------------------------------------

# Source Path
AUDIO_ALSA := $(SRCDIR)/audio-alsa

# cross-compiler flags
CFLAGS := -Wall 
CFLAGS += -Wundef 
CFLAGS += -Wstrict-prototypes 
CFLAGS += -Wno-trigraphs 

# cross-compile flags specific to shared objects
CFLAGS_SO := $(QCT_CFLAGS_SO)

# Preproc flags
CPPFLAGS := $(QCT_CPPFLAGS)
CPPFLAGS += -D_ENABLE_QC_MSG_LOG_
CPPFLAGS += -DAUDIOV2

# linker flags for shared objects
LDFLAGS_SO += -shared

# linker flags
LDFLAGS := -L$(SYSROOTLIB_DIR)

# hard coding target for 7630
TARGET := 7630

# ---------------------------------------------------------------------------------
#					BUILD
# ---------------------------------------------------------------------------------

all: deploy_headers libaudioalsa.so mm-audio-alsa-test

# ---------------------------------------------------------------------------------
#				Deploy Headers
# ---------------------------------------------------------------------------------

deploy_headers:
	@mkdir -p $(SYSROOTINC_DIR)/mm-audio
	@cp -f $(AUDIO_ALSA)/inc/*.h $(SYSROOTINC_DIR)/mm-audio

# ---------------------------------------------------------------------------------
#				COMPILE LIBRARY
# ---------------------------------------------------------------------------------

SRCS += $(AUDIO_ALSA)/src/hw.c

CPPFLAGS += -I$(AUDIO_ALSA)/inc
CPPFLAGS += -I$(KERNEL_DIR)/include
CPPFLAGS += -I$(KERNEL_DIR)/arch/arm/include

CPPFLAGS += -DAUDIOV2

LDLIBS := -lrt
LDLIBS += -lpthread

libaudioalsa.so:$(SRCS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CFLAGS_SO) $(LDFLAGS_SO) -Wl,-soname,libaudioalsa.so -o $@ $^ $(LDLIBS)

# ---------------------------------------------------------------------------------
#				COMPILE TEST APP
# ---------------------------------------------------------------------------------

TEST_LDLIBS := -lpthread
TEST_LDLIBS += -ldl

SRCS := $(AUDIO_ALSA)/test/client.c

mm-audio-alsa-test: libaudioalsa.so $(SRCS)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $^ $(TEST_LDLIBS)

# ---------------------------------------------------------------------------------
#					END
# ---------------------------------------------------------------------------------
