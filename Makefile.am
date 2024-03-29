## Autotools settings

ACLOCAL_AMFLAGS = -I m4

AM_DISTCHECK_CONFIGURE_FLAGS = \
	CFLAGS="-Wall -g -O0" \
	--enable-git-receiver \
	--enable-make \
	--enable-ronn \
	--enable-runserver \
	--enable-tests \
	--disable-silent-rules \
	--disable-valgrind \
	$(NULL)


## File listings

EXTRA_DIST = \
	build-aux/git-version-gen \
	build-aux/valgrind.sh \
	$(top_srcdir)/.version \
	autogen.sh \
	blogc.spec \
	LICENSE \
	README.md \
	$(NULL)

CLEANFILES = \
	$(NULL)

MAINTAINERCLEANFILES = \
	$(NULL)

BUILT_SOURCES = \
	$(top_srcdir)/.version \
	$(NULL)

noinst_HEADERS = \
	src/blogc/content-parser.h \
	src/blogc/datetime-parser.h \
	src/blogc/debug.h \
	src/blogc/filelist-parser.h \
	src/blogc/funcvars.h \
	src/blogc/loader.h \
	src/blogc/renderer.h \
	src/blogc/rusage.h \
	src/blogc/sysinfo.h \
	src/blogc/source-parser.h \
	src/blogc/template-parser.h \
	src/blogc/toctree.h \
	src/blogc-git-receiver/post-receive.h \
	src/blogc-git-receiver/pre-receive.h \
	src/blogc-git-receiver/pre-receive-parser.h \
	src/blogc-git-receiver/settings.h \
	src/blogc-git-receiver/shell.h \
	src/blogc-git-receiver/shell-command-parser.h \
	src/blogc-make/atom.h \
	src/blogc-make/ctx.h \
	src/blogc-make/exec.h \
	src/blogc-make/exec-native.h \
	src/blogc-make/httpd.h \
	src/blogc-make/reloader.h \
	src/blogc-make/rules.h \
	src/blogc-make/settings.h \
	src/blogc-make/utils.h \
	src/blogc-runserver/httpd.h \
	src/blogc-runserver/httpd-utils.h \
	src/blogc-runserver/mime.h \
	src/common/compat.h \
	src/common/config-parser.h \
	src/common/error.h \
	src/common/file.h \
	src/common/sort.h \
	src/common/stdin.h \
	src/common/utf8.h \
	src/common/utils.h \
	$(NULL)

noinst_LTLIBRARIES = \
	libblogc.la \
	libblogc_common.la \
	$(NULL)

noinst_PROGRAMS = \
	$(NULL)

bin_PROGRAMS = \
	blogc \
	$(NULL)

if BUILD_GIT_RECEIVER
bin_PROGRAMS += \
	blogc-git-receiver \
	$(NULL)

noinst_LTLIBRARIES += \
	libblogc_git_receiver.la \
	$(NULL)
endif

if BUILD_MAKE
bin_PROGRAMS += \
	blogc-make \
	$(NULL)

noinst_LTLIBRARIES += \
	libblogc_make.la \
	$(NULL)
endif

if BUILD_RUNSERVER
bin_PROGRAMS += \
	blogc-runserver \
	$(NULL)

noinst_LTLIBRARIES += \
	libblogc_runserver.la \
	$(NULL)
endif

check_PROGRAMS = \
	$(NULL)

check_SCRIPTS = \
	$(NULL)


libblogc_la_SOURCES = \
	src/blogc/content-parser.c \
	src/blogc/datetime-parser.c \
	src/blogc/debug.c \
	src/blogc/filelist-parser.c \
	src/blogc/funcvars.c \
	src/blogc/loader.c \
	src/blogc/renderer.c \
	src/blogc/rusage.c \
	src/blogc/sysinfo.c \
	src/blogc/source-parser.c \
	src/blogc/template-parser.c \
	src/blogc/toctree.c \
	$(NULL)

libblogc_la_CFLAGS = \
	$(AM_CFLAGS) \
	$(NULL)

