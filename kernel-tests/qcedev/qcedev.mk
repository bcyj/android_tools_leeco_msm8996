#sources and intermediate files are separated
vpath %.c $(SRCDIR)/qcedev

QCEDEV_TEST_CFLAGS := $(CFLAGS)
QCEDEV_TEST_CFLAGS += $(QCT_CFLAGS)

QCEDEV_TEST_CPPFLAGS := $(CPPFLAGS)
QCEDEV_TEST_CPPFLAGS += $(QCT_CPPFLAGS)

APP_NAME := qcedev_test
SRCLIST  := qcedev_test.c

all: $(APP_NAME)

$(APP_NAME): $(SRCLIST)
	$(CC) $(QCEDEV_TEST_CFLAGS) $(QCEDEV_CPPFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
