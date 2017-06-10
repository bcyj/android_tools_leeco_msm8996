# sources and intermediate files are separated
vpath %.c $(SRCDIR)/src
vpath %.c $(SRCDIR)/platform
vpath %.c $(SRCDIR)/core/lib/src
vpath %.c $(SRCDIR)/services
vpath %.c $(SRCDIR)/tests

CPPFLAGS += -DFEATURE_LE_DIAG 
CPPFLAGS += $(QCT_CPPFLAGS)
CPPFLAGS += -I$(SRCDIR)
CPPFLAGS += -I$(SRCDIR)/../diag/include/
CPPFLAGS += -I$(SRCDIR)/inc
CPPFLAGS += -I$(SRCDIR)/src
CPPFLAGS += -I$(SRCDIR)/platform
CPPFLAGS += -I$(SRCDIR)/../common/inc/
CPPFLAGS += -I$(SRCDIR)/core/lib/inc
CPPFLAGS += -I$(SRCDIR)/tests
CPPFLAGS += -I$(SRCDIR)/services
CPPFLAGS += -I$(SRCDIR)/proxy
CPPFLAGS += -I$(KERNEL_DIR)/arch/arm/include

#CPPFLAGS += -DFEATURE_DATA_LOG_ADB
#CPPFLAGS += -DFEATURE_DATA_LOG_STDERR
CPPFLAGS += -DFEATURE_DATA_LOG_QXDM

CFLAGS := $(patsubst -Werror,,$(QCT_CFLAGS))

QMI_SRCS := \
	linux_qmi_qmux_if_client.c \
	qmi_platform.c \
	qmi_service.c \
	qmi.c \
	qmi_cat_srvc.c \
	qmi_qmux_if.c \
	qmi_wds_srvc.c \
	qmi_qos_srvc.c \
	qmi_nas_srvc.c \
	qmi_eap_srvc.c \
	qmi_atcop_srvc.c \
	qmi_util.c \
	qmi_client.c \
	qmi_uim_srvc.c

QMI_IDL_SRCS := qmi_idl_lib.c

QMI_SERVICES_SRCS := \
        common_v01.c\
	voice_service_v02.c \
        wireless_data_service_v01.c\
        wireless_messaging_service_v01.c\
	device_management_service_v01.c \
	network_access_service_v01.c \
	user_identity_module_v01.c \
	user_identity_module_remote_v01.c \
	card_application_toolkit_v02.c \
	phonebook_manager_service_v01.c \
	control_service_v01.c \
	radio_frequency_radiated_performance_enhancement_v01.c\
	secure_filesystem_service_v01.c

LDLIBS += $(SYSROOTLIB_DIR)/libdiag.so.$(LIBVER)
LDLIBS += -lpthread

LDLIBS_SERVICES += $(SYSROOTLIB_DIR)/libqmiidl.so.$(LIBVER)
LDLIBS_SERVICES += $(SYSROOTLIB_DIR)/libqmi.so.$(LIBVER)

all: libqmi.so.$(LIBVER) libqmiidl.so.$(LIBVER) libqmiservices.so.$(LIBVER)

libqmi.so.$(LIBVER): $(QMI_SRCS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(QCT_CFLAGS_SO) $(QCT_LDFLAGS_SO) $(LDFLAGS) -Wl,-soname,libqmi.so.$(LIBMAJOR) -o $@ $^ $(LDLIBS)

libqmiidl.so.$(LIBVER): $(QMI_IDL_SRCS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(QCT_CFLAGS_SO) $(QCT_LDFLAGS_SO) $(LDFLAGS) -Wl,-soname,libqmiidl.so.$(LIBMAJOR) -o $@ $^

libqmiservices.so.$(LIBVER) : $(QMI_SERVICES_SRCS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(QCT_CFLAGS_SO) $(QCT_LDFLAGS_SO) $(LDFLAGS) -Wl,-soname,libqmiservices.so.$(LIBMAJOR) -o $@ $^ $(LDLIBS_SERVICS)