libblogc_la_LIBADD = \
	$(LIBM) \
	libblogc_common.la \
	$(NULL)


libblogc_common_la_SOURCES = \
	src/common/compat.c \
	src/common/config-parser.c \
	src/common/error.c \
	src/common/file.c \
	src/common/sort.c \
	src/common/stdin.c \
	src/common/utf8.c \
	src/common/utils.c \
	$(NULL)

libblogc_common_la_CFLAGS = \
	$(AM_CFLAGS) \
	$(NULL)


blogc_SOURCES = \
	src/blogc/main.c \
	$(NULL)

blogc_CFLAGS = \
	$(AM_CFLAGS) \
	$(NULL)

blogc_LDADD = \
	libblogc.la \
	libblogc_common.la \
	$(NULL)

if BUILD_MAKE_EMBEDDED
blogc_SOURCES += \
	src/blogc-make/main.c \
	$(NULL)

blogc_LDADD += \
	libblogc_make.la \
	$(NULL)
endif


if BUILD_GIT_RECEIVER
blogc_git_receiver_SOURCES = \
	src/blogc-git-receiver/main.c \
	$(NULL)

blogc_git_receiver_CFLAGS = \
	$(AM_CFLAGS) \
	$(NULL)

blogc_git_receiver_LDADD = \
	libblogc_git_receiver.la \
	libblogc_common.la \
	$(NULL)

libblogc_git_receiver_la_SOURCES = \
	src/blogc-git-receiver/post-receive.c \
	src/blogc-git-receiver/pre-receive.c \
	src/blogc-git-receiver/pre-receive-parser.c \
	src/blogc-git-receiver/settings.c \
	src/blogc-git-receiver/shell.c \
	src/blogc-git-receiver/shell-command-parser.c \
	$(NULL)

libblogc_git_receiver_la_CFLAGS = \
	$(AM_CFLAGS) \
	$(NULL)

libblogc_git_receiver_la_LIBADD = \
	libblogc_common.la \
	$(NULL)
endif


if BUILD_MAKE
blogc_make_SOURCES = \
	src/blogc-make/main.c \
	$(NULL)

blogc_make_CFLAGS = \
	$(AM_CFLAGS) \
	$(PTHREAD_CFLAGS) \
	$(NULL)

blogc_make_LDADD = \
	$(PTHREAD_LIBS) \
	libblogc_make.la \
	libblogc_common.la \
	$(NULL)
endif

if BUILD_MAKE_LIB
libblogc_make_la_SOURCES = \
	src/blogc-make/atom.c \
	src/blogc-make/ctx.c \
	src/blogc-make/exec.c \
	src/blogc-make/exec-native.c \
	src/blogc-make/httpd.c \
	src/blogc-make/reloader.c \
	src/blogc-make/rules.c \
	src/blogc-make/settings.c \
	src/blogc-make/utils.c \
	$(NULL)

libblogc_make_la_CFLAGS = \
	$(AM_CFLAGS) \
	$(PTHREAD_CFLAGS) \
	$(NULL)

libblogc_make_la_LIBADD = \
	$(PTHREAD_LIBS) \
	libblogc_common.la \
	$(NULL)
endif


if BUILD_RUNSERVER
blogc_runserver_SOURCES = \
	src/blogc-runserver/main.c \
	$(NULL)

blogc_runserver_CFLAGS = \
	$(AM_CFLAGS) \
	$(PTHREAD_CFLAGS) \
	$(NULL)

blogc_runserver_LDADD = \
	$(PTHREAD_LIBS) \
	libblogc_runserver.la \
	libblogc_common.la \
	$(NULL)

libblogc_runserver_la_SOURCES = \
	src/blogc-runserver/httpd.c \
	src/blogc-runserver/httpd-utils.c \
	src/blogc-runserver/mime.c \
	$(NULL)

libblogc_runserver_la_CFLAGS = \
	$(AM_CFLAGS) \
	$(PTHREAD_CFLAGS) \
	$(NULL)

