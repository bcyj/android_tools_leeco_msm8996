#sources and intermediate files are separated
vpath %.c $(SRCDIR)/fbtest

FBTEST_CFLAGS := $(CFLAGS)
FBTEST_CFLAGS += $(QCT_CFLAGS)
  
FBTEST_CPPFLAGS := $(CPPFLAGS)
FBTEST_CPPFLAGS += $(QCT_CPPFLAGS)
FBTEST_CPPFLAGS += -I$(KERNEL_DIR)/include
FBTEST_CPPFLAGS += -DFEATURE_MEMORY_VIRTUAL
FBTEST_CPPFLAGS += -Duint32_t="unsigned int"
FBTEST_CPPFLAGS += -Duint16_t="unsigned short"
  
APP_NAME := fbtest

SRCLIST  := fbtest.c \
        fbtestUtils.c

ifeq ($(TARGET_USES_ION),true)
        LOCAL_CFLAGS := -DUSE_ION
endif

MDP3_PRODUCT_LIST := msm7625_surf
MDP3_PRODUCT_LIST += msm7625_ffa
MDP3_PRODUCT_LIST += msm7627_surf
MDP3_PRODUCT_LIST += msm7627_ffa
MDP3_PRODUCT_LIST += msm7627a
MDP3_PRODUCT_LIST += qsd8250_surf
MDP3_PRODUCT_LIST += qsd8250_ffa
MDP3_PRODUCT_LIST += qsd8650a_st1x

MDP4_PRODUCT_LIST := msm7630_fusion
MDP4_PRODUCT_LIST += msm7630_surf
MDP4_PRODUCT_LIST += msm8660
MDP4_PRODUCT_LIST += msm8660_csfb
MDP4_PRODUCT_LIST += msm8960
QPIC_PRODUCT_LIST += mdm9625
QPIC_PRODUCT_LIST += mdm9635

TARGET_WITH_MDP3 := $(call is-board-platform-in-list,$(MDP3_PRODUCT_LIST))
TARGET_WITH_MDP4 := $(call is-board-platform-in-list,$(MDP4_PRODUCT_LIST))
TARGET_WITH_QPIC := $(call is-board-platform-in-list,$(QPIC_PRODUCT_LIST))

ifneq (,$(strip $(TARGET_WITH_MDP3)))
        LOCAL_CFLAGS := -DMDP3_FLAG
        LOCAL_SRC_FILES += mdp3.c
endif

ifneq (,$(strip $(TARGET_WITH_MDP4)))
        LOCAL_CFLAGS := -DMDP4_FLAG
        LOCAL_SRC_FILES += mdp4.c
endif

ifneq (,$(strip $(TARGET_WITH_QPIC)))
        LOCAL_CFLAGS += -DQPIC_FLAG
endif

FBTEST_LDLIBS   := $(LDLIBS)

all: $(APP_NAME)

$(APP_NAME): $(SRCLIST)
	$(CC) $(FBTEST_CFLAGS) $(FBTEST_CPPFLAGS) $(LDFLAGS) -o $@ $^ $(FBTEST_LDLIBS)

clean:
	rm -rf $(APP_NAME)
