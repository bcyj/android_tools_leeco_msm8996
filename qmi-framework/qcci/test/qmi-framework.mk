################################################################################
# Copyright (c) 2012 by Qualcomm Technologies, Inc.  All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
################################################################################

vpath %.c $(SRCDIR)

CFLAGS = -Wundef \
	-Wstrict-prototypes \
	-Wno-trigraphs \
	-Wp,-w \
	-g -O0 \
	-fno-inline \
	-fno-short-enums \
	-fpic

CFLAGS += \
	-I$(SRCDIR)/../../common/inc \
	-I$(SRCDIR)/../../inc \
	-I$(SRCDIR)/../inc \

common_sources = qmi_ping_api_v01.c qmi_ping_clnt_common.c qmi_ping_clnt_common_stats.c
common_objs = $(patsubst %.c,%.o,$(common_sources))
test_sources = qmi_ping_clnt_test_0000.c qmi_ping_clnt_test_0001.c qmi_ping_clnt_test_1000.c qmi_ping_clnt_test_1001.c qmi_ping_clnt_test_2000.c
test_bins = $(patsubst %.c,%,$(test_sources))
common_libs = -lpthread -lrt libqmi_cci.so.$(LIBVER) libqmi_common.so.$(LIBVER) libqmi_encdec.so.$(LIBVER)

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -MD -MP -c -o $@ $<$

%: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -MD -MP -o $@ $<$  $(common_objs) $(common_libs)

all: $(test_bins)

DEP=${wildcard *.d}
-include ${DEP}

$(test_bins): $(test_sources) $(common_objs)

