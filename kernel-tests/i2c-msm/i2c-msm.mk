#sources and intermediate files are separated
vpath %.c $(SRCDIR)/i2c-msm

GPIO_TLMM_TEST_CFLAGS := $(CFLAGS)
GPIO_TLMM_TEST_CFLAGS += $(QCT_CFLAGS)
  
GPIO_TLMM_TEST_CPPFLAGS := $(CPPFLAGS)
GPIO_TLMM_TEST_CPPFLAGS += $(QCT_CPPFLAGS)
  
APP_NAME := i2c_msm_test
SRCLIST  := i2c-msm-test.c

all: $(APP_NAME)

$(APP_NAME): $(SRCLIST)
	$(CC) $(GPIO_TLMM_TEST_CFLAGS) $(GPIO_TLMM_TEST_CPPFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
