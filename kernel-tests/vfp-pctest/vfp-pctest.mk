#sources and intermediate files are separated
vpath %.c $(SRCDIR)/vfp-pctest
vpath %.s $(SRCDIR)/vfp-pctest

VFP_PCTEST_CFLAGS := $(CFLAGS)
VFP_PCTEST_CFLAGS += $(QCT_CFLAGS)

VFP_PCTEST_CPPFLAGS := $(CPPFLAGS)
VFP_PCTEST_CPPFLAGS += $(QCT_CPPFLAGS)
VFP_PCTEST_CPPFLAGS += -I$(KERNEL_DIR)/include

VFP_PCTEST_LDLIBS   := $(LDLIBS)
VFP_PCTEST_LDFLAGS   := -static

VFP_NEON_FLAGS := -mcpu='cortex-a8' -mfpu='neon'
VFP_VFPV2_FLAGS := -mcpu='arm1136jf-s' -mfpu='arm1136jf-s'


APP_NAME := vfp-pctest

SRCLIST  := vfp-pctest.c ARMv7neon_sleepRestore.o_neon arm11vfp_sleepRestore.o_vfpv2

all: $(APP_NAME)

%.o_neon: %.s
	$(CC) $(VFP_NEON_FLAGS) -c -o $@ $^

%.o_vfpv2: %.s
	$(CC) $(VFP_VFPV2_FLAGS) -c -o $@ $^

$(APP_NAME): $(SRCLIST)
	$(CC) $(VFP_PCTEST_CFLAGS) $(VFP_PCTEST_CPPFLAGS) $(VFP_PCTEST_LDFLAGS) -o $@ $^ $(VFP_PCTEST_LDLIBS)

clean:
	rm -rf *.o_neon *.o_vfpv2
	rm -rf $(APP_NAME)

