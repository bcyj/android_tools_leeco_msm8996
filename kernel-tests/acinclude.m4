# _ENABLE_KERNEL_TEST(kernel-test,
#                     shell-kernel-test
#                     conditional-kernel-test
#                     help-string,
#                     enable-if-not-given
#                     [additional-checks])
#
# kernel-test: passed directly through to the
#              AC_ARG_ENABLE macro as the feature
#
# shell-kernel-test: name of the shell variable that AC_ARG_ENABLE
#                    creates from kernel-test
#
# conditional-kernel-test: name of the conditional to create
#
# help-string: passed directly through to the AS_HELP_STRING
#              macro as the right-hand-side
#
# enable-if-not-given: value used when neither --enable-kernel-test
#                      nor --disable-kernel-test is given
#
# additional-checks: shell commands run before the AM_CONDITIONAL macro;
#                    can be used to change the conditional by setting
#                    the shell variable specified in shell-kernel-test.
# ----------------------------------------------------------------------
AC_DEFUN([_ENABLE_KERNEL_TEST],
	[
		AC_ARG_ENABLE([$1],
			[AS_HELP_STRING([--enable-$1],[$4])],
			[case "${$2}" in
				yes|no) ;;
				     *) AC_MSG_ERROR([bad value '${$2}' for --enable-$1]) ;;
			esac],
			[$2=$5]
		)
		$6
		AM_CONDITIONAL([$3],[test "x${$2}" = "xyes"])
	]
)

# ENABLE_KERNEL_TEST(kernel-test,
#                    help-string,
#                    [enable-if-not-given = yes]
#                    [additional-checks])
#
# kernel-test: passed directly through to the
#              AC_ARG_ENABLE macro as the feature
#
# help-string: passed directly through to the
#              AS_HELP_STRING macro as the right-hand-side
#
# enable-if-not-given: value used when neither --enable-kernel-test
#                      nor --disable-kernel-test is given
#
# additional-checks: shell commands run before the AM_CONDITIONAL macro;
#                    can be used to change the conditional by setting
#                    the shell variable named enable_<kernel-test>, with
#                    any dashes and dots in kernel-test changed into '_'.
# -----------------------------------------------------------------------
AC_DEFUN([ENABLE_KERNEL_TEST],
	[
		_ENABLE_KERNEL_TEST([$1],
			m4_join([],[enable_],m4_translit([$1],[-.],[__])),
			m4_join([],[USE],m4_toupper(m4_translit([$1],[-.],[__]))),
			[$2],
			m4_ifnblank([$3],[$3],[yes]),
			[$4]
		)
	]
)
