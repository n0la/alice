AC_PREREQ([2.69])
AC_INIT([alice], [0.1], [florian@n0la.org])
AM_INIT_AUTOMAKE([subdir-objects])
LT_INIT()
AC_CONFIG_HEADERS([config.h])

AC_PROG_CC

PKG_CHECK_MODULES([DICE], [libdice >= 0.1])
PKG_CHECK_MODULES([IRC], [libirc >= 0.1])

AC_CHECK_HEADER_STDBOOL()

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
