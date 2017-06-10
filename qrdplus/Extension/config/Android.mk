LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

##############radio property#####################
# Set the default value for CSVT and this prop needn't control by toos.
ADDITIONAL_BUILD_PROPERTIES += persist.radio.calls.on.ims=0
ADDITIONAL_BUILD_PROPERTIES += persist.radio.jbims=0
ADDITIONAL_BUILD_PROPERTIES += persist.radio.csvt.enabled=false
# Enable the custom ecclist, need database file also
ADDITIONAL_BUILD_PROPERTIES += persist.radio.custom_ecc=1
# Enable radio tech support when PLMN search and select network
ADDITIONAL_BUILD_PROPERTIES += persist.radio.rat_on=combine
# Set the timeout of pending ACK for mt sms
ADDITIONAL_BUILD_PROPERTIES += persist.radio.mt_sms_ack=20

#################################################
PRE_LOAD_SPEC := .preloadspec
$(shell mkdir -p $(TARGET_OUT)/vendor/Default)
$(shell cp -r $(LOCAL_PATH)/$(PRE_LOAD_SPEC) $(TARGET_OUT)/vendor/Default/$(PRE_LOAD_SPEC))

#################################################
SWITCH_SPEC_SH := switch_spec.sh
$(shell mkdir -p $(TARGET_OUT)/vendor/speccfg)
$(shell cp -r $(LOCAL_PATH)/$(SWITCH_SPEC_SH) $(TARGET_OUT)/vendor/speccfg/$(SWITCH_SPEC_SH))

#################################################
GMS_INSTALL_SH := GMSInstall.sh
$(shell mkdir -p $(TARGET_OUT)/vendor/speccfg)
$(shell cp -r $(LOCAL_PATH)/$(GMS_INSTALL_SH) $(TARGET_OUT)/vendor/speccfg/$(GMS_INSTALL_SH))

################################################
DEFAULT_PROP := default.prop
$(shell mkdir -p $(TARGET_OUT)/vendor/Default/system/vendor/)
$(shell cp $(LOCAL_PATH)/$(DEFAULT_PROP) $(TARGET_OUT)/vendor/Default/system/vendor/$(DEFAULT_PROP))

################################################
# Add for proper install for carrier
InstallCarrier_MainCarrierList := Default
InstallCarrier_OutsideCarrierList := CherryMyanmar CherryPhilippines CherryThailand Fly iMobile IndiaCommon LatamBrazil LatamOpenAMX Xolo MTN Smartfren Micromax
InstallCarrier_OTASourcePackList := ChinaUnicom ChinaTelecom ChinaMobile CTA
InstallCarrier_OTABinaryPackList :=

# Local Parameter
InstallCarrierPathCarrier := $(LOCAL_PATH)
InstallCarrierPathSpec := $(PRODUCT_OUT)/system/vendor/speccfg
ifeq ($(TARGET_ARCH),arm64)
  InstallCarrierUpdateBinary := $(call intermediates-dir-for,EXECUTABLES,updater,,,32)/updater
  InstallCarrierWrapperUpdateBinary := $(call intermediates-dir-for,EXECUTABLES,wrapper-updater,,,32)/wrapper-updater
else
  InstallCarrierUpdateBinary := $(call intermediates-dir-for,EXECUTABLES,updater)/updater
  InstallCarrierWrapperUpdateBinary := $(call intermediates-dir-for,EXECUTABLES,wrapper-updater)/wrapper-updater
endif
InstallCarrierSignTool := $(HOST_OUT_JAVA_LIBRARIES)/signapk.jar
InstallCarrierSignPerm := $(DEFAULT_SYSTEM_DEV_CERTIFICATE).x509.pem
InstallCarrierSignKey := $(DEFAULT_SYSTEM_DEV_CERTIFICATE).pk8
InstallCarrierSysImage := $(call intermediates-dir-for,PACKAGING,systemimage)/system.img
InstallCarrierFileList := $(PRODUCT_OUT)/installed-files.txt
InstallCarrierUpdateScript := $(InstallCarrierPathCarrier)/updater-script

