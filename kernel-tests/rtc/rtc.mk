#sources and intermediate files are separated
vpath %.c $(SRCDIR)/rtc

RTCTEST_CFLAGS := $(CFLAGS)
RTCTEST_CFLAGS += $(QCT_CFLAGS)
  
RTCTEST_CPPFLAGS := $(CPPFLAGS)
RTCTEST_CPPFLAGS += $(QCT_CPPFLAGS)

RTCTEST_LDLIBS := $(LDLIBS)
RTCTEST_LDLIBS += -lstringl

APP_NAME := rtc_test
SRCLIST  := rtc_test.c

all: $(APP_NAME)

$(APP_NAME): $(SRCLIST)
	$(CC) $(RTCTEST_CFLAGS) $(RTCTEST_CPPFLAGS) $(LDFLAGS) -o $@ $^ $(RTCTEST_LDLIBS)
