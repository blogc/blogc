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

AC_PROG_SED

RPM_VERSION=`echo ${PACKAGE_VERSION} | sed "s/-/./g"`
AC_SUBST(RPM_VERSION)

AC_ARG_ENABLE([rpmbuild], AS_HELP_STRING([--disable-rpmbuild],
              [ignore presence of rpmbuild and disable srpm generation]))
AS_IF([test "x$enable_rpmbuild" != "xno"], [
  AC_PATH_PROG([rpmbuild], [rpmbuild])
  AS_IF([test "x$ac_cv_path_rpmbuild" = "x"], [
    have_rpmbuild=no
  ], [
    have_rpmbuild=yes
  ])
])
AS_IF([test "x$have_rpmbuild" = "xyes"], , [
  AS_IF([test "x$enable_rpmbuild" = "xyes"], [
    AC_MSG_ERROR([rpmbuild requested but not found])
  ])
])
AM_CONDITIONAL([USE_RPMBUILD], [test "x$ac_cv_path_rpmbuild" != "x"])
RPMBUILD="$ac_cv_path_rpmbuild"
AC_SUBST(RPMBUILD)

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

GIT_RECEIVER="disabled"
AC_ARG_ENABLE([git-receiver], AS_HELP_STRING([--enable-git-receiver],
              [build blogc-git-receiver tool]))
AS_IF([test "x$enable_git_receiver" = "xyes"], [
  AC_CHECK_HEADERS([sys/types.h sys/stat.h sys/wait.h time.h libgen.h unistd.h errno.h dirent.h], [
    GIT_RECEIVER="enabled"
    have_git_receiver=yes
  ], [
    AC_MSG_ERROR([blogc-git-receiver tool requested but required headers not found])
  ])
])
AM_CONDITIONAL([BUILD_GIT_RECEIVER], [test "x$have_git_receiver" = "xyes"])

RUNSERVER="disabled"
AC_ARG_ENABLE([runserver], AS_HELP_STRING([--enable-runserver],
              [build blogc-runserver tool]))
AS_IF([test "x$enable_runserver" = "xyes"], [
  AC_CHECK_HEADERS([signal.h limits.h fcntl.h unistd.h sys/stat.h sys/types.h sys/socket.h netinet/in.h arpa/inet.h],, [
    AC_MSG_ERROR([blogc-runserver tool requested but required headers not found])
  ])
  AX_PTHREAD([], [
    AC_MSG_ERROR([blogc-runserver tool requested but pthread is not supported])
  ])
  RUNSERVER="enabled"
  have_runserver=yes
])
AM_CONDITIONAL([BUILD_RUNSERVER], [test "x$have_runserver" = "xyes"])

AC_CHECK_HEADERS([sys/stat.h time.h])

LT_LIB_M

AC_CONFIG_FILES([
  Makefile
  blogc.spec
])

AC_OUTPUT

AS_ECHO("
        ====== ${PACKAGE_STRING} ======

        prefix:              ${prefix}
        exec_prefix:         ${exec_prefix}
        bindir:              ${bindir}

        compiler:            ${CC}
        cflags:              ${CFLAGS}
        ldflags:             ${LDFLAGS}

        blogc-git-receiver:  ${GIT_RECEIVER}
        blogc-runserver:     ${RUNSERVER}

        tests:               ${TESTS}

        ronn:                ${RONN}
        valgrind:            ${VALGRIND}
        rpmbuild:            ${RPMBUILD}
")
