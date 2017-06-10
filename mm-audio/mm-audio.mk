all:
	@echo "invoking audio alsa make"
	$(MAKE) -f $(SRCDIR)/audio-alsa/audio-alsa.mk
	@mv $(SYSROOTLIB_DIR)/mm-audio-*-test $(SYSROOTBIN_DIR)

	@echo "invoking omx-adec-aac make"
	$(MAKE) -f $(SRCDIR)/omx/adec-aac/qdsp5/adec-aac.mk
	@echo "invoking omx-adec-ac3 make"
	$(MAKE) -f $(SRCDIR)/omx/adec-ac3/qdsp5/adec-ac3.mk
	@echo "invoking omx-adec-adpcm make"
	$(MAKE) -f $(SRCDIR)/omx/adec-adpcm/qdsp5/adec-adpcm.mk
	@echo "invoking omx-adec-amr make"
	$(MAKE) -f $(SRCDIR)/omx/adec-amr/qdsp5/adec-amr.mk
	@echo "invoking omx-adec-amrwb make"
	$(MAKE) -f $(SRCDIR)/omx/adec-amrwb/qdsp5/adec-amrwb.mk
	@echo "invoking omx-adec-evrc make"
	$(MAKE) -f $(SRCDIR)/omx/adec-evrc/qdsp5/adec-evrc.mk
	@echo "invoking omx-adec-mp3 make"
	$(MAKE) -f $(SRCDIR)/omx/adec-mp3/qdsp5/adec-mp3.mk
	@echo "invoking omx-adec-qcelp13 make"
	$(MAKE) -f $(SRCDIR)/omx/adec-qcelp13/qdsp5/adec-qcelp13.mk
	@echo "invoking omx-adec-wma make"
	$(MAKE) -f $(SRCDIR)/omx/adec-wma/qdsp5/adec-wma.mk
	@mv $(SYSROOTLIB_DIR)/mm-adec-omx*-test $(SYSROOTBIN_DIR)

	@echo "invoking omx-enc-aac make"
	$(MAKE) -f $(SRCDIR)/omx/aenc-aac/qdsp5/aenc-aac.mk
	@echo "invoking omx-enc-amr make"
	$(MAKE) -f $(SRCDIR)/omx/aenc-amr/qdsp5/aenc-amr.mk
	@echo "invoking omx-enc-evrc make"
	$(MAKE) -f $(SRCDIR)/omx/aenc-evrc/qdsp5/aenc-evrc.mk
	@echo "invoking omx-enc-qcelp13 make"
	$(MAKE) -f $(SRCDIR)/omx/aenc-qcelp13/qdsp5/aenc-qcelp13.mk
	@mv $(SYSROOTLIB_DIR)/mm-aenc-omx*-test $(SYSROOTBIN_DIR)
