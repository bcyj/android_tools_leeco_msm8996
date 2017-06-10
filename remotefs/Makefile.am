AM_CFLAGS = -Wall \
        -Wundef \
        -Wstrict-prototypes \
	-Wno-trigraphs \
	$(QMI_CFLAGS) \
	$(QMIF_CFLAGS)

ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -D__packed__= \
	-DRMTS_LE_BUILD \
	-D_LARGEFILE64_SOURCE \
	-DLOG_NIDEBUG=0 \
	$(QMI_CFLAGS) \
	$(QMIF_CFLAGS)


bin_PROGRAMS = rmt_storage
rmt_storage_CC = @CC@
pkgconfigdir = $(libdir)/pkgconfig

rmt_storage_SOURCES = \
	qmi_rmt_storage/remote_storage_v01.c \
	qmi_rmt_storage/rmt_storage_svc.c

rmt_storage_LDFLAGS = -lpthread $(QMI_LIBS) $(QMIF_LIBS)
rmt_storage_LDADD = $(QMI_LIBS) $(QMIF_LIBS)

if USE_GLIB
 AM_CPPFLAGS  += -DUSE_GLIB @GLIB_CFLAGS@
 rmt_storage_LDFLAGS += @GLIB_LIBS@
endif