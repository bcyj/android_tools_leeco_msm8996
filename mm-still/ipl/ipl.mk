CPPFLAGS = $(QCT_CPPFLAGS)
CFLAGS   = $(QCT_CFLAGS)

CPPFLAGS += -Werror

CPPFLAGS += -I$(SRCDIR)/ipl/inc
CPPFLAGS += -I$(SRCDIR)/ipl/src
CPPFLAGS += -I$(SRCDIR)/../common/inc

CPPFLAGS += -I$(KERNEL_DIR)/include
CPPFLAGS += -I$(KERNEL_OBJDIR)/include
CPPFLAGS += -I$(KERNEL_OBJDIR)/include2
CPPFLAGS += -DIPL_DEBUG_STANDALONE

CPPFLAGS += -DTRUE="1"
CPPFLAGS += -DFALSE="0"

###############################################################################
# Build the IPL library
###############################################################################

vpath %.c $(SRCDIR)/ipl/src

IPL_SRC  = ipl_attic.c
IPL_SRC += ipl_compose.c
IPL_SRC += ipl_convert.c
IPL_SRC += ipl_downSize.c
IPL_SRC += ipl_efx.c
IPL_SRC += ipl_helper.c
IPL_SRC += ipl_hjr.c
IPL_SRC += ipl_rotAddCrop.c
IPL_SRC += ipl_upSize.c
IPL_SRC += ipl_util.c
IPL_SRC += ipl_xform.c

all: libmm-ipl.so.$(LIBVER)

libmm-ipl.so.$(LIBVER): $(IPL_SRC)
	$(CC) $(CPPFLAGS) $(QCT_CFLAGS_SO) $(QCT_LDFLAGS_SO) -Wl,-soname,libmm-ipl.so.$(LIBMAJOR) -o $@ $^ -lpthread
