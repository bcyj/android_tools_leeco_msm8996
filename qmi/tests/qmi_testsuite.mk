# sources and intermediate files are separated
vpath %.c $(SRCDIR)

CPPFLAGS := -DFEATURE_LE_DIAG
CPPFLAGS += $(QCT_CPPFLAGS)
CPPFLAGS += -I$(SRCDIR)
CPPFLAGS += -I$(SRCDIR)/../../diag/include/
CPPFLAGS += -I$(SRCDIR)/../inc
CPPFLAGS += -I$(SRCDIR)/../src
CPPFLAGS += -I$(SRCDIR)/../platform
CPPFLAGS += -I$(SRCDIR)/../../common/inc/
CPPFLAGS += -I$(SRCDIR)/../core/lib/inc
CPPFLAGS += -I$(SRCDIR)/../services
CPPFLAGS += -I$(KERNEL_DIR)/arch/arm/include

CPPFLAGS += -DFEATURE_DATA_LOG_QXDM

CFLAGS   := $(patsubst -Werror,,$(QCT_CFLAGS))

APPS := qmi_simple_ril_test

SRCLIST := \
	qmi_client_utils.c \
	qmi_simple_ril_core.c    \
	qmi_simple_ril_voice.c    \
	qmi_simple_ril_misc.c \
	qmi_simple_ril_nas.c \
	qmi_simple_ril_sms.c \
	qmi_simple_ril_ss.c \
	qmi_simple_ril_suite.c \
	qmi_test_console.c    \

LDLIBS_TEST := $(SYSROOTLIB_DIR)/libdiag.so.$(LIBVER)
LDLIBS_TEST += $(SYSROOTLIB_DIR)/libqmiidl.so.$(LIBVER)
LDLIBS_TEST += $(SYSROOTLIB_DIR)/libqmi.so.$(LIBVER)
LDLIBS_TEST += $(SYSROOTLIB_DIR)/libqmiservices.so.$(LIBVER)

all: $(APPS)

.SECONDEXPANSION:
$(APPS) : $(SRCLIST)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS_TEST)