libblogc_runserver_la_LIBADD = \
	$(PTHREAD_LIBS) \
	libblogc_common.la \
	$(NULL)
endif


## Build rules: man pages

# README: all the prebuilt manpages that are hidden behind --enable-*
# options should be added here, otherwise they may be not included
# in the distfiles.
EXTRA_DIST += \
	man/blogc.1.ronn \
	man/blogc-git-receiver.1.ronn \
	man/blogc-make.1.ronn \
	man/blogc-runserver.1.ronn \
	man/blogcfile.5.ronn \
	man/blogc-source.7.ronn \
	man/blogc-template.7.ronn \
	man/blogc-toctree.7.ronn \
	man/blogc-pagination.7.ronn \
	man/index.txt \
	blogc-git-receiver.1 \
	blogc-make.1 \
	blogc-runserver.1 \
	blogcfile.5 \
	$(NULL)

dist_man_MANS = \
	blogc.1 \
	blogc-source.7 \
	blogc-template.7 \
	blogc-toctree.7 \
	blogc-pagination.7 \
	$(NULL)

if BUILD_GIT_RECEIVER
dist_man_MANS += \
	blogc-git-receiver.1 \
	$(NULL)
endif

if BUILD_MAKE_LIB
dist_man_MANS += \
	blogc-make.1 \
	blogcfile.5
	$(NULL)
endif

if BUILD_RUNSERVER
dist_man_MANS += \
	blogc-runserver.1 \
	$(NULL)
endif

MAINTAINERCLEANFILES += \
	$(dist_man_MANS) \
	$(NULL)

if BUILD_MANPAGE

blogc.1: man/blogc.1.ronn
	$(AM_V_GEN)$(RONN) \
		--roff \
		--pipe \
		--organization "Rafael G. Martins" \
		--manual "$(PACKAGE_NAME) Manual" \
		$(top_srcdir)/man/blogc.1.ronn > blogc.1

blogc-git-receiver.1: man/blogc-git-receiver.1.ronn
	$(AM_V_GEN)$(RONN) \
		--roff \
		--pipe \
		--organization "Rafael G. Martins" \
		--manual "$(PACKAGE_NAME) Manual" \
		$(top_srcdir)/man/blogc-git-receiver.1.ronn > blogc-git-receiver.1

blogc-make.1: man/blogc-make.1.ronn
	$(AM_V_GEN)$(RONN) \
		--roff \
		--pipe \
		--organization "Rafael G. Martins" \
		--manual "$(PACKAGE_NAME) Manual" \
		$(top_srcdir)/man/blogc-make.1.ronn > blogc-make.1

blogc-runserver.1: man/blogc-runserver.1.ronn
	$(AM_V_GEN)$(RONN) \
		--roff \
		--pipe \
		--organization "Rafael G. Martins" \
		--manual "$(PACKAGE_NAME) Manual" \
		$(top_srcdir)/man/blogc-runserver.1.ronn > blogc-runserver.1

blogcfile.5: man/blogcfile.5.ronn
	$(AM_V_GEN)$(RONN) \
		--roff \
		--pipe \
		--organization "Rafael G. Martins" \
		--manual "$(PACKAGE_NAME) Manual" \
		$(top_srcdir)/man/blogcfile.5.ronn > blogcfile.5

blogc-source.7: man/blogc-source.7.ronn
	$(AM_V_GEN)$(RONN) \
		--roff \
		--pipe \
		--organization "Rafael G. Martins" \
		--manual "$(PACKAGE_NAME) Manual" \
		$(top_srcdir)/man/blogc-source.7.ronn > blogc-source.7

blogc-template.7: man/blogc-template.7.ronn
	$(AM_V_GEN)$(RONN) \
		--roff \
		--pipe \
		--organization "Rafael G. Martins" \
		--manual "$(PACKAGE_NAME) Manual" \
		$(top_srcdir)/man/blogc-template.7.ronn > blogc-template.7

