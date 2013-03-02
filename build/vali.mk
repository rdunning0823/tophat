# Rules for VALI-TOP.exe, the non-interactive G record validation tool

VALI_TOP_SOURCES = \
	$(SRC)/OS/FileDescriptor.cpp \
	$(SRC)/Logger/GRecord.cpp \
	$(SRC)/Logger/MD5.cpp \
	$(SRC)/Version.cpp \
	$(SRC)/VALI-TOP.cpp
VALI_TOP_DEPENDS = IO UTIL
VALI_TOP_STRIP = y

$(eval $(call link-program,vali-top,VALI_TOP))
