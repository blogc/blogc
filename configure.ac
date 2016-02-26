AC_PREREQ([2.69])

AC_INIT([blogc], m4_esyscmd([build-aux/git-version-gen .tarball-version]),
        [https://github.com/blogc/blogc], [blogc], [https://blogc.rgm.io/])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.13 foreign dist-bzip2 dist-xz dist-zip subdir-objects
                  serial-tests -Wall -Wno-extra-portability -Werror])
AC_CONFIG_HEADERS([config.h])
AM_SILENT_RULES([yes])
AM_MAINTAINER_MODE([enable])

AC_USE_SYSTEM_EXTENSIONS

LT_INIT

AC_PROG_CC_C99
AS_IF([test "x$ac_cv_prog_cc_c99" = "xno"], [
  AC_MSG_ERROR([no C99 compiler found, blogc requires a C99 compiler.])
])

PKG_PROG_PKG_CONFIG

AC_ARG_ENABLE([ronn], AS_HELP_STRING([--disable-ronn],
              [ignore presence of ronn and disable man pages generation]))
AS_IF([test "x$enable_ronn" != "xno"], [
  AC_PATH_PROG([ronn], [ronn])
  AS_IF([test "x$ac_cv_path_ronn" = "x"], [
    have_ronn=no
  ], [
    have_ronn=yes
  ])
])
AS_IF([test "x$have_ronn" = "xyes"], , [
  AS_IF([test "x$enable_ronn" = "xyes"], [
    AC_MSG_ERROR([ronn requested but not found])
  ])
])
AM_CONDITIONAL([USE_RONN], [test "x$have_ronn" = "xyes"])
RONN="$ac_cv_path_ronn"
AC_SUBST(RONN)

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

TESTS="disabled"
AC_ARG_ENABLE([tests], AS_HELP_STRING([--disable-tests],
              [disable unit tests, ignoring presence of cmocka]))
AS_IF([test "x$enable_tests" != "xno"], [
  PKG_CHECK_MODULES([CMOCKA], [cmocka], [
    AC_MSG_CHECKING([whether the linker supports -wrap])
    save_LDFLAGS="$LDFLAGS"
    LDFLAGS="$LDFLAGS -Wl,-wrap,exit"
    AC_RUN_IFELSE([
      AC_LANG_PROGRAM([
        [void __wrap_exit(int s){__real_exit(0);}]
      ], [
        [exit(1);]
      ])
    ], [
      TESTS="enabled"
      have_cmocka=yes
    ], [
      have_cmocka=no
    ], [
      have_cmocka=no
    ])
    AC_MSG_RESULT([$have_cmocka])
    LDFLAGS="$save_LDFLAGS"
  ], [
    have_cmocka=no
  ])
])
AS_IF([test "x$have_cmocka" = "xyes"], , [
  AS_IF([test "x$enable_tests" = "xyes"], [
    AC_MSG_ERROR([unit tests requested but cmocka not found])
  ])
])
AM_CONDITIONAL([USE_CMOCKA], [test "x$have_cmocka" = "xyes"])

AC_CHECK_HEADERS([sys/types.h sys/stat.h time.h])

LT_LIB_M

AC_ARG_WITH([squareball], [AS_HELP_STRING([--with-squareball=@<:@internal/system@:>@],
            [whether to use library squareball from system [default=internal]])])
AS_IF([test "x$with_squareball" = "xsystem"], [
  SQUAREBALL="system"
  PKG_CHECK_MODULES([SQUAREBALL], [squareball], , [
    AC_MSG_ERROR([library squareball requested from system but not found])
  ])
], [
  SQUAREBALL="internal"
  SQUAREBALL_CFLAGS='-I$(top_srcdir)/squareball/src'
  SQUAREBALL_LIBS='$(top_builddir)/squareball/libsquareball.la'
  AC_SUBST(SQUAREBALL_LIBS)
  AC_SUBST(SQUAREBALL_CFLAGS)
  ac_configure_args_pre="$ac_configure_args"
  ac_configure_args_post="$ac_configure_args --enable-bundleme"
  ac_configure_args="$ac_configure_args_post"
  AC_CONFIG_COMMANDS_PRE([ac_configure_args="$ac_configure_args_pre"])
  AC_CONFIG_COMMANDS_POST([ac_configure_args="$ac_configure_args_post"])
  AC_CONFIG_SUBDIRS([squareball])
  ac_configure_args="$ac_configure_args_pre"
])
AM_CONDITIONAL(INTERNAL_SQUAREBALL, [test "x$with_squareball" != "xsystem"])

AC_CONFIG_FILES([
  Makefile
  blogc.spec
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

        squareball:   ${SQUAREBALL}
        tests:        ${TESTS}

        ronn:         ${RONN}
        valgrind:     ${VALGRIND}
")
