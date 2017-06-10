# sources and intermediate files are separated
vpath %.c $(SRCDIR)

CFLAGS += $(QCT_CFLAGS)
CPPFLAGS += $(QCT_CPPFLAGS)
CPPFLAGS += -I$(SRCDIR)
CPPFLAGS += -I$(SRCDIR)/../inc
CPPFLAGS += -I$(SRCDIR)/../src
CPPFLAGS += -I$(SRCDIR)/../platform
CPPFLAGS += -I$(SRCDIR)/../../legacy-smd/inc -Isystem/kernel_headers/
CPPFLAGS += -I$(SRCDIR)/../../diag/include
CPPFLAGS += -I$(SRCDIR)/../../common/inc
CPPFLAGS += -I$(SRCDIR)/../core/lib/inc
CPPFLAGS += -I$(KERNEL_DIR)/arch/arm/include

#CPPFLAGS += -DFEATURE_DATA_LOG_ADB
#CPPFLAGS += -DFEATURE_DATA_LOG_STDERR
CPPFLAGS += -DFEATURE_DATA_LOG_QXDM

CFLAGS := $(patsubst -Werror,,$(QCT_CFLAGS))

QMUX_SRC_FILES := ../platform/qmi_platform.c
QMUX_SRC_FILES += ../platform/linux_qmi_qmux_if_server.c
QMUX_SRC_FILES += ../platform/qmi_platform_qmux_io.c
QMUX_SRC_FILES += ../src/qmi_qmux.c
QMUX_SRC_FILES += ../src/qmi_util.c

LDLIBS += $(SYSROOTLIB_DIR)/libdiag.so.$(LIBVER)

all:qmuxd

qmuxd:$(QMUX_SRC_FILES)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(LDLIBS) -o $@ $^ -lpthread

clean:
	rm -rf *.o qmuxd
