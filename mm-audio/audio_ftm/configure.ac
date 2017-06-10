#                                               -*- Autoconf -*-
# configure.ac -- Autoconf script for audio-ftm
#
# Process this file with autoconf to produce a configure script
# Requires autoconf tool later than 2.61
AC_PREREQ(2.61)
# Initialize the aud_ftm package version 1.0.0
AC_INIT([audio_ftm],1.0.0)
# Does not strictly follow GNU Coding standards
AM_INIT_AUTOMAKE([foreign])
# Disables auto rebuilding of configure, Makefile.ins
AM_MAINTAINER_MODE
# Verifies the --srcdir is correct by checking for the path
AC_CONFIG_SRCDIR([legacy_intf/src/ftm_audio_main.c])
# defines some macros variable to be included by source
AC_CONFIG_HEADERS([legacy_intf/src/version.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_PROG_AWK
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES([DIAG], [diag])
AC_SUBST([DIAG_CFLAGS])
AC_SUBST([DIAG_LIBS])

PKG_CHECK_MODULES([ALSAINTF],[alsa-intf])
        AC_SUBST([ALSAINTF_CFLAGS])
        AC_SUBST([ALSAINTF_LIBS])
AC_ARG_WITH(sanitized-headers,
        AS_HELP_STRING([--with-sanitized-headers=DIR],
                [Specify the location of the sanitized Linux headers]),
        [CPPFLAGS="$CPPFLAGS -idirafter $withval"])
AC_ARG_WITH([glib],
      AC_HELP_STRING([--with-glib],
         [enable glib, building HLOS systems which use glib]))

if (test "x${with_glib}" = "xyes"); then
        AC_DEFINE(ENABLE_USEGLIB, 1, [Define if HLOS systems uses glib])
        PKG_CHECK_MODULES(GTHREAD, gthread-2.0 >= 2.16, dummy=yes,
                                AC_MSG_ERROR(GThread >= 2.16 is required))
        PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.16, dummy=yes,
                                AC_MSG_ERROR(GLib >= 2.16 is required))
        GLIB_CFLAGS="$GLIB_CFLAGS $GTHREAD_CFLAGS"
        GLIB_LIBS="$GLIB_LIBS $GTHREAD_LIBS"

        AC_SUBST(GLIB_CFLAGS)
        AC_SUBST(GLIB_LIBS)
fi

AM_CONDITIONAL(USE_GLIB, test "x${with_glib}" = "xyes")
AC_ARG_WITH([common_includes],
        AC_HELP_STRING([--with-common-includes=@<:@dir@:>@],
        [Specify the location of the common headers]),
        [common_incdir=$withval],
        with_common_includes=no)
        if test "x$with_common_includes" != "xno"; then
         CFLAGS="${CFLAGS} -I${common_incdir}"
fi

AC_SUBST([CFLAGS])
AC_SUBST([CC])
AC_CONFIG_FILES([ \
        Makefile \
        legacy_intf/src/Makefile \
        audio_ftm.pc
        ])
AC_OUTPUT
