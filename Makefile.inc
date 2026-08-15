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
# The build's own -L has to precede the user's LDFLAGS, for the same reason
# CSDP_CPPFLAGS precedes CPPFLAGS: -L is searched in command line order, so a
# -L naming a directory that already holds an installed libsdp would otherwise
# win over the one just built here.
CSDP_LDFLAGS = -L$(CSDP_TOP)lib
CSDP_LIBS = -lsdp

# The probes below have to see the same compiler and flags as the build, or
# they will answer for the wrong target: a packager who selects 32 bit with
# CFLAGS=-m32 rather than CC="gcc -m32" would otherwise get -DBIT64.
CSDP_PROBE = $(CC) $(CPPFLAGS) $(CFLAGS)

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
CSDP_LINK = $(CC) $(CSDP_CFLAGS) $(OPENMP_CFLAGS) $(CFLAGS) \
	$(CSDP_LDFLAGS) $(LDFLAGS)

%.o: %.c
	$(CSDP_COMPILE) -c $< -o $@
