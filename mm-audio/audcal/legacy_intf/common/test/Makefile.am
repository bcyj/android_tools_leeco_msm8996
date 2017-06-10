AM_CFLAGS = -Wall \
            -Wundef \
            -Wstrict-prototypes \
            -Wno-trigraphs

AM_CPPFLAGS = $(ACDBLOADER_CFLAGS) \
              $(AUDIOALSA_CFLAGS) \
              $(AUDCAL_CFLAGS) \
            -I$(WORKSPACE)/common/inc

bin_PROGRAMS = acdbtest sendcal

acdbtest_c_sources = acdbtest.c
acdbtest_SOURCES = $(acdbtest_c_sources)
acdbtest_CFLAGS = $(AM_CFLAGS) -DUSE_GLIB @GLIB_CFLAGS@
acdbtest_CPPFLAGS =  $(AM_CPPFLAGS)
acdbtest_LDFLAGS = $(AUDIOALSA_LIBS) $(AUDCAL_LIBS)

sendcal_c_sources = sendcal.c
sendcal_SOURCES = $(sendcal_c_sources)
sendcal_CFLAGS = $(AM_CFLAGS) -DUSE_GLIB @GLIB_CFLAGS@
sendcal_CPPFLAGS =  $(AM_CPPFLAGS)
sendcal_LDFLAGS = $(ACDBLOADER_LIBS) $(AUDCAL_LIBS)
