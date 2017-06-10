# Boilerplate for linux-kernel-tests autoconf.
# Casual users of the autoconf templates should not need to customize
# anything in this file.
AC_PREREQ(2.64)
AC_INIT([kernel-tests],[1.0.0])
AM_INIT_AUTOMAKE([-Wall -Werror gnu foreign])

AM_MAINTAINER_MODE

AC_CONFIG_SRCDIR([../test_env_setup.sh])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

AC_ARG_WITH([kernel-source],
    AS_HELP_STRING([--with-kernel-source=DIR],
        [location of a built kernel; used for building kernel modules]),
    [case "${withval}" in
        yes|'') AC_MSG_ERROR([--with-kernel-source must be a path]) ;;
        no)     withval= ;;
        *)
            if [test ! -f ${withval}/Makefile]; then
                AC_MSG_ERROR([bad value '${withval}' for --with-kernel-source; cannot find '${withval}/Makefile'])
            fi ;;
    esac])
if [test "${with_kernel_source}"]; then
    AC_SUBST([KERNELDIR], [${with_kernel_source}])
fi

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_MAKE_SET

AC_SUBST([CFLAGS])
AC_SUBST([CC])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
