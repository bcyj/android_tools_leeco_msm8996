# Makefile.am - Automake script for diag
#
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src test klog mdlog PktRspTest qshrink4_daemon uart_log dci_client CallBack_Client socket_log
dist_doc_DATA =
docdir = doc

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = diag.pc
EXTRA_DIST = $(pkgconfig_DATA)
