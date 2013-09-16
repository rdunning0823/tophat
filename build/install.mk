ifeq ($(TARGET),UNIX)

DESTDIR =
prefix = $(DESTDIR)/usr

install-mo: mo
	install -d -m 0755 $(patsubst %,$(prefix)/share/locale/%/LC_MESSAGES,$(LINGUAS))
	for i in $(LINGUAS); do \
		install -m 0644 $(OUT)/po/$$i.mo $(prefix)/share/locale/$$i/LC_MESSAGES/tophat.mo; \
	done

install-bin: all
	install -d -m 0755 $(prefix)/bin
	install -m 0755 $(TARGET_BIN_DIR)/tophat $(TARGET_BIN_DIR)/vali-xcs $(prefix)/bin

install-manual: manual
	install -d -m 0755 $(prefix)/share/doc/tophat
	install -m 0644 $(MANUAL_PDF) $(prefix)/share/doc/tophat

install: install-bin install-mo install-manual

endif
