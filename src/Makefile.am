
bin_PROGRAMS = fuse-7z-ng

AM_CPPFLAGS  = -Wall -Werror -fno-strict-aliasing -std=c++0x

# AM_CPPFLAGS += -pedantic

AM_CPPFLAGS += @FUSE_CFLAGS@
fuse_7z_ng_LDADD = @FUSE_LIBS@

fuse_7z_ng_SOURCES = \
		 main.cpp \
		 fuse-7z.cpp \
		 fuse-7z-7zip.cpp \
		 fuse-7z-node.cpp \
		 lib7zip/HelperFuncs.cpp \
		 lib7zip/7zipLibrary.cpp \
		 lib7zip/7ZipArchiveItem.cpp \
		 lib7zip/7ZipArchive.cpp

	
