# Information about the kernel test and its maintainer.
# Fill these values in appropriately.
DESCRIPTION = "Your test description here"
MAINTAINER = "Your test POC <mailing@list.here>"
LICENSE = "unknown"
HOMEPAGE = "unknown"

# PV is the Package Version.  Increment this for changes to the test.
PV = "1.0"

# PR is the Package Revision.  Increment this for changes to this file.
PR = "r0"

# Set INSTALL_PATH to the root of the kernel-test's install location.
INSTALL_PATH = "${KERNEL_TESTS_INSTALL_PREFIX}/${PN}"

# Do the test need the C compiler and linker?  If so, uncomment this line.
DEPENDS += "virtual/${TARGET_PREFIX}gcc virtual/${TARGET_PREFIX}binutils"

# Does the test depend on kernel artifacts (headers) or is it a kernel
# module?  If so, uncomment the next line.
DEPENDS += "virtual/kernel"

# Does the test expect any third party packages to be installed on the
# in the target in oder to run?  If so, uncomment this line and
# add them here.
# RDEPENDS += "coreutils"

# If the kernel-test is a user-space application needing access to published
# kernel APIs, uncomment the following block.
export CFLAGS = "-I${STAGING_DIR}/${HOST_SYS}${KERNEL_HEADER_PATH}"

# If the kernel-test includes kernel modules (.ko files), uncomment these lines
# to pass the KERNEL_DIR variable to your Makefile.am via autotools.
EXTRA_OECONF += "--with-kernel-source=${STAGING_DIR}/${HOST_SYS}${KERNEL_PATH}"
EXTRA_OECONF += "--with-glib"
#----------------------------------------------------------------------------
# Bitbake details below.  Casual users should not need to go below this line.
export ARCH = "${TARGET_ARCH}"
export CROSS_COMPILE = "${TARGET_PREFIX}"
FILES_${PN} = "${INSTALL_PATH}"
KERNEL_PATH ?= "/usr/src/linux"
KERNEL_HEADER_PATH ?= "${KERNEL_PATH}/usr/include"
KERNEL_TESTS_INSTALL_PREFIX ?= "/kernel-tests"
LDFLAGS_pn-${PN} = ""
PACKAGES = "${PN}"
PROVIDES = "kernel-tests/${PN}"

prefix = "${INSTALL_PATH}"

inherit autotools srctree

__do_clean_make () {
    oe_runmake mostlyclean clean distclean maintainer-clean
}
