ifeq ($(strip $(LOCAL_MODULE_CLASS)),)
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
endif
ifeq ($(strip $(LOCAL_MODULE_SUFFIX)),)
LOCAL_MODULE_SUFFIX := .a
endif
LOCAL_UNINSTALLABLE_MODULE := true

include $(BUILD_SYSTEM)/binary.mk

$(LOCAL_BUILT_MODULE): $(all_objects)
	$(transform-o-to-static-lib)
	@echo "target OverrideHeapFunctions: $(PRIVATE_MODULE) ($@)"
	$(hide) $(TARGET_OBJCOPY) --redefine-sym malloc=__override_malloc   \
				  --redefine-sym calloc=__override_calloc   \
				  --redefine-sym realloc=__override_realloc \
				  --redefine-sym free=__override_free       \
				  $@

