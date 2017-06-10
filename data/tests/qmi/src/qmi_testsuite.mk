################################################################################
# $Id: //source/qcom/qct/platform/linux/tests/main/latest/unit/libs/dss/src/Makefile#7 $
# 
# @file tests/unit/libs/dss/src/Makefile
# @brief Makefile for building dss api tests.
#
################################################################################


vpath %.c $(SRCDIR)

CPPFLAGS += -g
CPPFLAGS += -I$(SRCDIR)
CPPFLAGS += -I$(TARGET_OUT_HEADERS)/qmi/inc
CPPFLAGS += $(QCT_CPPFLAGS)

CFLAGS += $(QCT_CFLAGS)
CFLAGS += $(QCT_CLFAGS_SO)

LDLIBS += $(OBJDIR)/libqmi.so.$(LIBVER)
LDLIBS += -lstringl
LDLIBS += -lpthread

APPS += qmi_all_call_bringup_test
APPS += qmi_umts_profile_config_test
APPS += qmi_umts_mult_pdp_test
APPS += qmi_umts_same_apn_test
APPS += qmi_umts_diff_apn_test

# Test for all configurations regardless of technology
SRCLIST-qmi_all_call_bringup_test := qmi_all_call_bringup_test.c

# UMTS specific tests
SRCLIST-qmi_umts_profile_config_test := qmi_umts_profile_config_test.c
SRCLIST-qmi_umts_mult_pdp_test := qmi_umts_mult_pdp_test.c
SRCLIST-qmi_umts_qos_test := qmi_umts_qos_test.c
SRCLIST-qmi_umts_same_apn_test := qmi_umts_same_apn_test.c
SRCLIST-qmi_umts_diff_apn_test := qmi_umts_diff_apn_test.c

LDLIBS += -ldiag

all: $(APPS)

.SECONDEXPANSION:
$(APPS) : $$(SRCLIST-$$@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
