## Autotools settings

ACLOCAL_AMFLAGS = -I m4

AM_DISTCHECK_CONFIGURE_FLAGS = \
	--enable-tests \
	--disable-ronn \
	--disable-valgrind \
	--enable-git-receiver \
	--enable-runserver \
	$(NULL)


## File listings

EXTRA_DIST = \
	build-aux/git-version-gen \
	$(top_srcdir)/.version \
	autogen.sh \
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
	src/blogc/loader.h \
	src/blogc/renderer.h \
	src/blogc/source-parser.h \
	src/blogc/template-parser.h \
	src/blogc-git-receiver/post-receive.h \
	src/blogc-git-receiver/pre-receive.h \
	src/blogc-git-receiver/shell.h \
	src/blogc-runserver/httpd.h \
	src/blogc-runserver/httpd-utils.h \
	src/blogc-runserver/mime.h \
	src/common/config-parser.h \
	src/common/error.h \
	src/common/file.h \
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


libblogc_la_SOURCES = \
	src/blogc/content-parser.c \
	src/blogc/datetime-parser.c \
	src/blogc/loader.c \
	src/blogc/renderer.c \
	src/blogc/source-parser.c \
	src/blogc/template-parser.c \
	$(NULL)

libblogc_la_CFLAGS = \
	$(AM_CFLAGS) \
	$(NULL)

libblogc_la_LIBADD = \
	$(LIBM) \
	$(NULL)


libblogc_common_la_SOURCES = \
	src/common/config-parser.c \
	src/common/error.c \
	src/common/file.c \
	src/common/utf8.c \
	src/common/utils.c \
	$(NULL)

libblogc_common_la_CFLAGS = \
	$(AM_CFLAGS) \
	$(NULL)


blogc_SOURCES = \
	src/blogc/main.c \
	src/blogc/debug.c \
	$(NULL)

blogc_CFLAGS = \
	$(AM_CFLAGS) \
	$(NULL)

blogc_LDADD = \
	libblogc.la \
	libblogc_common.la \
	$(NULL)


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
	src/blogc-git-receiver/shell.c \
	$(NULL)

libblogc_git_receiver_la_CFLAGS = \
	$(AM_CFLAGS) \
	$(NULL)

libblogc_git_receiver_la_LIBADD = \
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
	$(NULL)
endif


## Build rules: man pages

EXTRA_DIST += \
	man/blogc.1.ronn \
	man/blogc-git-receiver.1.ronn \
	man/blogc-runserver.1.ronn \
	man/blogc-source.7.ronn \
	man/blogc-template.7.ronn \
	man/blogc-pagination.7.ronn \
	man/index.txt \
	blogc-git-receiver.1 \
	blogc-runserver.1 \
	$(NULL)

dist_man_MANS = \
	blogc.1 \
	blogc-source.7 \
	blogc-template.7 \
	blogc-pagination.7 \
	$(NULL)

if BUILD_GIT_RECEIVER
dist_man_MANS += \
	blogc-git-receiver.1 \
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

if USE_RONN

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

blogc-runserver.1: man/blogc-runserver.1.ronn
	$(AM_V_GEN)$(RONN) \
		--roff \
		--pipe \
		--organization "Rafael G. Martins" \
		--manual "$(PACKAGE_NAME) Manual" \
		$(top_srcdir)/man/blogc-runserver.1.ronn > blogc-runserver.1

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

blogc-runserver.1:
	$(AM_V_GEN)echo "error: ronn not found. failed to build man page: $@"; exit 1

blogc-source.7:
	$(AM_V_GEN)echo "error: ronn not found. failed to build man page: $@"; exit 1

blogc-template.7:
	$(AM_V_GEN)echo "error: ronn not found. failed to build man page: $@"; exit 1

blogc-pagination.7:
	$(AM_V_GEN)echo "error: ronn not found. failed to build man page: $@"; exit 1

endif


## Build rules: tests

if USE_CMOCKA

check_PROGRAMS += \
	tests/blogc/check_content_parser \
	tests/blogc/check_datetime_parser \
	tests/blogc/check_loader \
	tests/blogc/check_renderer \
	tests/blogc/check_source_parser \
	tests/blogc/check_template_parser \
	tests/common/check_config_parser \
	tests/common/check_error \
	tests/common/check_utf8 \
	tests/common/check_utils \
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

TESTS = \
	$(check_PROGRAMS)


## Helpers: dist-srpm

if USE_RPMBUILD
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
		$(VALGRIND) \
			--tool=memcheck \
			--leak-check=full \
			--leak-resolution=high \
			--num-callers=20 \
			--error-exitcode=1 \
			--show-possibly-lost=no"

endif
