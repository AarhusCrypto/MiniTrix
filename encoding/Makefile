# This Makefile defines a new part. Give it a name:
PART_NAME=encoding
CURDIR ?= ${.CURDIR}

# This is the directory is the build yard where all the mess generated
# by automake, configure etc. is living. Also, the final binaries will
# be here and a sub-folder for the release directory will be here as
# well. The intention is that deleting build will clean "rm -rf build"
# almost everything (except with m4 scripts etc. generated for
# configure, aclocal.m4 and Makefile.in etc.)
BUILDDIR=$(CURDIR)/build

# The depshapedir is the directory this part will produce for others
# parts that depend on it to copy.
depshapedir=$(BUILDDIR)/$(PART_NAME)

# The DEPS folder contains all local dependencies and if this part of
# the project is going to depend on another part called e.g. part2 we
# use the style of creating deps_part2 variable here to refer where is
# it later. (This is by convention, not enforced or necessary)
# e.g. for creating a dependency on mpclib:
#
DEPS_VERSION=parvusfortis_r4098
DEPS=deps/
REPO="svn://sun.bitlab.dk/Rasmus/phd/mpccode/tags"
LOCAL_REPO="../../tags/"$(DEPS_VERSION)

# This "all" target builds the whole project every time. To re-compile
# the project without rebuilding the whole thing do "cd build && make
# all". This scripts is a META scripts invoking Autotool making the
# "real" Makefile's.
all:  $(BUILDDIR)/stamp-h1

# create the "build" directory
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# create the configure scripts in BUILDDIR invoking Autotools and then
# configure and build the whole thing.
$(BUILDDIR)/stamp-h1: $(BUILDDIR) configure.ac Makefile.am $(DEPS)/libosal.a
	aclocal
	autoconf
	autoheader
	automake --add-missing
	cd build && ../configure --prefix=$(BUILDDIR)
	cd build && make install


depshape: $(BUILDDIR)/stamp-h1 $(depshapedir)
	-[ -d $(BUILDDIR)/lib ] && cp -r $(BUILDDIR)/lib/* $(depshapedir)
	-[ -d $(BUILDDIR)/bin ] && cp -rf $(BUILDDIR)/bin/* $(depshapedir)
	-[ -d $(BUILDDIR)/include ] && cp -rf $(BUILDDIR)/include/* $(depshapedir)/headers

# create the build/part_name where other parts can copy from to depend
# on this part.
$(depshapedir):
	mkdir -p $(depshapedir)/headers



# this target copies the libmpc.a and the headers directory from the
# released mpclib (assuming mpclib is placed in the parent folder
# along side with this part).
$(DEPS)/libosal.a: 
	-@[ -d deps ] || ( [ -d $(LOCAL_REPO) ] && cp -r $(LOCAL_REPO) ./deps && echo "LOCAL COPY" )
	[ -d deps ] || svn export $(REPO)/$(DEPS_VERSION) ./deps


# Go Nuke it all. The wipe.sh script deletes Makefile.in and m4 stuff
# created by aclocal, autoconf and automake. 
clean:
	rm -rf deps
	rm -rf build
	./wipe.sh

.PHONY=all clean depshape
