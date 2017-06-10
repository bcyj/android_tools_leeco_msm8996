################################################################################
# Copyright (c) 2012, 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
################################################################################

vpath %.c $(SRCDIR)/common/src
vpath %.c $(SRCDIR)/encdec
vpath %.c $(SRCDIR)/smem_log
vpath %.c $(SRCDIR)/qcci/src
vpath %.c $(SRCDIR)/qcsi/src

CFLAGS = -Wundef \
	-Wstrict-prototypes \
	-Wno-trigraphs \
	-g -O0 \
	-fno-inline \
	-fno-short-enums \
	-fpic

CPPFLAGS += \
	-I$(SRCDIR)/inc \
	-I$(SRCDIR)/common/inc \
	-I$(SRCDIR)/qcci/inc \
	-I$(SRCDIR)/qcsi/inc \
	-I$(SRCDIR)/smem_log \
	-DQMI_FW_SYSLOG \
	-DQMI_FW_SYSLOG_LEVEL=LOG_ERR

QMI_COMMON_SRC := \
	common_v01.c
QMI_COMMON_OBJ := $(patsubst %.c,%.o,$(QMI_COMMON_SRC))

QMI_ENCDEC_SRC := \
	qmi_idl_accessor.c \
	qmi_idl_lib.c
QMI_ENCDEC_OBJ := $(patsubst %.c,%.o,$(QMI_ENCDEC_SRC))

QMI_CCI_SRC := \
	qmi_cci_common.c \
	qmi_cci_target.c \
	qmi_cci_xport_ipc_router.c \
	smem_log.c
QMI_CCI_OBJ := $(patsubst %.c,%.o,$(QMI_CCI_SRC))

QMI_CSI_SRC := \
	qmi_csi_common.c \
	qmi_csi_target.c \
	qmi_csi_xport_ipc_router.c \
	smem_log.c
QMI_CSI_OBJ := $(patsubst %.c,%.o,$(QMI_CSI_SRC))

LDLIBS += -lpthread

TARGET_SO = libqmi_common.so.$(LIBVER) libqmi_encdec.so.$(LIBVER) libqmi_cci.so.$(LIBVER) libqmi_csi.so.$(LIBVER)
SUBDIRS = test_service

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -MD -MP -c -o $@ $<

all: $(TARGET_SO)
	@for dir in $(SUBDIRS); \
		do $(MAKE) -f $(SRCDIR)/$$dir/qmi-framework.mk SRCDIR=$(SRCDIR)/$$dir LIBVER=$(LIBVER) SYSROOTLIB_DIR=$(SYSROOTLIB_DIR); \
	done

DEP=${wildcard *.d}
-include ${DEP}

libqmi_common.so.$(LIBVER): $(QMI_COMMON_OBJ)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(QCT_CFLAGS_SO) $(QCT_LDFLAGS_SO) $(LDFLAGS) -Wl,-soname,libqmi_common.so.$(LIBMAJOR) -o $@ $^ $(LDLIBS)

libqmi_encdec.so.$(LIBVER): $(QMI_ENCDEC_OBJ)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(QCT_CFLAGS_SO) $(QCT_LDFLAGS_SO) $(LDFLAGS) -Wl,-soname,libqmi_encdec.so.$(LIBMAJOR) -o $@ $^ $(LDLIBS)

libqmi_cci.so.$(LIBVER): $(QMI_CCI_OBJ)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(QCT_CFLAGS_SO) $(QCT_LDFLAGS_SO) $(LDFLAGS) -Wl,-soname,libqmi_cci.so.$(LIBMAJOR) -o $@ $^ $(LDLIBS)

libqmi_csi.so.$(LIBVER): $(QMI_CSI_OBJ)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(QCT_CFLAGS_SO) $(QCT_LDFLAGS_SO) $(LDFLAGS) -Wl,-soname,libqmi_csi.so.$(LIBMAJOR) -o $@ $^ $(LDLIBS)

