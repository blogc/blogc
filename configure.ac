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
AM_CONDITIONAL([BUILD_SRPM], [test "x$ac_cv_path_rpmbuild" != "x"])
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
AM_CONDITIONAL([BUILD_MANPAGE], [test "x$have_ronn" = "xyes"])
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

GIT_RECEIVER="disabled"
AC_ARG_ENABLE([git-receiver], AS_HELP_STRING([--enable-git-receiver],
              [build blogc-git-receiver tool]))
AS_IF([test "x$enable_git_receiver" = "xyes"], [
  AC_CHECK_HEADERS([sys/types.h sys/stat.h time.h libgen.h unistd.h errno.h dirent.h], [
    GIT_RECEIVER="enabled"
    have_git_receiver=yes
  ], [
    AC_MSG_ERROR([blogc-git-receiver tool requested but required headers not found])
  ])
])
AM_CONDITIONAL([BUILD_GIT_RECEIVER], [test "x$have_git_receiver" = "xyes"])

MAKE_="disabled"
AC_ARG_ENABLE([make-embedded], AS_HELP_STRING([--enable-make-embedded],
              [build blogc-make tool embedded on blogc binary]))
AC_ARG_ENABLE([make], AS_HELP_STRING([--enable-make],
              [build blogc-make tool]))
AS_IF([test "x$enable_make" = "xyes" -o "x$enable_make_embedded" = "xyes"], [
  AC_CHECK_HEADERS([dirent.h fcntl.h libgen.h sys/stat.h sys/wait.h time.h unistd.h],, [
    AC_MSG_ERROR([blogc-make tool requested but required headers not found])
  ])
  AX_PTHREAD([], [
    AC_MSG_ERROR([blogc-make tool requested but pthread is not supported])
  ])
  have_make_lib=yes
  AS_IF([test "x$enable_make_embedded" = "xyes"], [
    MAKE_="enabled (embedded)"
    have_make_embedded=yes
    AC_DEFINE([MAKE_EMBEDDED], [], [Build blogc-make embedded to blogc binary])
  ], [
    MAKE_="enabled"
    have_make=yes
  ])
])
AM_CONDITIONAL([BUILD_MAKE], [test "x$have_make" = "xyes"])
AM_CONDITIONAL([BUILD_MAKE_LIB], [test "x$have_make_lib" = "xyes"])
AM_CONDITIONAL([BUILD_MAKE_EMBEDDED], [test "x$have_make_embedded" = "xyes"])
AC_SUBST(MAKE_)

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

TESTS="disabled"
AC_ARG_ENABLE([tests], AS_HELP_STRING([--disable-tests],
              [disable unit tests, ignoring presence of cmocka]))
AS_IF([test "x$enable_tests" != "xno"], [
  PKG_CHECK_MODULES([CMOCKA], [cmocka], [
    have_cmocka=yes
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
      have_ld_wrap=yes
    ], [
      have_ld_wrap=no
    ], [
      have_ld_wrap=no
    ])
    AC_MSG_RESULT([$have_ld_wrap])
    AS_IF([test "x$have_ld_wrap" = "xyes"],, [
      AC_MSG_WARN([linker does not supports -wrap. tests requiring it will be disabled])
    ])
    LDFLAGS="$save_LDFLAGS"
  ], [
    have_cmocka=no
  ])
  AS_IF([test "x$have_cmocka" = "xyes"],, [
    have_tests=no
    TESTS="disabled"
    AS_IF([test "x$enable_tests" = "xyes"], [
      AC_MSG_ERROR([tests requested but cmocka was not found])
    ], [
      AC_MSG_WARN([cmocka was not found, disabling tests])
    ])
  ])

  AC_PATH_PROG([git], [git])
  AC_PATH_PROG([make], [make])
  AC_PATH_PROG([tar], [tar])
  AS_IF([test "x$ac_cv_path_git" = "x" -o "x$ac_cv_path_make" = "x" -o "x$ac_cv_path_tar" = "x"], [
    have_bgr_deps=no
    AS_IF([test "x$have_git_receiver" = "xyes"], [
      have_tests=no
      TESTS="disabled"
      AS_IF([test "x$enable_tests" = "xyes"], [
        AC_MSG_ERROR([tests requested for blogc-git-receiver but git and/or tar not found])
      ], [
        AC_MSG_WARN([git and/or tar not found, install it to run blogc-git-receiver tests])
      ])
    ])
  ], [
    have_bgr_deps=yes
  ])

  AC_PATH_PROG([bash], [bash])
  AC_PATH_PROG([diff], [diff])
  AC_PATH_PROG([tee], [tee])
  AS_IF([test "x$ac_cv_path_bash" = "x" -o "x$ac_cv_path_diff" = "x" -o "x$ac_cv_path_tee" = "x"], [
    have_tests=no
    TESTS="disabled"
    AS_IF([test "x$enable_tests" = "xyes"], [
      AC_MSG_ERROR([tests requested but bash, diff and/or tee not found])
    ], [
      AC_MSG_WARN([bash, diff and/or tee not found, install it to run tests])
    ])
  ], [
    have_tests=yes
    TESTS="enabled"
  ])
])
AM_CONDITIONAL([BUILD_TESTS], [test "x$have_tests" = "xyes"])
AM_CONDITIONAL([USE_CMOCKA], [test "x$have_cmocka" = "xyes"])
AM_CONDITIONAL([USE_LD_WRAP], [test "x$have_ld_wrap" = "xyes"])
AM_CONDITIONAL([USE_BGR_DEPS], [test "x$have_bgr_deps" = "xyes"])
BASH="$ac_cv_path_bash"
AC_SUBST(BASH)

AC_CHECK_HEADERS([sys/resource.h sys/stat.h sys/time.h sys/wait.h time.h unistd.h])
AC_CHECK_FUNCS([gethostname])

LT_LIB_M

AC_CONFIG_FILES([
  Makefile
  blogc.spec
])
AC_CONFIG_FILES([tests/blogc/check_blogc.sh],
                [chmod +x tests/blogc/check_blogc.sh])
AC_CONFIG_FILES([tests/blogc-git-receiver/check_pre_receive.sh],
                [chmod +x tests/blogc-git-receiver/check_pre_receive.sh])
AC_CONFIG_FILES([tests/blogc-git-receiver/check_post_receive.sh],
                [chmod +x tests/blogc-git-receiver/check_post_receive.sh])
AC_CONFIG_FILES([tests/blogc-git-receiver/check_shell.sh],
                [chmod +x tests/blogc-git-receiver/check_shell.sh])
AC_CONFIG_FILES([tests/blogc-make/check_blogc_make.sh],
                [chmod +x tests/blogc-make/check_blogc_make.sh])
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
        blogc-make:          ${MAKE_}
        blogc-runserver:     ${RUNSERVER}

        tests:               ${TESTS}

        ronn:                ${RONN}
        valgrind:            ${VALGRIND}
        rpmbuild:            ${RPMBUILD}
")
