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
	$(NULL)

CLEANFILES = \
	$(NULL)

noinst_HEADERS = \
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
	tests/check_utils \
	$(NULL)

tests_check_utils_SOURCES = \
	tests/check_utils.c \
	$(NULL)

tests_check_utils_CFLAGS = \
	$(CMOCKA_CFLAGS) \
	$(NULL)

tests_check_utils_LDFLAGS = \
	-no-install

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
	-rm -rf $(top_builddir)/doc/build/
	-rm -rf $(top_builddir)/valgrind-*.xml
