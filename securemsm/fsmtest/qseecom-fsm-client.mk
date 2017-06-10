#sources and intermediate files are separated
vpath %.c $(SRCDIR)/fsmtest

QSEECOM_API_CFLAGS := $(CFLAGS)
QSEECOM_API_CFLAGS += $(QCT_CFLAGS)

CPPFLAGS   += -I$(SRCDIR)/QSEEComAPI
QSEECOM_API_CPPFLAGS := $(CPPFLAGS)
QSEECOM_API_CPPFLAGS += $(QCT_CPPFLAGS)

QSEECOM_API_LDLIBS := -lstringl -lQseeComApi -lpthread -lQctPrivs -lstdc++

LDFLAGS  += -L$(OBJDIR)/lib

APP_NAME := qseecom_fsm_lte_client
SRCLIST  := qseecom_fsm_lte_client.c

all: $(APP_NAME)

$(APP_NAME): $(SRCLIST)
	$(CC) $(QSEECOM_API_CFLAGS) $(QSEECOM_API_CPPFLAGS) $(LDFLAGS) -o $@ $^ $(QSEECOM_API_LDLIBS)
