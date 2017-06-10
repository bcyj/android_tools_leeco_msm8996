ifneq ($(strip $(TARGET_NO_BOOTLOADER)),true)
ifneq ($(strip $(QTI_GENSECIMAGE_MSM_IDS)),)

# Required:
#   QTI_GENSECIMAGE_MSM_IDS - space separated list of MSM ID's (e.g. 8084)
#   - This will trigger generation of the gensecimage signed bootloader
#     under $OUT/signed for each MSM_IDS in the list
# Optional:
#   QTI_GENSECIMAGE_SIGNED_DEFAULT - one of the MSM ID in the MSM_IDS list
#   - This will override default unsigned bootloader binaries with the
#     gensecimage signed one for the specified MSM ID.  That is, the default
#     bootloader binaries in $OUT will be gensecimage signed

#define basedir as LOCAL_PATH cannot be used with direct make target rules
#Chipsets 8916,8994 use a new tool called secimage while older chipsets like 8084 use gensecimage.
#Select the tool to be used for signing based on the chipset.

# The following 3 bootloader image files are identical but with different names
BOOTLOADER_EMMC    = $(shell basename $(TARGET_EMMC_BOOTLOADER))
BOOTLOADER_EMMC2   = $(shell basename $(TARGET_BOOTLOADER))
BOOTLOADER_DEFAULT = $(shell basename $(INSTALLED_BOOTLOADER_MODULE))

ifeq ($(USESECIMAGETOOL), true)
SECIMAGE_BASE := vendor/qcom/proprietary/common/scripts/SecImage
else
GENSECIMAGE_BASE := vendor/qcom/proprietary/common/scripts/gensecimage
define gensectarget_param
$(eval GENSEC_MSM_PARAM :=) \
$(foreach t,$(1), \
  $(eval GENSEC_MSM_PARAM += --msmid=$(t)))
endef
endif

ifeq ($(USESECIMAGETOOL), true)
define sec-image-generate
	@echo Generating signed appsbl using secimage tool for $(strip $(QTI_GENSECIMAGE_MSM_IDS))
	@rm -rf $(PRODUCT_OUT)/signed
	$(hide) SECIMAGE_LOCAL_DIR=$(SECIMAGE_BASE) USES_SEC_POLICY_MULTIPLE_DEFAULT_SIGN=$(USES_SEC_POLICY_MULTIPLE_DEFAULT_SIGN) \
			USES_SEC_POLICY_DEFAULT_SUBFOLDER_SIGN=$(USES_SEC_POLICY_DEFAULT_SUBFOLDER_SIGN) \
			USES_SEC_POLICY_INTEGRITY_CHECK=$(USES_SEC_POLICY_INTEGRITY_CHECK) python $(SECIMAGE_BASE)/sectools_builder.py \
		-i $(TARGET_EMMC_BOOTLOADER) \
		-t $(SECIMAGE_BASE)/signed \
		-g appsbl \
		--install_base_dir=$(PRODUCT_OUT) \
		 > $(PRODUCT_OUT)/secimage.log 2>&1
	@echo Completed secimage signed appsbl \(logs in $(PRODUCT_OUT)/secimage.log\)

endef
else
define sec-image-generate
	@echo Generating signed appsbl using gensecimage tool for $(strip $(QTI_GENSECIMAGE_MSM_IDS))
	@rm -rf $(PRODUCT_OUT)/signed
	$(call gensectarget_param,$(QTI_GENSECIMAGE_MSM_IDS))
	$(hide) GENSECIMAGE_LOCAL_DIR=$(GENSECIMAGE_BASE) python $(GENSECIMAGE_BASE)/gensecimage_builder.py \
		--section=appsbl \
		-t $(PRODUCT_OUT)/signed \
		-s $(TARGET_EMMC_BOOTLOADER) \
		$(GENSEC_MSM_PARAM) \
		--basic_cfg_template=$(GENSECIMAGE_BASE)/resources/gensecimage.appsbl.cfg \
		--signing_cfg_template=$(GENSECIMAGE_BASE)/resources/signingattr_qpsa.appsbl.cfg \
		--msm_jtag_mapping_file=$(GENSECIMAGE_BASE)/resources/msm_jtag_mapping.txt \
		> $(PRODUCT_OUT)/gensecimage.log 2>&1
	@echo Completed gensecimage signed appsbl \(logs in $(PRODUCT_OUT)/gensecimage.log\)
endef
endif


.PHONY: gensecimage_target
gensecimage_target: $(TARGET_EMMC_BOOTLOADER) $(INSTALLED_BOOTLOADER_MODULE)
	$(hide) $(call sec-image-generate)

.PHONY: gensecimage_install
gensecimage_install: gensecimage_target
	@echo -e $(shell if [ -n "$(QTI_GENSECIMAGE_SIGNED_DEFAULT)" ] && \
	                    [ -d "$(PRODUCT_OUT)/signed/$(QTI_GENSECIMAGE_SIGNED_DEFAULT)" ]; then \
		rm -rf $(PRODUCT_OUT)/unsigned; \
		mkdir -p $(PRODUCT_OUT)/unsigned; \
		echo "Enabling gensecimage appsbl as default"; \
		cp $(TARGET_EMMC_BOOTLOADER) $(PRODUCT_OUT)/unsigned/$(BOOTLOADER_EMMC); \
		cp $(TARGET_EMMC_BOOTLOADER) $(PRODUCT_OUT)/unsigned/$(BOOTLOADER_EMMC2); \
		cp $(TARGET_EMMC_BOOTLOADER) $(PRODUCT_OUT)/unsigned/$(BOOTLOADER_DEFAULT); \
		cp $(PRODUCT_OUT)/signed/$(QTI_GENSECIMAGE_SIGNED_DEFAULT)/$(BOOTLOADER_EMMC) $(PRODUCT_OUT)/; \
		cp $(PRODUCT_OUT)/signed/$(QTI_GENSECIMAGE_SIGNED_DEFAULT)/$(BOOTLOADER_EMMC) $(PRODUCT_OUT)/$(BOOTLOADER_EMMC2); \
		cp $(PRODUCT_OUT)/signed/$(QTI_GENSECIMAGE_SIGNED_DEFAULT)/$(BOOTLOADER_EMMC) $(PRODUCT_OUT)/$(BOOTLOADER_DEFAULT); \
	fi)

ifeq ($(USESECIMAGETOOL), true)
droidcore: gensecimage_target
otapackage: gensecimage_target
else
droidcore: gensecimage_target gensecimage_install
otapackage: gensecimage_target gensecimage_install
endif

endif
endif
