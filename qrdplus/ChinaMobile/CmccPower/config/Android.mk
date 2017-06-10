LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#################################################
PRE_LOAD_SPEC := .preloadspec
$(shell mkdir -p $(TARGET_OUT)/vendor/CmccPower)
$(shell cp -r $(LOCAL_PATH)/$(PRE_LOAD_SPEC) $(TARGET_OUT)/vendor/CmccPower/$(PRE_LOAD_SPEC))

#################################################
EXCLUDE_LIST := exclude.list
$(shell mkdir -p $(TARGET_OUT)/vendor/CmccPower)
$(shell cp -r $(LOCAL_PATH)/$(EXCLUDE_LIST) $(TARGET_OUT)/vendor/CmccPower/$(EXCLUDE_LIST))

#################################################
SPEC_PROP := power.prop
$(shell mkdir -p $(TARGET_OUT)/vendor/CmccPower/system/vendor/)
$(shell cp -r $(LOCAL_PATH)/$(SPEC_PROP) $(TARGET_OUT)/vendor/CmccPower/system/vendor/$(SPEC_PROP))

################################################
# Local Parameter
InstallReferCarrierPathCarrier := $(LOCAL_PATH)
InstallCarrierPathSpec := $(PRODUCT_OUT)/system/vendor/speccfg
ifeq ($(TARGET_ARCH),arm64)
  InstallCarrierUpdateBinary := $(call intermediates-dir-for,EXECUTABLES,updater,,,32)/updater
else
  InstallCarrierUpdateBinary := $(call intermediates-dir-for,EXECUTABLES,updater)/updater
endif
InstallCarrierSignTool := $(HOST_OUT_JAVA_LIBRARIES)/signapk.jar
InstallCarrierSignPerm := $(DEFAULT_SYSTEM_DEV_CERTIFICATE).x509.pem
InstallCarrierSignKey := $(DEFAULT_SYSTEM_DEV_CERTIFICATE).pk8
InstallCarrierSysImage := $(call intermediates-dir-for,PACKAGING,systemimage)/system.img
InstallCarrierFileList := $(PRODUCT_OUT)/installed-files.txt
InstallReferCarrierUpdateScript := $(InstallReferCarrierPathCarrier)/updater-script

ReferCarrier := CmccPower
ReferMainCarrier := ChinaMobile

InstallCmccPower: $(InstallCarrierFileList) $(FULL_SYSTEMIMAGE_DEPS) $(InstallCarrierUpdateBinary) $(InstallCarrierSignTool) $(InstallCarrierSignPerm) $(InstallCarrierSignKey) $(TARGET_ROOT_OUT)/fstab.qcom
	@echo "###################### $(ReferCarrier) Installation ######################"
	@echo "### 1. Make the $(ReferCarrier) switch ota package"
	@mkdir -p $(PRODUCT_OUT)/META-INF/com/google/android/
	@cp "$(InstallCarrierUpdateBinary)" "$(PRODUCT_OUT)/META-INF/com/google/android/"
	@mv "$(PRODUCT_OUT)/META-INF/com/google/android/updater" "$(PRODUCT_OUT)/META-INF/com/google/android/update-binary"
	@while read line ; do \
           if [ "$${line/#mount\(/}" != "$$line" ] ; then \
             line="$${line/#mount\(\"/}" ;\
             line="$${line//\",\"/ }" ;\
             line="$${line//\"\)\;/}" ;\
             arrCfg=($$line) ;\
             while read item ; do \
               if [ "$${item/#\//}" != "$$item" ] ; then\
                 itemCfg=($$item) ; \
                 if [ "$${arrCfg[3]}" == "$${itemCfg[1]}" ] ; then \
                   echo "mount(\"$${itemCfg[2]}\",\"EMMC\",\"$${itemCfg[0]}\",\"$${itemCfg[1]}\");" >> $(PRODUCT_OUT)/META-INF/com/google/android/updater-script ;\
                 fi ;\
               fi ;\
             done < $(TARGET_ROOT_OUT)/fstab.qcom ;\
           else \
             echo $$line >> $(PRODUCT_OUT)/META-INF/com/google/android/updater-script ;\
           fi \
         done < $(InstallReferCarrierPathCarrier)/updater-script
	@cd $(PRODUCT_OUT) ;rm -rf $(ReferCarrier).ota.zip ;zip -r $(ReferCarrier).ota.zip META-INF/com/google/android/* ;cd -
	@echo "### 2. Make OTA Package for $(ReferCarrier)"
	@mkdir -p $(InstallCarrierPathSpec)
	@cd $(PRODUCT_OUT);\
           echo "newPackCount=2" > system/vendor/speccfg/spec.new ;\
           echo "strNewSpec1=$(ReferMainCarrier)" >> system/vendor/speccfg/spec.new ;\
           echo "strNewSpec2=$(ReferCarrier)" >> system/vendor/speccfg/spec.new ;\
           rm -rf $(ReferCarrier).zip ;\
           zip -r $(ReferCarrier).zip system/vendor/$(ReferCarrier)/* system/vendor/$(ReferMainCarrier)/* system/vendor/$(ReferCarrier)/.preloadspec system/vendor/$(ReferMainCarrier)/.preloadspec META-INF/* system/vendor/speccfg/spec.new ;\
           cd -
	@java -Xmx2048m -jar $(InstallCarrierSignTool) -w $(InstallCarrierSignPerm) $(InstallCarrierSignKey) $(PRODUCT_OUT)/$(ReferCarrier).zip $(PRODUCT_OUT)/$(ReferCarrier).ota.zip ;\
           rm $(PRODUCT_OUT)/$(ReferCarrier).zip
	@rm -rf $(PRODUCT_OUT)/META-INF
	@rm -rf system/vendor/speccfg/spec.new
	@echo "################### End Carrier Installation ######################"

InstallCarrier: InstallCmccPower
