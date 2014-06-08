AC_INIT([fuse-7z-ng], [0.1], [david.kedves@borgcollective.eu])
AC_CONFIG_SRCDIR([src/main.cpp])
AM_INIT_AUTOMAKE([subdir-objects])
AM_MAINTAINER_MODE

AC_CANONICAL_HOST

#AM_SILENT_RULES([yes])

AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_RANLIB
AC_PROG_MAKE_SET
AC_C_CONST
AC_C_INLINE
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

AC_LANG([C++])

AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stddef.h string.h unistd.h dlfcn.h dirent.h])
AC_CHECK_FUNCS([memset memmove])
AC_CHECK_FUNCS([sqrt])

AC_CHECK_LIB([pthread], [pthread_yield],,AC_MSG_ERROR([pthread library not found.]))
AC_CHECK_LIB([dl], [dlclose])

PKG_CHECK_MODULES([fuse], [fuse >= 2.8])
AC_SUBST(fuse_CFLAGS)
AC_SUBST(fuse_LIBS)

AC_CONFIG_FILES(Makefile lib7zip-165/Makefile src/Makefile)
AC_OUTPUT

