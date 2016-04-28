## Autotools settings

ACLOCAL_AMFLAGS = -I m4

AM_DISTCHECK_CONFIGURE_FLAGS = \
	--enable-tests \
	--enable-ronn \
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
	src/content-parser.h \
	src/datetime-parser.h \
	src/file.h \
	src/error.h \
	src/loader.h \
	src/renderer.h \
	src/source-parser.h \
	src/template-parser.h \
	src/utils.h \
	$(NULL)

noinst_LTLIBRARIES = \
	libblogc.la \
	libblogc_utils.la \
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
endif

if BUILD_RUNSERVER
bin_PROGRAMS += \
	blogc-runserver \
	$(NULL)
endif

check_PROGRAMS = \
	$(NULL)


libblogc_la_SOURCES = \
	src/content-parser.c \
	src/datetime-parser.c \
	src/file.c \
	src/error.c \
	src/loader.c \
	src/renderer.c \
	src/source-parser.c \
	src/template-parser.c \
	$(NULL)

libblogc_la_CFLAGS = \
	$(AM_CFLAGS) \
	-I$(top_srcdir)/src \
	$(NULL)

libblogc_la_LIBADD = \
	$(LIBM) \
	$(NULL)


libblogc_utils_la_SOURCES = \
	src/utils.c \
	$(NULL)

libblogc_utils_la_CFLAGS = \
	$(AM_CFLAGS) \
	-I$(top_srcdir)/src \
	$(NULL)


blogc_SOURCES = \
	src/blogc.c \
	$(NULL)

blogc_CFLAGS = \
	$(AM_CFLAGS) \
	-I$(top_srcdir)/src \
	$(NULL)

blogc_LDADD = \
	libblogc.la \
	libblogc_utils.la \
	$(NULL)


if BUILD_GIT_RECEIVER
blogc_git_receiver_SOURCES = \
	src/blogc-git-receiver.c \
	$(NULL)

blogc_git_receiver_CFLAGS = \
	$(AM_CFLAGS) \
	-I$(top_srcdir)/src \
	$(NULL)

blogc_git_receiver_LDADD = \
	libblogc_utils.la \
	$(NULL)
endif


if BUILD_RUNSERVER
blogc_runserver_SOURCES = \
	src/blogc-runserver.c \
	$(NULL)

blogc_runserver_CFLAGS = \
	$(AM_CFLAGS) \
	-I$(top_srcdir)/src \
	$(LIBEVENT_CFLAGS) \
	$(SQUAREBALL_CFLAGS) \
	$(NULL)

blogc_runserver_LDADD = \
	$(LIBEVENT_LIBS) \
	$(MAGIC_LIBS) \
	libblogc_utils.la \
	$(NULL)
endif


## Build rules: man pages

EXTRA_DIST += \
	man/blogc.1.ronn \
	man/blogc-source.7.ronn \
	man/blogc-template.7.ronn \
	man/index.txt \
	$(NULL)

dist_man_MANS = \
	blogc.1 \
	blogc-source.7 \
	blogc-template.7 \
	$(NULL)

if BUILD_GIT_RECEIVER
EXTRA_DIST += \
	man/blogc-git-receiver.1.ronn \
	$(NULL)

dist_man_MANS += \
	blogc-git-receiver.1 \
	$(NULL)
endif

if BUILD_RUNSERVER
EXTRA_DIST += \
	man/blogc-runserver.1.ronn \
	$(NULL)

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

endif


## Build rules: tests

if USE_CMOCKA

check_PROGRAMS += \
	tests/check_content_parser \
	tests/check_datetime_parser \
	tests/check_error \
	tests/check_loader \
	tests/check_renderer \
	tests/check_source_parser \
	tests/check_template_parser \
	tests/check_utils \
	$(NULL)

tests_check_error_SOURCES = \
	tests/check_error.c \
	$(NULL)

tests_check_error_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_check_error_LDFLAGS = \
	-no-install \
	$(NULL)

tests_check_error_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc.la \
	libblogc_utils.la \
	$(NULL)

tests_check_loader_SOURCES = \
	tests/check_loader.c \
	$(NULL)

tests_check_loader_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_check_loader_LDFLAGS = \
	-no-install \
	-Wl,--wrap=blogc_file_get_contents \
	-Wl,--wrap=blogc_fprintf \
	$(NULL)

tests_check_loader_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc.la \
	libblogc_utils.la \
	$(NULL)

tests_check_content_parser_SOURCES = \
	tests/check_content_parser.c \
	$(NULL)

tests_check_content_parser_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_check_content_parser_LDFLAGS = \
	-no-install \
	$(NULL)

tests_check_content_parser_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc.la \
	libblogc_utils.la \
	$(NULL)

tests_check_datetime_parser_SOURCES = \
	tests/check_datetime_parser.c \
	$(NULL)

tests_check_datetime_parser_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_check_datetime_parser_LDFLAGS = \
	-no-install \
	$(NULL)

tests_check_datetime_parser_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc.la \
	libblogc_utils.la \
	$(NULL)

tests_check_renderer_SOURCES = \
	tests/check_renderer.c \
	$(NULL)

tests_check_renderer_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_check_renderer_LDFLAGS = \
	-no-install \
	$(NULL)

tests_check_renderer_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc.la \
	libblogc_utils.la \
	$(NULL)

tests_check_source_parser_SOURCES = \
	tests/check_source_parser.c \
	$(NULL)

tests_check_source_parser_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_check_source_parser_LDFLAGS = \
	-no-install \
	$(NULL)

tests_check_source_parser_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc.la \
	libblogc_utils.la \
	$(NULL)

tests_check_template_parser_SOURCES = \
	tests/check_template_parser.c \
	$(NULL)

tests_check_template_parser_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_check_template_parser_LDFLAGS = \
	-no-install \
	$(NULL)

tests_check_template_parser_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc.la \
	libblogc_utils.la \
	$(NULL)

tests_check_utils_SOURCES = \
	tests/check_utils.c \
	$(NULL)

tests_check_utils_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_check_utils_LDFLAGS = \
	-no-install \
	$(NULL)

tests_check_utils_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc_utils.la \
	$(NULL)

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
