# Source Path

AM_CFLAGS =             -Wundef \
                        -Wstrict-prototypes \
                        -Wno-trigraphs \
                        -g -O0 \
                        -fno-inline \
                        -fno-short-enums \
                        -DMSM8960_ALSA \
                        -fpic

AM_CPPFLAGS =           -I../inc \
                        $(ALSAINTF_CFLAGS) \
                        $(DIAG_CFLAGS) \
                        -I.

audftm_c_sources =      DALSYS_common.c \
                        audio_ftm_afe_loopback.c \
                        audio_ftm_driver_fwk.c \
                        audio_ftm_dtmf_basic_op.c \
                        audio_ftm_dtmf_tone_gen.c \
                        audio_ftm_hw_drv-8960.c \
                        audio_ftm_pcm_loopback.c \
                        audio_ftm_pcm_record.c \
                        audio_ftm_tone_play.c \
                        audio_ftm_util_fifo.c \
                        ftm_audio_dispatch-8960.c \
                        ftm_audio_main.c \
                        audio_ftm_diag_mem.c \
                        audio_ftm_pcm_play.c

library_includedir = $(pkgincludedir)
library_include_HEADERS = $(h_sources)
bin_PROGRAMS = audio_ftm
audio_ftm_SOURCES = $(audftm_c_sources) $(h_sources)
audio_ftm_la_CC = @CC@
audio_ftm_CFLAGS = $(AM_CFLAGS) -DUSE_GLIB @GLIB_CFLAGS@
audio_ftm_CPPFLAGS =  $(AM_CPPFLAGS) -DUSE_GLIB @GLIB_CFLAGS@
audio_ftm_LDADD = $(ALSAINTF_LIBS) $(DIAG_LIBS)
