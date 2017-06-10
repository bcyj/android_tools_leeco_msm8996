
AC_PREREQ([2.61])

AC_INIT([fm],[1.0.0])

AM_INIT_AUTOMAKE([-Werror -Wall gnu foreign])

AM_MAINTAINER_MODE

AC_CONFIG_SRCDIR([patch_downloader/fm_qsoc_patches.c])
AC_CONFIG_SRCDIR([fmtest/main.c])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_MAKE_SET
AC_PROG_CPP
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_LN_S
AC_PROG_LIBTOOL
AC_PROG_INSTALL

AC_ARG_WITH([sanitized-headers],
	[AS_HELP_STRING([--with-sanitized-headers=DIR],[location of the sanitized Linux headers])],
	[CPPFLAGS="$CPPFLAGS -I$withval"])

AC_ARG_WITH([target],
[AS_HELP_STRING([--target=FLAG],[target specific flag])],
[CPPFLAGS="$CPPFLAGS -D$withval"])


AC_CHECK_HEADERS([media/tavarua.h],,
	[AC_MSG_ERROR([cannot find media/tavarua.h. Please check the --with-sanitized-headers option])])

PKG_CHECK_MODULES(QMI, qmi)
AC_SUBST(QMI_CFLAGS)
AC_SUBST(QMI_LIBS)


AC_CANONICAL_HOST
AS_CASE([$host],
	[arm*], [ARM_HOST_ARCH=yes],
	[ARM_HOST_ARCH=no]
)
AM_CONDITIONAL(ARM_HOST_ARCH, [test "x$ARM_HOST_ARCH" = "xyes"])

AC_CONFIG_FILES([
	Makefile
	patch_downloader/Makefile
	fmtest/Makefile
])

AC_OUTPUT
