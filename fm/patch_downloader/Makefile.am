
if ARM_HOST_ARCH
bin_PROGRAMS = fm_qsoc_patches

fm_qsoc_patches_LDADD:= @QMI_LIBS@
fm_qsoc_patches_CPPFLAGS:= @QMI_CFLAGS@
fm_qsoc_patches_CFLAGS := -DBOARD_MSM8x60

fm_qsoc_patches_LDFLAGS := -lpthread
fm_qsoc_patches_LDFLAGS += -lrt

endif

