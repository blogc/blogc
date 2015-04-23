## Autotools settings

ACLOCAL_AMFLAGS = -I m4

AM_DISTCHECK_CONFIGURE_FLAGS = \
	--enable-tests \
	--disable-valgrind


## File listings

EXTRA_DIST = \
	autogen.sh \
	LICENSE \
	README.md \
	$(NULL)

CLEANFILES = \
	$(NULL)

noinst_HEADERS = \
	src/error.h \
	src/loader.h \
	src/renderer.h \
	src/source-parser.h \
	src/template-parser.h \
	src/utils/utils.h \
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
	src/error.c \
	src/loader.c \
	src/renderer.c \
	src/source-parser.c \
	src/template-parser.c \
	src/utils/mem.c \
	src/utils/slist.c \
	src/utils/strings.c \
	src/utils/trie.c \
	$(NULL)

libblogc_la_CFLAGS = \
	$(AM_CFLAGS) \
	-I$(top_srcdir)/src \
	$(NULL)

libblogc_la_LIBADD = \
	$(NULL)


blogc_SOURCES = \
	src/main.c \
	$(NULL)

blogc_CFLAGS = \
	$(AM_CFLAGS) \
	-I$(top_srcdir)/src \
	$(NULL)

blogc_LDADD = \
	libblogc.la \
	$(NULL)


## Build rules: tests

if USE_CMOCKA

check_PROGRAMS += \
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
	$(NULL)

tests_check_loader_SOURCES = \
	tests/check_loader.c \
	$(NULL)

tests_check_loader_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_check_loader_LDFLAGS = \
	-no-install \
	$(NULL)

tests_check_loader_LDADD = \
	$(CMOCKA_LIBS) \
	libblogc.la \
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
