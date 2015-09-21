TARGET_DIST_NAME = TopHat-$(FULL_VERSION)-$(TARGET)
TARGET_DIST_DIR = $(TARGET_OUTPUT_DIR)/dist

dist: $(XCSOAR_BIN) $(VALI_XCS_BIN)
	rm -rf $(TARGET_DIST_DIR)/$(TARGET_DIST_NAME)
	$(MKDIR) -p $(TARGET_DIST_DIR)/$(TARGET_DIST_NAME)
	cp $^ COPYING \
		$(TARGET_DIST_DIR)/$(TARGET_DIST_NAME)/
	cd $(TARGET_DIST_DIR) && zip -qr $(TARGET_DIST_NAME).zip $(TARGET_DIST_NAME)
	mv $(TARGET_DIST_DIR)/$(TARGET_DIST_NAME).zip $(OUT)/

ifeq ($(TARGET),ALTAIR)
# Build a ZIP file to be unpacked on a USB stick.  The Altair will
# automatically pick up the EXE file with the "magic" file name
$(TARGET_OUTPUT_DIR)/XCSoarAltair.zip: $(TARGET_BIN_DIR)/TopHat.exe
	rm -rf $(@D)/ToAltair
	$(MKDIR) -p $(@D)/ToAltair
	cp $< $(@D)/ToAltair/
	upx --best $(@D)/ToAltair/TopHat.exe
	mv $(@D)/ToAltair/TopHat.exe $(@D)/ToAltair/XCSoarAltair-600-CRC3E.exe
	cd $(@D) && zip -r -Z store $(@F) ToAltair
endif
