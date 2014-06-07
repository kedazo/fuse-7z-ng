AC_INIT([fuse-7z-ng], [0.1], [david.kedves@borgcollective.eu])
AC_CONFIG_SRCDIR([src/main.cpp])
AM_INIT_AUTOMAKE([subdir-objects])
AM_MAINTAINER_MODE

#AM_SILENT_RULES([yes])

AC_PROG_CPP
AC_PROG_CXX
AC_PROG_MAKE_SET

AC_LANG([C++])

AC_HEADER_STDC

AC_CHECK_LIB(pthread,pthread_yield,,AC_MSG_ERROR([pthread library not found.]))

PKG_CHECK_MODULES([fuse], [fuse >= 2.8])
AC_SUBST(fuse_CFLAGS)
AC_SUBST(fuse_LIBS)

AC_CONFIG_FILES(Makefile src/Makefile)
AC_OUTPUT

