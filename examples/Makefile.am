#
# Copyright (C) 2012, Northwestern University and Argonne National Laboratory
# See COPYRIGHT notice in top-level directory.
#
# $Id$
#
# @configure_input@

SUBDIRS = C tutorial
DIST_SUBDIRS = C tutorial CXX F77 F90 burst_buffer

if HAS_MPICXX
SUBDIRS += CXX
endif

if HAS_FORTRAN
SUBDIRS += F77
if HAVE_MPI_MOD
SUBDIRS += F90
endif
endif

if ENABLE_BURST_BUFFER
SUBDIRS += burst_buffer
endif

EXTRA_DIST = README

# below is the alternative when AM_EXTRA_RECURSIVE_TARGETS is not defined
# TESTS_DIRS = $(SUBDIRS:%=tests-%)
# tests: $(TESTS_DIRS)
# $(TESTS_DIRS):
# 	$(MAKE) $(MFLAGS) -C $(@:tests-%=%) tests

if BUILD_COVERAGE
   PTEST_SUBDIRS =
else
   PTEST_SUBDIRS = $(SUBDIRS)
endif

ptest:
	for d in $(PTEST_SUBDIRS) ; do \
		$(MAKE) $(MFLAGS) -C $$d ptest $$* || exit 1 ; \
	done

ptests:
	for d in $(PTEST_SUBDIRS) ; do \
		$(MAKE) $(MFLAGS) -C $$d ptests $$* || exit 1 ; \
	done

# For VPATH build (parallel build), try delete all sub-directories
distclean-local:
	if [ "$(abs_builddir)" != "$(abs_srcdir)" ] ; then \
	   for d in $(DIST_SUBDIRS) ; do \
	       rmdir $$d || true ; \
	   done ; \
	fi

.PHONY: ptest ptests

