#sources and intermediate files are separated
vpath %.c $(SRCDIR)/rotator

ROTATOR_CFLAGS := $(CFLAGS)
ROTATOR_CFLAGS += $(QCT_CFLAGS)
 
ROTATOR_CPPFLAGS := $(CPPFLAGS)
ROTATOR_CPPFLAGS += $(QCT_CPPFLAGS)
ROTATOR_CPPFLAGS += -I$(KERNEL_DIR)/include
ROTATOR_CPPFLAGS += -DFEATURE_MEMORY_VIRTUAL
ROTATOR_CPPFLAGS += -Duint32_t="unsigned int"
ROTATOR_CPPFLAGS += -Duint16_t="unsigned short"
 
APP_NAME := rotator

SRCLIST  := rotator.c

ROTATOR_LDLIBS   := $(LDLIBS)

all: $(APP_NAME)

$(APP_NAME): $(SRCLIST)
	$(CC) $(ROTATOR_CFLAGS) $(ROTATOR_CPPFLAGS) $(LDFLAGS) -o $@ $^ $(ROTATOR_LDLIBS)

clean:
	rm -rf $(APP_NAME)
