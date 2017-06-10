AM_CFLAGS = -Wall \
            -Wundef \
            -Wstrict-prototypes \
            -Wno-trigraphs

AM_CPPFLAGS = $(ACDBLOADER_CFLAGS) \
              $(AUDCAL_CFLAGS) \
            -I$(WORKSPACE)/common/inc

bin_PROGRAMS = sendcal

sendcal_c_sources = sendcal.c
sendcal_SOURCES = $(sendcal_c_sources)
sendcal_CFLAGS = $(AM_CFLAGS) -DUSE_GLIB @GLIB_CFLAGS@
sendcal_CPPFLAGS =  $(AM_CPPFLAGS)
sendcal_LDFLAGS = $(ACDBLOADER_LIBS) $(AUDCAL_LIBS)
