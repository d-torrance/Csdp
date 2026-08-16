#
# Build settings shared by the subdirectory Makefiles.  Every variable here
# can be set in the environment or on the make command line, for example
#
#	make CC=clang CFLAGS="-O3 -g" BLAS_LIBS="-lopenblas"
#
# GNU make is required.  See INSTALL for the full list of variables.
#
# CSDP_CPPFLAGS and CSDP_CFLAGS hold the flags the build itself requires.
# They are kept separate from CPPFLAGS and CFLAGS so that setting the latter
# cannot silently drop -I../include or the feature macros.
#

CC ?= cc
INSTALL ?= install
INSTALL_PROGRAM ?= $(INSTALL) -m 755
INSTALL_DATA ?= $(INSTALL) -m 644

# theta, complement, graphtoprob and rand_graph have rather generic names, so
# distributors may want to install them under a prefix, e.g. TOOL_PREFIX=csdp-
TOOL_PREFIX ?=

prefix ?= /usr/local
exec_prefix ?= $(prefix)
bindir ?= $(exec_prefix)/bin
libdir ?= $(exec_prefix)/lib
includedir ?= $(prefix)/include
pkgconfigdir ?= $(libdir)/pkgconfig

CFLAGS ?= -O2 -Wall

# Set OPENMP_CFLAGS to nothing to build CSDP without OpenMP.
OPENMP_CFLAGS ?= -fopenmp
OPENMP_LIBS ?=

# The BLAS and LAPACK libraries, plus any Fortran run time library they need.
# On macOS this is "-framework Accelerate".
BLAS_LIBS ?= -llapack -lblas

# LIBS is anything else to link against.  It is deliberately additive and has
# no default, so that a build system which exports an empty LIBS -- FreeBSD's
# bsd.port.mk does -- cannot knock out the BLAS.

# Where the top of the tree is, relative to whichever makefile included this
# one: "./" at the top level, "../" from a subdirectory, and so on.  Spelling
# the paths this way rather than hardcoding ../ keeps them right whatever the
# depth, and keeps them relative, which -I and -L in a reproducible build want
# to be.
CSDP_TOP := $(dir $(lastword $(MAKEFILE_LIST)))

CSDP_CPPFLAGS = -I$(CSDP_TOP)include
CSDP_CFLAGS = -std=gnu99
CSDP_LIBS = -L$(CSDP_TOP)lib -lsdp

# The probes below have to see the same compiler and flags as the build, or
# they will answer for the wrong target: a packager who selects 32 bit with
# CFLAGS=-m32 rather than CC="gcc -m32" would otherwise get -DBIT64.
CSDP_PROBE = $(CC) $(CPPFLAGS) $(CFLAGS)

# Set SHARED=yes to build a shared library alongside libsdp.a.  The programs
# then link against it, since -lsdp prefers the shared library when both are
# present.
SHARED ?= no

# CSDP_VERSION is the release, and is what goes in csdp.pc.
CSDP_VERSION = 6.2.0

ifeq ($(SHARED),yes)

# The shared library is versioned independently of the release, because its
# ABI has nothing to do with the release number.  These are libtool's
# -version-info numbers, and the rules for bumping them are libtool's:
#
#	if the code changed at all                       revision + 1
#	if any interface was added, removed or changed   current + 1, revision 0
#	if any interface was added and none removed
#	  or changed                                     age + 1
#	if any interface was removed or changed          age 0
#
CSDP_CURRENT = 0
CSDP_REVISION = 0
CSDP_AGE = 0

# As libtool names them: the soname is libsdp.so.(current - age), and the file
# it points at is libsdp.so.(current - age).(age).(revision).
CSDP_SOVERSION := $(shell expr $(CSDP_CURRENT) - $(CSDP_AGE))
CSDP_LIBVERSION := $(CSDP_SOVERSION).$(CSDP_AGE).$(CSDP_REVISION)

CSDP_CFLAGS += -fPIC
CSDP_SHLIB = libsdp.so.$(CSDP_LIBVERSION)
CSDP_SONAME = libsdp.so.$(CSDP_SOVERSION)
CSDP_SHLIB_DEV = libsdp.so
CSDP_SHLIB_FLAGS = -shared -Wl,-soname,$(CSDP_SONAME)
CSDP_SHLIB_LINKS = ln -sf $(CSDP_SHLIB) $(CSDP_SONAME) && \
	ln -sf $(CSDP_SHLIB) $(CSDP_SHLIB_DEV)
endif

# Both of these are POSIX interfaces that Windows lacks: USESIGTERM needs
# setrlimit(), and USEGETTIME needs gettimeofday().  Probe with -include
# rather than a here-doc, so that these lines contain no "#", which make 3.81
# would take as the start of a comment.
ifneq ($(shell : | $(CSDP_PROBE) -E -include sys/resource.h - >/dev/null 2>&1 && echo yes),)
CSDP_CPPFLAGS += -DUSESIGTERM
endif

ifneq ($(shell : | $(CSDP_PROBE) -E -include sys/time.h - >/dev/null 2>&1 && echo yes),)
CSDP_CPPFLAGS += -DUSEGETTIME
endif

# BIT64 switches the indexing macros in include/index.h to long int, and
# disables the check in read_prob() that rejects problems too large for 32 bit
# indexing.  That is only correct where long is 64 bits, so ask the compiler
# rather than assume: it is 32 bits on ILP32 targets and on Windows.
LONG_BYTES := $(shell echo __SIZEOF_LONG__ | $(CSDP_PROBE) -E -P - 2>/dev/null | tr -dc 0-9)
ifeq ($(LONG_BYTES),8)
CSDP_CPPFLAGS += -DBIT64
endif

# USEOPENMP includes <omp.h> and calls the OpenMP runtime, so define it only
# when we are actually compiling with OpenMP.
ifneq ($(strip $(OPENMP_CFLAGS)),)
CSDP_CPPFLAGS += -DUSEOPENMP -DSETNUMTHREADS
endif

CSDP_COMPILE = $(CC) $(CSDP_CPPFLAGS) $(CPPFLAGS) $(CSDP_CFLAGS) \
	$(OPENMP_CFLAGS) $(CFLAGS)
CSDP_LINK = $(CC) $(CSDP_CFLAGS) $(OPENMP_CFLAGS) $(CFLAGS) $(LDFLAGS)

%.o: %.c
	$(CSDP_COMPILE) -c $< -o $@