blogc-toctree.7: man/blogc-toctree.7.ronn
	$(AM_V_GEN)$(RONN) \
		--roff \
		--pipe \
		--organization "Rafael G. Martins" \
		--manual "$(PACKAGE_NAME) Manual" \
		$(top_srcdir)/man/blogc-toctree.7.ronn > blogc-toctree.7

blogc-pagination.7: man/blogc-pagination.7.ronn
	$(AM_V_GEN)$(RONN) \
		--roff \
		--pipe \
		--organization "Rafael G. Martins" \
		--manual "$(PACKAGE_NAME) Manual" \
		$(top_srcdir)/man/blogc-pagination.7.ronn > blogc-pagination.7

else

blogc.1:
	$(AM_V_GEN)echo "error: ronn not found. failed to build man page: $@"; exit 1

blogc-git-receiver.1:
	$(AM_V_GEN)echo "error: ronn not found. failed to build man page: $@"; exit 1

blogc-make.1:
	$(AM_V_GEN)echo "error: ronn not found. failed to build man page: $@"; exit 1

blogc-runserver.1:
	$(AM_V_GEN)echo "error: ronn not found. failed to build man page: $@"; exit 1

blogc-source.7:
	$(AM_V_GEN)echo "error: ronn not found. failed to build man page: $@"; exit 1

blogc-template.7:
	$(AM_V_GEN)echo "error: ronn not found. failed to build man page: $@"; exit 1

blogc-toctree.7:
	$(AM_V_GEN)echo "error: ronn not found. failed to build man page: $@"; exit 1

blogc-pagination.7:
	$(AM_V_GEN)echo "error: ronn not found. failed to build man page: $@"; exit 1

endif


## Build rules: tests

if BUILD_TESTS

if BUILD_GIT_RECEIVER
if USE_BGR_DEPS
check_SCRIPTS += \
	tests/blogc-git-receiver/check_post_receive.sh \
	tests/blogc-git-receiver/check_pre_receive.sh \
	tests/blogc-git-receiver/check_shell.sh \
	$(NULL)
endif
endif

if BUILD_MAKE
check_SCRIPTS += \
	tests/blogc-make/check_blogc_make.sh \
	$(NULL)
endif

check_SCRIPTS += \
	tests/blogc/check_blogc.sh \
	$(NULL)

if USE_CMOCKA

check_PROGRAMS += \
	tests/blogc/check_content_parser \
	tests/blogc/check_datetime_parser \
	tests/blogc/check_filelist_parser \
	tests/blogc/check_renderer \
	tests/blogc/check_source_parser \
	tests/blogc/check_template_parser \
	tests/blogc/check_toctree \
	tests/common/check_config_parser \
	tests/common/check_error \
	tests/common/check_sort \
	tests/common/check_utf8 \
	tests/common/check_utils \
	$(NULL)

if USE_LD_WRAP
check_PROGRAMS += \
	tests/blogc/check_funcvars \
	tests/blogc/check_loader \
	tests/blogc/check_sysinfo \
	tests/blogc/check_sysinfo2 \
	tests/common/check_stdin \
	$(NULL)

tests_blogc_check_funcvars_SOURCES = \
	tests/blogc/check_funcvars.c \
	$(NULL)

tests_blogc_check_funcvars_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_blogc_check_funcvars_LDFLAGS = \
	-no-install \
	-Wl,--wrap=bc_file_get_contents \
	$(NULL)

tests_blogc_check_funcvars_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc.la \
	libblogc_common.la \
	$(NULL)

tests_blogc_check_loader_SOURCES = \
	tests/blogc/check_loader.c \
	$(NULL)

tests_blogc_check_loader_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_blogc_check_loader_LDFLAGS = \
	-no-install \
	-Wl,--wrap=bc_file_get_contents \
	$(NULL)

tests_blogc_check_loader_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc.la \
	libblogc_common.la \
	$(NULL)

tests_blogc_check_sysinfo_SOURCES = \
	tests/blogc/check_sysinfo.c \
	$(NULL)

tests_blogc_check_sysinfo_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_blogc_check_sysinfo_LDFLAGS = \
	-no-install \
	-Wl,--wrap=bc_file_get_contents \
	-Wl,--wrap=getenv \
	$(NULL)

if HAVE_NETDB_H
tests_blogc_check_sysinfo_LDFLAGS += \
	-Wl,--wrap=gethostbyname \
	$(NULL)
endif

if HAVE_UNISTD_H
tests_blogc_check_sysinfo_LDFLAGS += \
	-Wl,--wrap=gethostname \
	$(NULL)
endif

if HAVE_TIME_H
tests_blogc_check_sysinfo_LDFLAGS += \
	-Wl,--wrap=time \
	-Wl,--wrap=gmtime \
	$(NULL)
endif

tests_blogc_check_sysinfo_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc.la \
	libblogc_common.la \
	$(NULL)

tests_blogc_check_sysinfo2_SOURCES = \
	tests/blogc/check_sysinfo2.c \
	$(NULL)

tests_blogc_check_sysinfo2_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_blogc_check_sysinfo2_LDFLAGS = \
	-no-install \
	-Wl,--wrap=bc_file_get_contents \
	$(NULL)

tests_blogc_check_sysinfo2_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc.la \
	libblogc_common.la \
	$(NULL)

tests_common_check_stdin_SOURCES = \
	tests/common/check_stdin.c \
	$(NULL)

tests_common_check_stdin_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_common_check_stdin_LDFLAGS = \
	-no-install \
	-Wl,--wrap=fgetc \
	$(NULL)

tests_common_check_stdin_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc_common.la \
	$(NULL)

if HAVE_SYS_RESOURCE_H
if HAVE_SYS_TIME_H
check_PROGRAMS += \
	tests/blogc/check_rusage \
	$(NULL)

tests_blogc_check_rusage_SOURCES = \
	tests/blogc/check_rusage.c \
	$(NULL)

tests_blogc_check_rusage_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_blogc_check_rusage_LDFLAGS = \
	-no-install \
	-Wl,--wrap=getrusage \
	$(NULL)

tests_blogc_check_rusage_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc.la \
	libblogc_common.la \
	$(NULL)
endif
endif

endif

tests_blogc_check_content_parser_SOURCES = \
	tests/blogc/check_content_parser.c \
	$(NULL)

tests_blogc_check_content_parser_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_blogc_check_content_parser_LDFLAGS = \
	-no-install \
	$(NULL)

tests_blogc_check_content_parser_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc.la \
	libblogc_common.la \
	$(NULL)

tests_blogc_check_datetime_parser_SOURCES = \
	tests/blogc/check_datetime_parser.c \
	$(NULL)

tests_blogc_check_datetime_parser_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_blogc_check_datetime_parser_LDFLAGS = \
	-no-install \
	$(NULL)

tests_blogc_check_datetime_parser_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc.la \
	libblogc_common.la \
	$(NULL)

tests_blogc_check_filelist_parser_SOURCES = \
	tests/blogc/check_filelist_parser.c \
	$(NULL)

tests_blogc_check_filelist_parser_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_blogc_check_filelist_parser_LDFLAGS = \
	-no-install \
	$(NULL)

tests_blogc_check_filelist_parser_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc.la \
	libblogc_common.la \
	$(NULL)

tests_blogc_check_renderer_SOURCES = \
	tests/blogc/check_renderer.c \
	$(NULL)

tests_blogc_check_renderer_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_blogc_check_renderer_LDFLAGS = \
	-no-install \
	$(NULL)

tests_blogc_check_renderer_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc.la \
	libblogc_common.la \
	$(NULL)

tests_blogc_check_source_parser_SOURCES = \
	tests/blogc/check_source_parser.c \
	$(NULL)

tests_blogc_check_source_parser_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_blogc_check_source_parser_LDFLAGS = \
	-no-install \
	$(NULL)

tests_blogc_check_source_parser_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc.la \
	libblogc_common.la \
	$(NULL)

tests_blogc_check_template_parser_SOURCES = \
	tests/blogc/check_template_parser.c \
	$(NULL)

tests_blogc_check_template_parser_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_blogc_check_template_parser_LDFLAGS = \
	-no-install \
	$(NULL)

