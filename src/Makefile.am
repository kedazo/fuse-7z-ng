
bin_PROGRAMS = fuse-7z-ng

AM_CPPFLAGS  = -Wall -Werror -fno-strict-aliasing -std=c++0x
AM_CPPFLAGS += -I../lib7zip-165/

# AM_CPPFLAGS += -pedantic

AM_CPPFLAGS 	 += @fuse_CFLAGS@
fuse_7z_ng_LDADD  = \
		@fuse_LIBS@ \
		../lib7zip-165/lib7zip.a

fuse_7z_ng_SOURCES = \
		 main.cpp \
		 logger.cpp \
		 fuse_functions.cpp \
		 fuse-7z.cpp \
		 fuse-7z-node.cpp
	
