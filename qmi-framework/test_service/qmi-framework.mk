################################################################################
# Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
################################################################################

vpath %.c $(SRCDIR)
vpath %.c $(SRCDIR)/femto

CFLAGS = -Wundef \
	-Wstrict-prototypes \
	-Wno-trigraphs \
	-Wp,-w \
	-g -O0 \
	-fno-inline \
	-fno-short-enums \
	-fpic

CFLAGS += \
	-I$(SRCDIR)/../common/inc \
	-I$(SRCDIR)/../inc \

common_sources = test_service_v01.c qmi_test_service_clnt_common.c qmi_test_service_clnt_common_stats.c
common_objs = $(patsubst %.c,%.o,$(common_sources))
test_sources = qmi_test_service_clnt_test_async.c qmi_test_service_clnt_test_sync.c qmi_test_service_clnt_test_ind.c

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