tests_blogc_check_template_parser_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc.la \
	libblogc_common.la \
	$(NULL)

tests_blogc_check_toctree_SOURCES = \
	tests/blogc/check_toctree.c \
	$(NULL)

tests_blogc_check_toctree_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_blogc_check_toctree_LDFLAGS = \
	-no-install \
	$(NULL)

tests_blogc_check_toctree_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc.la \
	libblogc_common.la \
	$(NULL)

tests_common_check_config_parser_SOURCES = \
	tests/common/check_config_parser.c \
	$(NULL)

tests_common_check_config_parser_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_common_check_config_parser_LDFLAGS = \
	-no-install \
	$(NULL)

tests_common_check_config_parser_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc_common.la \
	$(NULL)

tests_common_check_error_SOURCES = \
	tests/common/check_error.c \
	$(NULL)

tests_common_check_error_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_common_check_error_LDFLAGS = \
	-no-install \
	$(NULL)

tests_common_check_error_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc_common.la \
	$(NULL)

tests_common_check_sort_SOURCES = \
	tests/common/check_sort.c \
	$(NULL)

tests_common_check_sort_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_common_check_sort_LDFLAGS = \
	-no-install \
	$(NULL)

tests_common_check_sort_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc_common.la \
	$(NULL)

tests_common_check_utf8_SOURCES = \
	tests/common/check_utf8.c \
	$(NULL)

tests_common_check_utf8_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_common_check_utf8_LDFLAGS = \
	-no-install \
	$(NULL)

tests_common_check_utf8_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc_common.la \
	$(NULL)

tests_common_check_utils_SOURCES = \
	tests/common/check_utils.c \
	$(NULL)

tests_common_check_utils_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_common_check_utils_LDFLAGS = \
	-no-install \
	$(NULL)

tests_common_check_utils_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc_common.la \
	$(NULL)

if BUILD_RUNSERVER
if USE_LD_WRAP
check_PROGRAMS += \
	tests/blogc-runserver/check_httpd_utils \
	tests/blogc-runserver/check_mime \
	$(NULL)

tests_blogc_runserver_check_httpd_utils_SOURCES = \
	tests/blogc-runserver/check_httpd_utils.c \
	$(NULL)

tests_blogc_runserver_check_httpd_utils_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_blogc_runserver_check_httpd_utils_LDFLAGS = \
	-no-install \
	-Wl,--wrap=read \
	$(NULL)

tests_blogc_runserver_check_httpd_utils_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc_runserver.la \
	libblogc_common.la \
	$(NULL)

tests_blogc_runserver_check_mime_SOURCES = \
	tests/blogc-runserver/check_mime.c \
	$(NULL)

tests_blogc_runserver_check_mime_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_blogc_runserver_check_mime_LDFLAGS = \
	-no-install \
	-Wl,--wrap=access \
	$(NULL)

tests_blogc_runserver_check_mime_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc_runserver.la \
	libblogc_common.la \
	$(NULL)
endif
endif

if BUILD_GIT_RECEIVER
check_PROGRAMS += \
	tests/blogc-git-receiver/check_pre_receive_parser \
	tests/blogc-git-receiver/check_shell_command_parser \
	$(NULL)

if USE_LD_WRAP
check_PROGRAMS += \
	tests/blogc-git-receiver/check_settings \
	$(NULL)

tests_blogc_git_receiver_check_settings_SOURCES = \
	tests/blogc-git-receiver/check_settings.c \
	$(NULL)

tests_blogc_git_receiver_check_settings_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_blogc_git_receiver_check_settings_LDFLAGS = \
	-no-install \
	-Wl,--wrap=realpath \
	$(NULL)

tests_blogc_git_receiver_check_settings_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc_git_receiver.la \
	libblogc_common.la \
	$(NULL)
endif

tests_blogc_git_receiver_check_pre_receive_parser_SOURCES = \
	tests/blogc-git-receiver/check_pre_receive_parser.c \
	$(NULL)

