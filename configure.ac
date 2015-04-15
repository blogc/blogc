AC_PREREQ([2.69])

AC_INIT([blogc], [0], [https://github.com/rafaelmartins/blogc], [blogc], [http://blogc.org])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.13 foreign dist-bzip2 dist-xz subdir-objects serial-tests])
AC_CONFIG_HEADERS([config.h])
AM_SILENT_RULES([yes])
AM_MAINTAINER_MODE([enable])

LT_INIT

AC_PROG_CC_C99
AS_IF([test "x$ac_cv_prog_cc_c99" = "xno"], [
  AC_MSG_ERROR([no C99 compiler found, blogc requires a C99 compiler.])
])

AC_ARG_ENABLE([examples], AS_HELP_STRING([--enable-examples], [build examples]))
AS_IF([test "x$enable_examples" = "xyes"], [
  build_examples=yes
  EXAMPLES="enabled"
], [
  build_examples=no
  EXAMPLES="disabled"
])
AM_CONDITIONAL([BUILD_EXAMPLES], [test "x$build_examples" = "xyes"])

AC_ARG_ENABLE([valgrind], AS_HELP_STRING([--disable-valgrind],
              [ignore presence of valgrind]))
AS_IF([test "x$enable_valgrind" != "xno"], [
  AC_PATH_PROG([valgrind], [valgrind])
  AS_IF([test "x$ac_cv_path_valgrind" = "x"], [
    have_valgrind=no
  ], [
    have_valgrind=yes
  ])
])
AS_IF([test "x$have_valgrind" = "xyes"], , [
  AS_IF([test "x$enable_valgrind" = "xyes"], [
    AC_MSG_ERROR([valgrind requested but not found])
  ])
])
AM_CONDITIONAL([USE_VALGRIND], [test "x$have_valgrind" = "xyes"])
VALGRIND="$ac_cv_path_valgrind"
AC_SUBST(VALGRIND)

AC_ARG_ENABLE([leg], AS_HELP_STRING([--disable-leg],
              [ignore presence of peg/leg and disable parser grammar regeneration]))
AS_IF([test "x$enable_leg" != "xno"], [
  AC_PATH_PROG([leg], [leg])
  AS_IF([test "x$ac_cv_path_leg" = "x"], [
    have_leg=no
  ], [
    have_leg=yes
  ])
])
AS_IF([test "x$have_leg" = "xyes"], , [
  AS_IF([test "x$enable_leg" = "xyes"], [
    AC_MSG_ERROR([peg/leg requested but not found])
  ])
])
AM_CONDITIONAL([USE_LEG], [test "x$have_leg" = "xyes"])
LEG="$ac_cv_path_leg"
AC_SUBST(LEG)

AC_ARG_ENABLE([cmocka], AS_HELP_STRING([--disable-cmocka],
              [ignore presence of cmocka. this will disable unit tests]))
AS_IF([test "x$enable_cmocka" != "xno"], [
  PKG_PROG_PKG_CONFIG
  PKG_CHECK_MODULES([CMOCKA], [cmocka], [
    CMOCKA="enabled"
    have_cmocka=yes
  ], [
    CMOCKA="disabled"
    have_cmocka=no
  ])
])
AS_IF([test "x$have_cmocka" = "xyes"], , [
  AS_IF([test "x$enable_cmocka" = "xyes"], [
    AC_MSG_ERROR([cmocka requested but not found])
  ])
])
AM_CONDITIONAL([USE_CMOCKA], [test "x$have_cmocka" = "xyes"])

AC_CONFIG_FILES([
    Makefile
])
AC_OUTPUT

AS_ECHO("
        ==== ${PACKAGE_STRING} ====

        prefix:       ${prefix}
        exec_prefix:  ${exec_prefix}
        bindir:       ${bindir}

        compiler:     ${CC}
        cflags:       ${CFLAGS}
        ldflags:      ${LDFLAGS}

        examples:     ${EXAMPLES}

        valgrind:     ${VALGRIND}
        leg:          ${LEG}
        cmocka:       ${CMOCKA}
")
