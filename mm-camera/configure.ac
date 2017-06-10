AC_PREREQ(2.61)

AC_INIT([mm-camera],1.0.0)

AM_INIT_AUTOMAKE([-Werror -Wall gnu foreign subdir-objects])

AM_MAINTAINER_MODE

AC_CONFIG_HEADER([configure.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AM_PROG_AS
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_LIBTOOL
AC_PROG_AWK
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

AC_CHECK_TOOL(OBJCOPY, objcopy, false)

COMPILE_CAMERA=yes
AMSS_VERSION=
VFE_VERS=
MSM_VERSION=
DEBUG_CPPFLAGS=
DEBUG_CFLAGS=
DEBUG_LDFLAGS=

AC_ARG_WITH([sanitized-headers],
	[AS_HELP_STRING([--with-sanitized-headers=DIR],[location of the sanitized Linux kernel headers])],
	[CFLAGS="$CFLAGS -I$withval/media -I $withval"])

AC_ARG_WITH([extra-cflags],
	[AS_HELP_STRING([--with-extra-cflags=DIR],[additional compliation flags])],
	[CFLAGS="$CFLAGS $withval"])

AC_ARG_ENABLE([target],
	[AS_HELP_STRING([--enable-target=TARGET],[target to build for])],
	[],
	[enable_target=none]
)

if test "x$enable_target" = "xqsd8250"; then
	VFE_VERS=vfe30
	MSM_VERSION=8650
elif test "x$enable_target" = "xqsd8650a"; then
	VFE_VERS=vfe30
	MSM_VERSION=8650
elif test "x$enable_target" = "xmsm7201a"; then
	VFE_VERS=vfe2x
	MSM_VERSION=7x0x
elif test "x$enable_target" = "xmsm7501a"; then
	VFE_VERS=vfe2x
	MSM_VERSION=7x0x
elif test "x$enable_target" = "xmsm7601"; then
	VFE_VERS=vfe2x
	MSM_VERSION=7x0x
elif test "x$enable_target" = "xmsm7627"; then
	VFE_VERS=vfe2x
	MSM_VERSION=7x2x
	DEBUG_CPPFLAGS=-DHW_ENCODE -DUSE_PREVIEW_TABLE2
	DEBUG_CFLAGS=-DHW_ENCODE -DUSE_PREVIEW_TABLE2
elif test "x$enable_target" = "xmsm7627a"; then
	VFE_VERS=vfe2x
	MSM_VERSION=7x27A
    FEATURE_WAVELET_DENOISE=true
	DEBUG_CPPFLAGS=-DHW_ENCODE -DUSE_PREVIEW_TABLE2 -DUSE_HFR_TABLE2
	DEBUG_CFLAGS=-DHW_ENCODE -DUSE_PREVIEW_TABLE2 -DUSE_HFR_TABLE2
	FEATURE_FACE_PROC=true
	FEATURE_VFE_TEST_VEC=false
	VFE_2X=true
elif test "x$enable_target" = "xmsm7630"; then
	VFE_VERS=vfe31
	MSM_VERSION=7x30
	DEBUG_CPPFLAGS=-DHW_ENCODE
	DEBUG_CFLAGS=-DHW_ENCODE
    FEATURE_FACE_PROC=true
    FEATURE_WAVELET_DENOISE=true
    FEATURE_VFE_TEST_VEC=false
elif test "x$enable_target" = "xmsm8660"; then
	VFE_VERS=vfe31
	MSM_VERSION=8x60
	FEATURE_ZSL=true
	FEATURE_WAVELET_DENOISE=true
	DEBUG_CPPFLAGS=-DHW_ENCODE -DCONFIG_MSG_THESHOLD=350 -DFEATURE_ZSL_SUPPORTED
	DEBUG_CFLAGS=-DHW_ENCODE -DCONFIG_MSG_THESHOLD=350 -DFEATURE_ZSL_SUPPORTED
elif test "x$enable_target" = "xmsm8960"; then
	VFE_VERS=vfe32
	MSM_VERSION=8960
	FEATURE_WAVELET_DENOISE=true
	FEATURE_ZSL=true
	FEATURE_FACE_PROC=true
	FEATURE_VFE_TEST_VEC=false
	DEBUG_CPPFLAGS=-DHW_ENCODE -DCONFIG_MSG_THESHOLD=350
	DEBUG_CFLAGS=-DHW_ENCODE -DCONFIG_MSG_THESHOLD=350
elif test "x$enable_target" = "xmsm7625"; then
	VFE_VERS=vfe2x
	MSM_VERSION=7x2x
else
	AC_MSG_NOTICE([A valid target was not specified. Please check the '--enable-target' option.])
	COMPILE_CAMERA=no
fi

if test "x$VFE_VERS" = "xvfe32"; then
	CHROMATIX_VERSION=0208
elif test "x$VFE_VERS" = "xvfe31"; then
	CHROMATIX_VERSION=0208
elif test "x$VFE_VERS" = "xvfe2x"; then
	CHROMATIX_VERSION=0208
else
	CHROMATIX_VERSION=0205
fi

AC_ARG_ENABLE(amss-version,
	[AS_HELP_STRING([--enable-amss-version=AMSS_VERSION],[Specify the AMSS version])],
	[AMSS_VERSION=$enableval],
	[AMSS_VERSION=]
)
AC_SUBST([AMSS_VERSION])

AC_ARG_ENABLE([debug],
	[AS_HELP_STRING([--enable-debug],[Build with debug flags and options])],
	[DEBUG=$enableval],
	[DEBUG=no]
)
if test "x$DEBUG" = "xyes"; then
	DEBUG_CPPFLAGS="${DEBUG_CPPFLAGS} -DLOG_DEBUG -DLOG_TAG=\"CameraService\""
	DEBUG_CFLAGS="${DEBUG_CFLAGS} -DLOG_DEBUG -DLOG_TAG=\"CameraService\" -g -O0"
fi

AM_CONDITIONAL([COMPILE_CAMERA], [test "x$COMPILE_CAMERA" = "xyes"])
AC_SUBST([VFE_VERS])
AC_SUBST([MSM_VERSION])
AC_SUBST([CHROMATIX_VERSION])
AC_SUBST([DEBUG_CPPFLAGS])
AC_SUBST([DEBUG_CFLAGS])
AC_SUBST([DEBUG_LDFLAGS])
AC_SUBST([FEATURE_FACE_PROC])
AC_SUBST([FEATURE_WAVELET_DENOISE])
AC_SUBST([FEATURE_VFE_TEST_VEC])

AM_CONDITIONAL([VFE2X], [test "x$VFE_VERS" = "xvfe2x"])
AM_CONDITIONAL([VFE30], [test "x$VFE_VERS" = "xvfe30"])
AM_CONDITIONAL([VFE31], [test "x$VFE_VERS" = "xvfe31"])
AM_CONDITIONAL([VFE32], [test "x$VFE_VERS" = "xvfe32"])
AM_CONDITIONAL([CHROMATIX0207], [test "x$CHROMATIX_VERSION" = "x0207"])
AM_CONDITIONAL([WAVELET_DENOISE], [test "x$FEATURE_WAVELET_DENOISE" = "xtrue"])
AM_CONDITIONAL([VFE_TEST_VEC], [test "x$FEATURE_VFE_TEST_VEC" = "xtrue"])
AM_CONDITIONAL([FACE_PROC], [test "x$FEATURE_FACE_PROC" = "xtrue"])

AM_CONDITIONAL([MSM7X0X], [test "x$MSM_VERSION" = "x7x0x"])
AM_CONDITIONAL([MSM7X2X], [test "x$MSM_VERSION" = "x7x2x"])
AM_CONDITIONAL([MSM7X27A], [test "x$MSM_VERSION" = "x7x27A"])
AM_CONDITIONAL([MSM7630], [test "x$MSM_VERSION" = "x7x30"])
AM_CONDITIONAL([MSM8660], [test "x$MSM_VERSION" = "x8660"])
AM_CONDITIONAL([MSM8960], [test "x$MSM_VERSION" = "x8960"])

AS_CASE([$host],
	[arm*], [ARM=yes],
	[ARM=no]
)
AM_CONDITIONAL([ARM], [test "x$ARM" = "xyes"])

AC_OUTPUT([ \
	Makefile \
	apps/Makefile \
	apps/appslib/Makefile \
	apps/mm-qcamera-app/Makefile \
	apps/v4l2-qcamera-app/Makefile \
    server/Makefile \
    server/chromatix/0208/libchromatix/chromatix_vx6953/preview/Makefile \
    server/chromatix/0208/libchromatix/chromatix_vx6953/video/Makefile \
    server/chromatix/0208/libchromatix/chromatix_s5k4e1/preview/Makefile \
    server/chromatix/0208/libchromatix/chromatix_s5k4e1/video/Makefile
])

