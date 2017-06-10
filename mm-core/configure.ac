#                                               -*- Autoconf -*-

# configure.ac -- Autoconf script for mm-video
#

# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([mm-core-prop-ship],1.0.0)

AM_INIT_AUTOMAKE([-Wall -Werror gnu foreign])
AM_MAINTAINER_MODE

AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AM_PROG_CC_C_O
AC_PROG_LIBTOOL
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

AC_ARG_ENABLE([target-msm7627a],
	AC_HELP_STRING([--enable-target-msm7627a],
		[Enable conditional compile for target msm7627a [default=no]]),
	[target_msm7627a="${enableval}"],
	target_msm7627a=no)

AM_CONDITIONAL([TARGET_MSM7627A], [test x$target_msm7627a = xyes])

AC_ARG_WITH([sanitized-headers],
	[AS_HELP_STRING([--with-sanitized-headers=DIR],[location of the sanitized Linux headers])],
	[CPPFLAGS="$CPPFLAGS -I$withval"])

AC_CONFIG_FILES([     \
	Makefile \
	adspsvc/Makefile \
	])
AC_OUTPUT
