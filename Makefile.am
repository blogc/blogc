## Autotools settings

ACLOCAL_AMFLAGS = -I m4

AM_DISTCHECK_CONFIGURE_FLAGS = \
	--enable-examples \
	--enable-cmocka \
	--disable-leg \
	--disable-valgrind


## File listings

EXTRA_DIST = \
	autogen.sh \
	README.md \
	src/template-grammar.leg \
	$(NULL)

CLEANFILES = \
	$(NULL)

noinst_HEADERS = \
	src/output.h \
	src/source-parser.h \
	src/template-grammar.h \
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
	src/output.c \
	src/source-parser.c \
	src/template-grammar.c \
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

if USE_LEG
src/%-grammar.c: src/%-grammar.leg
	$(AM_V_GEN)$(LEG) -o $@ $<
endif

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


## Build rules: examples

if BUILD_EXAMPLES

noinst_PROGRAMS += \
	$(NULL)

endif


## Build rules: tests

if USE_CMOCKA

check_PROGRAMS += \
	tests/check_source_parser \
	tests/check_template_grammar \
	tests/check_utils \
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

tests_check_template_grammar_SOURCES = \
	tests/check_template_grammar.c \
	$(NULL)

tests_check_template_grammar_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_check_template_grammar_LDFLAGS = \
	-no-install \
	$(NULL)

tests_check_template_grammar_LDADD = \
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


## Helpers: Valgrind runners

if USE_VALGRIND
valgrind: all
	$(MAKE) check TESTS_ENVIRONMENT=" \
		G_SLICE=always-malloc \
		G_DEBUG=gc-friendly \
		$(LIBTOOL) \
			--mode=execute \
			$(VALGRIND) \
				--tool=memcheck \
				--leak-check=full \
				--leak-resolution=high \
				--num-callers=20 \
				--show-possibly-lost=no"

valgrind-ci: all clean-local
	$(MAKE) check TESTS_ENVIRONMENT=" \
		G_SLICE=always-malloc \
		G_DEBUG=gc-friendly \
		$(LIBTOOL) \
			--mode=execute \
			$(VALGRIND) \
				--tool=memcheck \
				--xml=yes \
				--xml-file=valgrind-%p.xml \
				--leak-check=full \
				--leak-resolution=high \
				--num-callers=20 \
				--show-possibly-lost=no"
endif


# Helpers: Cleanup of helper files

clean-local:
	-rm -rf $(top_builddir)/valgrind-*.xml
