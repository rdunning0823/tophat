LIBPNG ?= n

ifeq ($(LIBPNG),y)

LIBPNG_CPPFLAGS := $(patsubst -I%,-isystem %,$(LIBPNG_CPPFLAGS))

LIBPNG_LDADD += $(ZLIB_LDADD)
LIBPNG_LDLIBS += -lpng $(ZLIB_LDLIBS)

ifneq ($(CLANG),y)
ifneq ($(filter 4.8%,$(CXX_VERSION)),)
# detected gcc 4.8

# this option disables a C++11 warning/error in libpng due to a missing space
# in a debug macro. unfortunately a GCC bug is producing the warning even
# though the code should be disabled by the preprocessor.
# (see http://gcc.gnu.org/bugzilla//show_bug.cgi?id=58155)

#LIBPNG_CPPFLAGS += -Wno-werror=literal-suffix
endif
endif

target_arch = -march=armv7-a -mcpu=cortex-a8 -mfpu=neon -mfloat-abi=hard
common_flags = -Os -g -ffunction-sections -fdata-sections -fvisibility=hidden $(target_arch)

.PHONY: libpng
libpng:
	cd lib/libpng; \
		./autogen.sh --maintainer --clean; \
		./autogen.sh --maintainer
	export PWD=`pwd`; \
	export CC=$(HOST_ARCH)-gcc; \
	export CFLAGS="$(common_flags)"; \
	export CPPFLAGS="-isystem \
		${PWD}/$(TARGET_OUTPUT_DIR)/lib/$(HOST_ARCH)/root/include \
		-DNDEBUG"; \
	export LDFLAGS="-L${PWD}/$(TARGET_OUTPUT_DIR)/lib/$(HOST_ARCH)/root/lib"; \
	cd lib/libpng; \
	./configure \
	--host=$(HOST_ARCH) \
	--prefix=${PWD}/$(TARGET_OUTPUT_DIR)/lib/$(HOST_ARCH)/root \
	--enable-silent-rules \
	--disable-shared \
	--enable-static \
	--enable-arm-neon
	make -C lib/libpng all install

endif
