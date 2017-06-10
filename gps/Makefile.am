# Makefile.am - Automake script for gps loc_api_app
#

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = vzwGpsLocationProvider/loc_ext

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = loc-api-app.pc
sysconf_DATA = etc/gps.conf
EXTRA_DIST = $(pkgconfig_DATA)
