# squareball: A general-purpose library for C99.
# Copyright (C) 2014-2018 Rafael G. Martins <rafael@rafaelmartins.eng.br>
#
# This program can be distributed under the terms of the BSD License.
# See the file LICENSE.

# SQUAREBALL_INIT([VERSION_ATOM])
#
# This should be added to Makefile.am
#
#   if INTERNAL_SQUAREBALL
#   SUBDIRS = squareball
#   endif
#
# The following variables will be exported to be used in the Makefile.am:
#
#   - SQUAREBALL_CFLAGS
#   - SQUAREBALL_LIBS
#
# This macro requires squareball to be installed as a git submodule in the
# top source dir.
# ----------------------------------
AC_DEFUN([SQUAREBALL_INIT], [
  AC_ARG_WITH([squareball], [AS_HELP_STRING([--with-squareball=@<:@internal/system@:>@],
              [whether to use library squareball from system [default=internal]])])
  AS_IF([test "x$with_squareball" = "xsystem"], [
    SQUAREBALL="system"
    PKG_CHECK_MODULES([SQUAREBALL], [squareball$1], , [
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
])
