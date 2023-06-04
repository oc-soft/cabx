AC_INIT([cabx], [0.1.0], [toshi@oc-soft.net])

AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE

LT_INIT([win32-dll])
AC_CONFIG_SUBDIRS([oclib])
AC_CONFIG_MACRO_DIRS([m4])

LT_LANG([Windows Resource])

AM_CONDITIONAL([MINGW_HOST], [test $host_os = mingw32])

AC_PROG_CC
AC_REQUIRE_AUX_FILE([tap-driver.sh])
AC_CHECK_PROG([GPERF], [gperf --version], [gperf])

AC_CONFIG_HEADERS([src/config.h])

AC_CONFIG_FILES([Makefile
  src/Makefile])

AC_OUTPUT

# vi: se ts=2 sw=2 et:
