VERSION = $(strip $(shell cat $(topdir)/VERSION.txt))
TOPHAT_VERSION = $(strip $(shell cat $(topdir)/VERSION_TOPHAT.txt))
FULL_VERSION = $(VERSION)
BUILD_DATE = $(strip $(shell date +"%b_%d_%Y"))

CPPFLAGS += -DXCSOAR_VERSION=\"$(VERSION)\"
CPPFLAGS += -DTOPHAT_VERSION=\"$(TOPHAT_VERSION)\"

GIT_COMMIT_ID := $(shell git rev-parse --short --verify HEAD 2>$(NUL))
RELEASE_COMMIT_ID := $(shell git rev-parse --short --verify "v$(VERSION)^{commit}" 2>$(NUL))
# only append the commit id for unreleased builds (no release tag)
ifneq ($(GIT_COMMIT_ID),$(RELEASE_COMMIT_ID))
CPPFLAGS += -DGIT_COMMIT_ID=\"$(GIT_COMMIT_ID)\"
FULL_VERSION := $(FULL_VERSION)~$(GIT_COMMIT_ID)
endif

$(call SRC_TO_OBJ,$(SRC)/Version.cpp): $(topdir)/VERSION.txt
