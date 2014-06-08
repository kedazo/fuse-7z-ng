noinst_LIBRARIES = lib7zip.a

A_EXTRA_FLAGS = 
A_LIBS = -no-undefined

AM_CPPFLAGS = \
	-I../lzma-sdk-922 \
	-I../lzma-sdk-922/CPP \
	-I../lzma-sdk-922/CPP/Common
	$(A_EXTRA_FLAGS)

SOURCES = \
	7ZipArchive.cpp \
	7ZipArchiveItem.cpp \
	7zipLibrary.cpp \
	HelperFuncs.cpp \
	HelperFuncs.h \
	lib7zip.h \
	7ZipArchiveOpenCallback.cpp \
	7ZipArchiveOpenCallback.h \
	7ZipCodecInfo.cpp \
	7ZipCodecInfo.h \
	7ZipCompressCodecsInfo.cpp \
	7ZipCompressCodecsInfo.h \
	7ZipDllHandler.cpp \
	7ZipDllHandler.h \
	7ZipFormatInfo.cpp \
	7ZipFormatInfo.h \
	7ZipFunctions.h \
	7ZipObjectPtrArray.cpp \
	7ZipOpenArchive.cpp \
	OSFunctions.h \
	OSFunctions_OS2.h \
	OSFunctions_UnixLike.cpp \
	OSFunctions_UnixLike.h \
	OSFunctions_Win32.cpp \
	OSFunctions_Win32.h \
	7ZipInStreamWrapper.cpp \
	7ZipInStreamWrapper.h \
	../lzma-sdk-922/CPP/Common/MyWindows.cpp \
	../lzma-sdk-922/CPP/Windows/PropVariant.cpp

lib7zip_a_SOURCES = $(SOURCES)

lib7zip_a_CFLAGS = $(AM_CFLAGS)
lib7zip_a_CPPFLAGS = $(AM_CPPFLAGS)