# Local Parameter for refactor build.porp
BuildPropFileName := system/build.prop
OriginalBuildPropPath := $(PRODUCT_OUT)/$(BuildPropFileName)

# For OTA package in block mode
TARGET_FILES := $(PRODUCT_OUT)/OTA_Target_Files
BINARY_PACKS := $(PRODUCT_OUT)/OTA_Binary_Packs

# Back up all carrier packages
BackupAllCarriers: $(InstallCarrierFileList) $(FULL_SYSTEMIMAGE_DEPS)
	@if [ -d "$(TARGET_FILES)" ] ; then \
           cd $(TARGET_FILES) ;\
           rm -rf `ls | grep -v "Default.zip"` ;\
           cd - ;\
         fi
	@if [ -d "$(BINARY_PACKS)" ] ; then \
           rm -rf $(BINARY_PACKS)/* ;\
         fi
	@mkdir -p $(TARGET_FILES)
	@mkdir -p $(BINARY_PACKS)
	@mkdir -p $(TARGET_FILES)/temp
	@for path in `find $(PRODUCT_OUT)/system/vendor -name ".preloadspec"` ; do \
           tmp="$${path%\/.preloadspec}" ;\
           carrier="$${tmp##*\/}" ;\
           if [ "$$carrier" != "" ] ; then \
             echo "Back up $$carrier to $(TARGET_FILES)/temp/$$carrier ..." ;\
             cp -rf $(PRODUCT_OUT)/system/vendor/$$carrier $(TARGET_FILES)/temp ;\
           fi ;\
         done

InstallCarrier: $(InstallCarrierFileList) $(FULL_SYSTEMIMAGE_DEPS) $(InstallCarrierUpdateBinary) $(InstallCarrierSignTool) $(InstallCarrierSignPerm) $(InstallCarrierSignKey) $(TARGET_ROOT_OUT)/fstab.qcom BackupAllCarriers
	@cp "$(OriginalBuildPropPath)" "$(PRODUCT_OUT)"/system/vendor/Default/"$(BuildPropFileName)"
	@if [ -f "$(OriginalBuildPropPath)".bakforspec ]; then \
	    cp "$(OriginalBuildPropPath)".bakforspec "$(PRODUCT_OUT)"/system/vendor/Default/"$(BuildPropFileName)" ;\
	 fi
	@echo -e "\n" >> "$(PRODUCT_OUT)"/system/vendor/Default/"$(BuildPropFileName)"
	@echo -e "import /system/vendor/default.prop\n" >> "$(PRODUCT_OUT)"/system/vendor/Default/"$(BuildPropFileName)"
	@echo -e "import /system/vendor/vendor.prop\n" >> "$(PRODUCT_OUT)"/system/vendor/Default/"$(BuildPropFileName)"
	@echo -e "import /system/vendor/power.prop\n" >> "$(PRODUCT_OUT)"/system/vendor/Default/"$(BuildPropFileName)"
	@echo "###################### Carrier Installation ######################"
	@echo "###  The current carrier list is $(InstallCarrier_MainCarrierList)"
	@echo "###  Not include $(InstallCarrier_OutsideCarrierList) in build"
	@echo "###  Make ota package for $(InstallCarrier_OTASourcePackList)"
	@echo "##################################################################"
	@echo "### 1. Make the default switch ota package"
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
        done < $(InstallCarrierPathCarrier)/updater-script
	@rm -rf $(PRODUCT_OUT)/ota.zip
	@cd $(PRODUCT_OUT) ;zip -r ota.zip META-INF/com/google/android/* ;cd -
	@rm -rf $(PRODUCT_OUT)/system/vendor/speccfg/ota.zip
	@java -Xmx2048m -jar $(InstallCarrierSignTool) -w $(InstallCarrierSignPerm) $(InstallCarrierSignKey) $(PRODUCT_OUT)/ota.zip $(PRODUCT_OUT)/system/vendor/speccfg/ota.zip
	@echo "### 2. Make OTA Package list for $(InstallCarrier_OTASourcePackList)"
	@rm -rf $(PRODUCT_OUT)/META-INF/com/google/android/updater-script
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
        done < $(InstallCarrierPathCarrier)/regional-updater-script
	@mkdir -p $(InstallCarrierPathSpec)
	@for item in $(InstallCarrier_OTASourcePackList) ; do \
           if [[ "$$item" =~ "_" ]] ; then \
             continue ;\
           fi ;\
           cd $(PRODUCT_OUT);\
           echo "strNewSpec=$$item" > system/vendor/speccfg/spec.new ;\
           rm -rf $$item.zip ;\
           zip -r $$item.zip system/vendor/$$item/* system/vendor/$$item/.preloadspec META-INF/* system/vendor/speccfg/spec.new ;\
           cd - ;\
           java -Xmx2048m -jar $(InstallCarrierSignTool) -w $(InstallCarrierSignPerm) $(InstallCarrierSignKey) $(PRODUCT_OUT)/$$item.zip $(PRODUCT_OUT)/$$item.ota.zip ;\
           rm $(PRODUCT_OUT)/$$item.zip ;\
           done
	@rm -rf $(PRODUCT_OUT)/META-INF
	@echo "### 3. Install the carriers $(InstallCarrier_MainCarrierList)"
	@rm -rf "$(PRODUCT_OUT)/system/vendor/app/google"
	@count=0; for item in $(InstallCarrier_MainCarrierList) ; do \
           let "count+=1" ;\
           done; echo "newPackCount=$$count" > $(PRODUCT_OUT)/system/vendor/speccfg/spec.new
	@x=1; for item in $(InstallCarrier_MainCarrierList) ; do \
           echo "$(PRODUCT_OUT)/system/vendor/$$item" ;\
           if [ ! -d "$(PRODUCT_OUT)/system/vendor/$$item" ] ; then \
             echo "Error: There is no pack for $$item !!!" ;\
             exit 1 ;\
           fi ;\
           echo "strNewSpec$$x=$$item" >> $(PRODUCT_OUT)/system/vendor/speccfg/spec.new ;\
           let "x+=1" ;\
           done
	@/bin/bash $(InstallCarrierPathCarrier)/switch_spec.sh $(PRODUCT_OUT)/system/vendor $(PRODUCT_OUT) $(PRODUCT_OUT) $(InstallCarrierPathSpec)/spec
	@mkdir -p $(InstallCarrierPathSpec)
	@echo "### 4. Try to remove the outside carrier list for $(InstallCarrier_OutsideCarrierList)"
	@for item in $(InstallCarrier_OutsideCarrierList) ; do \
           flag="" ;\
           for carrier in $(InstallCarrier_MainCarrierList) ; do \
             if [ "$$carrier" == "$$item" ] ; then \
                 flag="keep" ;\
                 break ;\
             fi ;\
             done ;\
           if [ "$$flag" != "keep" ] ; then \
             rm -rf $(PRODUCT_OUT)/system/vendor/$$item ;\
           fi ;\
           done
	@echo "################### End Carrier Installation ######################"

ifeq ($(call is-vendor-board-platform,QCOM),true)
$(InstallCarrierSysImage): InstallCarrier
endif

packageFiles: systemimage notice_files
InstallPackage: packageFiles otapackage
	@if [ "$(InstallCarrier_MainCarrierList)" == "Default" ] ; then \
           cp -f $(BUILT_TARGET_FILES_PACKAGE) $(TARGET_FILES)/Default.zip ;\
         fi
	@echo "######Generate OTA Target Files start...######"
	@if [ -f "$(TARGET_FILES)/Default.zip" ] ; then \
           cp -f $(InstallCarrierPathCarrier)/switch_spec.sh $(TARGET_FILES)/switch_spec.sh ;\
           cp -f $(PRODUCT_OUT)/root/file_contexts $(TARGET_FILES)/file_contexts ;\
           cp -f $(HOST_OUT_EXECUTABLES)/fs_config $(TARGET_FILES)/fs_config ;\
           mkdir -p $(TARGET_FILES)/DEFAULT ;\
           for item in $(InstallCarrier_OTABinaryPackList) ; do \
             echo "###Generate $$item target files...###" ;\
             cd $(TARGET_FILES)/DEFAULT ;\
             rm -rf ./* ;\
             unzip -o ../Default.zip -d ./ ;\
             rm -rf IMAGES ;\
             mv SYSTEM system ;\
             mv DATA data ;\
             x=0 ;\
             list="$${item//_/ }" ;\
             for carrier in $$list ; do \
               if [ -d "../temp/$$carrier" ] ; then \
                 if [ -d "system/vendor/$$carrier" ] ; then \
                   rm -rf system/vendor/$$carrier ;\
                 fi ;\
                 cp -rf ../temp/$$carrier system/vendor/ ;\
               else \
                 echo "Generate $$item target files failed: There is no pack for $$carrier !!!" ;\
                 exit 1 ;\
               fi ;\
               let "x+=1" ;\
               echo "strNewSpec$$x=$$carrier" >> system/vendor/speccfg/temp ;\
             done ;\
             echo "newPackCount=$$x" > system/vendor/speccfg/spec.new ;\
             cat system/vendor/speccfg/temp >> system/vendor/speccfg/spec.new ;\
             rm -f system/vendor/speccfg/temp ;\
             /bin/bash ../switch_spec.sh system/vendor ./ ./ system/vendor/speccfg/spec ;\
             mv system SYSTEM ;\
             mv data DATA ;\
             zip -qry ../$$item.zip ./* ;\
             zipinfo -1 ../$$item.zip | awk 'BEGIN { FS="SYSTEM/" } /^SYSTEM\// {print "system/" $$2}' | ../fs_config -C -S ../file_contexts > META/filesystem_config.txt ;\
             zip ../$$item.zip -r META/filesystem_config.txt ;\
             cd - ;\
             ./build/tools/releasetools/add_img_to_target_files -v $(TARGET_FILES)/$$item.zip ;\
           done ;\
           rm -f $(TARGET_FILES)/switch_spec.sh ;\
           rm -f $(TARGET_FILES)/file_contexts ;\
           rm -f $(TARGET_FILES)/fs_config ;\
           rm -rf $(TARGET_FILES)/DEFAULT ;\
         fi
	@echo "######Generate OTA Target Files end...######"
	@echo "######Generate OTA Binary Packs start...######"
	@if [ -f "$(TARGET_FILES)/Default.zip" ] ; then \
           mkdir -p $(BINARY_PACKS)/META-INF/com/google/android ;\
           cp -f $(InstallCarrierUpdateBinary) $(BINARY_PACKS)/META-INF/com/google/android/ ;\
           cp -f $(InstallCarrierWrapperUpdateBinary) $(BINARY_PACKS)/META-INF/com/google/android/ ;\
           mv $(BINARY_PACKS)/META-INF/com/google/android/updater $(BINARY_PACKS)/META-INF/com/google/android/ori-update-binary ;\
           mv $(BINARY_PACKS)/META-INF/com/google/android/wrapper-updater $(BINARY_PACKS)/META-INF/com/google/android/update-binary ;\
           for item in $(InstallCarrier_OTABinaryPackList) ; do \
             ./build/tools/releasetools/ota_from_target_files --block -k $(DEFAULT_SYSTEM_DEV_CERTIFICATE) -vw -i $(TARGET_FILES)/Default.zip $(TARGET_FILES)/$$item.zip $(BINARY_PACKS)/$$item.zip ;\
             ./build/tools/releasetools/ota_from_target_files --block -k $(DEFAULT_SYSTEM_DEV_CERTIFICATE) -vw -i $(TARGET_FILES)/$$item.zip $(TARGET_FILES)/Default.zip $(BINARY_PACKS)/$$item"2Default.zip" ;\
             \
             mkdir -p $(BINARY_PACKS)/system/vendor/speccfg ;\
             mkdir -p $(BINARY_PACKS)/data/modem_config ;\
             mkdir -p $(BINARY_PACKS)/data/modem_row_config ;\
             x=0 ;\
             list="$${item//_/ }" ;\
             for carrier in $$list ; do \
               let "x+=1" ;\
               echo "strNewSpec$$x=$$carrier" >> $(BINARY_PACKS)/system/vendor/speccfg/temp ;\
               if [ -f "$(TARGET_FILES)/temp/$$carrier/.preloadspec" ] ; then \
                 cp -f $(TARGET_FILES)/temp/$$carrier/.preloadspec $(BINARY_PACKS)/.preloadspec ;\
               fi ;\
               if [ -d "$(TARGET_FILES)/temp/$$carrier/data/modem_config" ] ; then \
                 cp -rf $(TARGET_FILES)/temp/$$carrier/data/modem_config/* $(BINARY_PACKS)/data/modem_config ;\
               fi ;\
               if [ -d "$(TARGET_FILES)/temp/$$carrier/data/modem_row_config" ] ; then \
                 cp -rf $(TARGET_FILES)/temp/$$carrier/data/modem_row_config/* $(BINARY_PACKS)/data/modem_row_config ;\
               fi ;\
             done ;\
             cd $(BINARY_PACKS) ;\
             echo "newPackCount=$$x" > system/vendor/speccfg/spec.new ;\
             cat system/vendor/speccfg/temp >> system/vendor/speccfg/spec.new ;\
             rm -f system/vendor/speccfg/temp ;\
             mkdir -p system/vendor/$$item ;\
             mv .preloadspec system/vendor/$$item/ ;\
             zip $$item.zip -ry system/ data/ META-INF/ ;\
             echo "newPackCount=1" > system/vendor/speccfg/spec.new ;\
             echo "strNewSpec1=Default" >> system/vendor/speccfg/spec.new ;\
             mv system/vendor/$$item system/vendor/$$item"2Default" ;\
             if [ -f "../system/vendor/Default/.preloadspec" ] ; then \
               cp -f ../system/vendor/Default/.preloadspec system/vendor/$$item"2Default"/ ;\
             fi ;\
             zip $$item"2Default.zip" -ry system/ META-INF/ ;\
             rm -rf system data ;\
             cd - ;\
             \
             java -Xmx2048m -jar $(InstallCarrierSignTool) -w $(InstallCarrierSignPerm) $(InstallCarrierSignKey) $(BINARY_PACKS)/$$item.zip $(BINARY_PACKS)/$$item.ota.zip ;\
             rm -f $(BINARY_PACKS)/$$item.zip ;\
             java -Xmx2048m -jar $(InstallCarrierSignTool) -w $(InstallCarrierSignPerm) $(InstallCarrierSignKey) $(BINARY_PACKS)/$$item"2Default.zip" $(BINARY_PACKS)/$$item"2Default.ota.zip" ;\
             rm -f $(BINARY_PACKS)/$$item"2Default.zip" ;\
           done ;\
           rm -rf $(BINARY_PACKS)/META-INF ;\
         fi
	@rm -rf $(TARGET_FILES)/temp
	@echo "######Generate OTA Binary Packs end!######"
	@cp $(BUILT_TARGET_FILES_PACKAGE) $(PRODUCT_OUT)/package_backup.zip

ifeq ($(call is-vendor-board-platform,QCOM),true)
droidcore: InstallPackage
endif
