#
# This Makefile can be used to automatically build the entire package.  
#
# If you make changes in the Makefile or code under any subdirectory, you can
# rebuild the system with "make clean" followed by "make all".
#
#
# Build settings are in Makefile.inc.  Every variable there can be set in the
# environment or on the make command line; see INSTALL.
#
include Makefile.inc

.PHONY: all clean install unitTest

#
# On most systems, this should handle everything.
#
all:
	$(MAKE) -C lib all
	$(MAKE) -C solver all
	$(MAKE) -C theta all
	$(MAKE) -C example all

#
# Perform a unitTest
#

unitTest: all
	$(MAKE) -C test all

#
# Install the executables in $(bindir).  install is among the .PHONY targets
# above so that the INSTALL file does not satisfy it on case insensitive
# filesystems.
#
install: all
	$(INSTALL) -d "$(DESTDIR)$(bindir)"
	$(INSTALL_PROGRAM) solver/csdp "$(DESTDIR)$(bindir)/csdp"
	$(INSTALL_PROGRAM) theta/theta "$(DESTDIR)$(bindir)/$(TOOL_PREFIX)theta"
	$(INSTALL_PROGRAM) theta/graphtoprob "$(DESTDIR)$(bindir)/$(TOOL_PREFIX)graphtoprob"
	$(INSTALL_PROGRAM) theta/complement "$(DESTDIR)$(bindir)/$(TOOL_PREFIX)complement"
	$(INSTALL_PROGRAM) theta/rand_graph "$(DESTDIR)$(bindir)/$(TOOL_PREFIX)rand_graph"

#
# Clean out all of the directories.
# 

clean:
	$(MAKE) -C lib clean
	$(MAKE) -C solver clean
	$(MAKE) -C theta clean
	$(MAKE) -C test clean
	$(MAKE) -C example clean