tests_blogc_git_receiver_check_pre_receive_parser_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_blogc_git_receiver_check_pre_receive_parser_LDFLAGS = \
	-no-install \
	$(NULL)

tests_blogc_git_receiver_check_pre_receive_parser_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc_git_receiver.la \
	libblogc_common.la \
	$(NULL)

tests_blogc_git_receiver_check_shell_command_parser_SOURCES = \
	tests/blogc-git-receiver/check_shell_command_parser.c \
	$(NULL)

tests_blogc_git_receiver_check_shell_command_parser_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_blogc_git_receiver_check_shell_command_parser_LDFLAGS = \
	-no-install \
	$(NULL)

tests_blogc_git_receiver_check_shell_command_parser_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc_git_receiver.la \
	libblogc_common.la \
	$(NULL)
endif

if BUILD_MAKE_LIB
check_PROGRAMS += \
	tests/blogc-make/check_atom \
	tests/blogc-make/check_rules \
	tests/blogc-make/check_settings \
	tests/blogc-make/check_utils \
	$(NULL)

if USE_LD_WRAP
check_PROGRAMS += \
	tests/blogc-make/check_exec \
	$(NULL)

tests_blogc_make_check_exec_SOURCES = \
	tests/blogc-make/check_exec.c \
	$(NULL)

tests_blogc_make_check_exec_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_blogc_make_check_exec_LDFLAGS = \
	-no-install \
	-Wl,--wrap=access \
	$(NULL)

tests_blogc_make_check_exec_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc_make.la \
	libblogc_common.la \
	$(NULL)
endif

tests_blogc_make_check_atom_SOURCES = \
	tests/blogc-make/check_atom.c \
	$(NULL)

tests_blogc_make_check_atom_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_blogc_make_check_atom_LDFLAGS = \
	-no-install \
	$(NULL)

tests_blogc_make_check_atom_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc_make.la \
	libblogc_common.la \
	$(NULL)

tests_blogc_make_check_rules_SOURCES = \
	tests/blogc-make/check_rules.c \
	$(NULL)

tests_blogc_make_check_rules_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_blogc_make_check_rules_LDFLAGS = \
	-no-install \
	$(NULL)

tests_blogc_make_check_rules_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc_make.la \
	libblogc_common.la \
	$(NULL)

tests_blogc_make_check_settings_SOURCES = \
	tests/blogc-make/check_settings.c \
	$(NULL)

tests_blogc_make_check_settings_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_blogc_make_check_settings_LDFLAGS = \
	-no-install \
	$(NULL)

tests_blogc_make_check_settings_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc_make.la \
	libblogc_common.la \
	$(NULL)

tests_blogc_make_check_utils_SOURCES = \
	tests/blogc-make/check_utils.c \
	$(NULL)

tests_blogc_make_check_utils_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_blogc_make_check_utils_LDFLAGS = \
	-no-install \
	$(NULL)

tests_blogc_make_check_utils_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc_make.la \
	libblogc_common.la \
	$(NULL)
endif

endif

endif

TESTS = \
	$(check_PROGRAMS) \
	$(check_SCRIPTS) \
	$(NULL)

CLEANFILES += \
	$(check_SCRIPTS) \
	$(NULL)


## Helpers: dist-srpm

if BUILD_SRPM
dist-srpm: blogc.spec dist-xz
	$(RPMBUILD) \
		--define "_srcrpmdir $(top_builddir)" \
		--define "_sourcedir $(top_builddir)" \
		-bs blogc.spec
endif


## Helpers: git-version-gen

$(top_srcdir)/.version:
	echo $(VERSION) > $@-t && mv $@-t $@

dist-hook:
	echo $(VERSION) > $(distdir)/.tarball-version


## Helpers: Valgrind runner

if USE_VALGRIND
valgrind: all
	$(MAKE) check TESTS_ENVIRONMENT=" \
		VALGRIND=$(VALGRIND) \
		$(BASH) -e $(top_srcdir)/build-aux/valgrind.sh"
endif


.PHONY: dist-srpm valgrind
