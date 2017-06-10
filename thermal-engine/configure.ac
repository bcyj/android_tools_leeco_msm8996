#                                               -*- Autoconf -*-

# configure.ac -- Autoconf script for thermal-engine
#

# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([thermal-engine],
	1.0.0)
AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_LIBTOOL
AC_PROG_AWK
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
PKG_PROG_PKG_CONFIG

AC_ARG_ENABLE([target-mdm9625],
	AC_HELP_STRING([--enable-target-mdm9625],
		[Enable conditional compile for target mdm9625]),
	[target_mdm9625="${enableval}"],
	target_mdm9625=no)

AM_CONDITIONAL(IS_MDM9625, test "x$target_mdm9625" = "xyes")

AC_ARG_ENABLE([target-msm8974],
	AC_HELP_STRING([--enable-target-msm8974],
		[Enable conditional compile for target msm8974]),
	[target_msm8974="${enableval}"],
	target_msm8974=no)

AM_CONDITIONAL(IS_MSM8974, test "x$target_msm8974" = "xyes")

AC_ARG_ENABLE([target-msm8610],
	AC_HELP_STRING([--enable-target-msm8610],
		[Enable conditional compile for target msm8610]),
	[target_msm8610="${enableval}"],
	target_msm8610=no)

AM_CONDITIONAL(IS_MSM8610, test "x$target_msm8610" = "xyes")

AC_ARG_ENABLE([target-mdm9635],
	AC_HELP_STRING([--enable-target-mdm9635],
		[Enable conditional compile for target mdm9635]),
	[target_mdm9635="${enableval}"],
	target_mdm9635=no)

AM_CONDITIONAL(IS_MDM9635, test "x$target_mdm9635" = "xyes")

AC_ARG_ENABLE([target-msm8226],
	AC_HELP_STRING([--enable-target-msm8226],
		[Enable conditional compile for target msm8226]),
	[target_msm8226="${enableval}"],
	target_msm8226=no)

AM_CONDITIONAL(IS_MSM8226, test "x$target_msm8226" = "xyes")

if (test "x$target_msm8974" = "xyes"); then
      ENABLE_THERMAL_ENGINE=yes
      QMI_TMD=true
      QMI_TS=true
      ENABLE_OLD_PARSER=true
      ENABLE_THERMAL_CLIENT_LIB=true
elif (test "x$target_mdm9625" = "xyes"); then
      ENABLE_THERMAL_ENGINE=yes
      QMI_TMD=true
      QMI_TS=false
      ENABLE_OLD_PARSER=true
      ENABLE_THERMAL_CLIENT_LIB=false
elif (test "x$target_msm8610" = "xyes"); then
      ENABLE_THERMAL_ENGINE=yes
      QMI_TMD=true
      QMI_TS=true
      ENABLE_OLD_PARSER=true
      ENABLE_THERMAL_CLIENT_LIB=true
elif (test "x$target_msm8226" = "xyes"); then
      ENABLE_THERMAL_ENGINE=yes
      QMI_TMD=true
      QMI_TS=true
      ENABLE_OLD_PARSER=true
      ENABLE_THERMAL_CLIENT_LIB=true
elif (test "x$target_mdm9635" = "xyes"); then
      ENABLE_THERMAL_ENGINE=yes
      QMI_TMD=true
      QMI_TS=false
      ENABLE_OLD_PARSER=false
      ENABLE_THERMAL_CLIENT_LIB=false
else
      AC_MSG_NOTICE([A valid target was not specified. Please check the '--enable-target' option.])
      ENABLE_THERMAL_ENGINE=no
      QMI_TMD=false
      QMI_TS=false
      ENABLE_OLD_PARER=false
      ENABLE_THERMAL_CLIENT_LIB=false
fi

AC_SUBST([ENABLE_THERMAL_ENGINE])
AC_SUBST([QMI_TMD])
AC_SUBST([QMI_TS])
AC_SUBST([ENABLE_OLD_PARSER])
AC_SUBST([ENABLE_THERMAL_CLIENT_LIB])

AM_CONDITIONAL(ENABLE_THERMAL_ENGINE, test "x$ENABLE_THERMAL_ENGINE" = "xyes")
AM_CONDITIONAL(QMI_TMD, test "x$QMI_TMD" = "xtrue")
AM_CONDITIONAL(QMI_TS, test "x$QMI_TS" = "xtrue")
AM_CONDITIONAL(ENABLE_OLD_PARSER, test "x$ENABLE_OLD_PARSER" = "xtrue")
AM_CONDITIONAL(ENABLE_THERMAL_CLIENT_LIB, test "x$ENABLE_THERMAL_CLIENT_LIB" = "xtrue")

AC_ARG_WITH([qmi_framework],
      AC_HELP_STRING([--with-qmi-framework],
         [enable qmi-framework for building thermal engine on LE]))

if (test "x${with_qmi_framework}" = "xyes"); then
	# Checks for libraries.
	PKG_CHECK_MODULES([QMIF], [qmi-framework])
	AC_SUBST([QMIF_CFLAGS])
	AC_SUBST([QMIF_LIBS])
	PKG_CHECK_MODULES([QMI], [qmi])
	AC_SUBST([QMI_CFLAGS])
	AC_SUBST([QMI_LIBS])
fi

AM_CONDITIONAL(USE_QMI, test "x${with_qmi_framework}" = "xyes")

AC_ARG_WITH([kernel],
      AC_HELP_STRING([--with-kernel=@<:@dir@:>@],
         [Specify the location of the Linux kernel headers]),
      [kerneldir=$withval],
      with_kernel=no)

if test "x$with_kernel" != "xno"; then
   CFLAGS="${CFLAGS} -I${kerneldir}/include -I${kerneldir}/arch/arm/include"
fi

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

AC_SUBST([CFLAGS])
AC_SUBST([CC])
AC_CONFIG_FILES([     \
	Makefile      \
	])
AC_OUTPUT

