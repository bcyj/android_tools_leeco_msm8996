# Makefile.am - Automake script for qmimsglib and qmuxd

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = core/lib/src services qmi_client_qmux src qmuxd qcci_legacy tests

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = qmi.pc
EXTRA_DIST = $(pkgconfig_DATA)
