AC_PREREQ(2.64)

AC_INIT([kernel-tests],[1.0.0])
AM_INIT_AUTOMAKE([-Wall -Werror gnu foreign])

AM_MAINTAINER_MODE

AC_CONFIG_SRCDIR([test_env_setup.sh])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_MAKE_SET

AC_ARG_WITH(sanitized-headers,
	AS_HELP_STRING([--with-sanitized-headers=DIR],
		[Specify the location of the sanitized Linux headers]),
	[CPPFLAGS="$CPPFLAGS -I$withval"])

AC_ARG_ENABLE(big-tests,
	AS_HELP_STRING([--enable-big-tests],
		[Include tests that use a lot of disk [default=yes]]),
	[case "${enableval}" in
	  yes) BIGTESTS=true ;;
	  no)  BIGTESTS=false ;;
	  *)   AC_MSG_ERROR([bad value ${enableval} for --enable-big-tests]) ;;
	esac],
	[BIGTESTS=true])
AM_CONDITIONAL(BIGTESTS, [test x$BIGTESTS = xtrue])

AC_ARG_ENABLE(target,
	AS_HELP_STRING([--enable-target],
		[Specify the target family for fbtest, e.g. msm7627a [default=<empty>]]),
	[case "${enableval}" in
	  msm7627a) MDP3=true ;;
	  msm8960)  MDP4=true ;;
	  msm8974)  MDSS=true ;;
	  mdm9625)  QPIC=true ;;
	  mdm9635)  QPIC=true ;;
	  apq8084)  MDSS=true ;;
	  msm8916)  MDSS=true ;;
	  *) AC_MSG_NOTICE([Target family not specified. fbtest might not be configured properly]) ;;
	esac],
	[])
AM_CONDITIONAL(MDP3, [test x$MDP3 = xtrue])
AM_CONDITIONAL(MDP4, [test x$MDP4 = xtrue])
AM_CONDITIONAL(MDSS, [test x$MDSS = xtrue])
AM_CONDITIONAL(QPIC, [test x$QPIC = xtrue])

AC_ARG_WITH([kernel],
	AS_HELP_STRING([--with-kernel=DIR],
		[location of a built kernel; used for building kernel modules]),
	[case "${with_kernel}" in
	  yes|'') AC_MSG_ERROR([--with-kernel must specify a path]) ;;
	  no)     with_kernel= ;;
	  *)
		if [test ! -f ${with_kernel}/Makefile]; then
			AC_MSG_ERROR([bad value '${with_kernel}' for --with-kernel; cannot find '${with_kernel}/Makefile'])
		fi ;;
	esac])
if [test -z "${with_kernel}"]; then
	AC_MSG_WARN([no kernel specified; kernel modules will not be included])
else
	AC_SUBST([KERNEL_DIR], [${with_kernel}])
fi
AM_CONDITIONAL(KERNELMODULES, [test -n "${with_kernel}"])

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

#
# Build all kernel tests by default
#
# To add new tests add ENABLE_KERNEL_TEST and update AC_CONFIG_FILES
#
# Tests which will be too large to fit in an initrd/initramfs should not
# be installed if the conditional BIGTESTS is false.
#

# adc
ENABLE_KERNEL_TEST([adc],[include the adc test (default=yes)],[yes],
	[if [test "x${enable_adc}" = "xyes"]; then
		AC_CHECK_HEADERS([linux/msm_adc.h],[],
		[AC_MSG_WARN([cannot find 'linux/msm_adc.h'; adc will not be included])
		enable_adc=no])
	fi])

#cacheflush
ENABLE_KERNEL_TEST([cacheflush],[include the cache flush test (default=yes)])

# clk
ENABLE_KERNEL_TEST([clk],[include the clk test (default=yes)])

# clocksource
ENABLE_KERNEL_TEST([clocksource],[include the clocksource test (default=yes)])

# cpu
ENABLE_KERNEL_TEST([cpu],[include the cpu test (default=yes)])

# cpufreq
ENABLE_KERNEL_TEST([cpufreq],[include the cpufreq test (default=yes)])

# dmov
ENABLE_KERNEL_TEST([dmov],[include the dmov test (default=yes)])

# fbtest
ENABLE_KERNEL_TEST([fbtest],[include the framebuffer test (default=yes)])

# gpio
ENABLE_KERNEL_TEST([gpio-tlmm],[include the gpio test (default=yes)])

# i2c-msm
ENABLE_KERNEL_TEST([i2c-msm],[include the MSM I2C driver test (default=yes)])

# irq
ENABLE_KERNEL_TEST([irq],[include the IRQ test (default=yes)])

# mpp
ENABLE_KERNEL_TEST([mpp],[include the mpp driver test (default=yes)])

# msm-bus
ENABLE_KERNEL_TEST([msm-bus],[include the MSM bus test (default=yes)])

# msm-uart
ENABLE_KERNEL_TEST([msm-uart],[include the MSM UART test (default=yes)])

# msm-uart-hs
ENABLE_KERNEL_TEST([msm-uart-hs],[include the MSM High UART driver test (default=yes)])

# mtd
ENABLE_KERNEL_TEST([mtd],[include the mtd test (default=yes)])

# pctest
ENABLE_KERNEL_TEST([pctest],[include the power collapse test (default=yes)])

# qcedev
ENABLE_KERNEL_TEST([qcedev],[include the qcedev test (default=yes)])

# rotator
ENABLE_KERNEL_TEST([rotator],[include the rotator test (default=yes)])

# sdcc
ENABLE_KERNEL_TEST([sdcc],[include the sdcc test (default=yes)])

# smd
ENABLE_KERNEL_TEST([smd],[include the smd test (default=yes)])

# socinfo
ENABLE_KERNEL_TEST([socinfo],[include the socinfo test (default=yes)])

# spi
ENABLE_KERNEL_TEST([spi],[include the spi test (default=yes)])

# sps
ENABLE_KERNEL_TEST([sps],[include the sps test (default=yes)])

# timer
ENABLE_KERNEL_TEST([timer],[include the timer test (default=yes)])

# vfp
ENABLE_KERNEL_TEST([vfp],[include the vfp test (default=yes)])

# vreg
ENABLE_KERNEL_TEST([vreg],[include the vreg test (default=yes)])

# usf
ENABLE_KERNEL_TEST([usf],[include the usf test (default=yes)])

AC_SUBST([CFLAGS])
AC_SUBST([CC])

AC_CONFIG_FILES([
	Makefile
	adc/Makefile
	cacheflush/Makefile
	clk/Makefile
	clocksource/Makefile
	cpu/Makefile
	cpufreq/Makefile
	dmov/Makefile
	fbtest/Makefile
	gpio_tlmm/Makefile
	i2c-msm/Makefile
	irq/Makefile
	mpp/Makefile
	msm_uart/Makefile
	msm_uart_hs/Makefile
	mtd/Makefile
	pctest/Makefile
	qcedev/Makefile
	rotator/Makefile
	sdcc/Makefile
	smd/Makefile
	socinfo/Makefile
	spi/Makefile
	sps/Makefile
	timer/Makefile
	vfp/Makefile
	vreg/Makefile
	usf/Makefile
	])
AC_OUTPUT
