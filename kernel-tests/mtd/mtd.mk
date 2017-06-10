# sources and intermediate files are separated
vpath %.c $(SRCDIR)/mtd
  
MTD_TEST_CFLAGS := $(CFLAGS)
MTD_TEST_CFLAGS += $(QCT_CFLAGS)
  
MTD_TEST_CPPFLAGS := $(CPPFLAGS)
MTD_TEST_CPPFLAGS += $(QCT_CPPFLAGS)
MTD_TEST_CPPFLAGS += -I$(KERNEL_DIR)/include
MTD_TEST_CPPFLAGS += -I$(KERNEL_DIR)/arch/arm/include
  
APP_NAME := geoinfo_flash
SRCLIST  := geoinfo_flash.c

all: $(APP_NAME)

$(APP_NAME): $(SRCLIST)
	$(CC) $(MTD_TEST_CFLAGS) $(MTD_TEST_CPPFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
