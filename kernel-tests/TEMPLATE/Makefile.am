#
# Rules for building kernel-tests should go in this file.
#
# replace names beginning with 'xyz' with suitable names.

xyzdir           = $(prefix)

# See the automake manual for full details of this syntax.
xyz_PROGRAMS     = xyzexe
dist_xyz_SCRIPTS = run.sh
dist_xyz_DATA    = README.txt

# Uncomment this if your script needs to install the 'common' setup
# script from android.
dist_xyz_SCRIPTS += ../test_env_setup.sh

#-----------------------------------------------------------------------------
# Below this line is automake boilerplate.  Casual users should not need
# to go below this line.
PWD := $(shell pwd)

all-local:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

install-exec-local:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) INSTALL_MOD_PATH=$(DESTDIR)$(prefix) modules_install
	depmod $(shell cat $(KERNELDIR)/include/config/kernel.release 2> /dev/null) -b $(DESTDIR)$(prefix)


