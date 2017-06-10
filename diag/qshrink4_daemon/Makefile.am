
AM_CFLAGS = -Wall \
		-Wundef \
		-Wstrict-prototypes \
		-Wno-trigraphs

AM_CPPFLAGS = -DIMAGE_APPS_PROC \
		-I../src \
		-I../include

bin_PROGRAMS = diag_qshrink4_daemon

diag_qshrink4_daemon_SOURCES = diag_qshrink4_daemon.c
diag_qshrink4_daemon_LDADD = ../src/libdiag.la

if USE_GLIB
diag_qshrink4_daemon_CFLAGS =  -DUSE_GLIB @GLIB_CFLAGS@
diag_qshrink4_daemon_LDFLAGS = -lpthread @GLIB_LIBS@
else
diag_qshrink4_daemon_LDFLAGS = -lpthread -lcutils
endif

init_ddir = $(sysconfdir)/init.d
init_d_SCRIPTS = start_diag_qshrink4_daemon


