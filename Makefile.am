## Autotools settings

ACLOCAL_AMFLAGS = -I m4

AM_DISTCHECK_CONFIGURE_FLAGS = \
	--enable-tests \
	--enable-ronn \
	--disable-valgrind \
	--with-squareball=internal \
	$(NULL)


## squareball stuff

if USE_SYSTEM_SQUAREBALL

SQUAREBALL_EXTRA_DIST_ =

SQUAREBALL_HEADERS_ =

SQUAREBALL_SOURCES_ =

SQUAREBALL_CFLAGS_ = \
	$(SQUAREBALL_CFLAGS) \
	$(NULL)

SQUAREBALL_LIBS_ = \
	$(SQUAREBALL_LIBS) \
	$(NULL)

else

SQUAREBALL_EXTRA_DIST_ = \
	squareball/LICENSE \
	squareball/README.md \
	$(NULL)

SQUAREBALL_HEADERS_ = \
	squareball/src/squareball.h \
	squareball/src/squareball/sb-mem.h \
	squareball/src/squareball/sb-slist.h \
	squareball/src/squareball/sb-string.h \
	squareball/src/squareball/sb-trie.h \
	squareball/src/squareball/sb-trie-private.h \
	$(NULL)

SQUAREBALL_SOURCES_ = \
	squareball/src/sb-mem.c \
	squareball/src/sb-slist.c \
	squareball/src/sb-string.c \
	squareball/src/sb-trie.c \
	$(NULL)

SQUAREBALL_CFLAGS_ = \
	-I$(top_srcdir)/squareball/src \
	$(NULL)

SQUAREBALL_LIBS_ =

endif


## File listings

EXTRA_DIST = \
	autogen.sh \
	LICENSE \
	README.md \
	$(SQUAREBALL_EXTRA_DIST_)
	$(NULL)

CLEANFILES = \
	$(NULL)

MAINTAINERCLEANFILES = \
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
	$(SQUAREBALL_HEADERS_)
	$(NULL)

noinst_LTLIBRARIES = \
	libblogc.la \
	$(NULL)

noinst_PROGRAMS = \
	$(NULL)

bin_PROGRAMS = \
	blogc \
	$(NULL)

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
	$(SQUAREBALL_SOURCES_) \
	$(NULL)

libblogc_la_CFLAGS = \
	$(AM_CFLAGS) \
	-I$(top_srcdir)/src \
	$(SQUAREBALL_CFLAGS_) \
	$(NULL)

libblogc_la_LIBADD = \
	$(LIBM) \
	$(SQUAREBALL_LIBS_) \
	$(NULL)

blogc_SOURCES = \
	src/main.c \
	$(NULL)

blogc_CFLAGS = \
	$(AM_CFLAGS) \
	-I$(top_srcdir)/src \
	$(SQUAREBALL_CFLAGS_) \
	$(NULL)

blogc_LDADD = \
	libblogc.la \
	$(SQUAREBALL_LIBS_) \
	$(NULL)


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
		$< > $@

blogc-source.7: man/blogc-source.7.ronn
	$(AM_V_GEN)$(RONN) \
		--roff \
		--pipe \
		--organization "Rafael G. Martins" \
		--manual "$(PACKAGE_NAME) Manual" \
		$< > $@

blogc-template.7: man/blogc-template.7.ronn
	$(AM_V_GEN)$(RONN) \
		--roff \
		--pipe \
		--organization "Rafael G. Martins" \
		--manual "$(PACKAGE_NAME) Manual" \
		$< > $@

else

blogc.1:
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
	$(NULL)

tests_check_error_SOURCES = \
	tests/check_error.c \
	$(NULL)

tests_check_error_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(SQUAREBALL_CFLAGS_) \
	$(NULL)

tests_check_error_LDFLAGS = \
	-no-install \
	$(NULL)

tests_check_error_LDADD = \
	$(CMOCKA_LIBS) \
	$(SQUAREBALL_LIBS_) \
	libblogc.la \
	$(NULL)

tests_check_loader_SOURCES = \
	tests/check_loader.c \
	$(NULL)

tests_check_loader_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(SQUAREBALL_CFLAGS_) \
	$(NULL)

tests_check_loader_LDFLAGS = \
	-no-install \
	-Wl,--wrap=blogc_file_get_contents \
	-Wl,--wrap=blogc_fprintf \
	$(NULL)

tests_check_loader_LDADD = \
	$(CMOCKA_LIBS) \
	$(SQUAREBALL_LIBS_) \
	libblogc.la \
	$(NULL)

tests_check_content_parser_SOURCES = \
	tests/check_content_parser.c \
	$(NULL)

tests_check_content_parser_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(SQUAREBALL_CFLAGS_) \
	$(NULL)

tests_check_content_parser_LDFLAGS = \
	-no-install \
	$(NULL)

tests_check_content_parser_LDADD = \
	$(CMOCKA_LIBS) \
	$(SQUAREBALL_LIBS_) \
	libblogc.la \
	$(NULL)

tests_check_datetime_parser_SOURCES = \
	tests/check_datetime_parser.c \
	$(NULL)

tests_check_datetime_parser_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(SQUAREBALL_CFLAGS_) \
	$(NULL)

tests_check_datetime_parser_LDFLAGS = \
	-no-install \
	$(NULL)

tests_check_datetime_parser_LDADD = \
	$(CMOCKA_LIBS) \
	$(SQUAREBALL_LIBS_) \
	libblogc.la \
	$(NULL)

tests_check_renderer_SOURCES = \
	tests/check_renderer.c \
	$(NULL)

tests_check_renderer_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(SQUAREBALL_CFLAGS_) \
	$(NULL)

tests_check_renderer_LDFLAGS = \
	-no-install \
	$(NULL)

tests_check_renderer_LDADD = \
	$(CMOCKA_LIBS) \
	$(SQUAREBALL_LIBS_) \
	libblogc.la \
	$(NULL)

tests_check_source_parser_SOURCES = \
	tests/check_source_parser.c \
	$(NULL)

tests_check_source_parser_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(SQUAREBALL_CFLAGS_) \
	$(NULL)

tests_check_source_parser_LDFLAGS = \
	-no-install \
	$(NULL)

tests_check_source_parser_LDADD = \
	$(CMOCKA_LIBS) \
	$(SQUAREBALL_LIBS_) \
	libblogc.la \
	$(NULL)

tests_check_template_parser_SOURCES = \
	tests/check_template_parser.c \
	$(NULL)

tests_check_template_parser_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(SQUAREBALL_CFLAGS_) \
	$(NULL)

tests_check_template_parser_LDFLAGS = \
	-no-install \
	$(NULL)

tests_check_template_parser_LDADD = \
	$(CMOCKA_LIBS) \
	$(SQUAREBALL_LIBS_) \
	libblogc.la \
	$(NULL)

endif

TESTS = \
	$(check_PROGRAMS)


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
